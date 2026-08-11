# modelprompt

Csound 7 opcode plugin that submits prompts to external generative models
(OpenAI, Anthropic) and returns text, numbers, arrays, or compiled
`InstrDef` values.

Design and opcode reference: [MODELPROMPT.md](MODELPROMPT.md).

## Installation

Built with csound-ac when `BUILD_MODELPROMPT` is ON (default; requires libcurl).

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

### Making Csound load the plugin

Csound must find the plugin directory. On macOS, user plugins are typically:

```bash
mkdir -p ~/Library/csound/7.0/plugins64
ln -sf /opt/homebrew/lib/csound/plugins64-7.0/modelprompt.dylib \
  ~/Library/csound/7.0/plugins64/modelprompt.dylib
```

(Adjust the source path if you installed under `/usr/local` or another prefix.)

On Linux, ensure `OPCODE7DIR64` includes `<prefix>/lib/csound/plugins64-7.0`,
or copy/symlink the plugin into a directory already listed there.

### API keys

Do not put keys in `.csd` files. Export before running Csound:

```bash
export ANTHROPIC_API_KEY=...
# or
export OPENAI_API_KEY=...
```

## Example

```bash
export ANTHROPIC_API_KEY=...
csound share/doc/csound-ac/modelprompt/examples/anthropic_sonnet_instr_score.csd
```

Or from a source tree:

```bash
csound modelprompt/examples/anthropic_sonnet_instr_score.csd
```

The example asks Anthropic Claude Sonnet to compile an instrument and generate
a short score. Network access and a valid API key are required.
