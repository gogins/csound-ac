# Computer Music Playpen

Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

This computer music playpen is a simple integrated development environment 
__specifically designed to support my particular style of algorithmic music 
composition__.

I write music by writing code. I almost always use Csound to render my pieces 
as fixed media, that is, soundfiles. I may use C++, Python, Common Lisp, or 
JavaScript in addition to writing Csound code. I sometimes write interactive 
visual music. I generally find that the Csound instruments that I use need 
some pretty fine-tuned tweaking while the music is playing. What I need is to 
minimize the time spent in the "edit code, render piece, listen to piece; edit 
code again, render piece again, listen to piece again;..." cycle without, 
however, in any way limiting the power of Csound, the power of Python, or the 
power of HTML5.

In addition, long experience has taught me that keeping a finished piece as 
one file of source code, with a minimum of external dependencies, and those 
dependencies as standardized and widely used as possible (such as Csound, 
Python, and HTML5), is essential to ensure that pieces are maintainable and 
will play into the indefinite future. After all, I have been making computer 
music now for 38 years. Of course, this means that I prefer to use a finished 
piece as a template for a new piece, rather than to create and use a library, 
because the template approach ensures the self-contained nature of the pieces.

Experience has also taught me that all-in-one systems such as CsoundQt never 
actually do everything that I need, and may have bugs that I am unwilling or 
unable to fix. 

After exhaustively trying many alternatives I have ended up with the system 
that I maintain in this repository and, more specifically, in this directory.

The basic principle is that each piece is always one single self-contained 
file, preferably a Csound .csd file. Other code used in the piece, such as 
C++, HTML/JavaScript, Python, or whatever, is if possible embedded in the .csd 
file as multi-line strings, or as the contents of the score using the `bin` 
attribute, or even as binary files such as soundfiles, SoundFonts, etc., 
encoded to base64 in <CsoundFileB> elements.

1.  I have added many Python functions for rendering Csound pieces, compiling 
    and running C++ code, and so on.
2.  These commands are implemented in `playpen.py` and can be invoked from any 
    code editor that provides custom commands.
4.  The `vscode-playpen` directory defines such custom commands for the Visual 
    Studio Code text editor.
5.  The `.SciTEUser.properties` file defines such custom commands for the SciTE 
    text editor.
6.  The `add2csd.py` script will embed any number of files into a .csd 
    file as <CsoundFileB> elements.

Then, if possible, these embedded languages are compiled and executed under 
the direction of Csound, during the Csound performance, by plugin opcodes.

To this end, I have created the csound-cxx-opcodes that allow C++ source 
code to be embedded in the Csound orchestra, and to be compiled, linked, 
and loaded during the Csound performance, and the csound-webserver-opcodes 
that allow HTML pages to be embedded in the Csound orchestra, and to be 
opened in a Web browser during the Csound performance, with a JavaScript 
interface the running instance of Csound.

The other option is to embed Csound via its API in a C++, Python, or HTML file.

If you compose by writing code, you may well find this system useful. If you 
do not compose by writing code, you will probably _not_ find this system useful.

## Getting Started

Support for the playpen is provided by the playpen.py file which contains 
routines for rendering and post-processing .csd files, .cpp files, and 
.html files. This file is used by external tools for the gedit text editor, 
so that the gedit menu will have convenient commands for working with 
algorithmic composition.

1.  Make a symbolic link in your home directory to csound-extended/playpen/
    playpen.py.
2.  Copy csound-extended/playpen/playpen.ini to your home directory and 
    customize it for your system and your personal metadata.
3.  Install the `playpen.vsix` extension in Visual Studio Code.
    
Install the following software requirements, each according to its own 
instructions. I give some some specific instructions for Ubuntu Linux. Many of 
these dependencies are however available on Windows or macOS.

Core requirements for composing with Csound

1.  [Csound](https://csound.com/download.html). For Linux, it is best to build 
    from source code according to [these  
    instructions](https://github.com/csound/csound/blob/develop/BUILD.md).
2.  The [Visual Studio Code](https://code.visualstudio.com/) text editor.
3.  The "computer music playpen", a Visual Studio Code extension with several 
    helpful commands and a context menu for developing and performing computer 
    music using Csound, CsoundAC, and HTML5.
4.  [sox](http://sox.sourceforge.net/). For Ubuntu, execute 
    `sudo apt install sox` and `sudo apt install libsox-fmt-all`.
5.  [FFmpeg](https://ffmpeg.org/) for converting soundfiles to mp4 video format to be uploaded to YouTube.
5.  [astyle](http://astyle.sourceforge.net/) for reformatting C/C++ style source code to a consistent, readable format.
5.  The [libsndfile](http://www.mega-nerd.com/libsndfile/) utilities. For Ubuntu, all 
    you need is to execute `sudo apt install sndfile-programs`. For other 
    operating systems, you may need to install libsndfile itself, which should 
    come with these utilities.
6.  [Audacity](https://www.audacityteam.org/). Please note, for Ubuntu, 
    install the regular Unbutu Packages Search result for Audacity, currently 
    `sudo apt install audacity`.
    
Requirements for composing with Python. Pleae note that everything should be 
installed for the __same version__ of Python. These instructions are for a global 
installation, but you can also install in your home directory or in a virtual 
environment.

7.  [Python 3.9](https://www.python.org/downloads/) or higher. For Ubuntu Linux, 
    follow [these instructions](https://linuxize.com/post/how-to-install-python-3-9-on-ubuntu-20-04/).
8.  Make sure that the `csound/interfaces/ctcsound.py` file 
    is the one that comes with your version of Csound and that it can be found 
    in your Python site-packages directory, e.g. 
    `/usr/local/lib/python3.9/dist-packages/ctcsound.py`. To use ctcsound you 
    also need numpy, which can be installed with 
    `sudo python3.8 -m pip install numpy`.

Requirements for composing with CsoundAC (usable from C++, Python, or 
JavaScript). These requirements also apply to composing with C++:

9.  CsoundAC is part of this [csound-ac](https://github.com/gogins/csound-extended)
    repository. CsoundAC has advanced features for algorithmic composition, 
    including support for tonal harmony and counterpoint. Install if you can 
    or build if you must, according to instructions 
    [here](https://github.com/gogins/csound-extended).
    
Requirements for composing with HTML5:

10. Install [csound-webserver-opcodes](https://github.com/gogins/csound-webserver-opcodes).
    
Requirements for composing in C++:

13. [csound-cxx-opcodes](https://github.com/gogins/csound-cxx-opcodes).

Additional useful things:

15. [csound-examples](https://github.com/gogins/csound-vst3-opcodes) is my 
    extensive compilation of Csound examples and pieces.
16. [michael.gogins.studio](https://github.com/gogins/michael.gogins.studio) 
    contains the source code for my own compositions, along with Csound 
    instruments that could be used in your own work.
17. The vst4cs opcodes for using VST2 plugins in Csound are available, for 
    Ubuntu linux, [here](https://href.li/?https://drive.google.com/file/d/1mYHyjoD7RUrPpST3ISa9CsxIg5wTspXc/view?usp=sharing).
18. [csound-vst3-opcodes](https://github.com/gogins/csound-vst3-opcodes) 
    enables Csound to use VST3 plugins.
19. [csound-extended-nudruz](https://github.com/gogins/csound-extended-nudruz) 
    provides Csound suppport for composing in 
    Common Lisp using Drew Krause's extensions to Common Music.
20. [musx](https://github.com/musx-admin/musx) is Rick Taube's translation of 
    his original Common Lisp system for algorithmic composition to Python, now 
    adapted to work seamlessly with Csound.
21. [MuseScore](https://musescore.org/en) is a open source, cross-platform 
    score editor.
22. [lilypond](http://lilypond.org/) is an open source, cross-platform, very 
    powerful music typesetting system.

## Building

If you modify this extension, make sure that functions defined in `extension.js` are 
registered in `package.json`.

To rebuild the extension: 

```
cd vscode-playpen
pnpm install -g @vscode/vsce
vsce package
```

To install the rebuilt extension:

1. In Visual Studio Code, open the `vscode-playpen` folder.
2. In the Visual Studio Code Explorer, select the most recent .vsix file.
3. Right-click and select the _Install Extension VSIX_ command.




