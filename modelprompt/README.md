# modelprompt

Csound 7 opcode plugin that submits prompts to external generative models
(OpenAI, Anthropic) and returns text, numbers, arrays, compiled
`InstrDef` values, or full orchestra fragments via `modelprompt_orc`.

The `modelprompt` opcodes are designed for use in cases when simply prompting models for .csd files does not suffice. For example:

- Keeping some parts of a piece is hand-coded form, and other parts in model-generated form.

- Easily "freezing" good responses.

- Issuing prompts that include data that arrives after the beginning of a performance, e.g. at random, from external inputs, or through live coding.

## Getting Started

### Installation

Built with csound-ac when `BUILD_MODELPROMPT` is ON (default; requires libcurl).
Disable with `-DBUILD_MODELPROMPT=OFF`.

The plugin installs to:

```text
<prefix>/lib/csound/plugins64-7.0/modelprompt.dylib   # macOS
<prefix>/lib/csound/plugins64-7.0/modelprompt.so      # Linux
<prefix>/lib/csound/plugins64-7.0/modelprompt.dll     # Windows
```

Documentation and examples install to:

```text
<prefix>/share/doc/csound-ac/modelprompt/
<prefix>/share/doc/csound-ac/modelprompt/examples/
```

Csound must find the plugin directory. On macOS, user plugins are typically:

```bash
mkdir -p ~/Library/csound/7.0/plugins64
ln -sf /opt/homebrew/lib/csound/plugins64-7.0/modelprompt.dylib \
  ~/Library/csound/7.0/plugins64/modelprompt.dylib
```

(Adjust the source path if you installed under `/usr/local` or another prefix.)

On Linux, ensure `OPCODE7DIR64` includes `<prefix>/lib/csound/plugins64-7.0`,
or copy/symlink the plugin into a directory already listed there.

### Configuration

Do not put API keys in `.csd` files. Export a provider key before starting Csound:

```bash
export ANTHROPIC_API_KEY="your-api-key"
# or
export OPENAI_API_KEY="your-api-key"
```

On Windows PowerShell:

```powershell
$env:ANTHROPIC_API_KEY="your-api-key"
# or
$env:OPENAI_API_KEY="your-api-key"
```

Every `modelprompt` / `modelprompt_orc` / `modelprompt_async` /
`modelprompt_orc_async` call is numbered sequentially in the Csound instance
(prompt 1, 2, …). Responses are cached under a directory named from the `.csd`
basename:

```text
{csd_directory}/{csd_basename}/modelprompt_cache/{prompt_index}.{version}
```

For example, running `examples/anthropic_sonnet_orc_score.csd` stores cache
files in `examples/anthropic_sonnet_orc_score/modelprompt_cache/`.

The `.csd` path is taken from `MODELPROMPT_CSD` if set, otherwise from a
`.csd` argument on the host process command line (for example `csound
piece.csd`). Set `MODELPROMPT_CSD` when compiling from a string or when the
host does not put the `.csd` on argv. If no `.csd` name is available, the
plugin uses `modelprompt_string/modelprompt_cache` under the current working
directory.

By default (`iregenerate` omitted) each call **reuses the latest cache** when
present, and only calls the model on a cache miss. Pass `iregenerate=1` to force
a new model response (next version), or `iregenerate=0` to require a cache hit
(`iregenerate=0, iversion=N` pins an earlier version).

### Example

```bash
export ANTHROPIC_API_KEY=...
csound modelprompt/examples/anthropic_sonnet_instr_score.csd
csound modelprompt/examples/anthropic_sonnet_orc_score.csd
csound modelprompt/examples/anthropic_sonnet_orc_async.csd
csound modelprompt/examples/anthropic_sonnet_glass_garden.csd
```

Or after install:

```bash
csound <prefix>/share/doc/csound-ac/modelprompt/examples/anthropic_sonnet_instr_score.csd
```

The first example asks Sonnet for an instrument body and a score. The second
uses `modelprompt_orc` for a connected orchestra plus a separate score prompt.
The third uses `modelprompt_orc_async` / `modelprompt_orc_result` so orchestra
generation does not block performance. The fourth, **Glass Garden**, is a longer
showcase: title, pitch set, three scored sections (Glass/Pad → Spark → Pulse at
rising tempos), two async score fetches, and a compiled FX graph. Network access
and a valid API key are required.

---

## Overview

The model interaction opcodes allow Csound orchestras to submit natural-language prompts to external generative models and to receive the results as Csound data.

The opcodes are implemented as a C++ plugin. The models themselves are not part of Csound. The plugin communicates with an external model provider, such as OpenAI or Anthropic, using the provider's network API.

The principal opcode, `modelprompt`, performs a synchronous request during initialization and may return text, numbers, arrays, or compiled instrument definitions. `modelprompt_orc` returns orchestra source after compiling and activating it. The asynchronous pairs `modelprompt_async` / `modelprompt_result` and `modelprompt_orc_async` / `modelprompt_orc_result` permit a request to execute in a worker thread while Csound continues to perform; orchestra compilation for the latter pair still occurs on the Csound thread when the result is polled.

A model can be used to generate or transform, for example:

- numerical synthesis parameters;
- vectors of pitches, frequencies, amplitudes, or durations;
- symbolic or textual data;
- Csound score text;
- synthesis descriptions;
- Csound instrument definitions; or
- complete orchestra fragments with `connect` / `alwayson` graphs
  (`modelprompt_orc`).

The model is selected by a provider name and a provider-specific model name. For example:

```csound
Sresult = modelprompt(
    "openai",
    "MODEL_NAME",
    "Generate twelve frequency ratios."
)
```

or:

```csound
Sresult = modelprompt(
    "anthropic",
    "MODEL_NAME",
    "Generate twelve frequency ratios."
)
```

The provider and model names are strings so that the opcode interface does not depend upon a particular model vendor or model release.

### Synchronous and asynchronous operation

`modelprompt` is intended primarily for composition, score generation, initialization-time synthesis design, and other operations for which Csound may wait for the model response.

For example:

```csound
freqs:i[] = modelprompt(
    "openai",
    "MODEL_NAME",
    {{
    Generate 12 frequencies in Hz forming an approximately
    harmonic spectrum above a fundamental of 110 Hz, with
    progressively increasing inharmonicity.
    }}
)
```

A synchronous request may require network communication and may take an unpredictable amount of time. `modelprompt` therefore blocks initialization of the calling instrument until the request completes or fails.

For performance-time operation, `modelprompt_async` starts a request in a worker thread and immediately returns a request handle:

```csound
ihandle = modelprompt_async(
    "openai",
    "MODEL_NAME",
    {{
    Generate a short textual description of a new
    transformation for the current musical material.
    }}
)
```

`modelprompt_result` can subsequently be called at control rate to determine whether the request has completed and, when it has, obtain the returned text:

```csound
kstatus, Sresult modelprompt_result ihandle
```

The asynchronous opcodes do not wait for network communication on Csound's audio-performance thread.

In the initial interface described here, asynchronous requests return text. Typed Csound values such as numbers and arrays are obtained with the synchronous `modelprompt` opcode. This keeps the asynchronous API small and avoids requiring a Csound result type to be specified before the request is launched.

### Typed results

`modelprompt` is overloaded according to its output type.

The interface supports the following kinds of results:

```text
i
S
i[]
S[]
InstrDef
```

The output type specifies the form of data that the model is required to produce.

For numerical and string arrays, the plugin requests a JSON array from the model provider and converts the response to the corresponding Csound value.

Conceptually:

```text
Csound output type
        |
        v
external model
        |
        v
parsed response
        |
        v
Csound value
```

### Model-generated score data

Score events may be requested as Csound score text:

```csound
Sscore = modelprompt(
    "openai",
    "MODEL_NAME",
    {{
    Generate 20 valid Csound i-statements for instrument 1.
    p4 is frequency in Hz and p5 is amplitude from 0 to 1.
    Return only Csound score text.
    }}
)

scorelinei(Sscore)
```

This representation is general and can contain ordinary Csound score syntax.

### Model-generated instrument definitions

When the output type is `InstrDef`, the model is required to produce valid Csound instrument code.

For example:

```csound
Bell:InstrDef = modelprompt(
    "openai",
    "MODEL_NAME",
    {{
    Write the body of a Csound instrument for an inharmonic
    modal bell.

    p4 is amplitude.
    p5 is fundamental frequency.

    Use at least 12 resonant modes and give higher modes
    progressively shorter decay times.

    Return only valid Csound instrument-body code.
    }}
)

schedule(Bell, 0, 8, 0.2, 220)
```

The returned source text is compiled into an `InstrDef`. If the generated source cannot be compiled, the opcode reports an initialization error.

Model-generated instrument definitions contain executable Csound code and should be treated differently from model-generated numeric or symbolic data.

### Model-generated orchestras

`modelprompt_orc` requests a full orchestra fragment, compiles it with Csound's runtime orchestra compiler, and returns the source text. Instruments started with `alwayson` become active as part of compilation. Note events are supplied separately, typically with a second `modelprompt` call and `scorelinei`.

```csound
Sorc = modelprompt_orc(
    "anthropic",
    "MODEL_NAME",
    {{
    Write instruments Tone and Reverb, connect Tone to Reverb,
    connect Reverb to a Master that writes to the dac, and
    alwayson Reverb and Master.
    }}
)

Sscore = modelprompt(
    "anthropic",
    "MODEL_NAME",
    {{
    Write 8 i-statements for instrument Tone.
    Return only score text.
    }}
)
scorelinei(Sscore)
```

If the orchestra cannot be compiled, the opcode reports an initialization error.

## Configuring an API Key

The model opcodes require an API key for the selected model provider. API keys should not be written directly into a `.csd` file. Instead, store the key in an environment variable before starting Csound.

For OpenAI on macOS, Linux, or another Unix-like system:

```bash
export OPENAI_API_KEY="your-api-key"
```

For Anthropic:

```bash
export ANTHROPIC_API_KEY="your-api-key"
```

On Windows PowerShell, for example:

```powershell
$env:OPENAI_API_KEY="your-api-key"
```

or:

```powershell
$env:ANTHROPIC_API_KEY="your-api-key"
```

The opcodes obtain the appropriate key from the process environment according to the selected provider. For example:

```csound
Sresult = modelprompt(
    "openai",
    "MODEL_NAME",
    "Generate twelve frequency ratios."
)
```

uses the value of `OPENAI_API_KEY`, while:

```csound
Sresult = modelprompt(
    "anthropic",
    "MODEL_NAME",
    "Generate twelve frequency ratios."
)
```

uses the value of `ANTHROPIC_API_KEY`.

The same environment variables are used by `modelprompt_async`.

If the required environment variable is not defined, or if the API key is rejected by the provider, the request fails and the plugin reports the provider error when available.

Environment variables must be visible to the process that launches Csound. In particular, when Csound is started from a graphical application rather than from a terminal, environment variables defined only in a shell startup file may not be available to Csound.

API keys are credentials and should be kept private. They should not be embedded in `.csd` files, committed to source-code repositories, printed in diagnostic output, or included in files distributed with a composition.

### Reproducibility

Responses from generative models are not necessarily deterministic. The same prompt submitted to the same nominal model at different times may return different results.

Model providers may also update, replace, or retire models without preserving exact behavior.

A `.csd` containing a model request should therefore not by itself be assumed to define a reproducible rendering.

For a composition whose result must remain fixed, model-generated data should be saved after generation and incorporated into the final composition as ordinary Csound data, score text, or source code.

This permits models to be used as part of the compositional process without requiring access to the model when the finished composition is subsequently rendered.

In `.csd`s with multiple prompts, caching responses, and then re-running with the cached responses, is normally advisable in order to remove unpredictable and sometimes long delays of responses.

### Security

Authentication credentials are obtained from the execution environment and are not supplied as ordinary opcode arguments.

When `modelprompt` returns an `InstrDef`, externally generated source is compiled and executed by Csound. Such output should be regarded as generated executable code.

Applications embedding Csound should also take account of the fact that prompts may contain musical, textual, or other data that are transmitted to the selected external provider.

---

# modelprompt

Submits a prompt synchronously to an external generative model and returns the response as a Csound value.

## Syntax

### String result

```csound
result:S = modelprompt(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

result:S = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

result:S = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

### Numeric result

```csound
result:i = modelprompt(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

result:i = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

result:i = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

### Numeric array

```csound
result:i[] = modelprompt(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

result:i[] = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

result:i[] = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

### String array

```csound
result:S[] = modelprompt(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

result:S[] = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

result:S[] = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

### Instrument definition

```csound
result:InstrDef = modelprompt(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

result:InstrDef = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

result:InstrDef = modelprompt(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

## Description

`modelprompt` sends `prompt` to an external generative model selected by `provider` and `model`.

The request is synchronous. Initialization of the calling instrument does not continue until the model request completes or fails.

Each result type has signatures with an optional JSON `options` string, an optional `iregenerate` flag, and an optional `iversion` pin used when freezing.

Every call is assigned the next sequential prompt number for the Csound instance and always participates in local response caching. Omitting `iregenerate` reuses the latest cache when present; pass `1` to force a new version, or `0` / `0, N` to freeze.

The opcode uses its output type to determine the form of response requested from the model and how that response is converted to a Csound value.

## Initialization

### provider

A string identifying the model provider.

Examples include:

```csound
"openai"
"anthropic"
```

The set of available providers depends on the installed version of the plugin.

### model

A string containing the provider-specific model identifier.

For example:

```csound
Smodel = "MODEL_NAME"
```

Model identifiers are not predefined by Csound because available models and model names are controlled by external providers.

### prompt

The natural-language request sent to the model.

The prompt may be an ordinary Csound string:

```csound
Sprompt = "Return the first 16 prime numbers."
```

or a multiline string:

```csound
Sprompt = {{
Generate 32 pitches.

Begin with a C major collection.
Gradually increase chromaticism.
Keep all MIDI pitches between 48 and 84.
}}
```

For array and numeric results, the plugin asks the model for JSON and parses it according to the Csound output type. It is therefore usually unnecessary for the prompt to specify serialization syntax beyond the musical content of the request.

For example:

```csound
values:i[] = modelprompt(
    "openai",
    "MODEL_NAME",
    "Return the first 16 prime numbers."
)
```

requests an array of numbers because `values` has type `i[]`.

### iregenerate

Optional flag controlling whether this prompt calls the model or reuses a cached response.

- Omitted (default): **auto** — reuse the latest cached version for this prompt number if one exists; otherwise call the model and store a new version.
- Non-zero: **force** — always call the model and write the next version.
- Zero: **freeze** — never call the model; reuse a cached version (see `iversion`). Missing cache fails initialization.

Prompt numbers are assigned automatically in call order within the Csound instance (1, 2, 3, …), including `modelprompt`, `modelprompt_orc`, `modelprompt_async`, and `modelprompt_orc_async`. Cached files are stored beside the `.csd` under a directory named from its basename:

```text
{csd_directory}/{csd_basename}/modelprompt_cache/{prompt_index}.{version}
```

where:

- `{csd_directory}` is the directory containing the `.csd`;
- `{csd_basename}` is the `.csd` file name without extension;
- `modelprompt_cache` is a fixed subdirectory name;
- `{prompt_index}` is the sequential prompt number; and
- `{version}` is an automatically incremented serial version number for that prompt.

The `.csd` path is taken from `MODELPROMPT_CSD` if set, otherwise from a `.csd` argument on the host process command line. If none is available, the plugin uses `modelprompt_string/modelprompt_cache` under the current working directory.

Earlier versions remain on disk. After a successful run, simply omit `iregenerate` (or leave it unset) to reuse the cache. Use `iregenerate=1` only when you want a fresh model response.

### iversion

Optional version pin consulted only when `iregenerate` is 0.

- Omitted or 0: reuse the latest cached version for this prompt number.
- `N > 0`: reuse exactly `{prompt_index}.{N}`.

If the requested version does not exist, initialization fails. When regenerating, `iversion` is ignored.

`iregenerate` and `iversion` precede the optional JSON `options` string.

### options

Optional string containing additional request options represented as a JSON object.

For example:

```csound
Soptions = {{
{
    "temperature": 0.2
}
}}
```

Supported options depend upon the provider. Portable `.csd` files should avoid unnecessary dependence on provider-specific options.

## Output Types

### String

```csound
Sresult = modelprompt(...)
```

returns the model's textual response.

No interpretation of the contents is performed. This form can be used for prose, symbolic notation, Csound source code, Csound score text, or other textual representations.

### Number

```csound
ivalue:i = modelprompt(...)
```

requires a single numerical value.

If the model response cannot be converted to the requested numeric representation, initialization fails.

### Numeric array

```csound
values:i[] = modelprompt(...)
```

requires an array of numerical values.

For example:

```csound
ratios:i[] = modelprompt(
    "openai",
    "MODEL_NAME",
    {{
    Generate 16 frequency ratios for an inharmonic spectrum.
    The first ratio is 1.0. Return the ratios in increasing
    order.
    }}
)
```

### String array

```csound
names:S[] = modelprompt(...)
```

requires an array of strings.

For example:

```csound
names:S[] = modelprompt(
    "anthropic",
    "MODEL_NAME",
    "Give ten names for progressively denser musical textures."
)
```

### InstrDef

When the output is an `InstrDef`:

```csound
NewInstrument:InstrDef = modelprompt(...)
```

the returned model text is treated as Csound instrument source and compiled as an instrument definition.

For example:

```csound
Resonator:InstrDef = modelprompt(
    "anthropic",
    "MODEL_NAME",
    {{
    Write the body of a Csound instrument.

    p4 is amplitude.
    p5 is fundamental frequency.

    Excite 16 parallel resonators with a short noise burst.
    Make the resonance frequencies progressively more
    inharmonic above the fundamental.

    Return only valid Csound instrument-body code.
    }}
)

schedule(Resonator, 0, 8, 0.2, 110)
```

If the returned Csound source cannot be compiled, the opcode fails at initialization.

## Performance

`modelprompt` has no performance-time operation.

The external request is made during initialization and the returned data become ordinary Csound values after initialization has completed.

For a model request that must execute without suspending Csound performance, use `modelprompt_async` and `modelprompt_result`.

## Errors

`modelprompt` reports an initialization error if, for example:

- the requested provider is unavailable;
- the required API key is unavailable;
- authentication fails;
- the specified model cannot be used;
- communication with the provider fails;
- the provider does not return a result;
- array or numeric output cannot be converted to the requested Csound type; or
- generated `InstrDef` source does not compile; or
- generated orchestra source for `modelprompt_orc` does not compile.

A provider error message should be included in Csound's diagnostic output when available.

## Example: Generate synthesis parameters

```csound
<CsoundSynthesizer>
<CsOptions>
-odac
</CsOptions>

<CsInstruments>

sr = 48000
ksmps = 64
nchnls = 2
0dbfs = 1

instr Main

    freqs:i[] = modelprompt(
        "openai",
        "MODEL_NAME",
        {{
        Generate 12 frequencies between 200 and 3000 Hz.
        They should form a progressively more inharmonic
        spectrum derived from a 200 Hz fundamental.
        }}
    )

    prints("Model returned %d frequencies.\n", lenarray(freqs))

endin

schedule(Main, 0, 1)

</CsInstruments>
</CsoundSynthesizer>
```

## Example: Generate score text

```csound
instr GenerateScore

    Sscore = modelprompt(
        "openai",
        "MODEL_NAME",
        {{
        Write exactly 16 Csound i-statements.

        Use instrument 2.
        p2 is onset time.
        p3 is duration.
        p4 is MIDI pitch.
        p5 is amplitude from 0 to 1.

        Keep the complete passage within 20 seconds.
        Return only the score statements.
        }}
    )

    scorelinei(Sscore)

endin
```

## See Also

`modelprompt_orc`, `modelprompt_async`, `modelprompt_result`, `create`, `schedule`, `scorelinei`

---

# modelprompt_orc

Submits a prompt synchronously, compiles the returned Csound orchestra fragment into the live orchestra, and returns the orchestra source text.

`alwayson` instruments in the fragment are started as part of compilation. Score events are not generated by this opcode; use a separate `modelprompt` (or hand-written score) with `scorelinei` / `schedule`.

## Syntax

```csound
Sorc = modelprompt_orc(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

Sorc = modelprompt_orc(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

Sorc = modelprompt_orc(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

Provider, model, prompt, cache, and options arguments behave as for `modelprompt`. Prompt numbering is shared with `modelprompt`, `modelprompt_async`, and `modelprompt_orc_async`.

## Example

See `examples/anthropic_sonnet_orc_score.csd`.

## See Also

`modelprompt_orc_async`, `modelprompt_orc_result`, `modelprompt`, `modelprompt_async`, `scorelinei`, `connect`, `alwayson`

---

# modelprompt_orc_async

Starts an asynchronous orchestra-generation request and returns a request handle. Network I/O runs on a worker thread. When the text is ready, poll with `modelprompt_orc_result`, which compiles and activates the fragment on the Csound thread (once).

Use this when orchestra generation must not block initialization or performance. Score events remain separate (`scoreline` / `scorelinei` / `schedule`).

## Syntax

```csound
ihandle = modelprompt_orc_async(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

ihandle = modelprompt_orc_async(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

ihandle = modelprompt_orc_async(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

Arguments and caching behave as for `modelprompt_async`. The request uses the orchestra system instruction (valid `instr` / `connect` / `alwayson` code only).

## Output

### ihandle

A positive integer identifying the asynchronous orchestra request. Poll it with `modelprompt_orc_result` (not `modelprompt_result`, which returns text without compiling).

## Example

See `examples/anthropic_sonnet_orc_async.csd`.

## See Also

`modelprompt_orc_result`, `modelprompt_orc`, `modelprompt_async`, `modelprompt_result`

---

# modelprompt_orc_result

Polls a `modelprompt_orc_async` request. On first successful completion it compiles the returned orchestra fragment into the live orchestra (`csoundCompileOrc`), activating any `alwayson` instruments, then returns status and source text.

## Syntax

```csound
kstatus, Sorc modelprompt_orc_result ihandle
```

## Performance

### kstatus

```text
 0   request is pending
 1   orchestra text received and compiled successfully
-1   request or compile failed
-2   invalid or unknown request handle
```

Compilation is attempted only once per handle. A compile failure sets status to `-1` and leaves the diagnostic in `Sorc`.

### Sorc

While pending, an empty string. On success, the compiled orchestra source. On failure, an error description.

## Example

```csound
gih init 0

instr Start
    gih = modelprompt_orc_async("anthropic", "MODEL_NAME", {{
    Write Tone, Reverb, Master, connect, and alwayson Reverb/Master.
    }})
endin

instr Poll
    kstatus, Sorc modelprompt_orc_result gih
    if kstatus == 1 then
        printf("%s\n", 1, Sorc)
        scoreline({{ i "Tone" 0 1 0.2 440 }}, 1)
        turnoff
    elseif kstatus < 0 then
        printf("failed: %s\n", 1, Sorc)
        turnoff
    endif
endin
```

## See Also

`modelprompt_orc_async`, `modelprompt_orc`, `modelprompt_result`

---

# modelprompt_async

Starts an asynchronous request to an external generative model and returns a request handle. Note that such prompts can be issued at any time during the Csound performance, by scheduling a new instrument instance that issues the prompt and then polls the model for the response.

## Syntax

```csound
ihandle = modelprompt_async(
    provider:S,
    model:S,
    prompt:S
    [, options:S]
)

ihandle = modelprompt_async(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i
    [, options:S]
)

ihandle = modelprompt_async(
    provider:S,
    model:S,
    prompt:S,
    iregenerate:i,
    iversion:i
    [, options:S]
)
```

## Description

`modelprompt_async` submits `prompt` to the external model selected by `provider` and `model`, but does not wait for the model to return a result.

The opcode creates a request owned by the plugin, starts the network operation in a worker thread, and immediately returns an integer request handle.

The handle is subsequently passed to `modelprompt_result`:

```csound
kstatus, Sresult modelprompt_result ihandle
```

This allows a model request to proceed while Csound continues audio and control-rate performance.

`modelprompt_async` returns model output as text. It does not infer a typed result from a Csound output variable because its only output is the request handle. When a Csound array, number, or `InstrDef` is required directly, use the synchronous `modelprompt` opcode.

## Initialization

### provider

A string identifying the model provider.

Examples include:

```csound
"openai"
"anthropic"
```

The set of providers depends on the plugin build.

### model

A provider-specific model identifier.

For example:

```csound
Smodel = "MODEL_NAME"
```

### prompt

The text submitted to the model.

For example:

```csound
Sprompt = {{
Generate eight Csound score i-statements for instrument 3.
Return only the score text.
}}
```

### iregenerate

Optional flag; same meaning as for `modelprompt` (omitted = auto cache, `1` = force, `0` = freeze).

Prompt numbering and cache paths are the same as for `modelprompt`:
`{csd_directory}/{csd_basename}/modelprompt_cache/{prompt_index}.{version}`.

### iversion

Optional version pin consulted only when `iregenerate` is 0.

- Omitted or 0: latest cached version for this prompt number.
- `N > 0`: exactly `{prompt_index}.{N}`.

Ignored when regenerating. Missing versions fail the request.

### options

Optional provider request options represented as a JSON object.

For example:

```csound
Soptions = {{
{
    "temperature": 0.3
}
}}
```

The supported options depend on the provider.

When regenerate/version overloads are used, `iregenerate` and `iversion` are opcode arguments and are not placed inside this JSON object.

## Output

### ihandle

A positive integer identifying the asynchronous request.

The handle is valid within the current Csound instance and may be passed to `modelprompt_result`.

A handle does not represent the textual result itself. The request may still be pending when `modelprompt_async` returns.

If the request cannot be created, the opcode reports an initialization error.

## Performance

The network request and waiting for the model response take place outside Csound's audio-performance thread.

`modelprompt_async` itself is an initialization-time opcode. It starts the request during initialization of the calling instrument, after which the request continues independently.

The returned handle can be polled by `modelprompt_result` at control rate.

The plugin must synchronize access to request state without holding the audio-performance thread while waiting for network I/O.

## Request lifetime

A request remains associated with its handle after it has completed so that `modelprompt_result` may continue to return its final status and result.

Request state is released when the Csound instance is destroyed. An implementation may additionally provide internal reclamation of completed requests, provided that a handle remains valid for the documented lifetime required by `modelprompt_result`.

## Errors

Errors occurring before a request can be started are reported as initialization errors.

Errors occurring after the worker thread has started are reported by `modelprompt_result` with a negative status. When possible, `Sresult` contains a diagnostic message returned by the provider or generated by the plugin.

## Example

```csound
instr AskModel

    gihandle = modelprompt_async(
        "openai",
        "MODEL_NAME",
        {{
        Generate a short description of a musical process
        that begins as a canon and gradually becomes a hocket.
        }}
    )

endin
```

The request starts when `AskModel` initializes. Another instrument may poll the handle:

```csound
instr ReceiveModelResult

    kstatus, Sresult modelprompt_result gihandle

    if kstatus == 1 then
        printf("%s\n", 1, Sresult)
        turnoff
    elseif kstatus < 0 then
        printf("Model request failed: %s\n", 1, Sresult)
        turnoff
    endif

endin
```

## See Also

`modelprompt`, `modelprompt_result`

---

# modelprompt_result

Polls an asynchronous model request and returns its status and textual result.

## Syntax

```csound
kstatus, Sresult modelprompt_result ihandle
```

## Description

`modelprompt_result` examines the asynchronous request identified by `ihandle`.

The opcode does not wait for the request to finish. If the request is still running, it immediately returns a pending status.

When the request has completed, `Sresult` contains the textual response returned by the model.

If the request has failed, `Sresult` contains an error description when one is available.

The opcode is intended to be called repeatedly at control rate while a request is pending.

## Initialization

### ihandle

A request handle previously returned by `modelprompt_async`.

For example:

```csound
ihandle = modelprompt_async(
    "openai",
    "MODEL_NAME",
    "Generate four Csound score events."
)
```

## Performance

### kstatus

The current state of the request.

The following values are defined:

```text
 0   request is pending
 1   request completed successfully
-1   request failed
-2   invalid or unknown request handle
```

The status remains `1` after successful completion, and the completed result remains available on subsequent calls for as long as the request handle remains valid.

If the request fails, the status remains negative and the diagnostic text remains available on subsequent calls.

### Sresult

The current textual result.

While `kstatus` is `0`, `Sresult` is an empty string.

When `kstatus` becomes `1`, `Sresult` contains the complete textual model response.

When `kstatus` is negative, `Sresult` contains an error description when available.

`modelprompt_result` does not parse or compile the returned text. The orchestra determines how the result is used.

## Example: Poll a request

```csound
gihandle init 0

instr StartRequest

    gihandle = modelprompt_async(
        "anthropic",
        "MODEL_NAME",
        {{
        Describe a sequence of eight increasingly
        inharmonic frequency ratios.
        }}
    )

endin

instr PollRequest

    kstatus, Sresult modelprompt_result gihandle

    if kstatus == 1 then
        printf("Model result:\n%s\n", 1, Sresult)
        turnoff

    elseif kstatus < 0 then
        printf("Model request failed:\n%s\n", 1, Sresult)
        turnoff
    endif

endin
```

## Example: Request score text asynchronously

```csound
gihandle init 0

instr GenerateScore

    gihandle = modelprompt_async(
        "openai",
        "MODEL_NAME",
        {{
        Generate 12 valid Csound i-statements for instrument 2.
        p4 is MIDI key number.
        p5 is amplitude from 0 to 1.
        Return only Csound score text.
        }}
    )

endin

instr ReceiveScore

    kstatus, Sscore modelprompt_result gihandle

    if kstatus == 1 then
        ; Sscore now contains the generated score text.
        ; It may be submitted to Csound or otherwise processed here.
        printf("Generated score:\n%s\n", 1, Sscore)
        turnoff

    elseif kstatus < 0 then
        printf("Model request failed:\n%s\n", 1, Sscore)
        turnoff
    endif

endin
```

## Notes

`modelprompt_result` is non-blocking. Its purpose is to separate potentially slow network/model execution from Csound's real-time performance.

A model request may take substantially longer than a Csound control period. Orchestras should therefore treat completion as an event rather than assume a particular response time.

Applications should also allow for the possibility that a request never succeeds because of network failure, authentication failure, provider limits, or another external error.

## See Also

`modelprompt_async`, `modelprompt`

---

## Credits

Opcode concept and design: Michael Gogins.

Initial documentation developed with OpenAI ChatGPT.
