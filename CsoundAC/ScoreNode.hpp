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
#pragma once

#include "Platform.hpp"
#ifdef SWIG
%module CsoundAC
%{
#include "Node.hpp"
#include "Score.hpp"
#include <Eigen/Dense>
%}
#else
#include "Node.hpp"
#include "Score.hpp"
#include <Eigen/Dense>
#endif

namespace csound
{
/**
 * Node class that produces events from the contained score,
 * which can be built up programmatically or imported from a standard MIDI file.
 */
class SILENCE_PUBLIC ScoreNode :
    public Node
{
protected:
    Score score;
public:
    /**
     * If not 0, the score is rescaled to this duration.
     */
    double duration;
    std::string importFilename;
    ScoreNode();
    virtual ~ScoreNode();
    virtual void generate(Score &collectingScore);
    virtual Score &getScore();
};

}