#ifndef EVENT_HPP_INCLUDED
#define EVENT_HPP_INCLUDED
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
#include <algorithm>
#include <Eigen/Dense>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#endif

namespace csound
{
/**
 * Represents an event in music space, such as a note of definite duration,
 * a MIDI-like "note on" or "note off" event, or a MIDI-like control event.
 * Fields have the same semantics as MIDI with some differences.
 * All fields are floats; status is stored separately from channel;
 * channel can have any positive value; spatial location in X, Y, and Z are stored;
 * phase in radians is stored; and pitch-class set is stored.
 * <p>
 * Events can be multiplied (matrix dot product) with the local coordinate system
 * of a Node or transform to translate, scale, or rotate them in any or all dimensions
 * of music space.
 * <p>
 * Events usually are value objects, not references.
 * <p>
 * Silence Events translate to Csound score statements ("i" statements),
 * but they are always real-time score statements at time 0, suitable
 * for use with Csound's -L or line event option.
 */

//template class __declspec(dllexport) std::allocator<double>;
//template class __declspec(dllexport) std::vector<double>;
//template class __declspec(dllexport) std::allocator<char>;
//template class __declspec(dllexport) std::basic_string<char>;
//template class __declspec(dllexport) Eigen::Matrix< double, Eigen::Dynamic, 1 >;
//template class __declspec(dllexport) Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic >;
//template class __declspec(dllexport) Eigen::Matrix< double, 13, 1 >;
//template class __declspec(dllexport) Eigen::Matrix< double, 13, 13 >;

class Score;

class SILENCE_PUBLIC Event :
    public Eigen::VectorXd
{
public:
    typedef enum
    {
        TIME = 0,
        DURATION,
        STATUS,
        INSTRUMENT,
        KEY,
        VELOCITY,
        PHASE,
        PAN,
        DEPTH,
        HEIGHT,
        PITCHES,
        HOMOGENEITY,
        ELEMENT_COUNT
    } Dimensions;
    enum
    {
        INDEFINITE = 16384
    };
    std::map<std::string,std::string> properties;
    Event();
    Event(const Event &a);
    Event(std::string text);
    Event(const Eigen::VectorXd &a);
    Event(double time, double duration, double status, double instrument, double key, double velocity, double phase, double pan, double depth, double height, double pitches);
    Event(const std::vector<double> &v);
    virtual ~Event();
    virtual void initialize();
    virtual bool isMidiEvent() const;
    virtual bool isNoteOn() const;
    virtual bool isNoteOff() const;
    virtual bool isNote() const;
    virtual bool isMatchingNoteOff(const Event& event) const;
    virtual bool isMatchingEvent(const Event& event) const;
    virtual void set(double time, double duration, double status, double instrument, double key, double velocity, double phase=0, double pan=0, double depth=0, double height=0, double pitches=4095);
    virtual void setMidi(double time, char status, char key, char velocity);
    virtual int getMidiStatus() const;
    virtual int getStatusNumber() const;
    virtual double getStatus() const;
    virtual void setStatus(double status);
    /**
     * MIDI channel numbers are 0-based, Csound instrument numbers are 
     * 1-based. Returns the Csound instrument number minus 1.
     */
    virtual int getChannel() const;
    /**
     * MIDI channel numbers are 0-based, Csound instrument numbers are 
     * 1-based. Sets the Csound instrument number to the channel plus 1.
     */
    virtual void setChannel(int channel);
    /**
     * MIDI channel numbers are 0-based, Csound instrument numbers are 
     * 1-based. Returns the Csound instrument number.
     */
    virtual double getInstrument() const;
    /**
     * MIDI channel numbers are 0-based, Csound instrument numbers are 
     * 1-based. Sets the Csound instrument number.
     */
    virtual void setInstrument(double instrument);
    virtual double getTime() const;
    virtual void setTime(double time);
    virtual double getDuration() const;
    virtual void setDuration(double duration);
    virtual double getOffTime() const;
    virtual void setOffTime(double offTime);
    virtual int getKeyNumber() const;
    virtual double getKey() const;
    virtual double getKey_tempered(double tonesPerOctave) const;
    virtual void setKey(double key);
    virtual double getFrequency() const;
    virtual void setFrequency(double frequency);
    virtual int getVelocityNumber() const;
    virtual double getVelocity() const;
    virtual void setVelocity(double velocity);
    virtual double getGain() const;
    virtual double getPan() const;
    virtual void setPan(double pan);
    virtual double getDepth() const;
    virtual void setDepth(double depth);
    virtual double getHeight() const;
    virtual void setHeight(double height);
    virtual double getPitches() const;
    virtual void setPitches(double pitches);
    virtual double getAmplitude() const;
    virtual void setAmplitude(double amplitude);
    virtual double getPhase() const;
    virtual void setPhase(double phase);
    virtual double getLeftGain() const;
    virtual double getRightGain() const;
    virtual void dump(std::ostream &stream);
    virtual std::string toString() const;
    /** Returns a Csound score statement suitable for use by athenaCL:
     * insno, time, duration, dbsp, pch, pan.
     */
    virtual std::string toBlueIStatement(double tempering = 12.0) const;
    virtual std::string toCsoundIStatement(double tempering = 12.0) const;
    virtual std::string toCsoundIStatementHeld(int tag, double tempering = 12.0) const;
    virtual std::string toCsoundIStatementRelease(int tag, double tempering = 12.0) const;
    virtual void conformToPitchClassSet();
    virtual void temper(double divisionsPerOctave);
    virtual std::string getProperty(std::string name);
    virtual void setProperty(std::string name, std::string value);
    virtual void removeProperty(std::string nameO);
    virtual void clearProperties();
    virtual void createNoteOffEvent(Event &event) const;
    /**
     * Process the data in this; called on all Events in a Score as the final
     * state of processing in ScoreModel. Typically, "process" is a closure
     * that contains references to any other data required to process this.
     * Example: put a Chord in the process closure, and when it is called,
     * conform the pitch of this Event to the Chord. The Score is sorted
     * before this is called.
     */
    std::function<void(csound::Score &, csound::Event &)> process;
    virtual Event &operator = (const Event &a);
#if __cpplusplus >= 201103L
    virtual Event &operator = (Event &&a) = default;
#endif
    virtual Event &operator = (const Eigen::VectorXd &a);
#ifndef SWIG
    /** 
     * Returns any properties of this as a string consisting of 
     * "key"="value" pairs in CSV format.
     */
    virtual std::string getProperties() const;
    /**
     * Gets, or sets by reference, a global flag that determines whether 
     * Events have all negative durations adjusted to positive durations with 
     * a new onset time. The default is to correct all negative durations.
     */
    virtual bool &correct_negative_durations();
    /**
     * If the duration of this Event is negative, first adds it to the 
     * Event's onset time, i.e. moves the onset to match the beginning 
     * of the duration, then makes the duration positive. This only has 
     * an effect if correct_negative_duratiuons() returns true.
     */
    virtual void correct_negative_duration();
    static int SORT_ORDER[];
    static const char *labels[];
#endif
};

bool SILENCE_PUBLIC operator < (const Event& a, const Event &b);

}

#endif


