#!/usr/bin/python3
# -*- coding: utf-8 -*-
import concurrent.futures
import contextlib
import datetime
import markdown
import math
import os.path
import pathlib
import random
import string
import subprocess
import sys
import time
import traceback
import warnings

# Comment this out during development?

warnings.filterwarnings("ignore")

'''
A few strict naming conventions greatly simplify the code.

Each piece is always a single text file of code; it could be piece.csd, 
piece.py, or piece.html. The piece should specify a real-time audio output 
device and, if needed, an audio input and MIDI input and output. For rendering 
to a soundfile, the playpen program will write to piece.wav and post-process 
that file to produce piece.normalized.wav, piece.mp3, piece.flac, and piece.mp4;
piece.normalized.wav will then be opened for inspection in Audacity.

All user-defined Python graphical user interfaces are stored in a glade file 
with the same filename, i.e. my_piece.py goes with my_piece.glade, and the 
widget tree thus defined is re-parented to the controls_layout widget. The 
user need write no code for handling UI events; the convention is that all UI 
events from the user-defined controls are handled in a single function that 
dispatches all widget event values to a Csound control channel with the same 
name as the widget. 

If you want to write a piece that combines different languages, e.g. a Csound 
orchestra with a Python score generator with HTML for display or control, then 
write a Python piece that embeds the Csound code and the HTML code as variables 
containing multiple line strings, and then call functions in the playpen 
program to set up the GUI and control the piece. All functions defined in the 
code below are in the scope of a Python piece.
'''

import gi
gi.require_version('Gdk', '3.0')
from gi.repository import Gdk
from gi.repository import GObject
from gi.repository import GLib

# Obtain user settings.
settings = GLib.KeyFile.new()
GLib.KeyFile.load_from_file(settings, "playpen.ini", GLib.KeyFileFlags.NONE)
metadata_author=settings.get_value("metadata", "author")
metadata_publisher=settings.get_value("metadata", "publisher")
metadata_year=settings.get_value("metadata", "year")
metadata_notes=settings.get_value("metadata", "notes")
metadata_license=settings.get_value("metadata", "license")
global csound_audio_output
csound_audio_output=settings.get_value("csound", "audio-output")
print("csound_audio_output: " + csound_audio_output)
soundfile_editor=settings.get_value("playpen", "soundfile-editor")
gnome_theme=settings.get_value("playpen", "gnome-theme")
editor_scheme = settings.get_value("playpen", "editor-scheme")

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk 
gi.require_version("GtkSource", "3.0")
from gi.repository import GtkSource
code_editor = GtkSource.View()

# Override some global Gnome settings with playpen.ini values.
gnome_settings = Gtk.Settings.get_default()
gnome_settings.set_property("gtk-theme-name", gnome_theme)

gi.require_version("WebKit2", "4.0")
from gi.repository import WebKit2
gi.require_version("JavaScriptCore", "4.0")
from gi.repository import JavaScriptCore
gi.require_version("GtkSource", "3.0")
from gi.repository import GtkSource

# Create a global instance of native Csound. For pure Csound and Python 
# pieces, this instance is a singleton.
import CsoundThreaded
csound = CsoundThreaded.CsoundThread()
print("Global Csound instance: {} CSOUND *: 0x{:x}.".format(csound, int(csound.GetCsound())))
        
title = "Playpen"
print("title:", title)

builder = Gtk.Builder()
builder.add_from_file("playpen.glade")

global filename
filename = ""
global piece
piece = ""

# An empty, version-free Glade template string.
# In Glade, the GtkGrid can be replaced by any other container, 
# but it must have the id "user_controls_layout".

glade_controls_template = '''<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk+" version="3.20"/>
  <object class="GtkGrid">
    <property id="name">user_controls_layout</property>
    <property name="name">user_controls_layout</property>
    <property name="visible">True</property>
    <property name="can_focus">False</property>
  </object>
</interface>
'''

def piece_is_csound():
    if filename.lower().endswith('.csd'):
        return True
    else:
        return False

def piece_is_python():
    if filename.lower().endswith('.py'):
        return True
    else:
        return False
    
def piece_is_html():
    if filename.lower().endswith('.html'):
        return True
    else:
        return False
    
def on_new_button_clicked(button):
    try:
        print(button.get_label())
        file_chooser_dialog = Gtk.FileChooserDialog(title="Please enter a filename", 
            parent=None, 
            action=Gtk.FileChooserAction.SAVE)
        file_chooser_dialog.add_buttons(
        Gtk.STOCK_CANCEL,
        Gtk.ResponseType.CANCEL,
        Gtk.STOCK_SAVE,
        Gtk.ResponseType.OK)
        file_chooser_dialog.run()
        filename = file_chooser_dialog.get_filename()
        glade_file = glade_filename_(filename)
        with open(glade_file, 'w') as file:
            file.write(glade_controls_template)
        print(filename)
        code_editor.get_buffer().set_text("")
    except:
        print(traceback.format_exc())
    
def load_piece(filename):
    try:
        with open(filename, "r") as file:
            piece = file.read()
            language = language_manager.guess_language(filename)
            if language is not None:
                code_editor.get_buffer().set_language(language)
            #print("load_piece: language: {}".format(language.get_name()))
            code_editor.get_buffer().set_text(piece)
        if piece_is_csound():
            load_glade(filename)
        if piece_is_python():
            load_glade(filename)
        if piece_is_html():
            piece_uri = pathlib.Path(filename).resolve().parent.as_uri()
            print("load_piece: uri: {}".format(piece_uri))
            webview.load_html(piece, piece_uri)            
        main_window.set_title(filename)
    except:
        print(traceback.format_exc())
        
'''
For only those widgets and those signals that are used here to control Csound 
performances using the Csound control channels, connect the on_control_changed 
signal to its callback.
'''
def connect_controls(container, on_control_changed_):
    children = container.get_children()
    for child in children:
        if isinstance(child, Gtk.Button):
            print(child.get_name())
            child.connect("pressed", on_control_changed_, 1.)
            child.connect("released", on_control_changed_, 0.)
        if isinstance(child, Gtk.MenuItem):
            print(child.get_name())
            child.connect("select", on_control_changed_, 1.)
            child.connect("deselect", on_control_changed_, 0.)
        if isinstance(child, Gtk.Scale):
            print(child.get_name())
            child.connect("value-changed", on_control_changed_, -1.)
        if isinstance(child, Gtk.Editable):
            print(child.get_name())
            child.connect("activate", on_control_changed_, -1.)
        if isinstance(child, Gtk.SpinButton):
            print(child.get_name())
            child.connect("value-changed", on_control_changed_, -1)
        if isinstance(child, Gtk.Container):
            connect_controls(child, on_control_changed_)

# Please note, the order of conditions matters; some subclasses do 
# not handle superclass signals.

def on_control_change(control, data):
    try:
        channel_name = control.get_name()
        channel_value = ""
        if isinstance(control, Gtk.ToggleButton):
            channel_value = control.get_active()
            csound.SetControlChannel(channel_name, float(channel_value))
        elif isinstance(control, Gtk.Button):
            channel_value = data
            csound.SetControlChannel(channel_name, float(channel_value))
        elif isinstance(control, Gtk.MenuItem):
            channel_value = data
            csound.SetControlChannel(channel_name, float(channel_value))
        elif isinstance(control, Gtk.Scale):
            channel_value = control.get_value()
            csound.SetControlChannel(channel_name, float(channel_value))
        #~ elif isinstance(control, Gtk.SpinButton):
            #~ channel_value = control.get_value()
            #~ csound.SetControlChannel(channel_name, channel_value)
        elif isinstance(control, Gtk.Editable):
            channel_value = control.get_text()
            csound.SetStringChannel(channel_name, channel_value)
        print("on_control_change: {}: {} {}".format(channel_name, type(channel_value), channel_value))
    except:
        print(traceback.format_exc())

def glade_filename_(filename):            
    pathlib_ = pathlib.PurePath(filename)
    glade_file = str(pathlib_.with_suffix(".glade"))
    # print("glade_file: {}".format(glade_file))
    return glade_file

def load_glade(filename):
    glade_file = glade_filename_(filename)
    if os.path.exists(glade_file) == True:
        try:
            with open(glade_file, "r") as file:
                glade_xml = file.read()
                # print("glade:", glade_xml)
                result = builder.add_from_string(glade_xml)
                if result == 0:
                    print("Failed to parse {} file.".format(glade_file))
                user_controls_layout = builder.get_object("user_controls_layout")
                # print("user_controls_layout: {}".format(user_controls_layout))
                children = controls_layout.get_children()
                for child in children:
                    if child.get_name() == "user_controls_layout":
                        controls_layout.remove(child)
                controls_layout.add(user_controls_layout)
                connect_controls(controls_layout, on_control_change)
        except:
            print("Error: failed to load user-defined controls layout.")
            print(traceback.format_exc())
    else:
        print("Glade file not found, not defining controls.")

def on_open_button_clicked(button):
    try:
        print(button.get_label())
        file_chooser_dialog = Gtk.FileChooserDialog(title="Please enter a filename", 
            parent=None, 
            action=Gtk.FileChooserAction.OPEN)
        file_chooser_dialog.add_buttons(
        Gtk.STOCK_CANCEL,
        Gtk.ResponseType.CANCEL,
        Gtk.STOCK_SAVE,
        Gtk.ResponseType.OK)
        file_chooser_dialog.run()
        filename = file_chooser_dialog.get_filename()
        load_piece(filename)
        file_chooser_dialog.close()
    except:
        print(traceback.format_exc())
        
def save_piece():
    global filename
    global piece
    try:
        buffer = code_editor.get_buffer()
        piece = buffer.get_text(buffer.get_start_iter(), buffer.get_end_iter(), True)
        with open(filename, "w") as file:
            file.write(piece)
            language = language_manager.guess_language(filename)
            if language is not None:
                code_editor.get_buffer().set_language(language)
    except:
        print(traceback.format_exc())
        
def on_save_button_clicked(button):
    try:
        print(button.get_label())
        save_piece()
    except:
        print(traceback.format_exc())
    
def on_save_as_button_clicked(button):
    global filename
    try:
        print(button.get_label())
        file_chooser_dialog = Gtk.FileChooserDialog(title="Please enter a filename", 
            parent=None, 
            action=Gtk.FileChooserAction.SAVE)
        file_chooser_dialog.add_buttons(
        Gtk.STOCK_CANCEL,
        Gtk.ResponseType.CANCEL,
        Gtk.STOCK_SAVE,
        Gtk.ResponseType.OK)
        file_chooser_dialog.run()
        filename = file_chooser_dialog.get_filename()
        save_piece()
    except:
        print(traceback.format_exc())
        
def on_play_audio_button_clicked(button):
    global filename
    global piece
    try:
        print(button.get_label())
        save_piece()
        load_glade(filename)
        if piece_is_python():
            # Only globals are passed, because otherwise recursive functions 
            # defined and invoked in the piece will not work. See:
            # https://stackoverflow.com/questions/871887/using-exec-with-recursive-functions
            exec(piece, globals())
        if piece_is_csound():
            # Change output target here.
            # Patch the csound options.
            print("Piece:")
            print(piece)
            csd = patch_csound_options(piece, output="realtime")
            print("Patched piece:")
            print(csd)
            csound.CompileCsdText(csd)
            csound.Start()
            csound.Perform()
    except:
        print(traceback.format_exc())
        
def on_render_soundfile_button_clicked(button):
    global filename
    global piece
    try:
        print(button.get_label())
        save_piece()
        load_glade(filename)
        if piece_is_csound():
            csd = patch_csound_options(piece, output="soundfile")
            csound.CompileCsdText(csd)
            csound.Start()
            # Keep the UI responsive during performance.
            while csound.PerformBuffer() == 0:
                Gtk.main_iteration_do(False)
            csound.Stop()
            csound.Join()
            csound.Cleanup()
            csound.Reset()
            post_process()
    except:
        print(traceback.format_exc())
        
def on_stop_button_clicked(button):
    try:
        print("Stopping csound...")
        csound.Stop()
        csound.Join()
        csound.Cleanup()
        csound.Reset()
        print("Csound has been stopped and reset.")
    except:
        print(traceback.format_exc())
        
def post_process():
    try:
        print("Post-processing...")
        cwd = os.getcwd()
        print('cwd:                    ' + cwd)
        author = metadata_author #'Michael Gogins'
        year = metadata_year #'2021'
        license = metadata_license #'ASCAP'
        publisher = metadata_publisher #'Irreducible Productions, ASCAP'
        notes = metadata_notes #'Electroacoustic Music'

        directory, basename = os.path.split(filename)
        rootname = os.path.splitext(basename)[0].split('.')[0]
        soundfile_name = rootname + ".wav"
        title = rootname.replace("-", " ").replace("_", " ")
        label = '{} -- {}'.format(author, title).replace(" ", "_")
        master_filename = '{}.normalized.wav'.format(label)
        spectrogram_filename = '%s.png' % label
        cd_quality_filename = '%s.cd.wav' % label
        mp3_filename = '%s.mp3' % label
        mp4_filename = '%s.mp4' % label
        flac_filename = '%s.flac' % label
        print('Basename:               ' + basename)
        print('Original soundfile:     ' + soundfile_name)
        print('Author:                 ' + author)
        print('Title:                  ' + title)
        print('Year:                   ' + year)
        str_copyright          = 'Copyright %s by %s' % (year, author)
        print('Copyright:              ' + str_copyright)
        print('Licence:                ' + license)
        print('Publisher:              ' + publisher)
        print('Notes:                  ' + notes)
        print('Master filename:        ' + master_filename)
        print('Spectrogram filename:   ' + spectrogram_filename)
        print('CD quality filename:    ' + cd_quality_filename)
        print('MP3 filename:           ' + mp3_filename)
        print('MP4 filename:           ' + mp4_filename)
        print('FLAC filename:          ' + flac_filename)
        bext_description       = notes
        bext_originator        = author
        bext_orig_ref          = basename
        #bext_umid              = xxx
        #bext_orig_date         = xxx
        #bext_orig_time         = xxx
        #bext_coding_hist       = xxx
        #bext_time_ref          = xxx
        str_comment            = notes
        str_title              = title
        str_artist             = author
        str_date               = year
        str_license            = license
        sox_normalize_command = '''sox -S "%s" "%s" gain -n -3''' % (soundfile_name, master_filename + 'untagged.wav')
        print('sox_normalize command:  ' + sox_normalize_command)
        os.system(sox_normalize_command)
        tag_wav_command = '''sndfile-metadata-set "%s" --bext-description "%s" --bext-originator "%s" --bext-orig-ref "%s" --str-comment "%s" --str-title "%s" --str-copyright "%s" --str-artist  "%s" --str-date "%s" --str-license "%s" "%s"''' % (master_filename + 'untagged.wav', bext_description, bext_originator, bext_orig_ref, str_comment, str_title, str_copyright, str_artist, str_date, str_license, master_filename)
        print('tag_wav_command:        ' + tag_wav_command)
        os.system(tag_wav_command)
        sox_spectrogram_command = '''sox -S "%s" -n spectrogram -o "%s" -t"%s" -c"%s"''' % (master_filename, spectrogram_filename, label, str_copyright + ' (%s' % publisher)
        print('sox_spectrogram_command:' + sox_spectrogram_command)
        os.system(sox_spectrogram_command)
        sox_cd_command = '''sox -S "%s" -b 16 -r 44100 "%s"''' % (master_filename, cd_quality_filename + 'untagged.wav')
        print('sox_cd_command:         ' + sox_cd_command)
        os.system(sox_cd_command)
        tag_wav_command = '''sndfile-metadata-set "%s" --bext-description "%s" --bext-originator "%s" --bext-orig-ref "%s" --str-comment "%s" --str-title "%s" --str-copyright "%s" --str-artist  "%s" --str-date "%s" --str-license "%s" "%s"''' % (cd_quality_filename + 'untagged.wav', bext_description, bext_originator, bext_orig_ref, str_comment, str_title, str_copyright, str_artist, str_date, str_license, cd_quality_filename)
        print('tag_wav_command:        ' + tag_wav_command)
        os.system(tag_wav_command)
        mp3_command = '''lame --add-id3v2 --tt "%s" --ta "%s" --ty "%s" --tn "%s" --tg "%s"  "%s" "%s"''' % (title, "Michael Gogins", year, notes, "Electroacoustic", master_filename, mp3_filename)
        print('mp3_command:            ' + mp3_command)
        os.system(mp3_command)
        sox_flac_command = '''sox -S "%s" "%s"''' % (master_filename, flac_filename)
        print('sox_flac_command:       ' + sox_flac_command)
        os.system(sox_flac_command)
        mp4_command = '''%s -r 1 -i "%s" -i "%s" -codec:a aac -strict -2 -b:a 384k -c:v libx264 -b:v 500k "%s"''' % ('ffmpeg', os.path.join(cwd, spectrogram_filename), os.path.join(cwd, master_filename), os.path.join(cwd, mp4_filename))
        mp4_metadata =  '-metadata title="%s" ' % title
        mp4_metadata += '-metadata date="%s" ' % year
        mp4_metadata += '-metadata genre="%s" ' % notes
        mp4_metadata += '-metadata copyright="%s" ' % str_copyright
        mp4_metadata += '-metadata composer="%s" ' % author
        mp4_metadata += '-metadata artist="%s" ' % author
        mp4_metadata += '-metadata publisher="%s" ' % publisher
        mp4_command = '''"%s" -y -loop 1 -framerate 2 -i "%s" -i "%s" -c:v libx264 -preset medium -tune stillimage -crf 18 -codec:a aac -strict -2 -b:a 384k -r:a 48000 -shortest -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" %s "%s"''' % ('ffmpeg', os.path.join(cwd, spectrogram_filename), os.path.join(cwd, master_filename), mp4_metadata, os.path.join(cwd, mp4_filename))
        mp4_command = mp4_command.replace('\\', '/')
        print('mp4_command:            ' + mp4_command)
        os.system(mp4_command)
        os.system('del *wavuntagged.wav')
        os.system('{} {}'.format(soundfile_editor, master_filename))
        print("")
    except:
        print(traceback.format_exc())
        
def patch_csound_options(csd, output="soundfile"):
    global filename
    global csound_audio_output
    '''
    -odac --output
    -iadc --input
    -M --midi-device
    -Q
    '''
    print("output: " + output)
    options_start_index = csd.find("<CsOptions>") + len("<CsOptions>")
    options_end_index =  csd.find("</CsOptions>") 
    csd_top = csd[0:options_start_index]
    # Remove spaces between flags and values, so that flag and value are one token.
    csd_options = csd[options_start_index:options_end_index]
    csd_options = csd_options.replace(" -o ", " -o")
    csd_options = csd_options.replace(" --output ", " -output")
    print("csound_options: {}".format(csd_options))
    csd_bottom = csd[options_end_index:-1]
    csd_options_tokens = csd_options.split()
    for i in range(len(csd_options_tokens)):
        token = csd_options_tokens[i]
        print("token: {}".format(token))
        if token.startswith("-o"):
            if output == "soundfile":
                directory, basename = os.path.split(filename)
                rootname = os.path.splitext(basename)[0].split('.')[0]
                output_soundfile = rootname + ".wav"
                print("output_soundfile: " + output_soundfile)
                token = "-o" + output_soundfile
                print(type(token))
                print("new token: " + token)
                print("boo")
                csd_options_tokens[i] = token
            else:
                print("csound_audio_output: " + csound_audio_output)
                token = "-o" + csound_audio_output
                print("new token: " + token)
                csd_options_tokens[i] = token
    csd_options = " ".join(csd_options_tokens)
    csd = "".join([csd_top, "\n", csd_options, "\n", csd_bottom])
    return csd
        
def glade_exit_callback(future):
    glade_file = glade_filename_(filename)
    print("Finished editing {}.".format(glade_file))
    load_glade(glade_file)
    
def on_edit_gui_button_clicked(button):
    try:
        print(button.get_label())
        glade_file = glade_filename_(filename)
        if os.path.exists(glade_file) == False:
            with open(glade_file, "wt") as file:
                print("Writing {} to {}.".format(glade_file, glade_controls_template))
                file.write(glade_controls_template)
        print("glade_file: {}".format(glade_file))    
        pool = concurrent.futures.ThreadPoolExecutor(max_workers=1)
        future_ = pool.submit(subprocess.call, "glade {}".format(glade_file), shell=True)
        future_.add_done_callback(glade_exit_callback)
    except:
        print(traceback.format_exc())
            
def on_destroy(source):
    csound.Stop()
    csound.Join()
    csound.Cleanup()
    csound.Reset()
    Gtk.main_quit()
    
global search_settings
search_settings = GtkSource.SearchSettings()
global search_context
search_context = None
    
'''
Activating the search entry (i.e. pressing [Enter]) always
begins a new search based on current settings. Subsequent 
searches and replacements continue to use these settings.
'''
def on_search_entry_activate(widget):
    global search_settings
    global search_context
    try:
        print(widget)
        search_text = widget.get_text()
        print("Search for: {}".format(widget.get_text()))
        buffer = code_editor.get_buffer()
        search_insert = buffer.get_iter_at_mark(buffer.get_insert())
        search_settings.set_wrap_around(True)
        search_settings.set_case_sensitive(check_button_case_sensitive.get_active())
        search_settings.set_at_word_boundaries(check_button_whole_word.get_active())
        search_settings.set_search_text(search_text)
        search_context = GtkSource.SearchContext.new(buffer, search_settings)
        match, start, end = search_context.forward(search_insert)
        # print("match: {} start: {} end: {}".format(match, start.get_offset(), end.get_offset()))
        if match:
            buffer.place_cursor(start)
            buffer.move_mark(buffer.get_selection_bound(), end)
            code_editor.scroll_to_mark(buffer.get_insert(), 0.25, True, 0.5, 0.5)
        else:
            print("search: {} not found.".format(search_settings.get_search_text()))
            
    except:
        print(traceback.format_exc())
    
def on_search_button_clicked(widget):
    global search_settings
    global search_context
    try:
        buffer = code_editor.get_buffer()
        search_insert = buffer.get_iter_at_mark(buffer.get_insert())
        search_insert.forward_chars(1)
        match, start, end = search_context.forward(search_insert)
        # print("match: {} start: {} end: {}".format(match, start.get_offset(), end.get_offset()))
        if match:
            buffer.place_cursor(start)
            buffer.move_mark(buffer.get_selection_bound(), end)
            code_editor.scroll_to_mark(buffer.get_insert(), 0.25, True, 0.5, 0.5)
        else:
            print("search: {} not found.".format(search_settings.get_search_text()))
    except:
        print(traceback.format_exc())
        
def on_replace_button_clicked(widget):
    global search_settings
    global search_context
    try:
        buffer = code_editor.get_buffer()
        oldsel = buffer.get_selection_bounds()
        search_insert = buffer.get_iter_at_mark(buffer.get_insert())
        #search_insert.forward_chars(1)
        match, start, end = search_context.forward(search_insert)
        # print("match: {} start: {} end: {}".format(match, start.get_offset(), end.get_offset()))
        if match:
            buffer.place_cursor(start)
            buffer.move_mark(buffer.get_selection_bound(), end)
            code_editor.scroll_to_mark(buffer.get_insert(), 0.25, True, 0.5, 0.5)
        newsel = buffer.get_selection_bounds()
        # Only replace if there is an already-selected match at the cursor
        if (match and oldsel and oldsel[0].equal(newsel[0]) and
                oldsel[1].equal(newsel[1])):
            search_context.replace(newsel[0], newsel[1], replacement_entry.get_text(), -1)
            search_insert = buffer.get_iter_at_mark(buffer.get_insert())
            search_insert.forward_chars(1)
            match, start, end = search_context.forward(search_insert)
            # print("match: {} start: {} end: {}".format(match, start.get_offset(), end.get_offset()))
            if match:
                buffer.place_cursor(start)
                buffer.move_mark(buffer.get_selection_bound(), end)
                code_editor.scroll_to_mark(buffer.get_insert(), 0.25, True, 0.5, 0.5)
            else:
                print("replace: {} not found.".format(search_settings.get_search_text()))
    except:
        print(traceback.format_exc())

def on_replace_all_button_clicked(widget):
    global search_settings
    global search_context
    try:
        buffer = code_editor.get_buffer()
        saved_insert = buffer.create_mark(
            None, buffer.get_iter_at_mark(buffer.get_insert()), True)
        search_context.replace_all(replacement_entry.get_text(), -1)
        if not saved_insert.get_deleted():
            buffer.place_cursor(buffer.get_iter_at_mark(saved_insert))
            code_editor.scroll_to_mark(
                buffer.get_insert(), 0.25, True, 0.5, 0.5)
    except:
        print(traceback.format_exc())

def on_apply_scheme_button(widget):
    scheme = style_scheme.get_style_scheme()
    code_editor.get_buffer().set_style_scheme(scheme)
    
def on_initialize_web_extensions(web_context):
    print("on_initialize_web_extensions: {}".format(web_context))
    web_context.set_web_extensions_directory("/home/mkg/csound-extended/playpen/")
    user_data_ = GLib.Variant.new_uint64(int(csound.GetCsound()))
    user_data_.ref_sink()
    web_context.set_web_extensions_initialization_user_data(user_data_)

main_window = builder.get_object("main_window")
main_window.connect("destroy", on_destroy)
html_window = builder.get_object("html_window")
code_editor = builder.get_object("code_editor")
code_editor.override_color(Gtk.StateFlags.NORMAL, Gdk.RGBA(255/255, 160/255, 122/255, 1.0))
controls_layout = builder.get_object("controls_layout")
webview = WebKit2.WebView() 
# Set the directory from which to load extensions.
web_context = webview.get_context()
web_context.connect("initialize-web-extensions", on_initialize_web_extensions)
# As this program runs locally, we authorize many things.
webview_settings = webview.get_settings()
webview_settings.set_allow_file_access_from_file_urls(True)
webview_settings.set_allow_universal_access_from_file_urls(True)
webview_settings.set_enable_developer_extras(True)
webview_settings.set_enable_javascript(True)
webview_settings.set_enable_webaudio(True)
webview_settings.set_enable_webgl(True)
webview_settings.set_media_playback_requires_user_gesture(False)
webview.load_uri("http://csound.com") 
html_window.add(webview)
help_window = builder.get_object("help_window")
helpview = WebKit2.WebView() 
try:
    with open("README.md", 'r') as file:
        readme_md = file.read()
        readme_html = markdown.markdown(readme_md)
        helpview.load_html(readme_html)
except:
    traceback.print(exc())
help_window.add(helpview)
main_window.resize(4 * 800, 3 * 800)
new_button = builder.get_object("new_button")
new_button.connect("clicked", on_new_button_clicked)
open_button = builder.get_object("open_button")
open_button.connect("clicked", on_open_button_clicked)
save_button = builder.get_object("save_button")
save_button.connect("clicked", on_save_button_clicked)
save_as_button = builder.get_object("save_as_button")
save_as_button.connect("clicked", on_save_as_button_clicked)
edit_gui_button = builder.get_object("edit_gui_button")
edit_gui_button.connect("clicked", on_edit_gui_button_clicked)
play_audio_button = builder.get_object("play_audio_button")
play_audio_button.connect("clicked", on_play_audio_button_clicked)
render_soundfile_button = builder.get_object("render_soundfile_button")
render_soundfile_button.connect("clicked", on_render_soundfile_button_clicked)
stop_button = builder.get_object("stop_button")
stop_button.connect("clicked", on_stop_button_clicked)
search_entry = builder.get_object("search_entry")
search_entry.connect("activate", on_search_entry_activate)
replacement_entry = builder.get_object("replacement_entry")
apply_scheme_button = builder.get_object("apply_scheme_button")
apply_scheme_button.connect("activate", on_apply_scheme_button)
apply_scheme_button.connect("clicked", on_apply_scheme_button)
style_scheme = builder.get_object("style_scheme")
style_scheme_manager = GtkSource.StyleSchemeManager().get_default()
code_editor_scheme = style_scheme_manager.get_scheme(editor_scheme)
scheme_ids = style_scheme_manager.get_scheme_ids()
for scheme_id in scheme_ids:
    print("scheme_id: {}".format(scheme_id))
print("code_editor_scheme: {}".format(code_editor_scheme))
code_editor.get_buffer().set_style_scheme(code_editor_scheme)
language_manager = GtkSource.LanguageManager()
search_context = GtkSource.SearchContext()
search_settings = GtkSource.SearchSettings()
search_button = builder.get_object("search_button")
search_button.connect("clicked", on_search_button_clicked)
replace_button = builder.get_object("replace_button")
replace_button.connect("clicked", on_replace_button_clicked)
replace_all_button = builder.get_object("replace_all_button")
replace_all_button.connect("clicked", on_replace_all_button_clicked)
check_button_case_sensitive = builder.get_object("check_button_case_sensitive")
check_button_whole_word = builder.get_object("check_button_whole_word")

def perform():
    csound.Perform()

main_window.show_all() 

if len(sys.argv) > 1:
    filename = sys.argv[1]
    load_piece(filename)
Gtk.main()