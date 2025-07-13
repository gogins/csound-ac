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
    
python3 playpen.py post-process {sounfile_name}
    Post-processes the file to normalize it, save it in various formats, etc., 
    and finally opens the soundfile using the editor specified in playpen.ini.
    
python3 playpen.py csound-patch {source.inc}
    Includes the inc file in a csd file, renders it to a soundfile, and 
    opens the soundfile using the editor specified in playpen.ini.
    
python3 playpen.py html-localhost {source.html}
    Uses Python to open source.html from a local webserver.

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

For post-processing, the ffmpeg program needs to be installed.

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
import json
import os
import os.path
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
print(f"command:                        {command}")
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
print(f'csound_audio_output:           {csound_audio_output}')
port = playpen_ini.get("playpen", "port")
print(f'HTTP server port:              {port}')
soundfile_editor = playpen_ini.get("playpen", "soundfile-editor")
print(f'Soundfile editor:              {soundfile_editor}')

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


print(f'\nffmpeg_mp3_command:\n{ffmpeg_concert_command}\n')

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

print(f'\nffmpeg_mp4_command:\n{ffmpeg_concert_command}\n')


