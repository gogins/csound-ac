#!/usr/bin/python3
# -*- coding: utf-8 -*-
'''
P L A Y P E N   C O M M A N D S
Author: Michael Gogins

This Python 3 script provides help with using Csound and related programs, 
either from any text editor with configurable external tools, or from the 
command line. User configuration variables for this script are stored in the 
user's home directory in the "playpen.ini" file, which has a format similar to 
a Windows .ini file.

Usage:

python3 playpen.py csd-audio {source.csd}
    Renders the csd file to the system audio device specified in playpen.ini.
    
python3 playpen.py csd-soundfile {source.csd}
    Renders the csd file to a soundfile constructed using metadata from 
    playpen.ini, then post-processes the file to normalize it, save it in 
    various formats, etc., and finally opens the soundfile using the editor 
    specified in playpen.ini.
    
python3 playpen.py csound-patch {source.inc}
    Includes the inc file in a csd file, renders it to a soundfile, and 
    opens the soundfile using the editor specified in playpen.ini.
    
python3 playpen.py html-localhost {source.html}
    Uses Python to opens source.html from a local webserver.

python3 playpen html-nw {source.html}
    Creates a package.json file for the specified HTML file, and runs that 
    file as a NW.js application.
    
python3 playpen.py cpp-lib {source.cpp}
    Compiles and links source.cpp file as a shared library source.so or 
    source.dylib, with build options for Csound plugin opcodes.
    
python3 playpen.py cpp-app {source.cpp}
    Compiles and links source.cpp file as a program source or source.app, 
    with build options for linking with libCsound64 and libCsoundAC.
    
python3 playpen.py cpp-audio {source.cpp}
    Compiles and links source.cpp file as a Csound/CsoundAC composition, and 
    runs it to render audio.
    
python3 playpen.py cpp-soundfile {source.cpp}
    Compiles and links source.cpp file as a Csound/CsoundAC composition, 
    renders the piece to a source.wav tagged with metadata from 
    playpen.ini, then post-processes the file to normalize it, save in various 
    formats, etc., and finally opens the soundfile using the editor specified 
    in playpen.ini.
    
python3 playpen.py man-csound
    Opens the online Csound Reference Manual.

python3 playpen.py man-python
    Opens the online Python documentation.

python3 playpen.py man-csoundac
    Opens the CsoundAC AP Reference.

python3 playpen.py cpp-astyle {source}
    Uses the astyle program to reform the source file, using 
    options configured in playpen.ini.

For post-processing, the sndfile-metadata-set, sox, and ffmpeg programs need 
to be installed.

For HTML related commands, NW.js needs to be installed.

For build commands, a C++ toolchain needs to be installed.

For reformatting files, the astyle program needs to be installed.

For playing, the audacity program is a good choice because it can display 
soundfiles as sonograms, which can be very informative, and audacity can 
export to various formats.


'''
import configparser
try:
    import ctcsound
except:
    pass
import datetime
import os
import os.path
import platform
import random
import string
import subprocess
import sys
import threading
import time
import traceback

running_subprocess = None

# repeatable "random" segment so renderings can be compared.
random.seed(119480)

if len(sys.argv) < 2:
    print(__doc__)
    exit(0)
command = sys.argv[1]
if command == 'help':
    print(__doc__)
    exit(0)
print("command:                 {}".format(command))
cwd = os.getcwd()
print('cwd:                    ', cwd)
platform_system = platform.system()
if platform_system == "Darwin":
    open_command = "open"
else:
    open_command = "xdg-open"
source_filepath = ""
if len(sys.argv) > 2:
    source_filepath = sys.argv[2]
home_directory = os.environ["HOME"]
playpen_ini_filepath = os.path.join(home_directory, "playpen.ini")
settings = configparser.ConfigParser()
settings.read_file(open(playpen_ini_filepath))
metadata_author = settings.get("metadata", "author")
metadata_publisher = settings.get("metadata", "publisher")
metadata_year = settings.get("metadata", "year")
metadata_notes = settings.get("metadata", "notes")
metadata_license = settings.get("metadata", "license")
csound_audio_output = settings.get("csound", "audio-output")
print("csound_audio_output:     " + csound_audio_output)
port = settings.get("playpen", "port")
print("HTTP server port:        " + port)
soundfile_editor = settings.get("playpen", "soundfile-editor")
print("soundfile_editor:        " + soundfile_editor)
directory, basename = os.path.split(source_filepath)
rootname, extension = os.path.splitext(basename)
title = rootname.replace("-", " ").replace("_", " ")
label = '%s -- %s' % (metadata_author, rootname)
label = label.replace(" ", "_")
output_filename = label + ".wav"
master_filename = '%s.normalized.wav' % label
spectrogram_filename = '%s.png' % label
cd_quality_filename = '%s.cd.wav' % label
mp3_filename = '%s.mp3' % label
mp4_filename = '%s.mp4' % label
flac_filename = '%s.flac' % label
common_csound_options = "-RWd -m163 -W -+msg_color=0 --midi-key=4 --midi-velocity=5"
compiler_command = settings.get("cplusplus", "compiler-command")
nwjs_command = settings.get("nwjs", "nwjs-command")
print('Common Csound options:  ', common_csound_options)
print('Source file:            ', source_filepath)
print('Directory:               ', directory)
print('Basename:               ', basename)
print('Author:                 ', metadata_author)
print('Title:                  ', title)
print('Year:                   ', metadata_year)
str_copyright          = 'Copyright %s by %s' % (metadata_year, metadata_author)
print('Copyright:              ', str_copyright)
print('Licence:                ', metadata_license)
print('Publisher:              ', metadata_publisher)
print('Notes:                  ', metadata_notes)
print('Output filename:        ', output_filename)
print('Master filename:        ', master_filename)
print('Spectrogram filename:   ', spectrogram_filename)
print('CD quality filename:    ', cd_quality_filename)
print('MP3 filename:           ', mp3_filename)
print('MP4 filename:           ', mp4_filename)
print('FLAC filename:          ', flac_filename)
print('C++ compiler command:   ', compiler_command)
print('NW.js command:          ', nwjs_command)
bext_description       = metadata_notes
bext_originator        = metadata_author
bext_orig_ref          = basename

def csd_audio():
    try:
        print("\ncsd_audio: {} to {}...".format(source_filepath, csound_audio_output))
        csound_command = "csound {} -o{}".format(source_filepath, csound_audio_output)
        print("csound command: {}".format(csound_command))
        result = subprocess.run(csound_command, shell=True)
        print("csound command: {} {}".format(csound_command, result))
       
    except:
        traceback.print_exc()
    finally:
        print("csd_audio: {} to {}.".format(source_filepath, csound_audio_output))
        return
        
def csd_soundfile():
    try:
        print("\ncsd_soundfile: {} to {}...".format(source_filepath, output_filename))
        csound_command = "csound {} -RWo{} --simple-sorted-score={}.srt".format(source_filepath, output_filename, source_filepath)
        print("csound command: {}".format(csound_command))
        result = subprocess.run(csound_command, shell=True)
        print("csd_soundfile result: {}".format(result))
    except:
        traceback.print_exc()
    finally:
        print("soundfile: {} to {}.\n".format(source_filepath, output_filename))
        return
        
def inc_soundfile():
    try:
        print("\ninc_soundfile: {} to {}...".format(source_filepath, output_filename))
        csound_command = "csound {} -RWo{} --simple-sorted-score={}.srt".format(source_filepath, output_filename, source_filepath)
        print("csound command: {}".format(csound_command))
        result = subprocess.run(csound_command, shell=True)
        print("csd_soundfile result: {}".format(result))
    except:
        traceback.print_exc()
    finally:
        print("soundfile: {} to {}.\n".format(source_filepath, output_filename))
        return
        
def post_process():
    try:
        print("\npost_process: {}...".format(source_filepath))
        bext_description       = metadata_notes
        bext_originator        = metadata_author
        bext_orig_ref          = basename
        #bext_umid              = xxx
        #bext_orig_date         = xxx
        #bext_orig_time         = xxx
        #bext_coding_hist       = xxx
        #bext_time_ref          = xxx
        str_comment            = metadata_notes
        str_title              = title
        str_artist             = metadata_author
        str_date               = metadata_year
        str_license            = metadata_license
        sox_normalize_command = '''sox -S "%s" "%s" norm -6''' % (output_filename, master_filename + 'untagged.wav')
        print('sox_normalize command:  ', sox_normalize_command)
        os.system(sox_normalize_command)
        tag_wav_command = '''sndfile-metadata-set "%s" --bext-description "%s" --bext-originator "%s" --bext-orig-ref "%s" --str-comment "%s" --str-title "%s" --str-copyright "%s" --str-artist  "%s" --str-date "%s" --str-license "%s" "%s"''' % (master_filename + 'untagged.wav', bext_description, bext_originator, bext_orig_ref, str_comment, str_title, str_copyright, str_artist, str_date, str_license, master_filename)
        print('tag_wav_command:        ', tag_wav_command)
        os.system(tag_wav_command)
        sox_spectrogram_command = '''sox -S "%s" -n spectrogram -x 1200 -Y 1200 -o "%s" -t"'%s' by %s" -c"%s"''' % (master_filename, spectrogram_filename, title, metadata_author, str_copyright + ' (%s' % metadata_publisher)
        print('sox_spectrogram_command:', sox_spectrogram_command)
        os.system(sox_spectrogram_command)
        resize_spectrogram_command = '''magick convert %s -resize 1400x1400! %s-cdbaby.png''' % (spectrogram_filename, label)
        os.system(resize_spectrogram_command)
        sox_cd_command = '''sox -S "%s" -b 16 -r 44100 "%s"''' % (master_filename, cd_quality_filename + 'untagged.wav')
        print('sox_cd_command:         ', sox_cd_command)
        os.system(sox_cd_command)
        tag_wav_command = '''sndfile-metadata-set "%s" --bext-description "%s" --bext-originator "%s" --bext-orig-ref "%s" --str-comment "%s" --str-title "%s" --str-copyright "%s" --str-artist  "%s" --str-date "%s" --str-license "%s" "%s"''' % (cd_quality_filename + 'untagged.wav', bext_description, bext_originator, bext_orig_ref, str_comment, str_title, str_copyright, str_artist, str_date, str_license, cd_quality_filename)
        print('tag_wav_command:        ', tag_wav_command)
        os.system(tag_wav_command)
        mp3_command = '''lame --add-id3v2 --tt "%s" --ta "%s" --ty "%s" --tn "%s" --tg "%s"  "%s" "%s"''' % (title, "Michael Gogins", metadata_year, metadata_notes, "Electroacoustic", master_filename, mp3_filename)
        print('mp3_command:            ', mp3_command)
        os.system(mp3_command)
        sox_flac_command = '''sox -b 24 -S "%s" "%s"''' % (master_filename, flac_filename)
        print('sox_flac_command:       ', sox_flac_command)
        os.system(sox_flac_command)
        mp4_command = '''%s -r 1 -i "%s" -i "%s" -codec:a aac -strict -2 -b:a 384k -c:v libx264 -b:v 500k "%s"''' % ('ffmpeg', os.path.join(cwd, spectrogram_filename), os.path.join(cwd, master_filename), os.path.join(cwd, mp4_filename))
        mp4_metadata =  '-metadata title="%s" ' % title
        mp4_metadata += '-metadata date="%s" ' % metadata_year
        mp4_metadata += '-metadata genre="%s" ' % metadata_notes
        mp4_metadata += '-metadata copyright="%s" ' % str_copyright
        mp4_metadata += '-metadata composer="%s" ' % metadata_author
        mp4_metadata += '-metadata artist="%s" ' % metadata_author
        mp4_metadata += '-metadata publisher="%s" ' % metadata_publisher
        mp4_command = '''"%s" -y -loop 1 -framerate 2 -i "%s" -i "%s" -c:v libx264 -preset medium -tune stillimage -crf 18 -codec:a aac -strict -2 -b:a 384k -r:a 48000 -shortest -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" %s "%s"''' % ('ffmpeg', os.path.join(cwd, spectrogram_filename), os.path.join(cwd, master_filename), mp4_metadata, os.path.join(cwd, mp4_filename))
        mp4_command = mp4_command.replace('\\', '/')
        print('mp4_command:            ', mp4_command)
        os.system(mp4_command)
    except:
        traceback.print_exc()
    finally:
        print("post_process: {}.".format(source_filepath))
        return
    
def play():
    try:
        print("play: {}".format(source_filepath))
        master_filepath = os.path.join(cwd, master_filename)
        print("master_filepath: {}".format(master_filepath))
        if platform_system == "Darwin":
            command = "open {} -a {}".format(master_filepath, soundfile_editor)            
        else:
            command = "{} {}".format(soundfile_editor, master_filepath)
        print("playback command: {}".format(command))
        subprocess.run(command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print("play: {} to {}.".format(source_filepath, output_filename))
        return
        
package_json_template = '''{
  "main": "%s",
  "name": "%s",
  "description": "HTML5 with Csound",
  "version": "0.1.0",
  "keywords": [ "Csound", "node-webkit" ],
  "nodejs": true,
  "window": {
    "title": "%s",
    "icon": "link.png",
    "toolbar": true,
    "frame": true,
    "maximized": true,
    "position": "mouse",
    "fullscreen": true
  },
  "webkit": {
    "plugin": true
  },
  "chromium-args": {
    "--enable-logging": true,
    "--context-mixed": true,
    "--v=1": 1,
    "--device-scale-factor": 4
  }
}'''
        
def html_nw():
    print("html_nw on {}: {}...".format(platform_system, source_filepath))
    try:
        # It seems the string.format method does not work with multi-line 
        # strings.
        package_json = package_json_template % (basename, rootname, rootname)
        print("package.json:", package_json)
        with open("package.json", "w") as file:
            file.write(package_json)
        print("Running on:", platform_system)
        if platform_system == "Darwin":
            os.chdir(directory)
            print("cwd: ", os.getcwd())
        else:
            # Remove symlink to piece directory and link it again.
            try:
                os.remove("/home/mkg/nwjs/package.nw");
            except:
                pass
            # os.symlink(target, linkname)
            os.chdir("/home/mkg/nwjs")
            linkname = "/home/mkg/nwjs/package.nw"
            print(f"directory: {directory} link: {linkname}")
            os.symlink(directory, linkname)
            os.chdir("/home/mkg/nwjs");
            #nwjs_command = "./nw --context-mixed --experimental-modules --alsa-input-device=plughw:2,0 --alsa-output-device=plughw:2,0 --device-scale-factor=2 {}".format(directory)
            #command = "./nw --context-mixed --experimental-modules --device-scale-factor=2 {}".format(directory)
        print("NW.js command:", nwjs_command)
        subprocess.run(nwjs_command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print("html_nw: {}.".format(source_filepath))
        return
        
def html_localhost():
    try:
        print("html_localhost: {}...".format(source_filepath))
        command = "cp package-pnpm.json package.json; python3 -m webbrowser http://localhost:{}/{}".format(port, basename)
    except:
        traceback.print_exc()
    finally:
        print("html_localhost: {}.".format(source_filepath))
        subprocess.run(command, shell=True)
        return
        
def cpp_app():
    try:
        print("platform_system:", platform_system)
        print("cpp_app: {}...".format(source_filepath))
        command = compiler_command + " -o{}; ls -ll {};./{}"
        command = command.format(source_filepath, rootname, rootname, rootname)
        subprocess.run(command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print("cpp_app: {}.".format(source_filepath))
        return

def cpp_audio():
    try:
        command = compiler_command + " -o{};ls -ll {};./{} --csound --audio PortAudio --device {}"
        command = command.format(source_filepath, rootname, rootname, rootname, csound_audio_output)
        print("Executing compiler command:", command)
        pid = subprocess.Popen(command, shell=True, stderr=subprocess.STDOUT)
    except:
        traceback.print_exc()
    finally:
        print("cpp_audio: {}.".format(source_filepath))
        return

def cpp_soundfile():
    print("platform_system:", platform_system)
    try:
        command = compiler_command + " -o{};ls -ll {};./{} --csound {} -o{}"
        command = command.format(source_filepath, rootname, rootname, rootname, common_csound_options, output_filename)
        print("Executing compiler command:", command)
        pid = subprocess.run(command, shell=True, stderr=subprocess.STDOUT)
    except:
        traceback.print_exc()
    finally:
        print("cpp_soundfile: {}.".format(source_filepath))
        return

def man_csound():
    try:
        command = "{} https://gogins.github.io/csound-extended-manual/indexframes.html".format(open_command)
        subprocess.run(command, shell=True)        
    except:
        traceback.print_exc()
    finally:
        print("man_csound: {}.".format(source_filepath))
        return

def man_python():
    try:
        command = "{} https://docs.python.org/3/".format(open_command)
        subprocess.run(command, shell=True)        
    except:
        traceback.print_exc()
    finally:
        print("man_csound: {}.".format(source_filepath))
        return

def man_csoundac():
    try:
        command = "{} file://$HOME/csound-extended/doc/html/index.html".format(open_command)
        subprocess.run(command, shell=True)        
    except:
        traceback.print_exc()
    finally:
        print("man_csound: {}.".format(source_filepath))
        return

def cpp_astyle():
    try:
        command = "astyle --style=1tbs --indent=spaces --indent-switches --indent-cases --indent-namespaces --unpad-paren --indent-classes --indent-after-parens --style=attach {}".format(source_filepath)
        subprocess.run(command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print("man_csound: {}.".format(source_filepath))
        return
        
def generate_csd(patch_filepath, instrument_name, output):
    csd = '''<CsoundSynthesizer>
<CsLicense>
Csound patch test program.
Copyright (c) 2022 by Michael Gogins.

This file is licensed under the terms of the GNU Lesser Public License version 
2.1.
</CsLicense>
<CsOptions>
-d -RWf -m163 -+msg_color=0 --midi-key=4 --midi-velocity=5 -Ffugue2.1.mid --sample-accurate -o{output}
</CsOptions>
<CsInstruments>
sr = 48000
ksmps = 128
nchnls = 2
nchnls_i = 1
0dbfs = 1

// These must be initialized here to be in scope for both 
// the note and the audio patches.

//gi_Organteq vstinit "/home/mkg/Organteq\ 1/x86-64bit/Organteq\ 1.so", 0
//vstinfo gi_Organteq 

//gi_Fluidsynth fluidEngine 0, 0
//gi_FluidSteinway fluidLoad "/home/mkg/Dropbox/Steinway_C.sf2", gi_Fluidsynth, 1
//fluidProgramSelect gi_Fluidsynth, 0, gi_FluidSteinway, 0, 1

//gi_Pianoteq vstinit "/home/mkg/Pianoteq\ 7/x86-64bit/Pianoteq\ 7.so", 0
//vstinfo gi_Pianoteq 

alwayson "PianoOutFluidsynth"
alwayson "PianoOutPianoteq"
alwayson "OrganOutOrganteq"

{patch_text}

// Comment out if you don't have the Steinway_C.sf2 SoundFont.
//#include "PianoOutFluidsynth.inc"

// Comment out if you don't have the Pianoteq and vst4cs.
//#include "PianoOutPianoteq.inc"

// Comment out if you don't have the Organtec and vst4cs.
//#include "OrganOutOrganteq.inc"

#include "MasterOutput.inc"

iampdbfs init 1
prints "Default amplitude at 0 dBFS: %9.4f\n", iampdbfs
idbafs init dbamp(iampdbfs)
prints "dbA at 0 dBFS:               %9.4f\n", idbafs
iheadroom init 6
prints "Headroom (dB):               %9.4f\n", iheadroom
idbaheadroom init idbafs - iheadroom
prints "dbA at headroom:             %9.4f\n", idbaheadroom
iampheadroom init ampdb(idbaheadroom)
prints "Amplitude at headroom:       %9.4f\n", iampheadroom
prints "Balance so the 'overall amps' at the end of performance is -6 dBFS.\n"
prints "nchnls:                      %9.4f\n", nchnls

connect "{instrument_name}", "outleft", "MasterOutput", "inleft"
connect "{instrument_name}", "outright", "MasterOutput", "inright"
connect "PianoOutFluidsynth", "outleft", "MasterOutput", "inleft"
connect "PianoOutFluidsynth", "outright", "MasterOutput", "inright"
connect "PianoOutPianoteq", "outleft", "MasterOutput", "inleft"
connect "PianoOutPianoteq", "outright", "MasterOutput", "inright"
connect "OrganOutOrganteq", "outleft", "MasterOutput", "inleft"
connect "OrganOutOrganteq", "outright", "MasterOutput", "inright"

alwayson "MasterOutput"

</CsInstruments>
<CsScore>
{standard_score}
</CsScore>
</CsoundSynthesizer>
'''
    with open(patch_filepath, "r") as file:
        patch_text = file.read()
    standard_score = generate_standard_score()
    csd = csd.format(patch_text=patch_text, instrument_name=instrument_name, output=output, standard_score=standard_score)
    return csd
def generate_standard_score():
    score = 'f 0 300\n'
    time_= 65.0
    # Generate a chord.
    score += 'i  1    %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 5, 36, 70)
    score += 'i  1    %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 5, 48, 70)
    score += 'i  1    %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 5, 55, 70)
    score += 'i  1    %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 5, 59, 70)
    score += 'i  1    %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 5, 64, 70)
    time_ += 7
    # Generate a random pattern.
    for note_i in range(50):
        duration = random.choice([0.125, 0.33334, 0.25, 0.66667, 1])
        time_ += duration / 2;
        key = random.choice(range(24, 112, 1))
        velocity = random.choice([80, 80-6, 80-12, 80-18]) / 3
        score += 'i  1    %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, duration, key, 60)
    time_ += 4
    # Generate notes to calibrate loudness.
    for duration in [0.125, .25, 2]:
        for key in range(24,108,3):
            time_ = time_ + (1.5 * duration + .5)
            velocity = random.choice([80, 80-6, 80-12, 80-18])
            score += 'i  1    %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, duration, key, velocity)
    time_ += 5
    # Test score-driven note-on, note-off. 
    score += 'i  1.01 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 36, 70)
    time_ += 1
    score += 'i  1.02 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 48, 70)
    time_ += 1
    score += 'i  1.03 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 55, 70)
    time_ += 1
    score += 'i  1.04 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 59, 70)
    time_ += 1
    score += 'i  1.05 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 64, 70)
    time_ += 1
    score += 'i  1.06 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 68, 70)
 
    time_ += 5
    score += 'i -1.01 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 0, 36, 0)
    time_ += 1
    score += 'i -1.02 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 0, 48, 0)
    time_ += 1
    score += 'i -1.03 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 0, 55, 0)
    time_ += 1
    score += 'i -1.04 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 0, 59, 0)
    time_ += 1
    score += 'i -1.05 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 0, 64, 0)
    time_ += 1
    score += 'i -1.06 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, 0, 67, 0)
    # Does one note-off statement turn off multiple note-on statements with 
    # the same tag?
    time_ += 15
    score += 'i  1.10 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 36, 70)
    time_ += 1
    score += 'i  1.10 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 48, 70)
    time_ += 1
    score += 'i  1.10 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 55, 70)
    time_ += 1
    score += 'i  1.10 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 59, 70)
    time_ += 1
    score += 'i  1.10 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 64, 70)
    time_ += 1
    score += 'i  1.10 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_, -1, 68, 70)
    time_ += 5
    score += 'i -1.10 %9.4f %9.4f %9.4f %9.4f 0 0.5\n' % (time_,  0, 77, 0)
    return score        
        
def csd_patch():
    try:
        print("csd_patch: {}...".format(source_filepath))
        print(sys.argv)
        patch_filename = source_filepath
        patch_name = title
        output = output_filename
        message_level = 1 + 2 + 32 + 128
        csound = ctcsound.Csound()
        csound.message("Patch file: {} Patch name: {} Output: {}\n".format(patch_filename, patch_name, output))
        csd = generate_csd(patch_filename, patch_name, output)
        print(csd)
        csound.compileCsdText(csd)
        csound.start()
        csound.perform()
        print("PERFORMING...")
    except:
        traceback.print_exc()
    finally:
        print("csd_patch: {}.".format(source_filepath))
        return

if command == 'csd-audio':
    csd_audio()
if command == 'csd-soundfile':
    csd_soundfile()
if command == 'csd-post':
    csd_soundfile()
    post_process()
if command == 'csd-play':
    csd_soundfile()
    post_process()
    play()
if command == 'html-localhost':
    html_localhost()
if command == 'html-nw':
    html_nw()
# if command == 'cpp-lib':
#     cpp_lib()
if command == 'cpp-app':
    cpp_app()
if command == 'cpp-audio':
    cpp_audio()
if command == 'cpp-soundfile':
    cpp_soundfile()
if command == 'cpp-play':
    cpp_soundfile()
    post_process()
    play()
if command == 'man-csound':
    man_csound()
if command == 'man-python':
    man_python()
if command == 'man-csoundac':
    man_csoundac()
if command == 'cpp-astyle':
    cpp_astyle()
if command == 'csd-patch':
    csd_patch()
    post_process()
    play()

