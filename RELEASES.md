# DEVELOPMENT AND RELEASE PROCESS
Michael Gogins

`csound-ac` is the core library for my work in algorithmic composition with 
Csound. It is written in C++, and has both C++ and Python interfaces. It 
implements my work in _music models_, which are kind of like scene graphs for 
musical compositions, and my work in chord spaces based on mathematical music 
theory, as well as a number of other facilities for algorithmic composition.
The library is developed and tested on macOS, and built and released on macOS, 
Linux, and Windows using GitHub Actions.

Many of my other projects depend on `csound-ac`. These include:

- `csound-wasm`, a WebAssembly build of both Csound `csound-ac` that can be 
   used in Web browsers.
- `csound-node`, a Node.js build of Csound that can be used either in Node.js 
   servers or in NW.js applications.
   and in local browser apps.
- `csound-vst3`, a VST3 plugin version of Csound enabling Csound code to be 
   edited and run in digital audio workstations.
- `csound-vst3-opcodes`, Csound opcodes for hosting VST3 plugins in Csound.
- `csound-cxx-opcodes`, Csound opcodes enbling C++ source code to be embedded 
   in Csound orchestra code to be compiled and linked at init time, and then 
   invoked during performance.
-  `cloud-5`, a Web-based system for composing and performing algorithmic 
   compositions, based on Csound, CsoundAC, and the Strudel live coding system.

To keep all of these projects in sync, observe the following workflow. 

# Development Triggers

All builds of any of these projects update any Git submodules or download 
dependencies of that project to their latest versions.

All pushes to the main branch of any of these projects trigger a build of the 
project on GitHub for macOS, Linux, and Windows.

All new releases, or new tags, of any of these projects trigger a build of 
the project on GitHub for macOS, Linux, and Windows, followed by a release of 
the build artifacts to GitHub Releases.

## Release Dependencies

When there is a new release of `csound-ac`, make a new releases of 
`csound-vst3`, `csound-vst3-opcodes`, `csound-cxx-opcodes`, and 
`csound-wasm`.

When there is a new release of `csound-wasm`, make a new release of 
`csound-node` and `cloud-5`.