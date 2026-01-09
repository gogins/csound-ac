#pragma once
/*
 * C S O U N D
 *
 * L I C E N S E
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "Platform.hpp"
#ifdef SWIG
%module CsoundAC
%{
#include "Conversions.hpp"
#include "Event.hpp"
#include "Score.hpp"
#include "ScoreNode.hpp"
#include <algorithm>
#include <Eigen/Dense>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
%}
%include "std_string.i"
%include "std_vector.i"
%template(EventVector) std::vector<csound::Event>;
#else
#include "Conversions.hpp"
#include "Event.hpp"
#include "Score.hpp"
#include "ScoreNode.hpp"
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace cmask {
using namespace std;
// Andre Bartetzki's original code is bodily included into this file 
// rather than adding his code to the CsoundAC build system.
#include "../dependencies/cmask/source/globals.h"
#if !defined(NL)
    #define NL "\n"
#endif
#include "../dependencies/cmask/source/event.h"
#include "../dependencies/cmask/source/field.h"
#include "../dependencies/cmask/source/fileio.h"
#include "../dependencies/cmask/source/gen.h"
#include "../dependencies/cmask/source/items.h"
#include "../dependencies/cmask/source/mask.h"
#include "../dependencies/cmask/source/parser.h"
#include "../dependencies/cmask/source/quant.h"
#include "../dependencies/cmask/source/tables.h"
#include "../dependencies/cmask/source/utils.h"
#include "../dependencies/cmask/source/cmask.cpp"
#include "../dependencies/cmask/source/event.cpp"
#include "../dependencies/cmask/source/field.cpp"
#include "../dependencies/cmask/source/fileio.cpp"
#include "../dependencies/cmask/source/gen.cpp"
#include "../dependencies/cmask/source/items.cpp"
#include "../dependencies/cmask/source/mask.cpp"
#include "../dependencies/cmask/source/parser.cpp"
#include "../dependencies/cmask/source/quant.cpp"
#include "../dependencies/cmask/source/tables.cpp"
#include "../dependencies/cmask/source/utils.cpp"
};
#endif

namespace csound
{
/**
 * Uses the CMask library for tendency masks to generate events as a Csound 
 * score in the format determined by the CMask parameters text. The generated 
 * Csound score is also translated to a Silence score in the Silence music 
 * graph. This only works if at least the first 5 pfields in the Csound score 
 * (instrument number, time, duration, MIDI key, MIDI velocity) follow the 
 * Silence conventions declared in Event.hpp.
 *
 * Score literals in the CMask parameters are simply appended to the Csound 
 * score header in the Score object, and likewise are copied directly and 
 * without any processing to the translated Csound Score.
 * 
 * For documentation and examples for CMask, see [Andre Bartetzki's original 
 * documentation](https://www.bartetzki.de/de/software.html).
 */

SILENCE_PUBLIC class CMaskNode : public ScoreNode {
    public:
        virtual std::string getParametersText() const {
            return parameters_text;
        }
        virtual void setParametersText(const std::string &parameters_text_) {
            parameters_text = parameters_text_;
        }
        virtual void generateLocally() {
            cmask::frandinit();
            cmask::scanner scanner_;
            //std::cout << "CMask parameters: " << std::endl << parameters_text << std::endl;
            // Rather than monkey with the existing code,
            // we just use temporary files.
            char parameters_filename[0x100];
            std::tmpnam(parameters_filename);
            std::string score_filename = parameters_filename;
            score_filename.append(".sco");
            std::ofstream parameters_file(parameters_filename);
            parameters_file << parameters_text->data << std::endl;
            parameters_file.close();
            scanner_.scn(parameters_filename, const_cast<char *>(score_filename.c_str()));
            scanner_.analyze(); 
            std::ifstream score_file(score_filename);
            std::stringstream score_buffer;
            score_buffer << score_file.rdbuf();
            score_file.close();
            score_text = score_buffer.str();
            //std::cout << "CMask generated: " << std::endl << score_text << std::endl;
            std::remove(parameters_filename);
            std::remove(score_filename.c_str());
            translate_to_silence();
        }
        /**
         * Maps Silence score fields to Csound "i" statement pfields. As noted 
         * above, the CMask fields _must_ be configured to match the Silence 
         * conventions for Csound scores. The first 5 fields are required.
         * No more than 10 fields are used.
         *
         * i_instrument = p1
         * i_time = p2
         * i_duration = p3
         * i_midi_key = p4
         * i_midi_velocity = p5
         * k_space_front_to_back = p6 ; Ambisonic X
         * k_space_left_to_right = p7 ; Ambisonic Y
         * k_space_bottom_to_top = p8; Ambisonic Z
         * i_phase = p9
         * i_pitches = p10 ; Mason number
         */
        virtual void translate_to_silence() {
            score.clear();
            std::stringstream score_stream;
            score_stream << score_text;
            for (std::string line; std::getline(input, line); ) {
                std::stringstream line_stream(line);
                // All lines that don't begin with "i" are treated as 
                // Csound score literals and appended to the score header.
                if (line.begins_with("i") == false) {
                    score.appendToCsoundScoreHeader(line);
                    score.appendToCsoundScoreHeader("\n");
                 } else {
                    csound::Event event;
                    int index = 0;
                    std::string token;
                    while (line_stream >> token) {
                         switch (index) {
                            case 0:
                                event.setStatus(144);
                                break;
                            case 1:
                                event.setInstrument(std::stof(token));
                                break;
                            case 2:
                                event.setTime(std::stof(token));
                                break;
                            case 3:
                                event.setDuration(std::stof(token));
                                break;
                            case 4:
                                event.setKey(std::stof(token));
                                break;
                            case 6:
                                event.setDepth(std::stof(token));
                                break;
                            case 7:
                                event.setPan(std::stof(token));
                                break;
                            case 8:
                                event.setHeight(std::stof(token));
                                break;
                            case 9:
                                event.setPhase(std::stof(token));
                                break;
                            case 10:
                                event.setPitches(std::stof(token));
                                break;
                         }
                    }
                    score.append(event);
                }
            }
        }
     protected:
        std::string parameters_text;
        std::string score_text;
    }
};