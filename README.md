# csound-ac
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-ac/total.svg)<br>
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Introduction

This repository contains:

1.  CsoundAC, an algorithmic composition library designed to be used with 
    Csound. CsoundAC is written in C++ and has C++ and Python interfaces.
    
2.  My computer music playpen, designed to facilitate algorithmic composition 
    with Csound and CsoundAC by extending standard text editors.
    
3.  silencio, a JavaScript library for algorithmic composition similar to 
    CsoundAC.
    
4.  patches, a library of Csound instrument definitions.

Please log any bug reports or requests for enhancements at
https://github.com/gogins/csound-extended/issues.

## Examples

Examples (some of which can also serve as tests) for the various aspects of 
csound-acare maintained in my separate 
[csound-examples](https://github.com/gogins/csound-examples) 
repository. Some of the examples there will run in WebBrowsers using 
WebAssembly, and these can be viewed at 
https://gogins.github.io/csound-examples.

## Changes

See https://github.com/gogins/csound-ac/commits/develop for the commit
log.

## Installation

1.  You must first install Csound on your system, e.g. as instructed at 
    `https://github.com/csound/csound`.
    
2.  There are prebuilt binary releases for this package available at 
    https://github.com/gogins/csound-ac/releases. These can be downloaded,
    unzipped in your home directory, and used from there. You will need to add 
    the directory containing the CsoundAC shared library to your binary search 
    path.

You may also install locally by first building from sources, as described
below. You may then install the software by running `sudo make install` in
the build-linux directory. However, be warned that this installs the
software in /usr/local. 

There are files and directories in the Git repository and in the packages that 
can be used as helpers for csound-extended. You can create symbolic links from 
these files to your home directory or other places.

- `build-env.sh`: Source this to set useful environment variables for the 
  build and runtime environment on Linux. You may need to copy and modify this 
  script.
  
- Create a symbolic link from `csound-ac/playpen/.SciTEUser.properties` to your 
  home directory, to create custom commands and editor features in the SciTE 
  text editor. This makes it possible to run various kinds of Csound pieces, 
  and even to build C++ pieces and plugin opcodes, from the editor. Believe 
  me, I tried all the other editors, and this is the one that is both simple 
  and useful. For more information, see `playpen/README.md`.

- `silencio`: Create a symbolic link to this directory in every directory in 
  which you are writing or running a piece that uses the Silencio library.
  
- `patches`: Include the full path of this directory in your Csound 
  environment variable `INCDIR`.

## Building On Your Local Computer

The following instructions are for macOS. Linux and Windows are similar. For 
more information, look at `./github/cmake.yaml`.

1.  Clone this Git repository.

2.  Install prerequisites as follows from the repository root directory:
```
    brew update
    brew install graphviz
    brew install doxygen
    brew install opencv
    brew install csound
    git clone "https://gitlab.com/libeigen/eigen.git"
```

3.  Execute `bash update-dependencies.sh`.
 
4.  Build like this:
```
    mkdir -p build-macos
    cd build-macos
    rm -f CMakeCache.txt
    cmake -Wno-dev .. -DCMAKE_PREFIX_PATH=/usr/local:/usr 
    make -j6 VERBOSE=1
    sudo make install
```
