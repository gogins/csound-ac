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
    playpen.ini, then post-processes the file to normalize it and save it in 
    various formats.
    
python3 playpen.py post-process {soundfile_name}
    Post-processes the file to normalize it and save it in various formats.
    
python3 playpen.py csound-patch {source.inc}
    Includes the inc file in a csd file, renders it to a soundfile, and 
    opens the soundfile using the editor specified in playpen.ini.
    
python3 playpen.py html-localhost {source.html}
    Uses Python to open source.html from a local webserver.

python3 playpen html-nw {source.html}
    Creates a package.json file for the specified HTML file, and runs that 
    file as a NW.js application.
    
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

For post-processing, the ffmpeg program needs to be installed.

For HTML related commands, NW.js needs to be installed.

For build commands, a C++ toolchain needs to be installed.

For reformatting files, the astyle program needs to be installed.

For playing, the Audacity program is a good choice because it can display 
soundfiles simultaneously as both waveforms and spectrograms, and export to 
various formats.

'''
import configparser
try:
    import ctcsound
except:
    pass
import datetime
import json
import os
import os.path
import platform
import random
import string
import subprocess
import sys
import tempfile
import time
import traceback

cwd = os.getcwd()

running_subprocess = None

# repeatable "random" segment, so that renderings can be compared.

random.seed(119480)

if len(sys.argv) < 2:
    print(__doc__)
    exit(0)
command = sys.argv[1]
if command == 'help':
    print(__doc__)
    exit(0)
print()
print(f"Playpen command:                {command}")
platform_system = platform.system()
print(f'Platform:                       {platform_system}')
if platform_system == "Darwin":
    open_command = "open"
else:
    open_command = "xdg-open"
print()
print("Configuration file:")
print()
home_directory = os.environ["HOME"]
playpen_ini_filepath = os.path.join(home_directory, "playpen.ini")
playpen_ini = configparser.ConfigParser()
playpen_ini.read_file(open(playpen_ini_filepath))
ini_author = playpen_ini.get("metadata", "author")
ini_publisher = playpen_ini.get("metadata", "publisher")
ini_year = playpen_ini.get("metadata", "year")
ini_notes = playpen_ini.get("metadata", "notes")
ini_license = playpen_ini.get("metadata", "license")
csound_audio_output = playpen_ini.get("csound", "audio-output")
print(f'Csound audio output:            {csound_audio_output}')
port = playpen_ini.get("playpen", "port")
print(f'HTTP server port:               {port}')
soundfile_editor = playpen_ini.get("playpen", "soundfile-editor")
print(f'Soundfile editor:               {soundfile_editor}')
nwjs_command = playpen_ini.get("nwjs", "nwjs-command")
print(f'nwjs_command:                   {nwjs_command}')
common_csound_options = "-RWd -m163 -W -+msg_color=0 --midi-key=4 --midi-velocity=5"
print(f'Common Csound options:          {common_csound_options}')
compiler_command = playpen_ini.get("cplusplus", "compiler-command")
print(f'C++ compiler command:           {compiler_command}')

metadata_album = ''
metadata_artist = ini_author
metadata_comment = ''
metadata_composer = ini_author
metadata_copyright = ini_author
metadata_date = ini_year
metadata_genre = 'electroacoustic'
metadata_performer = 'csound'
metadata_publisher = ini_publisher
metadata_source = 'csound'
if command in ['man-csound', 'man-csoundac', 'man-python']:
    composition_filepath = 'dummy'
else:
    composition_filepath = sys.argv[2]
composition_filename = os.path.basename(composition_filepath)
metadata_title, ext = os.path.splitext(composition_filename)
rendered_audio_filename = os.path.splitext(composition_filename)[0] + ".wav"
metadata_track = ''

normalized_filename = f'{metadata_title}-normalized.wav'
concert_filename = f'{metadata_artist}, {metadata_title}.wav'
flac_filename = f'{metadata_artist}, {metadata_title}.flac'
cd_filename = f'{metadata_artist}, {metadata_title}.cd.wav'
mp3_filename = f'{metadata_artist}, {metadata_title}.mp3'
png_filename = f'{metadata_artist}, {metadata_title}.png'
mp4_filename = f'{metadata_artist}, {metadata_title}.mp4'

print()
print('Filenames:')
print()
print(f'current working directory:      {cwd}')
print(f'composition_filename:           {composition_filename}')
print(f'rendered_soundfile_filename:    {rendered_audio_filename}')
print(f'normalized_filename:            {normalized_filename}')
print(f'concert_filename:               {concert_filename}')
print(f'flac_filename:                  {flac_filename}')
print(f'cd_filename:                    {cd_filename}')
print(f'mp3_filename:                   {mp3_filename}')
print(f'png_filename:                   {png_filename}')
print(f'mp4_filename:                   {mp4_filename}')
print()
print('Metadata:')
print()
print(f'metadata_album:                 {metadata_album}')
print(f'metadata_artist:                {metadata_artist}')
print(f'metadata_comment:               {metadata_comment}')
print(f'metadata_composer:              {metadata_composer}')
print(f'metadata_copyright:             {metadata_copyright}')
print(f'metadata_date:                  {metadata_date}')
print(f'metadata_genre:                 {metadata_genre}')
print(f'metadata_performer:             {metadata_performer}')
print(f'metadata_publisher:             {metadata_publisher}')
print(f'metadata_source:                {metadata_source}')
print(f'metadata_title:                 {metadata_title}')
print()

def measure_true_peak(input_file):
    command = [
        "ffmpeg", "-hide_banner", "-y", "-i", input_file,
        "-af", "loudnorm=I=-23:TP=-1.0:LRA=7:print_format=json",
        "-f", "null", "-"
    ]
    result = subprocess.run(command, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL, text=True)
    lines = result.stderr.splitlines()
    print('lines:', ''.join(lines))
    json_lines = []
    recording = False
    for line in lines:
        if line.strip() == '{':
            recording = True
        if recording:
            json_lines.append(line)
        if line.strip() == '}':
            break
    print(json_lines)
    loudnorm_data = json.loads('\n'.join(json_lines))
    input_tp = float(loudnorm_data["input_tp"])
    return input_tp

def apply_gain(input_file, output_file, gain_db):
    command = [
        "ffmpeg", "-hide_banner", "-y", "-i", input_file,
        "-af", f"volume={gain_db:.3f}dB",
        "-c:a", "pcm_s24le", output_file
    ]
    subprocess.run(command, check=True)

def normalize_to_minus1dbtp(input_file, output_file):
    input_tp = measure_true_peak(input_file)
    gain = -1.0 - input_tp
    print(f"Input true peak: {input_tp:.2f} dBTP")
    print(f"Applying gain: {gain:.2f} dB to reach -1 dBTP")
    apply_gain(input_file, output_file, gain)
    print(f"Output written to: {output_file}")

def post_process():
    try:
        print(f"\npost_process: {composition_filename}...\n")
        normalize_to_minus1dbtp(rendered_audio_filename, normalized_filename)

        ffmpeg_concert_command = f'ffmpeg -y -i "{normalized_filename}" -filter:a "volume=-1dB" -c:a pcm_s24le -f wav \
        -metadata album="{metadata_album}" \
        -metadata artist="{metadata_artist}" \
        -metadata comment="{metadata_comment}" \
        -metadata composer="{metadata_composer}" \
        -metadata copyright="{metadata_copyright}" \
        -metadata date="{metadata_date}" \
        -metadata genre="{metadata_genre}" \
        -metadata performer="{metadata_performer}" \
        -metadata publisher="{metadata_publisher}" \
        -metadata source="{metadata_source}" \
        -metadata title="{metadata_title}" \
        "{concert_filename}"'
        print(f'\nffmpeg_concert_command:\n{ffmpeg_concert_command}\n')
        os.system(ffmpeg_concert_command)

        ffmpeg_flac_command = f'''ffmpeg -y -i "{rendered_audio_filename}" \
        -af "loudnorm=I=-14:TP=-1.5:LRA=20:dual_mono=false" \
        -c:a flac -f flac \
        -metadata album="{metadata_album}" \
        -metadata artist="{metadata_artist}" \
        -metadata comment="{metadata_comment}" \
        -metadata composer="{metadata_composer}" \
        -metadata copyright="{metadata_copyright}" \
        -metadata date="{metadata_date}" \
        -metadata genre="{metadata_genre}" \
        -metadata performer="{metadata_performer}" \
        -metadata publisher="{metadata_publisher}" \
        -metadata source="{metadata_source}" \
        -metadata title="{metadata_title}" \
        "{flac_filename}"'''
        print(f'\nffmpeg_flac_command:\n{ffmpeg_flac_command}\n')
        os.system(ffmpeg_flac_command)

        ffmpeg_cd_command = f'ffmpeg -y -i "{normalized_filename}" \
        -af "loudnorm=I=-14:TP=-1:LRA=20:linear=true:dual_mono=false" \
        -ar 44100 -ac 2 -sample_fmt s16 -c:a pcm_s16le -f wav \
        -metadata album="{metadata_album}" \
        -metadata artist="{metadata_artist}" \
        -metadata comment="{metadata_comment}" \
        -metadata composer="{metadata_composer}" \
        -metadata copyright="{metadata_copyright}" \
        -metadata date="{metadata_date}" \
        -metadata genre="{metadata_genre}" \
        -metadata performer="{metadata_performer}" \
        -metadata publisher="{metadata_publisher}" \
        -metadata source="{metadata_source}" \
        -metadata title="{metadata_title}" \
        "{cd_filename}"'
        print(f'\nffmpeg_cd_command:\n{ffmpeg_cd_command}\n')
        os.system(ffmpeg_cd_command)

        ffmpeg_mp3_command = f'ffmpeg -y -i "{normalized_filename}" \
        -af "loudnorm=I=-14:TP=-1:LRA=20:linear=true:dual_mono=false" \
        -ar 44100 -ac 2 -sample_fmt s16 -c:a pcm_s16le -f wav \
        -metadata album="{metadata_album}" \
        -metadata artist="{metadata_artist}" \
        -metadata comment="{metadata_comment}" \
        -metadata composer="{metadata_composer}" \
        -metadata copyright="{metadata_copyright}" \
        -metadata date="{metadata_date}" \
        -metadata genre="{metadata_genre}" \
        -metadata performer="{metadata_performer}" \
        -metadata publisher="{metadata_publisher}" \
        -metadata source="{metadata_source}" \
        -metadata title="{metadata_title}" \
        "{mp3_filename}"'
        print(f'\ffmpeg_mp3_command:\n{ffmpeg_mp3_command}\n')
        os.system(ffmpeg_mp3_command)

        ffmpeg_png_command = (
            f'ffmpeg -y -i "{concert_filename}" -filter_complex "'
            '[0:a]showspectrumpic=s=1100x1200:legend=1:mode=separate[s]; '
            'color=c=black@1:s=1400x1400:d=1[bg]; '
            '[bg][s]overlay=0:0[tmp]; '
            f"[tmp]drawtext=text='{metadata_artist}, {metadata_title}':x=(w-text_w)/2:y=1340:fontsize=28:fontcolor=white"
            f'" -vframes 1 "{png_filename}"'
        )
        print(f'\nffmpeg_png_command:\n{ffmpeg_png_command}\n')
        os.system(ffmpeg_png_command)

        # Create a high-resolution static video with audio
        ffmpeg_mp4_command = f'ffmpeg -y -loop 1 -i "{png_filename}" -i "{normalized_filename}" \
        -af "loudnorm=I=-14:TP=-1:LRA=20:linear=true:dual_mono=false" \
        -c:v libx264 -tune stillimage -pix_fmt yuv420p -r 1 \
        -c:a aac -b:a 320k -shortest \
        -metadata album="{metadata_album}" \
        -metadata artist="{metadata_artist}" \
        -metadata comment="{metadata_comment}" \
        -metadata composer="{metadata_composer}" \
        -metadata copyright="{metadata_copyright}" \
        -metadata date="{metadata_date}" \
        -metadata genre="{metadata_genre}" \
        -metadata performer="{metadata_performer}" \
        -metadata publisher="{metadata_publisher}" \
        -metadata source="{metadata_source}" \
        -metadata title="{metadata_title}" \
        "{mp4_filename}"'
        print(f'\nffmpeg_mp4_command:\n{ffmpeg_mp4_command}\n')
        os.system(ffmpeg_mp4_command)
    except:
        traceback.print_exc()
    finally:
        print(f"\npost_process: {composition_filename}.\n")
        return

def csd_audio():
    try:
        print(f"\ncsd_audio: {composition_filepath} to {csound_audio_output}...")
        csound_command = f"csound {composition_filepath} -o{csound_audio_output}"
        print(f"csound command: {csound_command}")
        result = subprocess.run(csound_command, shell=True)
        print(f"csound command: {csound_command} {result}")
    except:
        traceback.print_exc()
    finally:
        print(f"\ncsd_audio: {composition_filepath} to {csound_audio_output}.\n")
        return
        
def csd_soundfile():
    try:
        print(f"\ncsd_soundfile: {composition_filepath} to {rendered_audio_filename}...")
        csound_command = f"csound {composition_filepath} -RWo{rendered_audio_filename} --simple-sorted-score={composition_filepath}.srt"
        print(f"csound command: {csound_command}")
        result = subprocess.run(csound_command, shell=True)
        print(f"csd_soundfile result: {result}")
        post_process()
    except:
        traceback.print_exc()
    finally:
        print(f"csd_soundfile: {composition_filepath} to {rendered_audio_filename}.\n")
        return
         
def play():
    try:
        print(f"play: {composition_filepath}")
        master_filepath = os.path.join(cwd, normalized_filename)
        print(f"master_filepath: {master_filepath}")
        if platform_system == "Darwin":
            command = f"open {master_filepath} -a {soundfile_editor}"            
        else:
            command = f"{soundfile_editor} {master_filepath}"
        print(f"playback command: {command}")
        subprocess.run(command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print(f"play: {composition_filepath} to {rendered_audio_filename}.")
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
    print(f"html_nw on {platform_system}: {composition_filepath}...")
    try:
        # It seems the string.format method does not work with multi-line 
        # strings.
        directory, basename = os.path.split(source_filepath)
        title, extension = os.path.splitext(basename)
        package_json = package_json_template % (basename, title, title)
        print("package.json:", package_json)
        with open("package.json", "w") as file:
            file.write(package_json)
        print(f"Running on: {platform_system}")
        if platform_system == "Darwin":
            os.chdir(directory)
            print(f"cwd: ", os.getcwd())
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
        print(f"NW.js command: {nwjs_command}")
        subprocess.run(nwjs_command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print(f"html_nw: {composition_filepath}.")
        return
        
def html_localhost():
    try:
        print(f"html_localhost: {composition_filename}...")
        command = "cp package-pnpm.json package.json; python3 -m webbrowser http://localhost:{port}/{composition_filename}"
    except:
        traceback.print_exc()
    finally:
        print(f"html_localhost: {composition_filename}.")
        subprocess.run(command, shell=True)
        return
        
def cpp_app():
    try:
        print(f"cpp_app: {composition_filepath}...")
        command = compiler_command + " -o{}; ls -ll {};./{}"
        command = command.format(composition_filepath, metadata_title, metadata_title, metadata_title)
        subprocess.run(command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print(f"cpp_app: {composition_filepath}.\n")
        return

def cpp_audio():
    try:
        command = compiler_command + " -o{};ls -ll {};./{} --csound --audio PortAudio --device {}"
        command = command.format(composition_filepath, metadata_title, metadata_title, metadata_title, csound_audio_output)
        print(f"Executing compiler command: {command}")
        pid = subprocess.Popen(command, shell=True, stderr=subprocess.STDOUT)
    except:
        traceback.print_exc()
    finally:
        print(f"cpp_audio: {composition_filepath}.\n")
        return

def cpp_soundfile():
    print(f"platform_system:", platform_system)
    try:
        command = compiler_command + " -o{};ls -ll {};./{} --csound {} -o{}"
        command = command.format(composition_filepath, metadata_title, metadata_title, metadata_title, common_csound_options, rendered_audio_filename)
        print(f"Executing compiler command: {command}")
        pid = subprocess.run(command, shell=True, stderr=subprocess.STDOUT)
    except:
        traceback.print_exc()
    finally:
        print(f"cpp_soundfile: {composition_filepath}.\n")
        return

def man_csound():
    try:
        command = f"{open_command} https://gogins.github.io/csound-extended-manual/indexframes.html"
        subprocess.run(command, shell=True)        
    except:
        traceback.print_exc()
    finally:
        print(f"man_csound: {composition_filepath}.\n")
        return

def man_python():
    try:
        command = f"{open_command} https://docs.python.org/3/"
        subprocess.run(command, shell=True)        
    except:
        traceback.print_exc()
    finally:
        print(f"man_csound: {composition_filepath}.\n")
        return

def man_csoundac():
    try:
        # TODO: a problem here.
        command = f"{open_command} file://$HOME/csound-ac/doc/latex/csound-ac.pdf"
        subprocess.run(command, shell=True)        
    except:
        traceback.print_exc()
    finally:
        print(f"man_csoundac: {composition_filepath}.\n")
        return

def cpp_astyle():
    try:
        command = "astyle --style=1tbs --indent=spaces --indent-switches --indent-cases --indent-namespaces --unpad-paren --indent-classes --indent-after-parens --style=attach {}".format(composition_filepath)
        subprocess.run(command, shell=True)
    except:
        traceback.print_exc()
    finally:
        print(f"cpp_astyle: {composition_filepath}.\n")
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
prints "Default amplitude at 0 dBFS: %9.4f", iampdbfs
idbafs init dbamp(iampdbfs)
prints "dbA at 0 dBFS:               %9.4f", idbafs
iheadroom init 6
prints "Headroom (dB):               %9.4f", iheadroom
idbaheadroom init idbafs - iheadroom
prints "dbA at headroom:             %9.4f", idbaheadroom
iampheadroom init ampdb(idbaheadroom)
prints "Amplitude at headroom:       %9.4f", iampheadroom
prints "Balance so the 'overall amps' at the end of performance is -6 dBFS."
prints "nchnls:                      %9.4f", nchnls

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
    with open('temp.csd', 'w') as file:
        file.write(csd)
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
        print(f"csd_patch: {composition_filepath}...")
        print(sys.argv)
        patch_filename = composition_filepath
        patch_name = metadata_title
        message_level = 1 + 2 + 32 + 128
        csound = ctcsound.Csound()
        csound.message(f"Patch file: {patch_filename} Patch name: {patch_name} Output: {rendered_audio_filename}\n")
        csd = generate_csd(patch_filename, patch_name, rendered_audio_filename)
        print("\n>>>>>>>>>>>>>>>>>>>>")
        print(csd)
        print(">>>>>>>>>>>>>>>>>>>>\n")
        csound.compileCsdText(csd)
        csound.start()
        csound.perform()
        print(f"PERFORMING...")
    except:
        traceback.print_exc()
    finally:
        print(f"csd_patch: {composition_filepath}.\n")
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
# TODO: C++ code writes the wrong soundfile name.
if command == 'cpp-play':
    cpp_soundfile()
    post_process()
    play()
if command == 'post-process':
    post_process()
    play()
if command == 'play':
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

