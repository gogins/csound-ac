/*
  modelprompt.cpp

  Csound 7 plugin opcodes for synchronous and asynchronous interaction
  with external generative models (OpenAI, Anthropic).

  Copyright (C) 2026 Michael Gogins

  This file is part of csound-ac / modelprompt.

  Design: see MODELPROMPT.md in this directory.

  OpcodeBase.hpp decision
  -----------------------
  This plugin does NOT use csound/Opcodes/OpcodeBase.hpp (nor
  CsoundAC/OpcodeBaseAC.hpp).

  Reasons:
  1. External plugin layout: OpcodeBase.hpp is written for in-tree opcode
     builds (BUILD_PLUGINS / csoundCore include paths). A standalone module
     under modelprompt/ should depend only on the public plugin header
     csdl.h.
  2. Little benefit: these opcodes are initialization-time and control-rate
     pollers. They do not need OpcodeBase's ksmps audio-frame helpers.
  3. Thread safety: shared async request state and cache-file versioning are
     clearer with C++20 std::mutex / std::shared_ptr than with Csound mutex
     wrappers intended for OpcodeBase instances.
  4. Heterogeneous dataspace: many OENTRY overloads (S, i, i[], S[],
     InstrDef, with/without cache) fit ordinary OPDS structs and shared
     free functions better than a single OpcodeBase hierarchy.

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
    InstrDef
};

std::string structured_instruction(ResultKind kind)
{
    switch (kind) {
    case ResultKind::Text:
    case ResultKind::InstrDef:
        return "Return only the requested text. Do not include markdown fences "
               "unless the user asks for them.";
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

std::string sanitize_cache_name(std::string_view name)
{
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ||
            c == '.') {
            out += c;
        } else {
            out += '_';
        }
    }
    if (out.empty()) {
        out = "unnamed";
    }
    return out;
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

int latest_cache_version(const fs::path &dir, const std::string &name)
{
    int latest = 0;
    if (!fs::exists(dir)) {
        return latest;
    }
    const std::regex re("^" + name + R"(\.([0-9]+)$)");
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

std::optional<std::string> read_latest_cache(CSOUND *csound, const std::string &raw_name,
                                            std::string &err)
{
    std::lock_guard<std::mutex> lock(g_cache_mutex);
    const std::string name = sanitize_cache_name(raw_name);
    const fs::path dir = cache_directory(csound);
    const int version = latest_cache_version(dir, name);
    if (version <= 0) {
        err = "no cached response for cache_name '" + name + "'";
        return std::nullopt;
    }
    const fs::path path = dir / (name + "." + std::to_string(version));
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        err = "failed to read cache file: " + path.string();
        return std::nullopt;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool write_new_cache_version(CSOUND *csound, const std::string &raw_name,
                            const std::string &payload, std::string &err)
{
    std::lock_guard<std::mutex> lock(g_cache_mutex);
    const std::string name = sanitize_cache_name(raw_name);
    const fs::path dir = cache_directory(csound);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        err = "failed to create cache directory: " + dir.string() + " (" +
              ec.message() + ")";
        return false;
    }
    const int version = latest_cache_version(dir, name) + 1;
    const fs::path path = dir / (name + "." + std::to_string(version));
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        err = "failed to write cache file: " + path.string();
        return false;
    }
    out << payload;
    return true;
}

bool obtain_model_text(CSOUND *csound,
                       const std::string &provider,
                       const std::string &model,
                       const std::string &prompt,
                       const std::string &options,
                       bool use_cache,
                       bool freeze,
                       const std::string &cache_name,
                       ResultKind kind,
                       std::string &text,
                       std::string &err)
{
    if (use_cache && freeze) {
        auto cached = read_latest_cache(csound, cache_name, err);
        if (!cached) {
            return false;
        }
        text = std::move(*cached);
        return true;
    }

    if (!call_provider(csound, provider, model, prompt, options, kind, text, err)) {
        return false;
    }

    if (use_cache) {
        std::string cache_err;
        if (!write_new_cache_version(csound, cache_name, text, cache_err)) {
            err = cache_err;
            return false;
        }
    }
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
std::optional<std::string> read_latest_cache_at(const fs::path &dir,
                                               const std::string &raw_name,
                                               std::string &err)
{
    std::lock_guard<std::mutex> lock(g_cache_mutex);
    const std::string name = sanitize_cache_name(raw_name);
    const int version = latest_cache_version(dir, name);
    if (version <= 0) {
        err = "no cached response for cache_name '" + name + "'";
        return std::nullopt;
    }
    const fs::path path = dir / (name + "." + std::to_string(version));
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        err = "failed to read cache file: " + path.string();
        return std::nullopt;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool write_new_cache_version_at(const fs::path &dir, const std::string &raw_name,
                               const std::string &payload, std::string &err)
{
    std::lock_guard<std::mutex> lock(g_cache_mutex);
    const std::string name = sanitize_cache_name(raw_name);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
        err = "failed to create cache directory: " + dir.string();
        return false;
    }
    const int version = latest_cache_version(dir, name) + 1;
    const fs::path path = dir / (name + "." + std::to_string(version));
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        err = "failed to write cache file: " + path.string();
        return false;
    }
    out << payload;
    return true;
}

/* -------------------------------------------------------------------------- */
/* Assign helpers                                                             */
/* -------------------------------------------------------------------------- */

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
struct ModelPromptCache {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    STRINGDAT *cache_name;
    MYFLT *freeze;
};

template <typename OutT>
struct ModelPromptCacheOpts {
    OPDS h;
    OutT *out;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    STRINGDAT *cache_name;
    MYFLT *freeze;
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

struct ModelPromptAsyncCache {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    STRINGDAT *cache_name;
    MYFLT *freeze;
};

struct ModelPromptAsyncCacheOpts {
    OPDS h;
    MYFLT *ihandle;
    STRINGDAT *provider;
    STRINGDAT *model;
    STRINGDAT *prompt;
    STRINGDAT *cache_name;
    MYFLT *freeze;
    STRINGDAT *options;
};

struct ModelPromptResult {
    OPDS h;
    MYFLT *kstatus;
    STRINGDAT *sresult;
    MYFLT *ihandle;
};

int32_t run_sync(CSOUND *csound, OPDS *h, void *out, ResultKind kind,
                 STRINGDAT *provider, STRINGDAT *model, STRINGDAT *prompt,
                 STRINGDAT *options, bool use_cache, STRINGDAT *cache_name,
                 MYFLT *freeze)
{
    if (!nonempty(provider) || !nonempty(model) || !nonempty(prompt)) {
        return csound->InitError(csound, "modelprompt: provider, model, and prompt "
                                         "are required");
    }
    std::string text;
    std::string err;
    const bool freeze_flag = use_cache && freeze != nullptr && *freeze != FL(0.0);
    if (!obtain_model_text(csound, cstr(provider), cstr(model), cstr(prompt),
                           options ? cstr(options) : "", use_cache, freeze_flag,
                           use_cache ? cstr(cache_name) : "", kind, text, err)) {
        return csound->InitError(csound, "modelprompt: %s", err.c_str());
    }
    return assign_result(csound, kind, out, h->insdshead, text);
}

template <typename P>
int32_t init_sync_base(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    nullptr, false, nullptr, nullptr);
}

template <typename P>
int32_t init_sync_opts(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    p->options, false, nullptr, nullptr);
}

template <typename P>
int32_t init_sync_cache(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    nullptr, true, p->cache_name, p->freeze);
}

template <typename P>
int32_t init_sync_cache_opts(CSOUND *csound, P *p, ResultKind kind)
{
    return run_sync(csound, &p->h, p->out, kind, p->provider, p->model, p->prompt,
                    p->options, true, p->cache_name, p->freeze);
}

int32_t start_async(CSOUND *csound, MYFLT *ihandle_out,
                    STRINGDAT *provider, STRINGDAT *model, STRINGDAT *prompt,
                    STRINGDAT *options, bool use_cache, STRINGDAT *cache_name,
                    MYFLT *freeze)
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

    const bool freeze_flag = use_cache && freeze != nullptr && *freeze != FL(0.0);
    const std::string cache = use_cache ? cstr(cache_name) : "";
    const fs::path cache_dir = use_cache ? cache_directory(csound) : fs::path{};

    const int handle = registry().create();
    auto req = registry().get(handle);
    *ihandle_out = static_cast<MYFLT>(handle);

    registry().add_worker(std::thread([snap, freeze_flag, use_cache, cache, cache_dir,
                                       req]() mutable {
        std::string text;
        std::string err;
        bool ok = false;
        if (use_cache && freeze_flag) {
            auto cached = read_latest_cache_at(cache_dir, cache, err);
            if (cached) {
                text = std::move(*cached);
                ok = true;
            }
        } else {
            ok = call_provider_snapshot(snap, ResultKind::Text, text, err);
            if (ok && use_cache) {
                std::string cache_err;
                if (!write_new_cache_version_at(cache_dir, cache, text, cache_err)) {
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
MP_WRAP(mp_S_cache, ResultKind::Text, ModelPromptCache<STRINGDAT>, init_sync_cache)
MP_WRAP(mp_S_cache_opts, ResultKind::Text, ModelPromptCacheOpts<STRINGDAT>,
        init_sync_cache_opts)

MP_WRAP(mp_i_base, ResultKind::Number, ModelPromptBase<MYFLT>, init_sync_base)
MP_WRAP(mp_i_opts, ResultKind::Number, ModelPromptOpts<MYFLT>, init_sync_opts)
MP_WRAP(mp_i_cache, ResultKind::Number, ModelPromptCache<MYFLT>, init_sync_cache)
MP_WRAP(mp_i_cache_opts, ResultKind::Number, ModelPromptCacheOpts<MYFLT>,
        init_sync_cache_opts)

MP_WRAP(mp_ia_base, ResultKind::NumberArray, ModelPromptBase<ARRAYDAT>, init_sync_base)
MP_WRAP(mp_ia_opts, ResultKind::NumberArray, ModelPromptOpts<ARRAYDAT>, init_sync_opts)
MP_WRAP(mp_ia_cache, ResultKind::NumberArray, ModelPromptCache<ARRAYDAT>,
        init_sync_cache)
MP_WRAP(mp_ia_cache_opts, ResultKind::NumberArray, ModelPromptCacheOpts<ARRAYDAT>,
        init_sync_cache_opts)

MP_WRAP(mp_Sa_base, ResultKind::StringArray, ModelPromptBase<ARRAYDAT>, init_sync_base)
MP_WRAP(mp_Sa_opts, ResultKind::StringArray, ModelPromptOpts<ARRAYDAT>, init_sync_opts)
MP_WRAP(mp_Sa_cache, ResultKind::StringArray, ModelPromptCache<ARRAYDAT>,
        init_sync_cache)
MP_WRAP(mp_Sa_cache_opts, ResultKind::StringArray, ModelPromptCacheOpts<ARRAYDAT>,
        init_sync_cache_opts)

MP_WRAP(mp_def_base, ResultKind::InstrDef, ModelPromptBase<INSTREF>, init_sync_base)
MP_WRAP(mp_def_opts, ResultKind::InstrDef, ModelPromptOpts<INSTREF>, init_sync_opts)
MP_WRAP(mp_def_cache, ResultKind::InstrDef, ModelPromptCache<INSTREF>, init_sync_cache)
MP_WRAP(mp_def_cache_opts, ResultKind::InstrDef, ModelPromptCacheOpts<INSTREF>,
        init_sync_cache_opts)

static int32_t mpa_base(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncBase *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, nullptr,
                       false, nullptr, nullptr);
}
static int32_t mpa_opts(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncOpts *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, p->options,
                       false, nullptr, nullptr);
}
static int32_t mpa_cache(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncCache *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, nullptr,
                       true, p->cache_name, p->freeze);
}
static int32_t mpa_cache_opts(CSOUND *csound, void *pp)
{
    auto *p = static_cast<ModelPromptAsyncCacheOpts *>(pp);
    return start_async(csound, p->ihandle, p->provider, p->model, p->prompt, p->options,
                       true, p->cache_name, p->freeze);
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
    {ochar("modelprompt"), sizeof(ModelPromptCache<STRINGDAT>), 0, ochar("S"), ochar("SSSSi"),
     (SUBR)mp_S_cache, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCacheOpts<STRINGDAT>), 0, ochar("S"), ochar("SSSSiS"),
     (SUBR)mp_S_cache_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<MYFLT>), 0, ochar("i"), ochar("SSS"), (SUBR)mp_i_base,
     nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<MYFLT>), 0, ochar("i"), ochar("SSSS"), (SUBR)mp_i_opts,
     nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCache<MYFLT>), 0, ochar("i"), ochar("SSSSi"),
     (SUBR)mp_i_cache, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCacheOpts<MYFLT>), 0, ochar("i"), ochar("SSSSiS"),
     (SUBR)mp_i_cache_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<ARRAYDAT>), 0, ochar("i[]"), ochar("SSS"),
     (SUBR)mp_ia_base, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<ARRAYDAT>), 0, ochar("i[]"), ochar("SSSS"),
     (SUBR)mp_ia_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCache<ARRAYDAT>), 0, ochar("i[]"), ochar("SSSSi"),
     (SUBR)mp_ia_cache, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCacheOpts<ARRAYDAT>), 0, ochar("i[]"), ochar("SSSSiS"),
     (SUBR)mp_ia_cache_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<ARRAYDAT>), 0, ochar("S[]"), ochar("SSS"),
     (SUBR)mp_Sa_base, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<ARRAYDAT>), 0, ochar("S[]"), ochar("SSSS"),
     (SUBR)mp_Sa_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCache<ARRAYDAT>), 0, ochar("S[]"), ochar("SSSSi"),
     (SUBR)mp_Sa_cache, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCacheOpts<ARRAYDAT>), 0, ochar("S[]"), ochar("SSSSiS"),
     (SUBR)mp_Sa_cache_opts, nullptr, nullptr},

    {ochar("modelprompt"), sizeof(ModelPromptBase<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSS"),
     (SUBR)mp_def_base, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptOpts<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSSS"),
     (SUBR)mp_def_opts, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCache<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSSSi"),
     (SUBR)mp_def_cache, nullptr, nullptr},
    {ochar("modelprompt"), sizeof(ModelPromptCacheOpts<INSTREF>), 0, ochar(":InstrDef;"), ochar("SSSSiS"),
     (SUBR)mp_def_cache_opts, nullptr, nullptr},

    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncBase), 0, ochar("i"), ochar("SSS"), (SUBR)mpa_base,
     nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncOpts), 0, ochar("i"), ochar("SSSS"), (SUBR)mpa_opts,
     nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncCache), 0, ochar("i"), ochar("SSSSi"),
     (SUBR)mpa_cache, nullptr, nullptr},
    {ochar("modelprompt_async"), sizeof(ModelPromptAsyncCacheOpts), 0, ochar("i"), ochar("SSSSiS"),
     (SUBR)mpa_cache_opts, nullptr, nullptr},

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
    IGN(csound);
    registry().shutdown();
    return 0;
}

PUBLIC int32_t csoundModuleInfo(void)
{
    return ((CS_VERSION << 16) + (CS_SUBVER << 8) + (int32_t)sizeof(MYFLT));
}

} // extern "C"
