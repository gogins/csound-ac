#ifndef SCORE_HPP_INCLUDED
#define SCORE_HPP_INCLUDED
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
#include "Event.hpp"
#include "Midifile.hpp"
#include <iostream>
#include <vector>
%}
%include "std_string.i"
%include "std_vector.i"
#else
#include "Event.hpp"
#include "Midifile.hpp"
#include <iostream>
#include <vector>
#endif

namespace csound
{
/**
 * Base class for collections of events in music space.
 * Can order events by time.
 * <p>
 * The implementation is a std::vector of Events.
 * The elements of the vector are value objects, not references.
 */
class SILENCE_PUBLIC Score :
    public std::vector<csound::Event>
{
protected:
    void createMusicModel();
public:
    Event scaleTargetMinima;
    std::vector<bool> rescaleMinima;
    Event scaleTargetRanges;
    std::vector<bool> rescaleRanges;
    Event scaleActualMaxima;
    Event scaleActualMinima;
    Event scaleActualRanges;
    MidiFile midifile;
    std::map<int, double> reassignments;
    std::map<int, double> gains;
    std::map<int, double> pans;
    /**
     * Arbitrary text that is prepended to the Csound score. Should normally 
     * be Csound comments or "f" statements, or pre-composed Csound events.
     */
    std::string csound_score_header;
    Score();
    virtual ~Score();
    void initialize();
    virtual void append(Event event);
    virtual void append_event(Event event);
    virtual void append(double time, double duration, double status, double instrument, double key, double velocity, double phase=0, double pan=0, double depth=0, double height=0, double pitches=4095);
    virtual void add(double time, double duration, double status, double instrument, double key, double velocity, double phase=0, double pan=0, double depth=0, double height=0, double pitches=4095);
    virtual void append_note(double time, double duration, double status, double instrument, double key, double velocity, double phase=0, double pan=0, double depth=0, double height=0, double pitches=4095);
    virtual Event &get(size_t index);
    virtual const Event &get_const(size_t index) const;

    virtual void remove(size_t index);
    /**
     * Loads score data from a MIDI (.mid) file,
     * or a MusicXML (.xml) file.
     * Non-sounding data is ignored.
     */
    virtual void load(std::string filename);
    virtual void load_filename(std::string filename);
    virtual void load(std::istream &stream);
    //virtual void load(MidiFile &midiFile);
    /**
     * Save as a MIDI file, format 1 (.mid) file,
     * or as a partwise MusicXML (.xml) file,
     * or as a Fomus music notation (.fms) file.
     * Only sounding data is saved.
     */
    virtual void save(std::string filename);
    virtual void save_filename(std::string filename);
    /**
     * Save as a MIDI file, format 1.
     */
    virtual void save(std::ostream &stream);
    /**
     * Save as a MIDI file, format 1.
     */
    //virtual void save(MidiFile &midiFile);
    static void getScale(std::vector<Event> &score, int dimension, size_t beginAt, size_t endAt, double &minimum, double &range);
    static void setScale(std::vector<Event> &score, int dimension, bool rescaleMinimum, bool rescaleRange, size_t beginAt, size_t endAt, double targetMinimum, double targetRange);
    virtual void findScale();
    virtual void rescale();
    virtual void rescale(Event &event);
    virtual void rescale_event(Event &event);
    /**
     * Sort all events in the score by time, instrument number, pitch, duration, loudness,
     * and other dimensions as given by Event::SORT_ORDER.
     */
    virtual void sort();
    virtual void dump(std::ostream &stream);
    virtual std::string toString();
    /**
     * Returns the time from the beginning of the earliest-starting event to 
     * the ending of the last-ending event. Does not assume that events are 
     * sorted, or that all no events start earlier than time 0.
     */
    virtual double getDuration();
    /**
     * Returns the time from time 0 to the final off time. Does not assume 
     * that events are sorted, or that no events start earlier than time 0.
     */
    virtual double getDurationFromZero() const;
    /**
     * Returns the first on time.
     */
    virtual double getFirstOnTime() const;
    /**
     * Returns the last off time.
     */
    virtual double getLastOffTime() const;
    /**
     * Returns the last off time minus the first on time (which can be less 
     * than 0).
     */
    virtual double getEventsDuration() const;    
    /**
     * Rescales Event times for performance, i.e. such that the first on time 
     * is 0, and `getEventsDuration` returns the target duration.
     */
    virtual void setPerformanceDuration(double target_duration); 
    virtual void rescale(int dimension, bool rescaleMinimum, double minimum, bool rescaleRange = false, double range = 0.0);
    /**
     * Translate the Silence events in this to a Csound score for blue, 
     * that is, to a list of i statements with with inso, time, duration, dbsp, pch, pan.
     */
    virtual std::string getBlueScore(double tonesPerOctave = 12.0, bool conformPitches = false);
    /**
     * Translate the Silence events in this to a Csound score, that is, to a list of i statements.
     * The Silence events are rounded off to the nearest equally tempered pitch by the
     * specified number of tones per octave; if this argument is zero, the pitch is not tempered.
     * The Silence events are conformed to the nearest pitch-class set in the pitch-class set
     * dimension of the event, if the conform pitches argument is true; otherwise, the pitches
     * are not conformed.
     */
    virtual std::string getCsoundScore(double tonesPerOctave = 12.0, bool conformPitches = false);
    /**
     * Re-assign instrument number for export to Csound score.
     */
    virtual void arrange(int oldInstrumentNumber, int newInstrumentNumber);
    /**
     * Re-assign instrument number and adjust gain for export to Csound score.
     */
    virtual void arrange(int oldInstrumentNumber, int newInstrumentNumber, double gain);
    /**
     * Re-assign instrument number, adjust gain, and change pan for export to Csound score.
     */
    virtual void arrange(int oldInstrumentNumber, int newInstrumentNumber, double gain, double pan);
    virtual void arrange_all(int oldInstrumentNumber, int newInstrumentNumber, double gain, double pan);
    /**
     * Remove instrument number, gain, and pan assignments.
     */
    virtual void removeArrangement();
    /**
     * Return a vector containing the MIDI key numbers
     * in the specified segment of the score.
     */
    virtual std::vector<double> getPitches(size_t begin,
                                           size_t end,
                                           size_t divisionsPerOctave = 12) const;
    /**
     * Set the pitches of the specified segment of the score
     * to the specified pitches.
     * Each pitch in the score is moved to the closest pitch in the specified pitches.
     */
    virtual void setPitches(size_t begin,
                            size_t end,
                            const std::vector<double> &pitches);
    /**
     * Set the pitches of the specified segment of the score
     * to the specified pitch-class set.
     * Each pitch in the score is moved to the closest pitch-class in the specified set.
     */
    virtual void setPitchClassSet(size_t begin,
                                  size_t end,
                                  const std::vector<double> &pcs,
                                  size_t divisionsPerOctave = 12);
    /**
     * For the specified segment of the score,
     * return the indexes for the prime chord, its transposition,
     * and their voicing within the specified range.
     * Each of these indexes forms an additive cyclic group.
     *
     * See: http://ruccas.org/pub/Gogins/music_atoms.pdf
     */
    virtual std::vector<double> getPTV(size_t begin,
                                       size_t end,
                                       double lowest,
                                       double range,
                                       size_t divisionsPerOctave = 12) const;
    /**
     * For the specified segment of the score,
     * adjust the pitches to match the specified indexes for the prime chord, its transposition,
     * and their voicing within the specified range.
     * Each of these indexes forms an additive cyclic group.
     *
     * See: http://ruccas.org/pub/Gogins/music_atoms.pdf
     */
    virtual void setPTV(size_t begin,
                        size_t end,
                        double prime,
                        double transposition,
                        double voicing,
                        double lowest,
                        double range,
                        size_t divisionsPerOctave = 12);
    /**
     * For the specified segment of the score,
     * return the indexes for the prime chord and its transposition,
     * within the specified range.
     *
     * See: http://ruccas.org/pub/Gogins/music_atoms.pdf
     */
    virtual std::vector<double> getPT(size_t begin,
                                      size_t end,
                                      double lowest,
                                      double range,
                                      size_t divisionsPerOctave = 12) const;
    /**
     * For the specified segment of the score,
     * adjust the pitches to match the specified indexes for the prime chord
     * and its transposition within the specified range.
     *
     * See: http://ruccas.org/pub/Gogins/music_atoms.pdf
     */
    virtual void setPT(size_t begin,
                       size_t end,
                       double prime,
                       double transposition,
                       double lowest,
                       double range,
                       size_t divisionsPerOctave = 12);
    /**
     * Performs voice-leading between
     * the specified segments of the score
     * within the specified range.
     * The voice-leading is first the closest by
     * taxicab norm, and then the simplest in motion,
     * optionally avoiding parallel fifths.
     * Only the pitches of the target notes
     * are affected. If necessary, the number of pitches
     * in the target chord is adjusted to match the source.
     *
     * See: http://ruccas.org/pub/Gogins/music_atoms.pdf
     */
    virtual void voicelead(size_t beginSource,
                           size_t endSource,
                           size_t beginTarget,
                           size_t endTarget,
                           double lowest,
                           double range,
                           bool avoidParallelFifths,
                           size_t divisionsPerOctave = 12);
    virtual void voicelead_segments(size_t beginSource,
                           size_t endSource,
                           size_t beginTarget,
                           size_t endTarget,
                           double lowest,
                           double range,
                           bool avoidParallelFifths,
                           size_t divisionsPerOctave = 12);
    /**
     * Performs voice-leading between
     * the specified segments of the score
     * within the specified range, using
     * the specified target pitches.
     * The voice-leading is first the closest by
     * taxicab norm, and then the simplest in motion,
     * optionally avoiding parallel fifths.
     * Only the pitches of the target notes
     * are affected. If necessary, the number of pitches
     * in the target chord is adjusted to match the source.
     *
     * See: http://ruccas.org/pub/Gogins/music_atoms.pdf
     */
    virtual void voicelead(size_t beginSource,
                           size_t endSource,
                           size_t beginTarget,
                           size_t endTarget,
                           const std::vector<double> &targetPitches,
                           double lowest,
                           double range,
                           bool avoidParallelFifths,
                           size_t divisionsPerOctave = 12);
    virtual void voicelead_pitches(size_t beginSource,
                           size_t endSource,
                           size_t beginTarget,
                           size_t endTarget,
                           const std::vector<double> &targetPitches,
                           double lowest,
                           double range,
                           bool avoidParallelFifths,
                           size_t divisionsPerOctave = 12);
    /**
     * Return the index of the first event at or after the specified time,
     * that is, return "begin" for the time;
     * if the time is not found, return the size of the score.
     * Iterating from indexAtTime(t1) to indexAfterTime(t2) is guaranteed
     * to iterate over all and only those events included
     * between t1 and t2.
     */
    virtual int indexAtTime(double time);
    /**
     * Return the index of the first event after the specified time,
     * that is return "end" for the time;
     * if the time is not found, return the size of the score.
     * Iterating from indexAtTime(t1) to indexAfterTime(t2) is guaranteed
     * to iterate over all and only those events included
     * from and including t1 and up to but not including t2.
     */
    virtual int indexAfterTime(double time);
    /**
     * Return the time of the first event at or after the specified index;
     * if the index is not found, return DBL_MAX.
     */
    virtual double indexToTime(size_t index);
    /**
     * Iterate over each note from the beginning to end of the segment;
     * sort the unique pitches;
     * return those unique pitches which also have unique pitch-class sets,
     * in order from lowest to highest in pitch;
     * this has the effect of returning the "inversion" or "voicing",
     * in the musician's informal sense, of the pitches in that segment of the score.
     */
    virtual std::vector<double> getVoicing(size_t begin, size_t end, size_t divisionsPerOctave = 12) const;
    /**
     * Move the pitches in the segment as little as possible
     * to make them have the same ordering of pitch-class sets as the voicing,
     * from the bottom to the top of the range. This has the effect of
     * "inverting" or "re-voicing", in the musician's informal sense, the
     * pitches in that segment of the score.
     */
    virtual void setVoicing(size_t begin, size_t end, const std::vector<double> &voicing, double range, size_t divisionsPerOctave = 12);
    /**
     * Find the non-unique pitch-class set of the prior segment;
     * invert the set such that the inversion's first two pitch-classes
     * are exchanged from the origina; conform the pitches of the
     * current segment to that inversion.
     */
    virtual void setK(size_t priorBegin, size_t begin, size_t end, double base, double range);
    /**
     * Find the non-unique pitch-class set of the prior segment;
     * invert the set such that the inversion's first two pitch-classes
     * are exchanged from the original; conform the pitches of the
     * current segment to that inversion, with voicing V.
     */
    virtual void setKV(size_t priorBegin, size_t begin, size_t end, double V, double base, double range);
    /**
     * Find the non-unique pitch-class set of the prior segment;
     * invert the set such that the inversion's first two pitch-classes
     * are exchanged from the original; conform the pitches of the
     * current segment to that inversion, using the closest voice-leading
     * from the pitches of the prior segment, optionally avoiding parallel fifths.
     */
    virtual void setKL(size_t priorBegin, size_t begin, size_t end, double base, double range, bool avoidParallels = true);
    /**
     * Find the non-unique pitch-class set of the prior segment; transpose the set up by Q
     * if the set is a T-form of the context, or down by Q if the set is an I-form of the context;
     * then conform the pitches of the current segment to that set.
     * The context will be reduced or doubled as required to match the cardinality of the set.
     */
    virtual void setQ(size_t priorBegin, size_t begin, size_t end, double Q, const std::vector<double> &context, double base, double range);
    /**
     * Find the non-unique pitch-class set of the prior segment; transpose the set up by Q
     * if the set is a T-form of the context, or down by Q if the set is an I-form of the context;
     * then conform the pitches of the current segment to that set, with the voicing V.
     * The context will be reduced or doubled as required to match the cardinality of the set.
     */
    virtual void setQV(size_t priorBegin, size_t begin, size_t end, double Q, const std::vector<double> &context, double V, double base, double range);
    /**
     * Find the non-unique pitch-class set of the prior segment; transpose the set up by Q
     * if the set is a T-form of the context, or down by Q if the set is an I-form of the context;
     * then conform the pitches of the segment to that set, using the closest voice-leading
     * from the pitches of the prior segment, optionally avoiding parallel fifths.
     * The context will be reduced or doubled as required to match the cardinality of the set.
     */
    virtual void setQL(size_t priorBegin, size_t begin, size_t end, double Q, const std::vector<double> &context, double base, double range, bool avoidParallels = true);
    /**
     * Multiply existing times and durations by (targetDuration / getDuration()), i.e.
     * stretch or shrink musical time.
     */
    virtual void setDuration(double targetDuration);
    virtual void setDurationFromZero(double targetDuration);
    /**
     * If the score contains two notes of the same pitch
     * and loudness greater than 0 that overlap in time,
     * extend the earlier note and discard the later note.
     */
    virtual void tieOverlappingNotes(bool considerInstrumentNumber = false);
    /**
     * Confirm pitches in this score to the closest pitch in the indicated
     * system of equal temperament.
     */
    virtual void temper(double tonesPerOctave = 12.0);
    /**
     * Calls Event::process on all Events in this.
     */
    virtual void process();
    /**
     * Multiply each event in this by the transformation.
     */
    virtual void transform(const Eigen::MatrixXd &transformation);
    /*
     * Exposed as functions for Embind.
     */     
    virtual Event &getScaleTargetMinima();
    virtual std::vector<bool> &getRescaleMinima();
    virtual Event &getScaleTargetRanges();
    virtual std::vector<bool> &getRescaleRanges();
    virtual const Event &getScaleActualMinima() const;
    virtual const Event &getScaleActualRanges() const;
    /**
     * Translates most of this Score to JSON: 
     * 1. The vector of Events, sorted and otherwise massaged.
     * 2. The actual minima, maxima, and ranges.
     * The JSON schema is:
     * {
     *      events: [[],...],
     *      minima: [],'
     *      maxima: [], 
     *      ranges: []
     * };
     * This is useful, e.g., for sending a complete score to the JavaScript 
     * context of a Web page for display using WebGL or Three.js.
     */
    virtual std::string toJson();
    virtual void setCsoundScoreHeader(const std::string &text);
    virtual void appendToCsoundScoreHeader(const std::string &text);
    virtual std::string getCsoundScoreHeader() const;
};

}

#endif
