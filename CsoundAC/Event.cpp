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
#if defined(_MSC_VER) && !defined(__GNUC__)
#pragma warning (disable:4786)
#endif
#include "Event.hpp"
#include "Midifile.hpp"

namespace csound
{
int Event::SORT_ORDER[] = {
    Event::TIME,
    Event::INSTRUMENT,
    Event::KEY,
    Event::DURATION,
    Event::VELOCITY,
    Event::PAN,
    Event::DEPTH,
    Event::HEIGHT,
    Event::PHASE,
    Event::PITCHES,
    Event::STATUS,
    Event::HOMOGENEITY
};

const char *Event::labels[] = {
    "Time",
    "Duration",
    "Status",
    "Instrument",
    "Key",
    "Velocity",
    "Pan",
    "Depth",
    "Height",
    "Phase",
    "PitchClassSet",
    "Homogeneity"
};

bool SILENCE_PUBLIC operator < (const Event &a, const Event &b)
{
    for(int i = 0; i < Event::ELEMENT_COUNT; i++)
    {
        if(a[Event::SORT_ORDER[i]] < b[Event::SORT_ORDER[i]])
        {
            return true;
        }
        else if(a[Event::SORT_ORDER[i]] > b[Event::SORT_ORDER[i]])
        {
            return false;
        }
    }
    return false;
}

Event::Event()
{
    initialize();
}

Event::Event(const Event &a)
{
    *this = a;
}

Event::Event(std::string text)
{
    initialize();
    std::istringstream stream(text);
    std::vector<double> buffer;
    double f;
    while(!stream.eof())
    {
        stream >> f;
        buffer.push_back(f);
    }
    resize(buffer.size());
    *this = buffer;
}

Event::Event(const Eigen::VectorXd &a)
{
    *this = a;
}

Event::Event(const std::vector<double> &a)
{
    *this = a;
}

Event::Event(double time, double duration, double status, double instrument, double key, double velocity, double phase, double pan, double depth, double height, double pitches)
{
    initialize();
    set(time, duration, status, instrument, key, velocity, phase, pan, depth, height, pitches);
}

Event::~Event()
{
}

void Event::initialize()
{
    resize(ELEMENT_COUNT);
    operator *= (0);
    (*this)[HOMOGENEITY] = 1.0;
}

bool Event::isNoteOn() const
{
    return (Conversions::round(getStatusNumber()) == MidiFile::CHANNEL_NOTE_ON) &&
           (getVelocity() > 0.0);
}

bool Event::isNoteOff() const
{
    return (Conversions::round(getStatusNumber()) == MidiFile::CHANNEL_NOTE_OFF) ||
           ((Conversions::round(getStatusNumber()) == MidiFile::CHANNEL_NOTE_ON) &&
            (getVelocity() <= 0.0));
}

bool Event::isNote() const
{
    return isNoteOn() || isNoteOff();
}

bool Event::isMatchingNoteOff(const Event& event) const
{
    if(!isNoteOn())
    {
        return false;
    }
    if(!event.isNoteOff())
    {
        return false;
    }
    if(Conversions::round((*this)[INSTRUMENT]) != Conversions::round(event[INSTRUMENT]))
    {
        return false;
    }
    if(Conversions::round((*this)[KEY]) != Conversions::round(event[KEY]))
    {
        return false;
    }
    return true;
}

bool Event::isMatchingEvent(const Event& event) const
{
    if(Conversions::round((*this)[INSTRUMENT]) != Conversions::round(event[INSTRUMENT]))
    {
        return false;
    }
    return true;
}

void Event::set(double time, double duration, double status, double instrument, double key, double velocity, double phase, double pan, double depth, double height, double pitches)
{
    (*this)[TIME] = time;
    (*this)[DURATION] = duration;
    (*this)[STATUS] = status;
    (*this)[INSTRUMENT] = instrument;
    (*this)[KEY] = key;
    (*this)[VELOCITY] = velocity;
    (*this)[PHASE] = phase;
    (*this)[PAN] = pan;
    (*this)[DEPTH] = depth;
    (*this)[HEIGHT] = height;
    (*this)[PITCHES] = pitches;
}

void Event::setMidi(double time, char status, char key, char velocity)
{
    (*this)[TIME] = time;
    (*this)[STATUS] = (status & 0xf0);
    setChannel(status & 0xf);
    (*this)[DURATION] = INDEFINITE;
    (*this)[KEY] = key;
    (*this)[VELOCITY] = velocity;
}

int Event::getMidiStatus() const
{
    int status = getStatusNumber();
    status |= (getChannel() % 16);
    return status;
}

int Event::getStatusNumber() const
{
    return int(Conversions::round((*this)[STATUS]));
}

double Event::getStatus() const
{
    return (*this)[STATUS];
}

int Event::getChannel() const
{
    return int(Conversions::round((*this)[INSTRUMENT] - 1));
}

void Event::setChannel(int channel)
{
    setInstrument(channel + 1);
}

double Event::getInstrument() const
{
    return (*this)[INSTRUMENT];
}

double Event::getTime() const
{
    return (*this)[TIME];
}

double Event::getDuration() const
{
    return (*this)[DURATION];
}

double Event::getOffTime() const
{
    if((*this)[DURATION] < 0)
    {
        return (*this)[TIME] + INDEFINITE;
    }
    else
    {
        return (*this)[TIME] + (*this)[DURATION];
    }
}

void Event::setOffTime(double offTime)
{
    setDuration(offTime - getTime());
}

int Event::getKeyNumber() const
{
    return int(Conversions::round((*this)[KEY]));
}

double Event::getKey() const
{
    return (*this)[KEY];
}

double Event::getFrequency() const
{
    return Conversions::midiToHz(getKey());
}

int Event::getVelocityNumber() const
{
    return int(Conversions::round((*this)[VELOCITY]));
}

double Event::getVelocity() const
{
    return (*this)[VELOCITY];
}

double Event::getPhase() const
{
    return (*this)[PHASE];
}

double Event::getPan() const
{
    return (*this)[PAN];
}

void Event::dump(std::ostream &stream)
{
    for(size_t i = 0, n = size(); i < n; ++i)
    {
        stream << (*this)[i];
        stream << " ";
    }
    stream << std::endl;
}

Event &Event::operator = (const Event &a)
{
    Eigen::VectorXd::operator=(a);
    properties = a.properties;
    process = a.process;
    return *this;
}

Event &Event::operator = (const Eigen::VectorXd &a)
{
    Eigen::VectorXd::operator=(a);
    return *this;
}

double Event::getLeftGain() const
{
    return Conversions::leftPan(getPan());
}

double Event::getRightGain() const
{
    return Conversions::rightPan(getPan());
}

double Event::getGain() const
{
    return Conversions::midiToGain(getVelocity());
}

double Event::getKey_tempered(double tonesPerOctave) const
{
    if(tonesPerOctave == 0.0)
    {
        return getKey();
    }
    else
    {
        double tones = Conversions::round(Conversions::midiToOctave(getKey()) * tonesPerOctave);
        return Conversions::octaveToMidi(tones / tonesPerOctave, false);
    }
}

std::string Event::getProperties() const {
    std::string result = "";
    if (properties.empty() == false) {
        result += "\"";
        for (auto pair : properties) {
            result += "'" + pair.first + "'='" + pair.second + "', ";
         }
        result += "\"";
    }
    return result;
}

std::string Event::toBlueIStatement(double tonesPerOctave) const 
{
    char buffer[0x1000];
    /** 
     * Csound dimensions for athenaCL are:
     * i_instrument = p1
     * i_time = p2
     * i_duration = p3
     * i_dbspa = p4
     * i_pch = p5
     * optional properties = p12, printed as a string 
     * ("'name'='value', ['name'='value']")
     */
    double velocity = getVelocity();
    double dbsp = velocity / 127. * 90.;
    double midi_key = getKey_tempered(tonesPerOctave) - 60.;
    double octave = std::floor(Conversions::midiToOctave(midi_key));
    double pitch_class = std::fmod(midi_key, 12.);
    double pch = octave + pitch_class;
    snprintf(buffer, sizeof(buffer), "i %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %s\n",
            getInstrument(),
            getTime(),
            getDuration(),
            dbsp,
            pch,
            getPan(),
            getProperties().c_str());
    return buffer;
}

std::string Event::toCsoundIStatement(double tonesPerOctave) const
{
    char buffer[0x1000];
    /** 
     * Csound dimensions now are:
     * i_instrument = p1
     * i_time = p2
     * i_duration = p3
     * i_midi_key = p4
     * i_midi_velocity = p5
     * k_space_front_to_back = p6 ; Ambisonic X
     * k_space_left_to_right = p7 ; Ambisonic Y
     * k_space_bottom_to_top = p8; Ambisonic Z
     * i_phase = p9
     * i_pitches = p10
     * i_homogeneity = p11    
     * optional properties = p12, printed as a string 
     * ("'name'='value', ['name'='value']")
     */
    snprintf(buffer, sizeof(buffer), "i %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %s\n",
            getInstrument(),
            getTime(),
            getDuration(),
            getKey_tempered(tonesPerOctave),
            getVelocity(),
            getDepth(),
            getPan(),
            getHeight(),
            getPhase(),
            getPitches(),
            (*this)[HOMOGENEITY],
            getProperties().c_str());
    return buffer;
}

std::string Event::toCsoundIStatementHeld(int tag, double tempering) const
{
    char buffer[0x1000];
    double octave = Conversions::midiToOctave(getKey());
    if(tempering != 0.0)
    {
        octave = Conversions::temper(octave, tempering);
    }
    snprintf(buffer, sizeof(buffer), "i %d.%d %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %s\n",
            (int) Conversions::round(getInstrument()),
            tag,
            getTime(),
            std::fabs(getDuration()) * -1.0,
            Conversions::octaveToMidi(octave, false),
            getVelocity(),
            getDepth(),
            getPan(),
            getHeight(),
            getPhase(),
            getPitches(),
            (*this)[HOMOGENEITY],
            getProperties().c_str());
    return buffer;
}

std::string Event::toCsoundIStatementRelease(int tag, double tempering) const
{
    char buffer[0x1000];
    double octave = Conversions::midiToOctave(getKey());
    if(tempering != 0.0)
    {
        octave = Conversions::temper(octave, tempering);
    }
    snprintf(buffer, sizeof(buffer), "i %d.%d %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %-1.7g %s\n",
            (int) Conversions::round(getInstrument()),
            tag,
            getTime(),
            getDuration(),
            Conversions::octaveToMidi(octave, false),
            getVelocity(),
            getDepth(),
            getPan(),
            getHeight(),
            getPhase(),
            getPitches(),
            (*this)[HOMOGENEITY],
            getProperties().c_str());
    return buffer;
}

std::string Event::toString() const
{
    char buffer[0x100];
    // Fields have more digits than may seem necessary, because generative 
    // algorithms may create Events that span a very broad range.
    snprintf(buffer, sizeof(buffer), "Event: t%9.3f d%9.3f s%4.0f i%8.2f k%8.2f v%8.2f y%8.2f pcs%8.2f %s",
            getTime(),
            getDuration(),
            getStatus(),
            getInstrument(),
            getKey(),
            getVelocity(),
            getPan(),
            getPitches(),
            getProperties().c_str());
    return buffer;
}

void Event::setStatus(double value)
{
    (*this)[STATUS] = value;
}

void Event::setInstrument(double value)
{
    (*this)[INSTRUMENT] = value;
}

void Event::setTime(double value)
{
    (*this)[TIME] = value;
}

void Event::setDuration(double value)
{
    (*this)[DURATION] = value;
}

void Event::setKey(double value)
{
    (*this)[KEY] = value;
}

void Event::setFrequency(double value)
{
    (*this)[KEY] = Conversions::hzToMidi(value, false);
}

void Event::setVelocity(double value)
{
    (*this)[VELOCITY] = value;
}

void Event::setPan(double value)
{
    (*this)[PAN] = value;
}

void Event::setPhase(double value)
{
    (*this)[PHASE] = value;
}

double Event::getAmplitude() const
{
    return Conversions::midiToAmplitude(getVelocity());
}

void Event::setAmplitude(double amplitude)
{
    setVelocity(Conversions::amplitudeToMidi(amplitude));
}

bool Event::isMidiEvent() const
{
    return (*this)[STATUS] >= MidiFile::CHANNEL_NOTE_OFF;
}

void Event::conformToPitchClassSet()
{
    double midiKey = (*this)[KEY];
    int pitchClass = int(Conversions::midiToPitchClass(midiKey));
    double octave = Conversions::midiToRoundedOctave(midiKey);
    int pitchClassSet = int(Conversions::round((*this)[PITCHES])) % 4096;
    pitchClass = (int) Conversions::findClosestPitchClass(pitchClassSet, pitchClass);
    (*this)[KEY] = Conversions::octaveToMidi(octave, true) + Conversions::pitchClassToMidi(pitchClass);
}

void Event::temper(double divisionsPerOctave)
{
    double octave = Conversions::midiToOctave(getKey());
    octave = Conversions::temper(octave, divisionsPerOctave);
    setKey(Conversions::octaveToMidi(octave, true));
}

double Event::getDepth() const
{
    return (*this)[DEPTH];
}

void Event::setDepth(double depth)
{
    (*this)[DEPTH] = depth;
}

double Event::getHeight() const
{
    return (*this)[HEIGHT];
}

void Event::setHeight(double height)
{
    (*this)[HEIGHT] = height;
}

double Event::getPitches() const
{
    return (*this)[PITCHES];
}

void Event::setPitches(double pitches)
{
    (*this)[PITCHES] = pitches;
}

std::string Event::getProperty(std::string name)
{
    if(properties.find(name) != properties.end())
    {
        return properties[name];
    }
    else
    {
        return "";
    }
}

void Event::setProperty(std::string name, std::string value)
{
    properties[name] = value;
}

void Event::removeProperty(std::string name)
{
    properties.erase(name);
}

void Event::clearProperties()
{
    properties.clear();
}

void Event::createNoteOffEvent(Event &event) const
{
    event = *this;
    event.setStatus(128);
    event.setTime(event.getTime() + event.getDuration());
    event.setDuration(0.0);
}

bool &Event::correct_negative_durations() {
    static bool correct_negative_durations_ = true;
    return correct_negative_durations_;
}

void Event::correct_negative_duration() {
    if (correct_negative_durations() == false) {
        return;
    } 
    auto current_duration = getDuration();
    if (current_duration >= 0.) {
        return;
    }
    auto current_onset = getTime();
    // Recall that current_duration here is always negative!
    auto corrected_onset = current_onset + current_duration;
    setTime(corrected_onset);
    auto corrected_duration = std::fabs(current_duration);
    setDuration(corrected_duration);
}

bool getCorrectNegativeDurations() {
    return Event::correct_negative_durations();
}

void setCorrectNegativeDurations(bool do_correct) {
    Event::correct_negative_durations() = do_correct;
}

double &Event::get(size_t index) {
    return (*this)[index];
}

}
