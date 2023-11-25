# csound-ac
![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-ac/total.svg)<br>
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Introduction

This repository contains:

1.  CsoundAC, an algorithmic composition library, designed to be used with 
    Csound. CsoundAC is written in C++, and has both C++ and Python 
    interfaces. CsoundAC implements _music models_, which are kind of like 
    scene graphs for pieces. CsoundAC has sophisticated facilities for 
    working with tonal and non-tonal chords, progressions, and scales, and 
    for implementing classical-style voice-leading in generated scores.
    
2.  My computer music playpen, designed to facilitate algorithmic composition 
    with Csound and CsoundAC by extending standard text editors. The playpen 
    makes it possible to run various kinds of Csound pieces, and even to build 
    C++ pieces and plugin opcodes, from the editor. For more information, see 
    `playpen/README.md`.
    
3.  My Visual Studio Code extension that implements the computer music 
    playpen. Consider working in this environment. For more information, see 
    `vscode-playpen/README.md`
    
4.  silencio, a JavaScript library for algorithmic composition similar to 
    CsoundAC. However, using the WebAssembly build of CsoundAC in 
    [csound-wasm](https://github.com/gogins/csound-wasm) is now recommended in 
    place of silencio.
    
5.  patches, a library of Csound instrument definitions, developed over many 
    years and used in many of my pieces.

Currently, CsoundAC is supported on macOS and Linux.

Please log any bug reports or requests for enhancements at 
https://github.com/gogins/csound-ac/issues.

## Changes

See https://github.com/gogins/csound-ac/commits/develop for the commit
log.

## Using

CsoundAC can be used both as a C++ library, as a Python extension module, 
and as a WebAssembly kodue. Python is easier to use, but C++ offers 
considerably more power and speed. The WebAssembly build of CsoundAC in 
[csound-wasm](https://github.com/gogins/csound-wasm) has the same power 
as the C++ library and somewhat less speed.

Examples (some of which can also serve as tests) for the various aspects of 
csound-ac are maintained in my separate 
[csound-examples](https://github.com/gogins/csound-examples) 
repository. Some of the examples there will run in WebBrowsers using 
WebAssembly, and these can be viewed at 
https://gogins.github.io/csound-examples.

## Installation

1.  You must first install the following pre-requisites on your system:

    1.1  [Libsndfile](http://libsndfile.github.io/libsndfile/) for reading and 
         writing most any format of soundfile.

    1.2  [Csound](https://github.com/csound/csound) for sound synthesis.
    
    1.3  The [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) 
         header-file-only library for linear algebra.
         
    1.4  The [Boost C++ Libraries](https://www.boost.org/). Only the header 
         files are used.
    
    1.5  The [OpenCV](https://opencv.org/) library for image processing.
    
    1.6  The [Python](https://www.python.org/) programming language, version 
         3.9 or higher.
    
2.  There are prebuilt binary releases for this package available at 
    https://github.com/gogins/csound-ac/releases. These can be downloaded,
    unzipped to `/usr/local` (or even `~/usr/local`), and used from there. 
    The binary files are archives, not installers. They should be installed in 
    the same way on all platforms:

    1.  Download the archive from the releases page.

    2.  Use the 7z program to unzip the archive to the output directory, e.g.
        `7z x -o/usr/local csound-ac-0.5.0-Darwin.zip`. 7z will ask you what 
        to do about any files that it might overwrite.

    3.  Run `sudo ldconfig` or take equivalent steps to ensure that the 
        libraries can be found by the operating system. You may need to add 
        appropriate directories to your compiler's header files path, and 
        to the operating system `PATH` environment variable.

    4.  To uninstall, you can list the contents of the archive to a file, 
        e.g. `unzip unzip csound-ac-0.5.0-Darwin.zip -l > listing.txt`. You 
        can use this to identify files to remove, and you could even write 
        a script to parse `listing.txt` and remove all files listed therein.

    Please note, on macOS the CsoundAC libraries are installed by default to 
    `/usr/local/lib`. They can be installed elsewhere, but if so, you will 
    probably need to set up install names and rpaths using `otool`.

## Helpers

There are files and directories in the Git repository and in the packages that 
can be used as helpers for csound-ac. You can create symbolic links from 
these files to your home directory or other places.

- `build-env.sh`: Source this to set useful environment variables for the 
  build and runtime environment on Linux. You may need to copy and modify this 
  script.

- Creata a symbolic link from `csound-ac/playpen/playpen.py` to your home 
  directory, to enable use of the computer music plapen.

- Copy `csound-ac/playpen/playpen.ini` to your home directory and customize 
  it for your environment, to configure the computer music playen.

- If you use Visual Studio Code, install in it the `playpen.vsix` extension, 
  which makes the computer music playpen part of Visual Studio Code.
  
- If you use the SciTE text editor, Create a symbolic link from 
  `csound-ac/playpen/.SciTEUser.properties` to your home directory, which 
  makes the computer music playpen part of SciTE.

- `silencio`: Create a symbolic link to this directory in every directory in 
  which you are writing or running a piece that uses the Silencio library.
  
- `patches`: Include the full path of this directory in your Csound 
  environment variable `INCDIR`.

## Building On Your Local Computer

The following instructions are for macOS. Linux is similar. For 
more information, look at `./github/cmake.yaml`. However, on Linux it may be 
better to build Csound for source code.

1.  Clone this Git repository.

2.  Install prerequisites as follows from the repository root directory:
```
    brew update
    brew upgrade
    brew install graphviz
    brew install doxygen
    brew install opencv
    brew install csound
    brew install bwfmetaedit
    brew install sox
    brew install ffmpeg
    brew install lame
    brew install sndfile
    brew install imagemagick
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

## Release Notes

### [v7.0](https://github.com/gogins/csound-ac/releases/tag/v7.0-darwin)

- There is a new version of the Computer Music Playpen, in the form of a 
  Visual Studio Code extension. This is now the recommnded environment for 
  using CsoundAC.
  
- CsoundAC no longer implements the Soundfile class.

- CsoundAC no longer maintains the Lua binding.

- CsoundAC has internalized support for the CppSound class, because the brew 
  package for Csound on macOS includes the the `libcsnd6` dylib without the 
  corresponding header files. This may create a breaking change in the API for 
  some users, but makes it possible to keep maintaining the continuous 
  integration builds and releases of CsoundAC on GitHub.



   