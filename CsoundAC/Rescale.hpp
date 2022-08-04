#ifndef RESCALE_HPP_INCLUDED
#define RESCALE_HPP_INCLUDED
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
#include "ScoreNode.hpp"
%}
#else
#include "ScoreNode.hpp"
#endif

namespace csound
{
/**
 * Rescales all child events to fit a bounding hypercube in music space.
 * No, some, or all dimensions may be rescaled to fit the minimum alone,
 * the range alone, or both the minimum and the range.
 */
class SILENCE_PUBLIC Rescale :
    public ScoreNode
{
    static bool initialized;
    static std::map<std::string, size_t> dimensions;
public:
    Rescale();
    virtual ~Rescale();
    virtual void initialize();
    virtual void transform(Score &score);
    virtual void setRescale(int dimension, bool rescaleMinimum, bool rescaleRange, double targetMinimum, double targetRange);
    virtual void getRescale(int dimension, bool &rescaleMinimum, bool &rescaleRange, double &targetMinimum, double &targetRange);
};

}

#endif
