/*
  modelprompt.cpp

  Csound 7 plugin opcodes for synchronous and asynchronous interaction
  with external generative models (OpenAI, Anthropic).

  Copyright (C) 2026 Michael Gogins

  This file is part of csound-ac / modelprompt.

  Design: see README.md in this directory.

  Thread safety
  -------------
  - Network I/O for modelprompt_async runs on std::thread workers.
  - Request status/result are guarded by per-request and registry mutexes;
    modelprompt_result never blocks on I/O.
  - Cache directory versioning uses a process-wide mutex.
  - Csound API calls that mutate orchestra state occur only on Csound
    threads (init / control), never from worker threads.
*/

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <curl/curl.h>

#include "csdl.h"
#include "arrays.h"
#include "version.h"

/* Public host symbol resolved at load time (-undefined dynamic_lookup). */
extern "C" int32_t csoundCompileOrc(CSOUND *csound, const char *str, int32_t async);

namespace {
/* OENTRY still wants char*; silence writable-string warnings at the table. */
inline char *ochar(const char *s) { return const_cast<char *>(s); }
} // namespace

namespace fs = std::filesystem;

namespace {

/* -------------------------------------------------------------------------- */
/* Small utilities                                                            */
/* -------------------------------------------------------------------------- */

std::string trim_copy(std::string s)
{
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

const char *cstr(const STRINGDAT *s)
{
    if (s == nullptr || s->data == nullptr) {
        return "";
    }
    return s->data;
}

bool nonempty(const STRINGDAT *s)
{
    return s != nullptr && s->data != nullptr && s->data[0] != '\0';
}

void set_string(CSOUND *csound, STRINGDAT *dest, std::string_view text)
{
    const size_t need = text.size() + 1;
    if (dest->data == nullptr || dest->size < need) {
        dest->data = static_cast<char *>(csound->ReAlloc(csound, dest->data, need));
        dest->size = need;
    }
    std::memcpy(dest->data, text.data(), text.size());
    dest->data[text.size()] = '\0';
}

std::string json_escape(std::string_view in)
{
    std::string out;
    out.reserve(in.size() + 8);
    for (char c : in) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '"':  out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            } else {
                out += c;
            }
        }
    }
    return out;
}

std::optional<std::string> extract_json_string_value(std::string_view json,
                                                    std::string_view key)
{
    const std::string pattern =
        std::string("\"") + std::string(key) + "\"\\s*:\\s*\"";
    const std::regex re(pattern);
    std::cmatch m;
    if (!std::regex_search(json.begin(), json.end(), m, re)) {
        return std::nullopt;
    }
    std::string out;
    const char *p = m[0].second;
    const char *end = json.data() + json.size();
    while (p < end) {
        if (*p == '\\' && p + 1 < end) {
            ++p;
            switch (*p) {
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 't': out += '\t'; break;
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'u':
                if (p + 4 < end) {
                    out += '?';
                    p += 4;
                }
                break;
            default: out += *p; break;
            }
            ++p;
            continue;
        }
        if (*p == '"') {
            return out;
        }
        out += *p++;
    }
    return std::nullopt;
}

std::optional<std::string> extract_openai_content(std::string_view json)
{
    // Prefer message.content, then output_text-style fields.
    if (auto s = extract_json_string_value(json, "content")) {
        // May match tool content; try to find the message content near "choices".
        const auto choices = json.find("\"choices\"");
        if (choices != std::string_view::npos) {
            const auto sub = json.substr(choices);
            if (auto c = extract_json_string_value(sub, "content")) {
                return c;
            }
        }
        return s;
    }
    return std::nullopt;
}

std::optional<std::string> extract_anthropic_text(std::string_view json)
{
    const auto content = json.find("\"content\"");
    if (content == std::string_view::npos) {
        return std::nullopt;
    }
    return extract_json_string_value(json.substr(content), "text");
}

std::string strip_code_fences(std::string text)
{
    text = trim_copy(std::move(text));
    if (text.rfind("```", 0) == 0) {
        const auto nl = text.find('\n');
        if (nl != std::string::npos) {
            text.erase(0, nl + 1);
        }
        const auto end = text.rfind("```");
        if (end != std::string::npos) {
            text.erase(end);
        }
        text = trim_copy(std::move(text));
    }
    return text;
}

bool parse_number_list(std::string_view text_in, std::vector<MYFLT> &out, std::string &err)
{
    out.clear();
    const std::string text = trim_copy(std::string(text_in));
    const auto lb = text.find('[');
    const auto rb = text.rfind(']');
    std::string_view body = text;
    if (lb != std::string::npos && rb != std::string::npos && rb > lb) {
        body = std::string_view(text).substr(lb + 1, rb - lb - 1);
    }
    std::string token;
    auto flush = [&]() {
        if (token.empty()) {
            return true;
        }
        try {
            out.push_back(static_cast<MYFLT>(std::stod(token)));
            token.clear();
            return true;
        } catch (...) {
            err = "failed to parse numeric token: " + token;
            return false;
        }
    };
    for (char c : body) {
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '+' || c == '-' ||
            c == '.' || c == 'e' || c == 'E') {
            token += c;
        } else if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
            if (!flush()) {
                return false;
            }
        } else {
            err = std::string("unexpected character in numeric array: '") + c + "'";
            return false;
        }
    }
    if (!flush()) {
        return false;
    }
    if (out.empty()) {
        err = "numeric array is empty";
        return false;
    }
    return true;
}

bool parse_string_list(std::string_view text_in, std::vector<std::string> &out,
                       std::string &err)
{
    out.clear();
    const std::string text = trim_copy(std::string(text_in));
    const auto lb = text.find('[');
    const auto rb = text.rfind(']');
    if (lb == std::string::npos || rb == std::string::npos || rb <= lb) {
        err = "expected a JSON string array";
        return false;
    }
    size_t i = lb + 1;
    while (i < rb) {
        while (i < rb && std::isspace(static_cast<unsigned char>(text[i]))) {
            ++i;
        }
        if (i >= rb) {
            break;
        }
        if (text[i] == ',') {
            ++i;
            continue;
        }
        if (text[i] != '"') {
            err = "expected string element in array";
            return false;
        }
        ++i;
        std::string item;
        while (i < rb) {
            if (text[i] == '\\' && i + 1 < rb) {
                ++i;
                item += text[i++];
                continue;
            }
            if (text[i] == '"') {
                ++i;
                break;
            }
            item += text[i++];
        }
        out.push_back(std::move(item));
    }
    if (out.empty()) {
        err = "string array is empty";
        return false;
    }
    return true;
}

bool parse_single_number(std::string_view text, MYFLT &value, std::string &err)
{
    std::vector<MYFLT> values;
    if (parse_number_list(text, values, err)) {
        if (values.size() == 1) {
            value = values[0];
            return true;
        }
    }
    try {
        value = static_cast<MYFLT>(std::stod(trim_copy(std::string(text))));
        return true;
    } catch (...) {
        err = "expected a single numeric value";
        return false;
    }
}

/* -------------------------------------------------------------------------- */
/* HTTP via libcurl                                                           */
/* -------------------------------------------------------------------------- */

struct CurlGlobal {
    CurlGlobal() { curl_global_init(CURL_GLOBAL_DEFAULT); }
    ~CurlGlobal() { curl_global_cleanup(); }
};

CurlGlobal &curl_global()
{
    static CurlGlobal g;
    return g;
}

size_t curl_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto *out = static_cast<std::string *>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

struct HttpResult {
    long status = 0;
    std::string body;
    std::string error;
};

HttpResult http_post_json(const std::string &url,
                          const std::vector<std::string> &headers,
                          const std::string &body)
{
    curl_global();
    HttpResult result;
    CURL *curl = curl_easy_init();
    if (curl == nullptr) {
        result.error = "curl_easy_init failed";
        return result;
    }

    struct curl_slist *hdrs = nullptr;
    hdrs = curl_slist_append(hdrs, "Content-Type: application/json");
    for (const auto &h : headers) {
        hdrs = curl_slist_append(hdrs, h.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.body);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "csound-modelprompt/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    const CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        result.error = curl_easy_strerror(rc);
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    }

    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    return result;
}

/* -------------------------------------------------------------------------- */
/* Providers                                                                  */
/* -------------------------------------------------------------------------- */

enum class ResultKind {
    Text,
    Number,
    NumberArray,
    StringArray,
    InstrDef,
    Orchestra
};

std::string structured_instruction(ResultKind kind)
{
    switch (kind) {
    case ResultKind::Text:
    case ResultKind::InstrDef:
        return "Return only the requested text. Do not include markdown fences "
               "unless the user asks for them.";
    case ResultKind::Orchestra:
        return "Return only valid Csound orchestra code such as instrument "
               "definitions, connect statements, and alwayson statements. "
               "Do not include CsInstruments tags, score events, markdown "
               "fences, or commentary.";
    case ResultKind::Number:
        return "Return only a single JSON number and nothing else.";
    case ResultKind::NumberArray:
        return "Return only a JSON array of numbers and nothing else.";
    case ResultKind::StringArray:
        return "Return only a JSON array of strings and nothing else.";
    }
    return {};
}

std::string merge_options_object(const std::string &options_json)
{
    const std::string trimmed = trim_copy(options_json);
    if (trimmed.empty()) {
        return {};
    }
    if (trimmed.front() == '{' && trimmed.back() == '}') {
        // Insert as sibling fields: strip braces.
        std::string inner = trimmed.substr(1, trimmed.size() - 2);
        inner = trim_copy(std::move(inner));
        if (!inner.empty()) {
            return "," + inner;
        }
    }
    return {};
}

std::string env_key(CSOUND *csound, const char *name)
{
    const char *v = csound->GetEnv(csound, name);
    if (v == nullptr || v[0] == '\0') {
        v = std::getenv(name);
    }
    return v ? std::string(v) : std::string{};
}

bool call_openai(CSOUND *csound,
                 const std::string &model,
                 const std::string &prompt,
                 const std::string &options,
                 ResultKind kind,
                 std::string &out,
                 std::string &err)
{
    const std::string key = env_key(csound, "OPENAI_API_KEY");
    if (key.empty()) {
        err = "OPENAI_API_KEY is not set";
        return false;
    }
    const std::string system = structured_instruction(kind);
    std::ostringstream body;
    body << "{"
         << "\"model\":\"" << json_escape(model) << "\","
         << "\"messages\":["
         << "{\"role\":\"system\",\"content\":\"" << json_escape(system) << "\"},"
         << "{\"role\":\"user\",\"content\":\"" << json_escape(prompt) << "\"}"
         << "]"
         << merge_options_object(options)
         << "}";

    const auto http = http_post_json(
        "https://api.openai.com/v1/chat/completions",
        {"Authorization: Bearer " + key},
        body.str());
    if (!http.error.empty()) {
        err = http.error;
        return false;
    }
    if (http.status < 200 || http.status >= 300) {
        err = "OpenAI HTTP " + std::to_string(http.status) + ": " + http.body;
        return false;
    }
    auto content = extract_openai_content(http.body);
    if (!content) {
        err = "OpenAI response missing message content: " + http.body;
        return false;
    }
    out = strip_code_fences(*content);
    return true;
}

bool call_anthropic(CSOUND *csound,
                    const std::string &model,
                    const std::string &prompt,
                    const std::string &options,
                    ResultKind kind,
                    std::string &out,
                    std::string &err)
{
    const std::string key = env_key(csound, "ANTHROPIC_API_KEY");
    if (key.empty()) {
        err = "ANTHROPIC_API_KEY is not set";
        return false;
    }
    const std::string system = structured_instruction(kind);
    std::ostringstream body;
    body << "{"
         << "\"model\":\"" << json_escape(model) << "\","
         << "\"max_tokens\":4096,"
         << "\"system\":\"" << json_escape(system) << "\","
         << "\"messages\":[{\"role\":\"user\",\"content\":\""
         << json_escape(prompt) << "\"}]"
         << merge_options_object(options)
         << "}";

    const auto http = http_post_json(
        "https://api.anthropic.com/v1/messages",
        {"x-api-key: " + key, "anthropic-version: 2023-06-01"},
        body.str());
    if (!http.error.empty()) {
        err = http.error;
        return false;
    }
    if (http.status < 200 || http.status >= 300) {
        err = "Anthropic HTTP " + std::to_string(http.status) + ": " + http.body;
        return false;
    }
    auto content = extract_anthropic_text(http.body);
    if (!content) {
        err = "Anthropic response missing text content: " + http.body;
        return false;
    }
    out = strip_code_fences(*content);
    return true;
}

bool call_provider(CSOUND *csound,
                   const std::string &provider,
                   const std::string &model,
                   const std::string &prompt,
                   const std::string &options,
                   ResultKind kind,
                   std::string &out,
                   std::string &err)
{
    std::string p = provider;
    std::transform(p.begin(), p.end(), p.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (p == "openai") {
        return call_openai(csound, model, prompt, options, kind, out, err);
    }
    if (p == "anthropic") {
        return call_anthropic(csound, model, prompt, options, kind, out, err);
    }
    err = "unsupported provider: " + provider;
    return false;
}

/* -------------------------------------------------------------------------- */
/* Local response cache                                                       */
/* -------------------------------------------------------------------------- */

std::mutex g_cache_mutex;
std::mutex g_prompt_mutex;
std::unordered_map<CSOUND *, int> g_prompt_counters;

int next_prompt_index(CSOUND *csound)
{
    std::lock_guard<std::mutex> lock(g_prompt_mutex);
    return ++g_prompt_counters[csound];
}

void clear_prompt_counter(CSOUND *csound)
{
    std::lock_guard<std::mutex> lock(g_prompt_mutex);
    g_prompt_counters.erase(csound);
}

fs::path cache_directory(CSOUND *csound)
{
    // External plugins only see the public CSOUND_ vtable; csdname/orchname are
    // private host fields. Prefer MODELPROMPT_CSD (full path to the .csd).
    const char *csd = csound->GetEnv(csound, "MODELPROMPT_CSD");
    if (csd == nullptr || csd[0] == '\0') {
        csd = std::getenv("MODELPROMPT_CSD");
    }
    fs::path base;
    if (csd == nullptr || csd[0] == '\0' || std::string_view(csd) == "*string*") {
        base = fs::current_path() / "modelprompt_string";
    } else {
        const fs::path csd_path(csd);
        base = csd_path.parent_path() / csd_path.stem();
        if (base.empty()) {
            base = fs::current_path() / csd_path.stem();
        }
    }
    return base / "modelprompt_cache";
}

int latest_cache_version(const fs::path &dir, int prompt_index)
{
    int latest = 0;
    if (!fs::exists(dir)) {
        return latest;
    }
    const std::regex re("^" + std::to_string(prompt_index) + R"(\.([0-9]+)$)");
    for (const auto &entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const std::string filename = entry.path().filename().string();
        std::smatch m;
        if (std::regex_match(filename, m, re)) {
            latest = std::max(latest, std::stoi(m[1].str()));
        }
    }
    return latest;
}

fs::path cache_file_path(const fs::path &dir, int prompt_index, int version)
{
    return dir / (std::to_string(prompt_index) + "." + std::to_string(version));
}

std::optional<std::string> read_cache_version_at(const fs::path &dir, int prompt_index,
                                                int version, int &version_out,
                                                std::string &err)
{
    std::lock_guard<std::mutex> lock(g_cache_mutex);
    if (version <= 0) {
        err = "iversion must be positive for prompt " + std::to_string(prompt_index);
        return std::nullopt;
    }
    const fs::path path = cache_file_path(dir, prompt_index, version);
    if (!fs::exists(path)) {
        err = "no cached response for prompt " + std::to_string(prompt_index) +
              " version " + std::to_string(version);
        return std::nullopt;
    }
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        err = "failed to read cache file: " + path.string();
        return std::nullopt;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    version_out = version;
    return ss.str();
}

std::optional<std::string> read_latest_cache_at(const fs::path &dir, int prompt_index,
                                               int &version_out, std::string &err)
{
    std::lock_guard<std::mutex> lock(g_cache_mutex);
    const int version = latest_cache_version(dir, prompt_index);
    if (version <= 0) {
        err = "no cached response for prompt " + std::to_string(prompt_index);
        return std::nullopt;
    }
    const fs::path path = cache_file_path(dir, prompt_index, version);
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        err = "failed to read cache file: " + path.string();
        return std::nullopt;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    version_out = version;
    return ss.str();
}

bool write_new_cache_version_at(const fs::path &dir, int prompt_index,
                               const std::string &payload, int &version_out,
                               std::string &err)
{
    std::lock_guard<std::mutex> lock(g_cache_mutex);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        err = "failed to create cache directory: " + dir.string() + " (" +
              ec.message() + ")";
        return false;
    }
    const int version = latest_cache_version(dir, prompt_index) + 1;
    const fs::path path = cache_file_path(dir, prompt_index, version);
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        err = "failed to write cache file: " + path.string();
        return false;
    }
    out << payload;
    version_out = version;
    return true;
}

std::optional<std::string> read_latest_cache(CSOUND *csound, int prompt_index,
                                            int &version_out, std::string &err)
{
    return read_latest_cache_at(cache_directory(csound), prompt_index, version_out, err);
}

std::optional<std::string> read_cache_version(CSOUND *csound, int prompt_index, int version,
                                             int &version_out, std::string &err)
{
    return read_cache_version_at(cache_directory(csound), prompt_index, version,
                                 version_out, err);
}

bool write_new_cache_version(CSOUND *csound, int prompt_index,
                            const std::string &payload, int &version_out,
                            std::string &err)
{
    return write_new_cache_version_at(cache_directory(csound), prompt_index, payload,
                                      version_out, err);
}

bool obtain_model_text(CSOUND *csound,
                       const std::string &provider,
                       const std::string &model,
                       const std::string &prompt,
                       const std::string &options,
                       bool regenerate,
                       int prompt_index,
                       int requested_version,
                       ResultKind kind,
                       std::string &text,
                       std::string &err)
{
    int version = 0;
    if (!regenerate) {
        std::optional<std::string> cached;
        if (requested_version > 0) {
            cached = read_cache_version(csound, prompt_index, requested_version, version,
                                       err);
        } else {
            cached = read_latest_cache(csound, prompt_index, version, err);
        }
        if (!cached) {
            return false;
        }
        text = std::move(*cached);
        csound->Message(csound, "modelprompt: prompt %d version %d (frozen)\n",
                        prompt_index, version);
        return true;
    }

    if (!call_provider(csound, provider, model, prompt, options, kind, text, err)) {
        return false;
    }
    std::string cache_err;
    if (!write_new_cache_version(csound, prompt_index, text, version, cache_err)) {
        err = cache_err;
        return false;
    }
    csound->Message(csound, "modelprompt: prompt %d version %d (regenerated)\n",
                    prompt_index, version);
    return true;
}

/* -------------------------------------------------------------------------- */
/* Asynchronous request registry                                              */
/* -------------------------------------------------------------------------- */

enum RequestStatus : int {
    kPending = 0,
    kOk = 1,
    kFailed = -1,
    kInvalid = -2
};

struct AsyncRequest {
    std::mutex mu;
    int status = kPending;
    std::string result;
};

struct AsyncRegistry {
    std::mutex mu;
    int next_handle = 1;
    std::unordered_map<int, std::shared_ptr<AsyncRequest>> requests;
    std::vector<std::thread> workers;

    int create()
    {
        std::lock_guard<std::mutex> lock(mu);
        const int handle = next_handle++;
        requests.emplace(handle, std::make_shared<AsyncRequest>());
        return handle;
    }

    std::shared_ptr<AsyncRequest> get(int handle)
    {
        std::lock_guard<std::mutex> lock(mu);
        const auto it = requests.find(handle);
        if (it == requests.end()) {
            return nullptr;
        }
        return it->second;
    }

    void add_worker(std::thread &&t)
    {
        std::lock_guard<std::mutex> lock(mu);
        workers.push_back(std::move(t));
    }

    void shutdown()
    {
        std::vector<std::thread> joinable;
        {
            std::lock_guard<std::mutex> lock(mu);
            joinable.swap(workers);
        }
        for (auto &t : joinable) {
            if (t.joinable()) {
                t.join();
            }
        }
        std::lock_guard<std::mutex> lock(mu);
        requests.clear();
    }
};

AsyncRegistry &registry()
{
    static AsyncRegistry r;
    return r;
}

/* Snapshot of CSOUND env needed by workers (avoid touching CSOUND* off-thread). */
struct ProviderSnapshot {
    std::string provider;
    std::string model;
    std::string prompt;
    std::string options;
    std::string openai_key;
    std::string anthropic_key;
};

bool call_provider_snapshot(const ProviderSnapshot &snap,
                            ResultKind kind,
                            std::string &out,
                            std::string &err)
{
    std::string p = snap.provider;
    std::transform(p.begin(), p.end(), p.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (p == "openai") {
        if (snap.openai_key.empty()) {
            err = "OPENAI_API_KEY is not set";
            return false;
        }
        const std::string system = structured_instruction(kind);
        std::ostringstream body;
        body << "{"
             << "\"model\":\"" << json_escape(snap.model) << "\","
             << "\"messages\":["
             << "{\"role\":\"system\",\"content\":\"" << json_escape(system) << "\"},"
             << "{\"role\":\"user\",\"content\":\"" << json_escape(snap.prompt) << "\"}"
             << "]"
             << merge_options_object(snap.options)
             << "}";
        const auto http = http_post_json(
            "https://api.openai.com/v1/chat/completions",
            {"Authorization: Bearer " + snap.openai_key},
            body.str());
        if (!http.error.empty()) {
            err = http.error;
            return false;
        }
        if (http.status < 200 || http.status >= 300) {
            err = "OpenAI HTTP " + std::to_string(http.status) + ": " + http.body;
            return false;
        }
        auto content = extract_openai_content(http.body);
        if (!content) {
            err = "OpenAI response missing message content";
            return false;
        }
        out = strip_code_fences(*content);
        return true;
    }

    if (p == "anthropic") {
        if (snap.anthropic_key.empty()) {
            err = "ANTHROPIC_API_KEY is not set";
            return false;
        }
        const std::string system = structured_instruction(kind);
        std::ostringstream body;
        body << "{"
             << "\"model\":\"" << json_escape(snap.model) << "\","
             << "\"max_tokens\":4096,"
             << "\"system\":\"" << json_escape(system) << "\","
             << "\"messages\":[{\"role\":\"user\",\"content\":\""
             << json_escape(snap.prompt) << "\"}]"
             << merge_options_object(snap.options)
             << "}";
        const auto http = http_post_json(
            "https://api.anthropic.com/v1/messages",
            {"x-api-key: " + snap.anthropic_key, "anthropic-version: 2023-06-01"},
            body.str());
        if (!http.error.empty()) {
            err = http.error;
            return false;
        }
        if (http.status < 200 || http.status >= 300) {
            err = "Anthropic HTTP " + std::to_string(http.status) + ": " + http.body;
            return false;
        }
        auto content = extract_anthropic_text(http.body);
        if (!content) {
            err = "Anthropic response missing text content";
            return false;
        }
        out = strip_code_fences(*content);
        return true;
    }

    err = "unsupported provider: " + snap.provider;
    return false;
}

/* Cache helpers that do not touch CSOUND* (path provided by caller). */
int32_t assign_text(CSOUND *csound, STRINGDAT *out, const std::string &text)
{
    set_string(csound, out, text);
    return OK;
}

int32_t assign_number(CSOUND *csound, MYFLT *out, const std::string &text)
{
    MYFLT value = 0;
    std::string err;
    if (!parse_single_number(text, value, err)) {
        return csound->InitError(csound, "%s", err.c_str());
    }
    *out = value;
    return OK;
}

int32_t assign_number_array(CSOUND *csound, ARRAYDAT *out, INSDS *ctx,
                            const std::string &text)
{
    std::vector<MYFLT> values;
    std::string err;
    if (!parse_number_list(text, values, err)) {
        return csound->InitError(csound, "%s", err.c_str());
    }
    tabinit(csound, out, static_cast<int32_t>(values.size()), ctx);
    for (size_t i = 0; i < values.size(); ++i) {
        out->data[i] = values[i];
    }
    if (out->dimensions >= 1 && out->sizes != nullptr) {
        out->sizes[0] = static_cast<int32_t>(values.size());
    }
    return OK;
}

int32_t assign_string_array(CSOUND *csound, ARRAYDAT *out, INSDS *ctx,
                            const std::string &text)
{
    std::vector<std::string> values;
    std::string err;
    if (!parse_string_list(text, values, err)) {
        return csound->InitError(csound, "%s", err.c_str());
    }
    tabinit(csound, out, static_cast<int32_t>(values.size()), ctx);
    const size_t member = static_cast<size_t>(out->arrayMemberSize);
    for (size_t i = 0; i < values.size(); ++i) {
        auto *slot = reinterpret_cast<STRINGDAT *>(
            reinterpret_cast<char *>(out->data) + i * member);
        set_string(csound, slot, values[i]);
    }
    if (out->dimensions >= 1 && out->sizes != nullptr) {
        out->sizes[0] = static_cast<int32_t>(values.size());
    }
    return OK;
}

int32_t assign_instrdef(CSOUND *csound, INSTREF *out, const std::string &text)
{
    INSTRTXT **instrs = csound->GetInstrumentList(csound);
    if (instrs == nullptr) {
        return csound->InitError(csound, "instrument list unavailable");
    }
    int32_t num = 1;
    while (instrs[num] != nullptr) {
        ++num;
    }
    std::string code = "instr " + std::to_string(num) + "\n" + text + "\nendin\n";
    if (csoundCompileOrc(csound, code.c_str(), 0) != CSOUND_SUCCESS) {
        return csound->InitError(csound, "failed to compile model-generated InstrDef");
    }
    out->instr = instrs[num];
    out->readonly = 0;
    return OK;
}

int32_t assign_orchestra(CSOUND *csound, STRINGDAT *out, const std::string &text)
{
    if (csoundCompileOrc(csound, text.c_str(), 0) != CSOUND_SUCCESS) {
        return csound->InitError(csound,
                                 "failed to compile model-generated orchestra");
    }
    /* CompileOrc activates alwayson / graph wiring; score remains separate. */
    return assign_text(csound, out, text);
}

int32_t assign_result(CSOUND *csound, ResultKind kind, void *out, INSDS *ctx,
                      const std::string &text)
{
    switch (kind) {
    case ResultKind::Text:
        return assign_text(csound, static_cast<STRINGDAT *>(out), text);
    case ResultKind::Number:
        return assign_number(csound, static_cast<MYFLT *>(out), text);
    case ResultKind::NumberArray:
        return assign_number_array(csound, static_cast<ARRAYDAT *>(out), ctx, text);
    case ResultKind::StringArray:
        return assign_string_array(csound, static_cast<ARRAYDAT *>(out), ctx, text);
    case ResultKind::InstrDef:
        return assign_instrdef(csound, static_cast<INSTREF *>(out), text);
    case ResultKind::Orchestra:
        return assign_orchestra(csound, static_cast<STRINGDAT *>(out), text);
    }
    return NOTOK;
}

/* -------------------------------------------------------------------------- */
/* Opcode dataspaces                                                          */
/* -------------------------------------------------------------------------- */

template <typename OutT>
struct ModelPromptBase {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
};

template <typename OutT>
struct ModelPromptOpts {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    STRINGDAT *options;
};

template <typename OutT>
struct ModelPromptRegen {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
};

template <typename OutT>
struct ModelPromptRegenOpts {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
    STRINGDAT *options;
};

template <typename OutT>
struct ModelPromptRegenVer {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
    MYFLT *version;
};

template <typename OutT>
struct ModelPromptRegenVerOpts {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
    MYFLT *version;
    STRINGDAT *options;
};

struct ModelPromptAsyncBase {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
};

struct ModelPromptAsyncOpts {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    STRINGDAT *options;
};

struct ModelPromptAsyncRegen {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
};

struct ModelPromptAsyncRegenOpts {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
    STRINGDAT *options;
};

struct ModelPromptAsyncRegenVer {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
    MYFLT *version;
};

struct ModelPromptAsyncRegenVerOpts {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    MYFLT *regenerate;
    MYFLT *version;
    STRINGDAT *options;
};

struct ModelPromptResult {
    OPDS h;
    MYFLT *kstatus;
    STRINGDAT *sresult;
    MYFLT *ihandle;
};

bool regenerate_flag(MYFLT *regenerate)
{
    // Default ON when the argument is omitted (null) or non-zero.
    return regenerate == nullptr || *regenerate != FL(0.0);
}

int requested_version_number(MYFLT *version)
{
    if (version == nullptr) {
        return 0;
    }
    const int v = static_cast<int>(*version);
    return v > 0 ? v : 0;
}

int32_t run_sync(CSOUND *csound, OPDS *h, void *out, ResultKind kind,
                 STRINGDAT *provider, STRINGDAT *model, STRINGDAT *prompt,
                 STRINGDAT *options, MYFLT *regenerate, MYFLT *version)
{
    if (!nonempty(provider) || !nonempty(model) || !nonempty(prompt)) {
        return csound->InitError(csound, "modelprompt: provider, model, and prompt "
                                         "are required");
    }
    std::string text;
    std::string err;
    const int prompt_index = next_prompt_index(csound);
    if (!obtain_model_text(csound, cstr(provider), cstr(model), cstr(prompt),
                           options ? cstr(options) : "", regenerate_flag(regenerate),
                           prompt_index, requested_version_number(version), kind, text,
                           err)) {
        return csound->InitError(csound, "modelprompt: %s", err.c_str());
    }
    return assign_result(csound, kind, out, h->insdshead, text);
}

template <typename P>
int32_t init_sync_base(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    nullptr, nullptr, nullptr);
}

template <typename P>
int32_t init_sync_opts(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    p->options, nullptr, nullptr);
}

template <typename P>
int32_t init_sync_regen(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    nullptr, p->regenerate, nullptr);
}

template <typename P>
int32_t init_sync_regen_opts(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    p->options, p->regenerate, nullptr);
}

template <typename P>
int32_t init_sync_regen_ver(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    nullptr, p->regenerate, p->version);
}

template <typename P>
int32_t init_sync_regen_ver_opts(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    p->options, p->regenerate, p->version);
}

int32_t start_async(CSOUND *csound, MYFLT *ihandle_out,
                    STRINGDAT *provider, STRINGDAT *model, STRINGDAT *prompt,
                    STRINGDAT *options, MYFLT *regenerate, MYFLT *version)
{
    if (!nonempty(provider) || !nonempty(model) || !nonempty(prompt)) {
        return csound->InitError(csound, "modelprompt_async: provider, model, and "
                                         "prompt are required");
    }

    ProviderSnapshot snap;
    snap.provider = cstr(provider);
    snap.model = cstr(model);
    snap.prompt = cstr(prompt);
    snap.options = options ? cstr(options) : "";
    snap.openai_key = env_key(csound, "OPENAI_API_KEY");
    snap.anthropic_key = env_key(csound, "ANTHROPIC_API_KEY");

    const bool do_regen = regenerate_flag(regenerate);
    const int prompt_index = next_prompt_index(csound);
    const int want_version = requested_version_number(version);
    const fs::path cache_dir = cache_directory(csound);

    const int handle = registry().create();
    auto req = registry().get(handle);
    *ihandle_out = static_cast<MYFLT>(handle);

    registry().add_worker(std::thread([snap, do_regen, prompt_index, want_version,
                                       cache_dir, req]() mutable {
        std::string text;
        std::string err;
        bool ok = false;
        int got_version = 0;
        if (!do_regen) {
            std::optional<std::string> cached;
            if (want_version > 0) {
                cached = read_cache_version_at(cache_dir, prompt_index, want_version,
                                               got_version, err);
            } else {
                cached = read_latest_cache_at(cache_dir, prompt_index, got_version, err);
            }
            if (cached) {
                text = std::move(*cached);
                ok = true;
            }
        } else {
            ok = call_provider_snapshot(snap, ResultKind::Text, text, err);
            if (ok) {
                std::string cache_err;
                if (!write_new_cache_version_at(cache_dir, prompt_index, text, got_version,
                                                cache_err)) {
                    ok = false;
                    err = cache_err;
                }
            }
        }
        std::lock_guard<std::mutex> lock(req->mu);
        if (ok) {
            req->status = kOk;
            req->result = std::move(text);
        } else {
            req->status = kFailed;
            req->result = std::move(err);
        }
    }));

    return OK;
}

int32_t modelprompt_result_perf(CSOUND *csound, ModelPromptResult *p)
{
    const int handle = static_cast<int>(*p->ihandle);
    auto req = registry().get(handle);
    if (!req) {
        *p->kstatus = static_cast<MYFLT>(kInvalid);
        set_string(csound, p->sresult, "invalid or unknown request handle");
        return OK;
    }
    int status = 0;
    std::string text;
    {
        std::lock_guard<std::mutex> lock(req->mu);
        status = req->status;
        text = req->result;
    }
    *p->kstatus = static_cast<MYFLT>(status);
    if (status == kPending) {
        set_string(csound, p->sresult, "");
    } else {
        set_string(csound, p->sresult, text);
    }
    return OK;
}

/* Typed init wrappers */
#define MP_WRAP(name, kind, Struct, fn)                                        \
    static int32_t name(CSOUND *csound, void *p)                               \
    {                                                                          \
        return fn(csound, static_cast<Struct *>(p), kind);                     \
    }

MP_WRAP(mp_S_base, ResultKind::Text, ModelPromptBase<STRINGDAT>, init_sync_base)
MP_WRAP(mp_S_opts, ResultKind::Text, ModelPromptOpts<STRINGDAT>, init_sync_opts)
MP_WRAP(mp_S_regen, ResultKind::Text, ModelPromptRegen<STRINGDAT>, init_sync_regen)
MP_WRAP(mp_S_regen_opts, ResultKind::Text, ModelPromptRegenOpts<STRINGDAT>,
        init_sync_regen_opts)
MP_WRAP(mp_S_regen_ver, ResultKind::Text, ModelPromptRegenVer<STRINGDAT>,
        init_sync_regen_ver)
MP_WRAP(mp_S_regen_ver_opts, ResultKind::Text, ModelPromptRegenVerOpts<STRINGDAT>,
        init_sync_regen_ver_opts)

MP_WRAP(mp_i_base, ResultKind::Number, ModelPromptBase<MYFLT>, init_sync_base)
MP_WRAP(mp_i_opts, ResultKind::Number, ModelPromptOpts<MYFLT>, init_sync_opts)
MP_WRAP(mp_i_regen, ResultKind::Number, ModelPromptRegen<MYFLT>, init_sync_regen)
MP_WRAP(mp_i_regen_opts, ResultKind::Number, ModelPromptRegenOpts<MYFLT>,
        init_sync_regen_opts)
MP_WRAP(mp_i_regen_ver, ResultKind::Number, ModelPromptRegenVer<MYFLT>,
        init_sync_regen_ver)
MP_WRAP(mp_i_regen_ver_opts, ResultKind::Number, ModelPromptRegenVerOpts<MYFLT>,
        init_sync_regen_ver_opts)

MP_WRAP(mp_ia_base, ResultKind::NumberArray, ModelPromptBase<ARRAYDAT>, init_sync_base)
MP_WRAP(mp_ia_opts, ResultKind::NumberArray, ModelPromptOpts<ARRAYDAT>, init_sync_opts)
MP_WRAP(mp_ia_regen, ResultKind::NumberArray, ModelPromptRegen<ARRAYDAT>,
        init_sync_regen)
MP_WRAP(mp_ia_regen_opts, ResultKind::NumberArray, ModelPromptRegenOpts<ARRAYDAT>,
        init_sync_regen_opts)
MP_WRAP(mp_ia_regen_ver, ResultKind::NumberArray, ModelPromptRegenVer<ARRAYDAT>,
        init_sync_regen_ver)
MP_WRAP(mp_ia_regen_ver_opts, ResultKind::NumberArray, ModelPromptRegenVerOpts<ARRAYDAT>,
        init_sync_regen_ver_opts)

MP_WRAP(mp_Sa_base, ResultKind::StringArray, ModelPromptBase<ARRAYDAT>, init_sync_base)
MP_WRAP(mp_Sa_opts, ResultKind::StringArray, ModelPromptOpts<ARRAYDAT>, init_sync_opts)
MP_WRAP(mp_Sa_regen, ResultKind::StringArray, ModelPromptRegen<ARRAYDAT>,
        init_sync_regen)
MP_WRAP(mp_Sa_regen_opts, ResultKind::StringArray, ModelPromptRegenOpts<ARRAYDAT>,
        init_sync_regen_opts)
MP_WRAP(mp_Sa_regen_ver, ResultKind::StringArray, ModelPromptRegenVer<ARRAYDAT>,
        init_sync_regen_ver)
MP_WRAP(mp_Sa_regen_ver_opts, ResultKind::StringArray, ModelPromptRegenVerOpts<ARRAYDAT>,
        init_sync_regen_ver_opts)

MP_WRAP(mp_def_base, ResultKind::InstrDef, ModelPromptBase<INSTREF>, init_sync_base)
MP_WRAP(mp_def_opts, ResultKind::InstrDef, ModelPromptOpts<INSTREF>, init_sync_opts)
MP_WRAP(mp_def_regen, ResultKind::InstrDef, ModelPromptRegen<INSTREF>, init_sync_regen)
MP_WRAP(mp_def_regen_opts, ResultKind::InstrDef, ModelPromptRegenOpts<INSTREF>,
        init_sync_regen_opts)
MP_WRAP(mp_def_regen_ver, ResultKind::InstrDef, ModelPromptRegenVer<INSTREF>,
        init_sync_regen_ver)
MP_WRAP(mp_def_regen_ver_opts, ResultKind::InstrDef, ModelPromptRegenVerOpts<INSTREF>,
        init_sync_regen_ver_opts)

MP_WRAP(mp_orc_base, ResultKind::Orchestra, ModelPromptBase<STRINGDAT>, init_sync_base)
MP_WRAP(mp_orc_opts, ResultKind::Orchestra, ModelPromptOpts<STRINGDAT>, init_sync_opts)
MP_WRAP(mp_orc_regen, ResultKind::Orchestra, ModelPromptRegen<STRINGDAT>, init_sync_regen)
MP_WRAP(mp_orc_regen_opts, ResultKind::Orchestra, ModelPromptRegenOpts<STRINGDAT>,
        init_sync_regen_opts)
MP_WRAP(mp_orc_regen_ver, ResultKind::Orchestra, ModelPromptRegenVer<STRINGDAT>,
        init_sync_regen_ver)
MP_WRAP(mp_orc_regen_ver_opts, ResultKind::Orchestra, ModelPromptRegenVerOpts<STRINGDAT>,
        init_sync_regen_ver_opts)

static int32_t mpa_base(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncBase *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, nullptr,
                       nullptr, nullptr);
}
static int32_t mpa_opts(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncOpts *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, p->options,
                       nullptr, nullptr);
}
static int32_t mpa_regen(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncRegen *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, nullptr,
                       p->regenerate, nullptr);
}
static int32_t mpa_regen_opts(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncRegenOpts *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, p->options,
                       p->regenerate, nullptr);
}
static int32_t mpa_regen_ver(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncRegenVer *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, nullptr,
                       p->regenerate, p->version);
}
static int32_t mpa_regen_ver_opts(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncRegenVerOpts *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, p->options,
                       p->regenerate, p->version);
}

static int32_t mpr_perf(CSOUND *csound, void *pp)
{
    return modelprompt_result_perf(csound, static_cast<ModelPromptResult *>(pp));
}

OENTRY localops[] = {
    {ochar("modelprompt"), sizeof(ModelPromptBase<STRINGDAT>), 0, ochar("S"), ochar("SSS"),
     (SUBR)mp_S_base, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<STRINGDAT>), 0, ochar("S"), ochar("SSSS"),
     (SUBR)mp_S_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegen<STRINGDAT>), 0, ochar("S"), ochar("SSSi"),
     (SUBR)mp_S_regen, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenOpts<STRINGDAT>), 0, ochar("S"), ochar("SSSiS"),
     (SUBR)mp_S_regen_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVer<STRINGDAT>), 0, ochar("S"), ochar("SSSii"),
     (SUBR)mp_S_regen_ver, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVerOpts<STRINGDAT>), 0, ochar("S"),
     ochar("SSSiiS"), (SUBR)mp_S_regen_ver_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<MYFLT>), 0, ochar("i"), ochar("SSS"), (SUBR)mp_i_base,
     nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<MYFLT>), 0, ochar("i"), ochar("SSSS"), (SUBR)mp_i_opts,
     nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegen<MYFLT>), 0, ochar("i"), ochar("SSSi"),
     (SUBR)mp_i_regen, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenOpts<MYFLT>), 0, ochar("i"), ochar("SSSiS"),
     (SUBR)mp_i_regen_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVer<MYFLT>), 0, ochar("i"), ochar("SSSii"),
     (SUBR)mp_i_regen_ver, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVerOpts<MYFLT>), 0, ochar("i"), ochar("SSSiiS"),
     (SUBR)mp_i_regen_ver_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<ARRAYDAT>), 0, ochar("i[]"), ochar("SSS"),
     (SUBR)mp_ia_base, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<ARRAYDAT>), 0, ochar("i[]"), ochar("SSSS"),
     (SUBR)mp_ia_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegen<ARRAYDAT>), 0, ochar("i[]"), ochar("SSSi"),
     (SUBR)mp_ia_regen, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenOpts<ARRAYDAT>), 0, ochar("i[]"), ochar("SSSiS"),
     (SUBR)mp_ia_regen_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVer<ARRAYDAT>), 0, ochar("i[]"), ochar("SSSii"),
     (SUBR)mp_ia_regen_ver, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVerOpts<ARRAYDAT>), 0, ochar("i[]"),
     ochar("SSSiiS"), (SUBR)mp_ia_regen_ver_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<ARRAYDAT>), 0, ochar("S[]"), ochar("SSS"),
     (SUBR)mp_Sa_base, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<ARRAYDAT>), 0, ochar("S[]"), ochar("SSSS"),
     (SUBR)mp_Sa_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegen<ARRAYDAT>), 0, ochar("S[]"), ochar("SSSi"),
     (SUBR)mp_Sa_regen, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenOpts<ARRAYDAT>), 0, ochar("S[]"), ochar("SSSiS"),
     (SUBR)mp_Sa_regen_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVer<ARRAYDAT>), 0, ochar("S[]"), ochar("SSSii"),
     (SUBR)mp_Sa_regen_ver, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVerOpts<ARRAYDAT>), 0, ochar("S[]"),
     ochar("SSSiiS"), (SUBR)mp_Sa_regen_ver_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSS"),
     (SUBR)mp_def_base, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSSS"),
     (SUBR)mp_def_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegen<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSSi"),
     (SUBR)mp_def_regen, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenOpts<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSSiS"),
     (SUBR)mp_def_regen_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVer<INSTREF>), 0, ochar(":InstrDef;"),
     ochar("SSSii"), (SUBR)mp_def_regen_ver, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptRegenVerOpts<INSTREF>), 0, ochar(":InstrDef;"),
     ochar("SSSiiS"), (SUBR)mp_def_regen_ver_opts, nullptr, nullptr},

    {ochar("modelprompt_orc"), sizeof(ModelPromptBase<STRINGDAT>), 0, ochar("S"), ochar("SSS"),
     (SUBR)mp_orc_base, nullptr, nullptr},
    {ochar("modelprompt_orc"), sizeof(ModelPromptOpts<STRINGDAT>), 0, ochar("S"), ochar("SSSS"),
     (SUBR)mp_orc_opts, nullptr, nullptr},
    {ochar("modelprompt_orc"), sizeof(ModelPromptRegen<STRINGDAT>), 0, ochar("S"), ochar("SSSi"),
     (SUBR)mp_orc_regen, nullptr, nullptr},
    {ochar("modelprompt_orc"), sizeof(ModelPromptRegenOpts<STRINGDAT>), 0, ochar("S"),
     ochar("SSSiS"), (SUBR)mp_orc_regen_opts, nullptr, nullptr},
    {ochar("modelprompt_orc"), sizeof(ModelPromptRegenVer<STRINGDAT>), 0, ochar("S"),
     ochar("SSSii"), (SUBR)mp_orc_regen_ver, nullptr, nullptr},
    {ochar("modelprompt_orc"), sizeof(ModelPromptRegenVerOpts<STRINGDAT>), 0, ochar("S"),
     ochar("SSSiiS"), (SUBR)mp_orc_regen_ver_opts, nullptr, nullptr},

    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncBase), 0, ochar("i"), ochar("SSS"), (SUBR)mpa_base,
     nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncOpts), 0, ochar("i"), ochar("SSSS"), (SUBR)mpa_opts,
     nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncRegen), 0, ochar("i"), ochar("SSSi"),
     (SUBR)mpa_regen, nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncRegenOpts), 0, ochar("i"), ochar("SSSiS"),
     (SUBR)mpa_regen_opts, nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncRegenVer), 0, ochar("i"), ochar("SSSii"),
     (SUBR)mpa_regen_ver, nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncRegenVerOpts), 0, ochar("i"), ochar("SSSiiS"),
     (SUBR)mpa_regen_ver_opts, nullptr, nullptr},

    {ochar("modelprompt_result"), sizeof(ModelPromptResult), 0, ochar("kS"), ochar("i"), nullptr,
     (SUBR)mpr_perf, nullptr},

    {nullptr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr}};

} // namespace

extern "C" {

PUBLIC int32_t csoundModuleCreate(CSOUND *csound)
{
    IGN(csound);
    curl_global();
    return 0;
}

PUBLIC int32_t csoundModuleInit(CSOUND *csound)
{
    int32_t err = 0;
    for (OENTRY *ep = localops; ep->opname != nullptr; ++ep) {
        err |= csound->AppendOpcode(csound, ep->opname, ep->dsblksiz, ep->flags,
                                    ep->outypes, ep->intypes, (SUBR)ep->init,
                                    (SUBR)ep->perf, (SUBR)ep->deinit);
    }
    return err;
}

PUBLIC int32_t csoundModuleDestroy(CSOUND *csound)
{
    clear_prompt_counter(csound);
    registry().shutdown();
    return 0;
}

PUBLIC int32_t csoundModuleInfo(void)
{
    return ((CS_VERSION << 16) + (CS_SUBVER << 8) + (int32_t)sizeof(MYFLT));
}

} // extern "C"
