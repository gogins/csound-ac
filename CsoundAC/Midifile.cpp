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
#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif
#include "Midifile.hpp"
#include "System.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <cstring>

using namespace std;

namespace csound
{
int MidiFile::readVariableLength(std::istream &stream)
{
    char c = 0;
    stream.get(c);
    int value = c;
    if ( c & 0x80 )
    {
        value &= 0x7f;
        do {
            stream.get(c);
            value = (value << 7) + (c & 0x7f);
        } while (c & 0x80);
    }
    return value;
}

void MidiFile::writeVariableLength(std::ostream &stream, int value)
{
    unsigned long buffer = value & 0x7f;
    while((value >>= 7) > 0)
    {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (value & 0x7f);
    }
    for(;;)
    {
        unsigned char c = (unsigned char) (buffer & 0xff);
        stream.put(c);
        if(buffer & 0x80)
        {
            buffer >>= 8;
        }
        else
        {
            return;
        }
    }
}

int MidiFile::toInt(int c1, int c2, int c3, int c4)
{
    int value = 0;
    value =                (c1 & 0xff);
    value = (value << 8) + (c2 & 0xff);
    value = (value << 8) + (c3 & 0xff);
    value = (value << 8) + (c4 & 0xff);
    return value;
}

short MidiFile::toShort(int c1, int c2)
{
    return ((c1 & 0xff ) << 8) + (c2 & 0xff);
}

int MidiFile::readInt(std::istream &stream)
{
    char c1, c2, c3, c4;
    stream.get(c1);
    stream.get(c2);
    stream.get(c3);
    stream.get(c4);
    return toInt(c1,c2,c3,c4);
}

void MidiFile::writeInt(std::ostream &stream, int value)
{
    stream.put((char)((0xff000000 & value) >> 24));
    stream.put((char)((0x00ff0000 & value) >> 16));
    stream.put((char)((0x0000ff00 & value) >>  8));
    stream.put((char)((0xf00000ff & value)      ));
}

short MidiFile::readShort(std::istream &stream)
{
    char c1, c2;
    stream.get(c1);
    stream.get(c2);
    return toShort(c1,c2);
}

void MidiFile::writeShort(std::ostream &stream, short value)
{
    stream.put((unsigned char)((0x0000ff00 & value) >>  8));
    stream.put((unsigned char)((0x000000ff & value)      ));
}

int MidiFile::chunkName(int a, int b, int c, int d)
{
    return (a << 24) + (b << 16) + (c << 8) + d;
}

Chunk::Chunk() :
    id(0),
    chunkSize(0),
    chunkSizePosition(0),
    chunkStart(0),
    chunkEnd(0)
{
}

Chunk::Chunk(const char *_id) : chunkSize(0), chunkSizePosition(0), chunkStart(0), chunkEnd(0)
{
    id = MidiFile::chunkName(_id[0], _id[1], _id[2], _id[3]);
}

Chunk::Chunk(const Chunk &a)
{
    *this = a;
}

Chunk::~Chunk()
{
}

Chunk &Chunk::operator = (const Chunk &a)
{
    if (this != &a) {
        id = a.id;
        chunkSize = a.chunkSize;
        chunkSizePosition = a.chunkSizePosition;
        chunkStart = a.chunkStart;
        chunkEnd = a.chunkEnd;
    }
    return *this;
}

void Chunk::read(std::istream &stream)
{
    int _id = MidiFile::readInt(stream);
    char idString[5];
    memcpy(idString, &id, 4);
    idString[4] = 0;
    char _idString[5];
    memcpy(_idString, &_id, 4);
    _idString[4] = 0;
    if(id != _id)
    {
        System::warn("Unexpected chunk id: %s (should be %s).\n", _idString, idString);
    }
    else
    {
        System::inform("Read chunk: %s\n", _idString);
    }
    chunkSize = MidiFile::readInt(stream);
}

void Chunk::write(std::ostream &stream)
{
    MidiFile::writeInt(stream, id);
    markChunkSize(stream);
    MidiFile::writeInt(stream, chunkSize);
    markChunkStart(stream);
}

void Chunk::markChunkSize(std::ostream &stream)
{
    chunkSizePosition = stream.tellp();
}

void Chunk::markChunkStart(std::ostream &stream)
{
    chunkStart = stream.tellp();
}

void Chunk::markChunkEnd(std::ostream &stream)
{
    chunkEnd = stream.tellp();
    chunkSize = chunkEnd - chunkStart;
    stream.seekp(chunkSizePosition);
    MidiFile::writeInt(stream, chunkSize);
    stream.seekp(chunkEnd);
}

MidiHeader::MidiHeader() : Chunk("MThd")
{
    clear();
}

MidiHeader::MidiHeader(const MidiHeader &a)
{
    *this = a;
}

MidiHeader::~MidiHeader()
{
    clear();
}

MidiHeader &MidiHeader::operator = (const MidiHeader &a)
{
    if (this != &a) {
        id = a.id;
        chunkSize = a.chunkSize;
        chunkSizePosition = a.chunkSizePosition;
        chunkStart = a.chunkStart;
        chunkEnd = a.chunkEnd;
        type = a.type;
        trackCount = a.trackCount;
        timeFormat = a.timeFormat;
    }
    return *this;
}

void MidiHeader::clear()
{
    type = 0;
    trackCount = 1;
    timeFormat = 480;
}

void MidiHeader::read(std::istream &stream)
{
    Chunk::read(stream);
    type = MidiFile::readShort(stream);
    trackCount = MidiFile::readShort(stream);
    timeFormat = MidiFile::readShort(stream);
}

void MidiHeader::write(std::ostream &stream)
{
    Chunk::write(stream);
    MidiFile::writeShort(stream, type);
    MidiFile::writeShort(stream, trackCount);
    MidiFile::writeShort(stream, timeFormat);
    Chunk::markChunkEnd(stream);
}

MidiEvent::MidiEvent() : ticks(0), time(0)
{
}

MidiEvent::MidiEvent(const MidiEvent &a)
{
    *this = a;
}

MidiEvent::~MidiEvent()
{
}

MidiEvent &MidiEvent::operator = (const MidiEvent &a)
{
    if (this != &a) {
        this->ticks = a.ticks;
        this->time = a.time;
        resize(a.size());
        for (size_t i = 0; i < a.size(); i++) {
            (*this)[i] = a[i];
        }
    }
    return *this;
}

void MidiEvent::writeOut(std::ostream &stream, const MidiFile &midiFile, int lastTick) const
{
    int deltaTicks = ticks - lastTick;
    MidiFile::writeVariableLength(stream, deltaTicks);
    //  Channel event.
    if(getStatusNybble() != 0xf) {
        for(size_t i = 0, n = size(); i < n; i++) {
            stream.put((*this)[i]);
        }
        // Meta event.
    } else {
        stream.put(getStatus());
        stream.put(getMetaType());
        MidiFile::writeVariableLength(stream, getMetaSize());
        for(size_t i = 0, n = getMetaSize(); i < n; i++) {
            stream.put((*this)[2 + i]);
        }
    }
}

int MidiEvent::getStatus() const
{
    return (*this)[0];
}

int MidiEvent::getStatusNybble() const
{
    return (*this)[0] & 0xf0;
}

int MidiEvent::getChannelNybble() const
{
    return (*this)[0] & 0x0f;
}

int MidiEvent::getKey() const
{
    return (*this)[1];
}

int MidiEvent::getVelocity() const
{
    return (*this)[2];
}

int MidiEvent::getMetaType() const
{
    if(getStatus() == MidiFile::META_EVENT)
    {
        return (*this)[1];
    }
    return -1;
}

unsigned char MidiEvent::getMetaData(int i) const
{
    return (*this)[i + 2];
}

size_t MidiEvent::getMetaSize() const
{
    return (size() - 2);
}

unsigned char MidiEvent::readByte(std::istream &stream)
{
    char c;
    stream.get(c);
    push_back((unsigned char) c);
    return (unsigned char) c;
}

bool MidiEvent::isChannelVoiceMessage() const
{
    if(getStatusNybble() < MidiFile::CHANNEL_NOTE_OFF) {
        return false;
    }
    if(getStatusNybble() > MidiFile::CHANNEL_PITCH_BEND) {
        return false;
    }
    return true;
}

bool MidiEvent::isNoteOn() const
{
    if(getStatusNybble() == MidiFile::CHANNEL_NOTE_ON) {
        if (getVelocity() > 0) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool MidiEvent::isNoteOff() const
{
    if(getStatusNybble() == MidiFile::CHANNEL_NOTE_OFF) {
        return true;
    }
    if(getStatusNybble() == MidiFile::CHANNEL_NOTE_ON) {
        if (getVelocity() == 0) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool MidiEvent::matchesNoteOffEvent(const MidiEvent &offEvent) const
{
    if(!isNoteOn()) {
        return false;
    }
    if(!offEvent.isNoteOff()) {
        return false;
    }
    if(getChannelNybble() != offEvent.getChannelNybble()) {
        return false;
    }
    if(getKey() != offEvent.getKey()) {
        return false;
    }
    if(!(offEvent.time >= time)) {
        return false;
    }
    return true;
}

std::string MidiEvent::toString() const
{
    std::ostringstream stream;
    char buffer[0x100];
    std::snprintf(buffer, sizeof(buffer), "MidiEvent:  tick: %5d  sec: %8.4f  status: %3d", ticks, time, getStatus());
    stream << buffer;
    for (size_t i = 1, n = size(); i < n; ++i) {
        std::snprintf(buffer, sizeof(buffer), " %3d", at(i));
        stream << buffer;
    }
    stream << std::endl;
    return stream.str();
}

MidiTrack::MidiTrack() : Chunk("MTrk")
{
}

MidiTrack::~MidiTrack()
{
}

MidiTrack &MidiTrack::operator = (const MidiTrack &a)
{
    if (this != &a) {
        id = a.id;
        chunkSize = a.chunkSize;
        chunkSizePosition = a.chunkSizePosition;
        chunkStart = a.chunkStart;
        chunkEnd = a.chunkEnd;
        resize(a.size());
        for (size_t i = 0; i < size(); i++) {
            (*this)[i] = a[i];
        }
    }
    return *this;
}

void MidiTrack::readIn(std::istream &stream, MidiFile &midiFile)
{
    Chunk::read(stream);
    for(int eventCount = 0; ; eventCount++)
    {
        MidiEvent midiEvent;
        midiEvent.readIn(stream, midiFile);
        push_back(midiEvent);
        if(stream.eof())
        {
            break;
        }
        if(midiEvent.getMetaType() == MidiFile::META_END_OF_TRACK)
        {
            break;
        }
    }
}

void MidiEvent::readIn(std::istream &stream, MidiFile &midiFile)
{
    ticks = MidiFile::readVariableLength(stream);
    midiFile.currentTick += ticks;
    double secondsPerTick = midiFile.tempoMap.getCurrentSecondsPerTick(midiFile.currentTick);
    if(secondsPerTick == -1)
    {
        secondsPerTick = midiFile.currentSecondsPerTick;
    }
    this->time = midiFile.currentTime = (midiFile.currentTime + (secondsPerTick * double(ticks)));
    int peeked = stream.peek();
    if(stream.eof())
    {
        std::cerr << "MIDI file incorrectly read EOF." << std::endl;
        return;
    }
    if(peeked < 0x80)
    {
        System::debug("Running status: %x\n", midiFile.lastStatus);
        push_back(midiFile.lastStatus);
    }
    else
    {
        readByte(stream);
        midiFile.lastStatus = getStatus();
        System::debug("Status:         %x\n", getStatus());
    }
    switch(getStatusNybble())
    {
    case MidiFile::CHANNEL_NOTE_OFF:
    case MidiFile::CHANNEL_NOTE_ON:
    case MidiFile::CHANNEL_KEY_PRESSURE:
    case MidiFile::CHANNEL_CONTROL_CHANGE:
    case MidiFile::CHANNEL_PITCH_BEND:
    {
        readByte(stream);
        readByte(stream);
    }
    break;
    case MidiFile::CHANNEL_PROGRAM_CHANGE:
    case MidiFile::CHANNEL_AFTER_TOUCH:
    {
        readByte(stream);
    }
    break;
    case MidiFile::SYSTEM_EXCLUSIVE:
    {
        switch(getStatus())
        {
        case MidiFile::SYSTEM_EXCLUSIVE:
            while(readByte(stream) != 0xf7);
            break;
        case MidiFile::META_EVENT:
        {
            //      Type.
            readByte(stream);
            //      Size.
            int n = MidiFile::readVariableLength(stream);
            //      Data.
            for(int i = 0; i < n; i++)
            {
                readByte(stream);
            }
             System::inform("Meta event %d (%d bytes):\n");
            switch(getMetaType())
            {
            case MidiFile::META_SET_TEMPO:
            {
                System::inform("set tempo\n");
                midiFile.microsecondsPerQuarterNote = (getMetaData(0) << 16) + (getMetaData(1) << 8) + getMetaData(2);
                midiFile.computeTimes();
            }
            break;
            case MidiFile::META_TIME_SIGNATURE:
            {
                System::inform("time signature\n");
                double numerator = getMetaData(0);
                double denominator = getMetaData(1);
                double clocksPerBeat = getMetaData(2);
                double thirtySecondNotesPerMidiQuarterNote = getMetaData(3);
                System::inform("numerator: %9.4f\n", numerator);
                System::inform("denominator: %9.4f\n", denominator);
                System::inform("clocksPerBeat: %9.4f\n", clocksPerBeat);
                System::inform("thirtySecondNotesPerMidiQuarterNote: %9.4f\n", thirtySecondNotesPerMidiQuarterNote);

            }
            break;
            case MidiFile::META_SEQUENCER_SPECIFIC:
                System::inform("sequencer specific\n");
                break;
            case MidiFile::META_END_OF_TRACK:
                System::inform("end of track\n");
                break;
            default:
                 System::inform("not handled\n");
                break;
            }
        }
        break;
        }
    }
    break;
    default:
    {
        int badStatus = getStatus();
        System::error("Error reading midi event: status == %d\n", badStatus);
    }
    break;
    }
    System::inform("%s\n", toString().c_str());
}

void MidiTrack::writeOut(std::ostream &stream, MidiFile &midiFile)
{
    Chunk::write(stream);
    int lastTick = 0;
    for(std::vector<MidiEvent>::iterator it = begin(); it != end(); ++it)
    {
        MidiEvent &event = (*it);
        event.writeOut(stream, midiFile, lastTick);
        lastTick = event.ticks;
    }
    Chunk::markChunkEnd(stream);
}

double TempoMap::getCurrentSecondsPerTick(int tick)
{
    std::map<int,double>::iterator it = lower_bound(tick);
    if(it == end())
    {
        return -1;
    }
    return (*it).second;
}

MidiFile::MidiFile()
{
    clear();
}

MidiFile::~MidiFile()
{
}

void MidiFile::clear()
{
    currentTick = 0;
    currentTime = 0;
    currentSecondsPerTick = 0;
    lastStatus = 0;
    midiHeader.clear();
    midiTracks.clear();
    tempoMap.erase(tempoMap.begin(), tempoMap.end());
    microsecondsPerQuarterNote = (60.0 / 120.0) * 1000000.0;
    computeTimes();
}

void MidiFile::read(std::istream &stream)
{
    clear();
    midiHeader.read(stream);
    computeTimes();
    for(int i = 0; i < midiHeader.trackCount; i++)
    {
        currentTick = 0;
        currentTime = 0;
        MidiTrack midiTrack;
        midiTrack.readIn(stream, *this);
        midiTracks.push_back(midiTrack);
    }
}

void MidiFile::write(std::ostream &stream)
{
    midiHeader.write(stream);
    for(int i = 0; i < midiHeader.trackCount; i++)
    {
        midiTracks[i].writeOut(stream, *this);
    }
}

void MidiFile::load(std::string filename)
{
    std::ifstream stream(filename.c_str(), std::ifstream::binary | std::ifstream::in);
    read(stream);
}

void MidiFile::save(std::string filename)
{
    std::ofstream stream(filename.c_str(), std::ofstream::binary | std::ofstream::out);
    write(stream);
}

void MidiFile::computeTimes()
{
    if(midiHeader.timeFormat < 0)
    {
        int frameCode = (-midiHeader.timeFormat) >> 8;
        double framesPerSecond;
        //cerr << " frameCode:   " << frameCode;
        switch(frameCode)
        {
        case 24:
            framesPerSecond = 24.0;
            break;
        case 25:
            framesPerSecond = 25.0;
            break;
        case 29:
            framesPerSecond = 29.97;
            break;
        case 30:
            framesPerSecond = 30.0;
            break;
        default:
            framesPerSecond = 30.0;
        }
        //cerr << " framesPerSecond: " << framesPerSecond;
        int ticksPerFrame = midiHeader.timeFormat & 0xff;
        //cerr << " ticksPerFrame:   " << ticksPerFrame;
        currentSecondsPerTick = (1.0 / framesPerSecond) / ticksPerFrame;
    }
    else
    {
        double ticksPerQuarterNote = double(midiHeader.timeFormat);
        double secondsPerQuarterNote = microsecondsPerQuarterNote / 1000000.0;
        currentSecondsPerTick = secondsPerQuarterNote / ticksPerQuarterNote;
        //cerr << " ticksPerQuarterNote:   " << ticksPerQuarterNote;
        //cerr << " secondsPerQuarterNote: " << secondsPerQuarterNote;
    }
    //cerr << " currentSecondsPerTick: " << currentSecondsPerTick;
    //cerr << endl;
    tempoMap[currentTick] = currentSecondsPerTick;
}

void MidiFile::dump(std::ostream &stream)
{
    stream << "CHUNK ID: " << midiHeader.id << std::endl;
    stream << "Type: " << midiHeader.type << std::endl;
    stream << "Tracks: " << midiHeader.trackCount << std::endl;
    stream << "Time format: " << midiHeader.timeFormat << std::endl;
    for(size_t i = 0; i < midiTracks.size(); i++)
    {
        stream << "TRACK: " << (unsigned int) i << std::endl;
        MidiTrack &midiTrack = midiTracks[i];
        for(size_t j = 0; j < midiTrack.size(); j++)
        {
            MidiEvent &midiEvent = midiTrack[j];
            stream << (unsigned int) j << " (" << midiEvent.ticks << ":" << midiEvent.time << ") ";
            for(size_t k = 0; k < midiEvent.size(); k++)
            {
                stream << (int) midiEvent[k] << " ";
            }
            stream << std::endl;
        }
    }
}

/// inline bool SILENCE_PUBLIC operator < (const MidiEvent &a, const MidiEvent &b)
bool SILENCE_PUBLIC operator < (const MidiEvent &a, const MidiEvent &b)
{
    MidiEventComparator comparator;
    return comparator(a, b);
};

}
