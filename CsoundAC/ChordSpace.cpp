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
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
// Header file only library.
#include "Platform.hpp"
#ifdef SWIG
%module CsoundAC
%{
#include <algorithm>
#include <cfloat>
#include <ChordSpace.hpp>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <Eigen/Dense>
#include <Event.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <Score.hpp>
#include <set>
#include <sstream>
#include <vector>
%}
%include "std_string.i"
%include "std_vector.i"
#else
#include <algorithm>
// Header file only library.
#include <boost/math/special_functions/ulp.hpp>
#include <cfloat>
// Header file only library.
#include "ChordSpace.hpp"
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <Eigen/Dense>
#include "Event.hpp"
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include "Score.hpp"
#include <set>
#include <sstream>
#include <vector>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

namespace csound {
    
SILENCE_PUBLIC std::vector<Chord> allOfEquivalenceClass(int voice_count, std::string equivalence_class, double range, double g, int sector, bool printme) {
    std::vector<Chord> fundamental_domain;
    if (equivalence_class == "RP") {
        fundamental_domain = fundamentalDomainByPredicate<EQUIVALENCE_RELATION_RP>(voice_count, range, g, sector, printme);       
    } else if (equivalence_class == "RPT") {
        fundamental_domain = fundamentalDomainByPredicate<EQUIVALENCE_RELATION_RPT>(voice_count, range, g, sector, printme);       
    } else if (equivalence_class == "RPTg") {
        fundamental_domain = fundamentalDomainByPredicate<EQUIVALENCE_RELATION_RPTg>(voice_count, range, g, sector, printme);       
    } else if (equivalence_class == "RPI") {
        fundamental_domain = fundamentalDomainByPredicate<EQUIVALENCE_RELATION_RPI>(voice_count, range, g, sector, printme);       
    } else if (equivalence_class == "RPTI") {
        fundamental_domain = fundamentalDomainByPredicate<EQUIVALENCE_RELATION_RPTI>(voice_count, range, g, sector, printme);       
    } else if (equivalence_class == "RPTgI") {
        fundamental_domain = fundamentalDomainByPredicate<EQUIVALENCE_RELATION_RPTgI>(voice_count, range, g, sector, printme);       
    }
    return fundamental_domain;
}
    
SILENCE_PUBLIC Chord gather(Score &score, double startTime, double endTime) {
    std::vector<Event *> slice_ = slice(score, startTime, endTime);
    std::set<double> pitches;
    for (int i = 0; i < slice_.size(); ++i) {
        pitches.insert(slice_[i]->getKey());
    }
    Chord chord;
    chord.resize(pitches.size());
    int voice = 0;
    for (std::set<double>::iterator it = pitches.begin(); it != pitches.end(); ++it) {
        chord.setPitch(voice, *it);
        voice++;
    }
    return chord;
}

Event note(const Chord &chord,
    int voice,
    double time_,
    double duration_,
    double channel_,
    double velocity_,
    double pan_) {
    Event note;
    note.setTime(time_);
    note.setKey(chord.getPitch(voice));
    if (duration_ != DBL_MAX) {
        note.setDuration(duration_);
    } else {
        note.setDuration(chord.getDuration(voice));
    }
    if (channel_ != DBL_MAX) {
        note.setInstrument(channel_);
    } else {
        note.setInstrument(chord.getInstrument(voice));
    }
    if (velocity_ != DBL_MAX) {
        note.setVelocity(velocity_);
    } else {
        note.setVelocity(chord.getLoudness(voice));
    }
    if (pan_ != DBL_MAX) {
        note.setPan(pan_);
    } else {
        note.setPan(chord.getPan(voice));
    }
    return note;
}

Score notes(const Chord &chord,
    double time_,
    double duration_,
    double channel_,
    double velocity_,
    double pan_) {
    Score score;
    for (int voice = 0; voice < chord.voices(); ++voice) {
        Event event = note(chord, voice, time_, duration_, channel_, velocity_, pan_);
        score.append(event);
    }
    return score;
}

SILENCE_PUBLIC void numerics_information(double a, double b, int epsilons, int ulps) {
    SCOPED_DEBUGGING debugging;
    static const int PRECISION = DBL_DIG; 
    static const double machine_epsilon = std::numeric_limits<double>::epsilon();
    static const double double_max_ = std::numeric_limits<double>::max();
    CHORD_SPACE_DEBUG("numerics_information: a: %.*g b: %.*g machine_epsilon: %.*g double_max: %.*g epsilons: %5d ulps: %5d:\n", PRECISION, a, PRECISION, b, PRECISION, machine_epsilon, PRECISION, double_max_, epsilons, ulps);
    //~ lt_tolerance(a, b, epsilons, ulps);
    //~ le_tolerance(a, b, epsilons, ulps);
    eq_tolerance(a, b, epsilons, ulps);
    //~ ge_tolerance(a, b, epsilons, ulps);
    //~ gt_tolerance(a, b, epsilons, ulps);
}

SILENCE_PUBLIC std::vector<Event *> slice(Score &score, double startTime, double endTime) {
    std::vector<Event *> result;
    for (int i = 0, n = score.size(); i < n; ++i) {
        Event *event = &score[i];
        if (event->isNoteOn()) {
            double eventStart = event->getTime();
            if (eventStart >= startTime && eventStart < endTime) {
                result.push_back(event);
            }
        }
    }
    return result;
}

SILENCE_PUBLIC void toScore(const Chord &chord, 
    Score &score,
    double time_, bool voiceIsInstrument) {
    for (int voice = 0; voice < chord.voices(); ++voice) {
        double instrument = double(voice);
        if (!voiceIsInstrument) {
            instrument = chord.getInstrument(voice);
        }
        score.append(time_,
                     chord.getDuration(voice),
                     144.0,
                     instrument,
                     chord.getPitch(voice),
                     chord.getLoudness(voice),
                     0.0,
                     chord.getPan(voice));
    }
}

SILENCE_PUBLIC void apply(Score &score, const Chord &chord, double startTime, double endTime, bool octaveEquivalence) {
    std::vector<Event *> slice_ = slice(score, startTime, endTime);
    for (int i = 0; i < slice_.size(); ++i) {
        Event &event = *slice_[i];
        conformToChord_equivalence(event, chord, octaveEquivalence);
    }
}

SILENCE_PUBLIC void conformToChord_equivalence(Event &event, const Chord &chord, bool octaveEquivalence) {
    if (!event.isNoteOn()) {
        return;
    }
    double pitch = event.getKey();
    if (octaveEquivalence) {
        Chord pcs = chord.epcs();
        pitch = conformToPitchClassSet(pitch, pcs);
    } else {
        pitch = closestPitch(pitch, chord);
    }
    event.setKey(pitch);
}

SILENCE_PUBLIC void conformToChord(Event &event, const Chord &chord) {
    conformToChord_equivalence(event, chord, true);
}

SILENCE_PUBLIC void ChordScore::insertChord(double tyme, const Chord chord) {
    chords_for_times[tyme] = chord;
}
/**
 * Returns a pointer to the first chord that starts at or after the
 * specified time. If there is no such chord, a null pointer is returned.
 */
SILENCE_PUBLIC Chord *ChordScore::getChord(double time_) {
    auto it = chords_for_times.lower_bound(time_);
    if (it != chords_for_times.end()) {
        return &it->second;
    } else {
        return nullptr;
    }
}

/**
 * Conforms the pitch-classes of the events in this to the closest
 * pitch-class of the chord, if any, that obtains at that time.
 */
SILENCE_PUBLIC void ChordScore::conformToChords(bool tie_overlaps, bool octave_equivalence) {
    sort();
    if (chords_for_times.begin() != chords_for_times.end()) {
        for (auto event_iterator = begin(); event_iterator != end(); ++event_iterator) {
            auto chord_iterator = chords_for_times.lower_bound(event_iterator->getTime());
            if (chord_iterator != chords_for_times.end()) {
                conformToChord_equivalence(*event_iterator, chord_iterator->second, octave_equivalence);
            }
        }
    }
    if (tie_overlaps == true) {
        tieOverlappingNotes(true);
    }
}

SILENCE_PUBLIC void ChordScore::getScale(std::vector<Event> &score, int dimension, size_t beginAt, size_t endAt, double &minimum, double &range)
{
    if(beginAt == endAt) {
        return;
    }
    sort();
    const Event &beginEvent = score[beginAt];
    double maximum = beginEvent[dimension];
    const Event &endEvent = score[endAt - 1];
    minimum = endEvent[dimension];
    if(dimension == Event::TIME) {
        const Event &e = score[beginAt];
        maximum = std::max(e.getTime(), e.getTime() + e.getDuration());
        minimum = std::min(e.getTime(), e.getTime() + e.getDuration());
        double beginning;
        double ending;
        for( ; beginAt != endAt; ++beginAt) {
            const Event &event = score[beginAt];
            beginning = std::min(event.getTime(), event.getTime() + event.getDuration());
            ending = std::max(event.getTime(), event.getTime() + event.getDuration());
            if(ending > maximum) {
                maximum = ending;
            } else if(beginning < minimum) {
                minimum = beginning;
            }
        }
        // Also take into account chord times.
        auto chord_begin = chords_for_times.begin();
        auto chord_rbegin = chords_for_times.rbegin();
        if (chord_begin != chords_for_times.end() && chord_rbegin != chords_for_times.rend()) {
            minimum = std::min(minimum, chord_begin->first);
            maximum = std::max(maximum, chord_rbegin->first);
        }
    } else {
        for( ; beginAt != endAt; ++beginAt) {
            const Event &event = score[beginAt];
            if(event[dimension] > maximum) {
                maximum = event[dimension];
            }
            if(event[dimension] < minimum) {
                minimum = event[dimension];
            }
        }
    }
    range = maximum - minimum;
}

SILENCE_PUBLIC void ChordScore::setScale(std::vector<Event> &score,
              int dimension,
              bool rescaleMinimum,
              bool rescaleRange,
              size_t beginAt,
              size_t endAt,
              double targetMinimum,
              double targetRange)
{
    if(!(rescaleMinimum || rescaleRange)) {
        return;
    }
    if(beginAt == endAt) {
        return;
    }
    sort();
    double actualMinimum;
    double actualRange;
    getScale(score, dimension, beginAt, endAt, actualMinimum, actualRange);
    double scale;
    if(actualRange == 0.0) {
        scale = 1.0;
    } else {
        scale = targetRange / actualRange;
    }
    for( ; beginAt != endAt; ++beginAt) {
        Event &event = score[beginAt];
        event[dimension] = event[dimension] - actualMinimum;
        if(rescaleRange) {
            event[dimension] = event[dimension] * scale;
        }
        if(rescaleMinimum) {
            event[dimension] = event[dimension] + targetMinimum;
        } else {
            event[dimension] = event[dimension] + actualMinimum;
        }
    }
    // Also rescale chord times.
    if (dimension == Event::TIME) {
        std::map<double, Chord> temp;
        for (auto it = chords_for_times.begin(); it != chords_for_times.end(); ++it) {
            double tyme = it->first;
            const Chord &chord = it->second;
            tyme = tyme - actualMinimum;
            if (rescaleRange) {
                tyme = tyme * scale;
            }
            if (rescaleMinimum) {
                tyme = tyme + targetMinimum;
            } else {
                tyme = tyme + actualMinimum;
            }
            temp[tyme] = chord;
        }
        chords_for_times = temp;
    }
}

SILENCE_PUBLIC double ChordScore::getDuration()
{
    double start = 0.0;
    double end = 0.0;
    sort();
    for (int i = 0, n = size(); i < n; ++i) {
        const Event &event = at(i);
        if (i == 0) {
            start = event.getTime();
            end = event.getOffTime();
        } else {
            if (event.getTime() < start) {
                start = event.getTime();
            }
            if (event.getOffTime() > end) {
                end = event.getOffTime();
            }
        }
    }
    auto chord_begin = chords_for_times.begin();
    auto chord_rbegin = chords_for_times.rbegin();
    if (chord_begin != chords_for_times.end() && chord_rbegin != chords_for_times.rend()) {
        start = std::min(start, chord_begin->first);
        end = std::max(end, chord_rbegin->first);
    }
    return end - start;
}

SILENCE_PUBLIC void ChordScore::setDuration(double targetDuration)
{
    double currentDuration = getDuration();
    if (currentDuration == 0.0) {
        return;
    }
    sort();
    double factor = targetDuration / currentDuration;
    for (size_t i = 0, n = size(); i < n; i++) {
        Event &event = (*this)[i];
        double time_ = event.getTime();
        double duration = event.getDuration();
        event.setTime(time_ * factor);
        event.setDuration(duration * factor);
    }
    std::map<double, Chord> temp;
    for (auto it = chords_for_times.begin(); it != chords_for_times.end(); ++it) {
        double tyme = it->first;
        const Chord &chord = it->second;
        tyme = tyme * factor;
        temp[tyme] = chord;
    }
    chords_for_times = temp;
}

SILENCE_PUBLIC void insert(Score &score,
                                  const Chord &chord,
                                  double time_) {
    toScore(chord, score, time_, true);
}

SILENCE_PUBLIC void insert(Score &score,
                                  const Chord &chord,
                                  double time_, 
                                  bool voice_is_instrument) {
    toScore(chord, score, time_, voice_is_instrument);
}

static std::mt19937 mersenne_twister;

SILENCE_PUBLIC std::string toString(const Matrix& mat){
    std::stringstream ss;
    ss << mat;
    return ss.str();
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_r>(const Chord &chord, double range, double g, int opt_sector) {
    Chord normal = chord;
    for (int voice = 0; voice < chord.voices(); ++voice) {
        double pitch = chord.getPitch(voice);
        pitch = modulo(pitch, range);
        normal.setPitch(voice, pitch);
    }
    return normal;
}

bool Chord::self_inverse(int opt_sector) const
{
    // Get the inversion hyperplane for this OPT sector.
    const HyperplaneEquation &hyperplane_equation_ = hyperplane_equation(opt_sector);
    bool result = hyperplane_equation_.is_invariant(*this); 
    return result;
}

/** 
 * To identify the sector to which a chord in RP belongs, we project the chord 
 * onto RPT. Then we measure the Euclidean distance of the midpoint of each 
 * sector's base face to the chord, and the closest midpoint belongs to that 
 * chord's sector. A chord may belong to more than one sector if it is 
 * equidistant from two or more midpoints. This is numerically tricky.
 */
bool Chord::is_in_rpt_sector(int sector, double range) const
{
    if (sector < 0 || sector >= voices())
    {
        return false;
    }
    // Project chord onto RP/T base.
    const Chord base = eRP(range).eT();
    return base.is_in_rpt_sector_base(sector, range);
}

bool Chord::is_in_rpt_sector_base(int sector, double range) const
{
    (void)range;

    if (sector < 0 || sector >= voices())
    {
        return false;
    }

    const Vector chord_v = this->col(0);

    double best_score = -std::numeric_limits<double>::infinity();
    std::vector<double> scores;
    scores.reserve(size_t(voices()));

    for (int sector_i = 0; sector_i < voices(); ++sector_i)
    {
        const auto &hp = hyperplane_equation(sector_i);
        const Vector &m = hp.base_midpoint;

        const double score = chord_v.dot(m) - 0.5 * m.dot(m);

        if (score > best_score)
        {
            best_score = score;
        }
        scores.push_back(score);
    }

    const double score_for_sector = scores[size_t(sector)];
    return eq_tolerance(score_for_sector, best_score, 20, 1024);
}

bool Chord::is_in_minor_rpti_sector(int opt_sector) const
{
    const auto &hyperplane_equation_ = hyperplane_equation(opt_sector);
    return hyperplane_equation_.is_minor(*this);
}

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_R>(
    const Chord &chord, double range, double g, int opt_sector)
{
    double max_pitch = chord.max()[0];
    double min_pitch = chord.min()[0];

    if (le_tolerance(max_pitch, min_pitch + range) == false)
    {
        return false;
    }

    double sum_ = chord.layer();
    if (le_tolerance(0.0, sum_) == false)
    {
        return false;
    }
    if (le_tolerance(sum_, range) == false)
    {
        return false;
    }

    return true;
}

bool Chord::iseR(double range_) const {
    return predicate<EQUIVALENCE_RELATION_R>(*this, range_, 1.0, 0);
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_R>(const Chord &chord, double range_, double g, int opt_sector) {
    CHORD_SPACE_DEBUG("equate<EQUIVALENCE_RELATION_R>: chord: %s normal: %d\n", chord.toString().c_str(), chord.iseR(range_));
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range_, g, opt_sector) == true) {
        Chord copy = chord;
        CHORD_SPACE_DEBUG("equate<EQUIVALENCE_RELATION_R>: returning copy: %s normal: %d\n", copy.toString().c_str(), copy.iseR(range_));
        return copy;
    }
    Chord er = equate<EQUIVALENCE_RELATION_r>(chord, range_, g, opt_sector);
    CHORD_SPACE_DEBUG("equate<EQUIVALENCE_RELATION_R>: starting with er: %s normal: %d\n", er.toString().c_str(), er.iseR(range_));
    while (le_tolerance(er.layer(), range_) == false) {
        std::vector<double> maximum = er.max();
        double maximum_pitch = maximum[0];
        double new_pitch = maximum_pitch - range_;
        double maximum_voice = maximum[1];
        er.setPitch(maximum_voice, new_pitch);
        CHORD_SPACE_DEBUG("equate<EQUIVALENCE_RELATION_R>: revoiced er: %s normal: %d\n", er.toString().c_str(), er.iseR(range_));
    }
    CHORD_SPACE_DEBUG("equate<EQUIVALENCE_RELATION_R>: returning er: %s normal: %d\n", er.toString().c_str(), er.iseR(range_));
    return er;
}

Chord Chord::eR(double range) const {
    return csound::equate<EQUIVALENCE_RELATION_R>(*this, range, 1.0, 0);
}

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_P>(const Chord &chord, double range, double g, int opt_sector) {
    for (size_t voice = 1; voice < chord.voices(); voice++) {
        double a = chord.getPitch(voice - 1);
        double b = chord.getPitch(voice);
        if (le_tolerance(a, b) == false) {
            return false;
        }
    }
    return true;
}

bool Chord::iseP() const {
    return predicate<EQUIVALENCE_RELATION_P>(*this, OCTAVE(), 1.0, 0);
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_P>(const Chord &chord, double range, double g, int opt_sector) {
    Chord normal = chord;
    bool sorted = false;
    // A bubble sort is adequate for small numbers of elements.
    while (!sorted) {
        sorted = true;
        for (int voice = 1; voice < normal.voices(); voice++) {
            if (gt_tolerance(normal.getPitch(voice - 1), normal.getPitch(voice))) {
                sorted = false;
                normal.row(voice - 1).swap(normal.row(voice));
            }
        }
    }
    return normal;
}

Chord Chord::eP() const {
    return csound::equate<EQUIVALENCE_RELATION_P>(*this, OCTAVE(), 1.0, 0);
}

//	EQUIVALENCE_RELATION_T

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_T>(const Chord &chord, double range, double g, int opt_sector) {
    auto layer_ = chord.layer();
    CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_T>: chord: %s sector: %d layer: %12.7f\n", chord.toString().c_str(), opt_sector, layer_);
    if (eq_tolerance(layer_, 0.) == false) {
        return false;
    } else {
        return true;
    }
}

bool Chord::iseT() const {
    return predicate<EQUIVALENCE_RELATION_T>(*this, OCTAVE(), 1., 0);
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_T>(const Chord &chord, double range, double g, int opt_sector) {
    Chord result = chord;
    double sum = chord.layer();
    double sum_per_voice = sum / chord.voices();
    result = result.T(-sum_per_voice);
    return result;
}

Chord Chord::eT() const {
    return csound::equate<EQUIVALENCE_RELATION_T>(*this, OCTAVE(), 1.0, 0);
}

//	EQUIVALENCE_RELATION_Tg

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_Tg>(const Chord &chord, double range, double g, int opt_sector)
{
    (void)range;
    (void)opt_sector;

    if (g <= 0.0)
    {
        g = 1.0;
    }

    // A chord is Tg-normal iff it equals its Tg representative (component-wise).
    const Chord representative = csound::equate<EQUIVALENCE_RELATION_Tg>(chord, OCTAVE(), g, 0);

    for (int v = 0; v < chord.voices(); ++v)
    {
        if (eq_tolerance(chord.getPitch(v), representative.getPitch(v)) == false)
        {
            return false;
        }
    }
    return true;
}

template<> 
SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_Tg>(const Chord &chord, double range, double g, int opt_sector) {
    auto self_t = chord.eT();
    auto self_t_ceiling = self_t.ceiling(g);
    while (lt_tolerance(self_t_ceiling.layer(), 0.) == true) {
        /// self_t_ceiling = self_t_ceiling.T(g);
        auto self_t = chord.eT();
        auto self_t_ceiling = self_t.ceiling(g);
    }
    return self_t_ceiling;
}

Chord Chord::eTT(double g) const {
    return csound::equate<EQUIVALENCE_RELATION_Tg>(*this, OCTAVE(), g, 0);
}

bool Chord::iseTT(double g) const {
    return predicate<EQUIVALENCE_RELATION_Tg>(*this, OCTAVE(), g, 0);
}

//	EQUIVALENCE_RELATION_I

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_I>(const Chord &chord, double range, double g, int opt_sector) {
    const HyperplaneEquation &hyperplane_equation_ = chord.hyperplane_equation(opt_sector);
    if (hyperplane_equation_.is_minor(chord) == true) {
        CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_I>: %s is in minor opti sector: %d\n", chord.toString().c_str(), true);
       return true;
    } else {
        CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_I>: %s is in major opti sector: %d\n", chord.toString().c_str(), false);
        return false;
    }
}

bool Chord::iseI_chord(Chord *inverse, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_I>(*this, OCTAVE(), 1.0, opt_sector);
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_I>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_I>(chord, range, g, opt_sector) == true) {
        return chord;
    } else {
        return reflect_in_inversion_flat(chord, opt_sector);
    }
}

Chord Chord::eI(int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_I>(*this, OCTAVE(), 1.0, opt_sector);
}

//  EQUIVALENCE_RELATION_RP

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RP>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    return true;
}

bool Chord::iseRP(double range) const {
    return predicate<EQUIVALENCE_RELATION_RP>(*this, range, 1.0, 0);
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RP>(const Chord &chord, double range, double g, int opt_sector) {
    Chord normal = equate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector);
    normal = equate<EQUIVALENCE_RELATION_P>(normal, range, g, opt_sector);
    return normal;
}

Chord Chord::eRP(double range) const {
    return csound::equate<EQUIVALENCE_RELATION_RP>(*this, range, 1.0, 0);
}

// EQUIVALENCE_RELATION_RT

template<>
SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RT>(
    const Chord &chord, double range, double g, int opt_sector)
{
    if (predicate<EQUIVALENCE_RELATION_r>(chord, range, g, opt_sector) == false)
    {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_T>(chord, range, g, opt_sector) == false)
    {
        return false;
    }
    return true;
}

bool Chord::iseRT(double range) const
{
    return predicate<EQUIVALENCE_RELATION_RT>(*this, range, 1.0, 0);
}

template<>
SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RT>(
    const Chord &chord, double range, double g, int opt_sector)
{
    Chord normal = equate<EQUIVALENCE_RELATION_r>(chord, range, g, opt_sector);
    normal = equate<EQUIVALENCE_RELATION_T>(normal, range, g, opt_sector);
    return normal;
}

Chord Chord::eRT(double range) const
{
    return csound::equate<EQUIVALENCE_RELATION_RT>(*this, range, 1.0, 0);
}

//	EQUIVALENCE_RELATION_RI

//	EQUIVALENCE_RELATION_PT

//	EQUIVALENCE_RELATION_PTg

//	EQUIVALENCE_RELATION_PI

//	EQUIVALENCE_RELATION_TI

//	EQUIVALENCE_RELATION_RPT

template<>
SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPT>(
    const Chord &chord,
    double range,
    double g,
    int rpt_sector)
{
    (void)g;

    // Decomposable definition:
    //   chord is RPT-normal iff it is already in the RP prism, already in the T base,
    //   and lies in the requested sector (tested without further reduction).
    if (predicate<EQUIVALENCE_RELATION_RP>(chord, range, 1.0, 0) == false)
    {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_T>(chord, range, 1.0, 0) == false)
    {
        return false;
    }
    // Sectoring is defined in the RP/T base, not in the Tg base.
    // Using raw-sectoring on a Tg-normal chord will generally fail.
    if (chord.eT().is_in_rpt_sector_base(rpt_sector, range) == false)
    {
        return false;
    }
    return true;
}

bool Chord::iseRPT(double range, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPT>(*this, range, 1.0, opt_sector);
}

template<>
SILENCE_PUBLIC Chord
equate<EQUIVALENCE_RELATION_RPT>(
    const Chord &chord,
    double range,
    double g,
    int rpt_sector)
{
    (void)g; // not used for plain RPT

    // 1) Reduce into the RP prism (range + permutation)
    const Chord rp = chord.eRP(range);

    // 2) Enumerate octavewise revoicings that tile RP into RPT sectors
    const std::vector<Chord> rp_voicings = rp.voicings();

    // 3) Apply strict transposition reduction (T) to each candidate
    std::vector<Chord> candidates;
    candidates.reserve(rp_voicings.size());

    for (const auto &v : rp_voicings)
    {
        candidates.push_back(v.eT());
    }

    // 4) Return the candidate that lies in the requested RPT sector
    for (const auto &c : candidates)
    {
        // Sectoring is defined in the RP/T base, not in the Tg base.
        if (c.eT().is_in_rpt_sector_base(rpt_sector, range))
        {
            return c;
        }
    }
    System::error("Error:   Chord equate<EQUIVALENCE_RELATION_RPT>: no RPT in sector %d.\n", rpt_sector);
    return candidates.front();
}

Chord Chord::eRPT(double range, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPT>(*this, range, 1.0, opt_sector);
}

//	EQUIVALENCE_RELATION_RPTg

template<>
SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTg> (
    const Chord &chord,
    double range,
    double g,
    int rpt_sector)
{
    if (g <= 0.0)
    {
        g = 1.0;
    }

    // Decomposable definition:
    //   chord is RPTg-normal iff it is already in the RP prism, already Tg-normal,
    //   and lies in the requested sector (tested without further reduction).
    if (predicate<EQUIVALENCE_RELATION_RP>(chord, range, 1.0, 0) == false)
    {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_Tg>(chord, range, g, 0) == false)
    {
        return false;
    }
        // Sectoring is defined in the RP/T base, not in the Tg base.
    // Using raw-sectoring on a Tg-normal chord will generally fail.
    if (chord.eT().is_in_rpt_sector_base(rpt_sector, range) == false)
    {
        return false;
    }
    return true;
}

bool Chord::iseRPTT(double range, double g, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPTg>(*this, range, g, opt_sector);
}

template<>
SILENCE_PUBLIC Chord
equate<EQUIVALENCE_RELATION_RPTg> (
    const Chord &chord,
    double range,
    double g,
    int rpt_sector)
{
    // Defensive defaults
    if (g <= 0.0)
    {
        g = 1.0;
    }

    // 1) Reduce into the RP prism (range + permutation)
    //    This is the "tall prism" you described.
    const Chord rp = chord.eRP(range);

    // 2) Enumerate octavewise revoicings that tile RP into RPT sectors
    const std::vector<Chord> rp_voicings = rp.voicings();

    // 3) Apply Tg reduction to each candidate
    std::vector<Chord> candidates;
    candidates.reserve(rp_voicings.size());

    for (const auto &v : rp_voicings)
    {
        candidates.push_back(v.eTT(g));   // Tg representative
    }

    // 4) Return the candidate that lies in the requested RPT sector
    for (const auto &c : candidates)
    {
        // Sectoring is defined in the RP/T base, not in the Tg base.
        if (c.eT().is_in_rpt_sector_base(rpt_sector, range))
        {
            return c;
        }
    }
    System::error("Error:  Chord equate<EQUIVALENCE_RELATION_RPTg>: no RPTg in sector %d.\n", rpt_sector);
    return candidates.front();
}

Chord Chord::eRPTT(double range, double g, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPTg>(*this, range, g, opt_sector);
}

std::vector<Chord> Chord::eRPTs(double range) const {
    auto rp = eRP(range);
    std::vector<Chord> rptts;
    auto rp_vs = rp.voicings();
    for (auto rp_v : rp_vs) {
        auto rp_v_tt = rp_v.eT();
        rptts.push_back(rp_v_tt);
    }
    return rptts;
}

std::vector<Chord> Chord::eRPTTs(double range, double g) const {
    auto rp = eRP(range);
    std::vector<Chord> rptts;
    auto rp_vs = rp.voicings();
    for (auto rp_v : rp_vs) {
        auto rp_v_tt = rp_v.eTT(g);
        rptts.push_back(rp_v_tt);
    }
    return rptts;
}

//	EQUIVALENCE_RELATION_RPI

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPI>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_I>(chord, range, g, opt_sector) == false) {
        return false;
    }
    return true;
}

bool Chord::iseRPI(double range, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPI>(*this, range, 1.0, opt_sector);
}

// TODO: Verify.

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPI>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_RPI>(chord, range, g, opt_sector) == true) {
        return chord;
    }
    return chord.eI(opt_sector).eRP(range);
}

Chord Chord::eRPI(double range, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPI>(*this, range, 1.0, opt_sector);
}

//	EQUIVALENCE_RELATION_RTI

//	EQUIVALENCE_RELATION_RTgI

//	EQUIVALENCE_RELATION_RPTI

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (chord.is_in_rpt_sector(opt_sector, range) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_T>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_I>(chord, range, g, opt_sector) == false) {
        return false;
    }
    return true;
}

bool Chord::iseRPTI(double range, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPTI>(*this, range, 1.0, opt_sector);
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord, double range, double g, int opt_sector) {
    auto rpt = equate<EQUIVALENCE_RELATION_RPT>(chord, range, g, opt_sector);
    if (predicate<EQUIVALENCE_RELATION_I>(rpt, range, g, opt_sector) == true) {
        return rpt;
    } else {
        auto rpt_i = equate<EQUIVALENCE_RELATION_I>(rpt, range, g, opt_sector);
        auto rpt_i_rpt = equate<EQUIVALENCE_RELATION_RPT>(rpt_i, range, g, opt_sector);
        return rpt_i_rpt;
    }
}

Chord Chord::eRPTI(double range, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPTI>(*this, range, 1.0, opt_sector);
}

//	EQUIVALENCE_RELATION_RPTgI

template<>
SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTgI>(
    const Chord &chord, double range, double g, int opt_sector)
{
    if (!predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector)) return false;
    if (!predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector)) return false;

    // Must already be in the Tg base.
    if (!predicate<EQUIVALENCE_RELATION_Tg>(chord, range, g, opt_sector)) return false;

    // Now sector membership and inversion tests are meaningful.
    if (!chord.is_in_rpt_sector(opt_sector, range)) return false;
    if (!predicate<EQUIVALENCE_RELATION_I>(chord, range, g, opt_sector)) return false;

    return true;
}

bool Chord::iseRPTTI(double range, double g, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPTgI>(*this, range, g, opt_sector);
}

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTgI>(const Chord &chord, double range, double g, int opt_sector) {
    Chord self = chord;
    if (predicate<EQUIVALENCE_RELATION_RPTgI>(self, range, g, opt_sector) == true) {
        return self;
    } else {
        auto rptt = equate<EQUIVALENCE_RELATION_RPTg>(self, range, g, opt_sector);
        if (predicate<EQUIVALENCE_RELATION_I>(rptt, range, g, opt_sector) == true) {
            return rptt;
        } else {
            auto rptt_i = equate<EQUIVALENCE_RELATION_I>(rptt, range, g, opt_sector);
            auto rptt_i_rptt = equate<EQUIVALENCE_RELATION_RPTg>(rptt_i, range, g, opt_sector);
            return rptt_i_rptt;
        }
    }
}

Chord Chord::eRPTTI(double range, double g, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPTgI>(*this, range, g, opt_sector);
}

SILENCE_PUBLIC const std::map<std::string, double> &pitchClassesForNames() {
    static bool pitchClassesForNamesInitialized = false;
    static std::map<std::string, double> pitchClassesForNames_;
    if (!pitchClassesForNamesInitialized) {
        pitchClassesForNamesInitialized = true;
        pitchClassesForNames_["Ab"] =   8.;
        pitchClassesForNames_["A" ] =   9.;
        pitchClassesForNames_["A#"] =  10.;
        pitchClassesForNames_["Bb"] =  10.;
        pitchClassesForNames_["B" ] =  11.;
        pitchClassesForNames_["B#"] =   0.;
        pitchClassesForNames_["Cb"] =  11.;
        pitchClassesForNames_["C" ] =   0.;
        pitchClassesForNames_["C#"] =   1.;
        pitchClassesForNames_["Db"] =   1.;
        pitchClassesForNames_["D" ] =   2.;
        pitchClassesForNames_["D#"] =   3.;
        pitchClassesForNames_["Eb"] =   3.;
        pitchClassesForNames_["E" ] =   4.;
        pitchClassesForNames_["E#"] =   5.;
        pitchClassesForNames_["Fb"] =   4.;
        pitchClassesForNames_["F" ] =   5.;
        pitchClassesForNames_["F#"] =   6.;
        pitchClassesForNames_["Gb"] =   6.;
        pitchClassesForNames_["G" ] =   7.;
        pitchClassesForNames_["G#"] =   8.;
    }
    return const_cast<std::map<std::string, double> &>(pitchClassesForNames_);
}

SILENCE_PUBLIC double pitchClassForName(std::string name) {
    const std::map<std::string, double> &pitchClassesForNames_ = pitchClassesForNames();
    std::map<std::string, double>::const_iterator it = pitchClassesForNames_.find(name);
    if (it == pitchClassesForNames_.end()) {
        return DBL_MAX;
    } else {
        return it->second;
    }
}

/**
 * Returns the name of the pitch-class of the pitch.
 * The first of enharmonic names is always used, sorry. 
 * If there is no matching name, an empty string is returned.
 */
SILENCE_PUBLIC std::string nameForPitchClass(double pitch) {
    auto pc = epc(pitch);
    const std::map<std::string, double> &pitchClassesForNames_ = pitchClassesForNames();
    for (auto it = pitchClassesForNames_.begin(); it != pitchClassesForNames_.end(); ++it) {
        if (eq_tolerance(it->second, pc) == true) {
            return it->first;
        }
    }
    return "";
}

SILENCE_PUBLIC std::multimap<Chord, std::string, ChordTickLess> &namesForChords() {
    static std::multimap<Chord, std::string, ChordTickLess> namesForChords_;
    return namesForChords_;
}

SILENCE_PUBLIC std::map<std::string, Chord> &chordsForNames() {
    static std::map<std::string, Chord> chordsForNames_;
    return chordsForNames_;
}

SILENCE_PUBLIC std::multimap<Scale, std::string, ChordTickLess> &namesForScales() {
    static std::multimap<Scale, std::string, ChordTickLess> namesForScales_;
    return namesForScales_;
}

SILENCE_PUBLIC std::map<std::string, Scale> &scalesForNames() {
    static std::map<std::string, Scale> scalesForNames_;
    return scalesForNames_;
}

SILENCE_PUBLIC std::set<Chord, ChordTickLess> &unique_chords() {
    static std::set<Chord, ChordTickLess> unique_chords_;
    return unique_chords_;
}

SILENCE_PUBLIC std::set<Scale, ChordTickLess> &unique_scales() {
    static std::set<Scale, ChordTickLess> unique_scales_;
    return unique_scales_;
}

SILENCE_PUBLIC void add_chord(std::string name, const Chord &chord) {
    unique_chords().insert(chord);
    chordsForNames().insert(std::make_pair(name, chord));
    namesForChords().insert(std::make_pair(chord, name));
}

SILENCE_PUBLIC void add_scale(std::string name, const Scale &scale) {
    unique_scales().insert(scale);
    scalesForNames().insert(std::make_pair(name, scale));
    namesForScales().insert(std::make_pair(scale, name));
}

SILENCE_PUBLIC std::vector<std::string> split(std::string string_) {
    std::vector<std::string> tokens;
    std::istringstream iss(string_);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter<std::vector<std::string> >(tokens));
    return tokens;
}

void fill(std::string rootName, double rootPitch, std::string typeName, std::string typePitches, bool is_scale) {
    Chord chord;
    std::string chordName = rootName + typeName;
    std::vector<std::string> splitPitches = split(typePitches);
    CHORD_SPACE_DEBUG("chordName: %s = rootName: %s  rootPitch: %f  typeName: %s  typePitches: %s\n", chordName.c_str(), rootName.c_str(), rootPitch, typeName.c_str(), typePitches.c_str());
    chord.resize(splitPitches.size());
    for (int voice = 0, voiceN = splitPitches.size(); voice < voiceN; ++voice) {
        double pitch = pitchClassForName(splitPitches[voice]);
        CHORD_SPACE_DEBUG("voice: %3d  pc: %-4s  pitch: %9.4f\n", voice, splitPitches[voice].c_str(), pitch);
        chord.setPitch(voice, pitch);
    }
    CHORD_SPACE_DEBUG("chord type: %s\n", chord.toString().c_str());
    chord = chord.T(rootPitch);
    Chord eOP_ = chord.eOP();
    CHORD_SPACE_DEBUG("eOP_:   %s  chordName: %s\n", eOP_.toString().c_str(), chordName.c_str());
    ///chordsForNames()[chordName] = eOP_;
    ///namesForChords()[eOP_] = chordName;
    add_chord(chordName, eOP_);
    if (is_scale == true) {
        Scale scale(chordName, chord);
        ///scalesForNames()[chordName] = scale;
        ///namesForScales()[scale] = chordName;
        add_scale(chordName, scale);
    }
}

void initializeNames() {
    static bool initializeNamesInitialized = false;
    if (!initializeNamesInitialized) {
        initializeNamesInitialized = true;
        CHORD_SPACE_DEBUG("Initializing chord names...\n");
        const std::map<std::string, double> &pitchClassesForNames_ = pitchClassesForNames();
        for (std::map<std::string, double>::const_iterator it = pitchClassesForNames_.begin();
                it != pitchClassesForNames_.end();
                ++it) {
            const std::string &rootName = it->first;
            const double &rootPitch = it->second;
            CHORD_SPACE_DEBUG("rootName: %-3s  rootPitch: %9.5f\n", rootName.c_str(), rootPitch);
            fill(rootName, rootPitch, " minor second",     "C  C#                             ");
            fill(rootName, rootPitch, " major second",     "C     D                           ");
            fill(rootName, rootPitch, " minor third",      "C        Eb                       ");
            fill(rootName, rootPitch, " major third",      "C           E                     ");
            fill(rootName, rootPitch, " perfect fourth",   "C              F                  ");
            fill(rootName, rootPitch, " tritone",          "C                 F#              ");
            fill(rootName, rootPitch, " perfect fifth",    "C                    G            ");
            fill(rootName, rootPitch, " augmented fifth",  "C                       G#        ");
            fill(rootName, rootPitch, " sixth",            "C                          A      ");
            fill(rootName, rootPitch, " minor seventh  ",  "C                             Bb  ");
            fill(rootName, rootPitch, " major seventh",    "C                                B");
            // Scales.
            fill(rootName, rootPitch, " major",            "C     D     E  F     G     A     B", true);
            fill(rootName, rootPitch, " minor",            "C     D  Eb    F     G  Ab    Bb  ", true);
            fill(rootName, rootPitch, " natural minor",    "C     D  Eb    F     G  Ab    Bb  ", true);
            fill(rootName, rootPitch, " harmonic minor",   "C     D  Eb    F     G  Ab       B", true);
            fill(rootName, rootPitch, " chromatic",        "C  C# D  D# E  F  F# G  G# A  A# B", true);
            fill(rootName, rootPitch, " whole tone",       "C     D     E     F#    G#    A#  ", true);
            fill(rootName, rootPitch, " diminished",       "C     D  D#    F  F#    G# A     B", true);
            fill(rootName, rootPitch, " pentatonic",       "C     D     E        G     A      ", true);
            fill(rootName, rootPitch, " pentatonic major", "C     D     E        G     A      ", true);
            fill(rootName, rootPitch, " pentatonic minor", "C        Eb    F     G        Bb  ", true);
            fill(rootName, rootPitch, " augmented",        "C        Eb E        G  Ab    Bb  ", true);
            fill(rootName, rootPitch, " Lydian dominant",  "C     D     E     Gb G     A  Bb  ", true);
            fill(rootName, rootPitch, " 3 semitone",       "C        D#       F#       A      ", true);
            fill(rootName, rootPitch, " 4 semitone",       "C           E           G#        ", true);
            fill(rootName, rootPitch, " blues",            "C     D  Eb    F  Gb G        Bb  ", true);
            fill(rootName, rootPitch, " bebop",            "C     D     E  F     G     A  Bb B", true);
            // Modes.
            fill(rootName, rootPitch, " Ionian",           "C     D     E  F     G     A     B", true);
            fill(rootName, rootPitch, " Dorian",           "C     D  Eb    F     G     A  Bb  ", true);
            fill(rootName, rootPitch, " Phrygian",         "C  Db    Eb    F     G  Ab    Bb  ", true);
            fill(rootName, rootPitch, " Lydian",           "C     D     E     F# G     A     B", true);
            fill(rootName, rootPitch, " Mixolydian",       "C     D     E  F     G     A  Bb  ", true);
            fill(rootName, rootPitch, " Aeolian",          "C     D  Eb    F     G  Ab    Bb  ", true);
            fill(rootName, rootPitch, " Locrian",          "C  Db    Eb    F  Gb    Ab    Bb B", true);
            // Major chords.
            fill(rootName, rootPitch, "M",                 "C           E        G            ");
            fill(rootName, rootPitch, "6",                 "C           E        G     A      ");
            fill(rootName, rootPitch, "69",                "C     D     E        G     A      ");
            fill(rootName, rootPitch, "69b5",              "C     D     E     Gb       A      ");
            fill(rootName, rootPitch, "M7",                "C           E        G           B");
            fill(rootName, rootPitch, "M9",                "C     D     E        G           B");
            fill(rootName, rootPitch, "M11",               "C     D     E  F     G           B");
            fill(rootName, rootPitch, "M#11",              "C     D     E  F#    G           B");
            fill(rootName, rootPitch, "M13",               "C     D     E  F     G     A     B");
            // Minor chords.
            fill(rootName, rootPitch, "m",                 "C        Eb          G            ");
            fill(rootName, rootPitch, "m6",                "C        Eb          G     A      ");
            fill(rootName, rootPitch, "m69",               "C     D  Eb          G     A      ");
            fill(rootName, rootPitch, "m7",                "C        Eb          G        Bb  ");
            fill(rootName, rootPitch, "m7b9",              "C  Db    Eb          G        Bb  ");
            fill(rootName, rootPitch, "m7b9b5",            "C  Db    Eb       Gb          Bb  ");
            fill(rootName, rootPitch, "m#7",               "C        Eb          G           B");
            fill(rootName, rootPitch, "m7b5",              "C        Eb       Gb          Bb  ");
            fill(rootName, rootPitch, "m9",                "C     D  Eb          G        Bb  ");
            fill(rootName, rootPitch, "m9#7",              "C     D  Eb          G           B");
            fill(rootName, rootPitch, "m11",               "C     D  Eb    F     G        Bb  ");
            fill(rootName, rootPitch, "m#11",              "C     D  Eb    F     G        Bb  ");
            fill(rootName, rootPitch, "m13",               "C     D  Eb    F     G     A  Bb  ");
            // Augmented chords.
            fill(rootName, rootPitch, "+",                 "C            E         G#         ");
            fill(rootName, rootPitch, "7#5",               "C            E         G#     Bb  ");
            fill(rootName, rootPitch, "7b9#5",             "C  Db        E         G#     Bb  ");
            fill(rootName, rootPitch, "9#5",               "C     D      E         G#     Bb  ");
            // Diminished chords.
            fill(rootName, rootPitch, "o",                 "C        Eb       Gb              ");
            fill(rootName, rootPitch, "o7",                "C        Eb       Gb       A      ");
            // Suspended chords.
            fill(rootName, rootPitch, "6sus",              "C              F     G     A      ");
            fill(rootName, rootPitch, "69sus",             "C     D        F     G     A      ");
            fill(rootName, rootPitch, "7sus",              "C              F     G        Bb  ");
            fill(rootName, rootPitch, "9sus",              "C     D        F     G        Bb  ");
            fill(rootName, rootPitch, "M7sus",             "C              F     G           B");
            fill(rootName, rootPitch, "M9sus",             "C     D        F     G           B");
            // Dominant chords.
            fill(rootName, rootPitch, "7",                 "C            E       G        Bb  ");
            fill(rootName, rootPitch, "7b5",               "C            E    Gb          Bb  ");
            fill(rootName, rootPitch, "7b9",               "C  Db        E       G        Bb  ");
            fill(rootName, rootPitch, "7b9b5",             "C  Db        E    Gb          Bb  ");
            fill(rootName, rootPitch, "9",                 "C     D      E       G        Bb  ");
            fill(rootName, rootPitch, "9#11",              "C     D      E F#    G        Bb  ");
            fill(rootName, rootPitch, "13",                "C     D      E F     G     A  Bb  ");
            fill(rootName, rootPitch, "13#11",             "C     D      E F#    G     A  Bb  ");
        }
    }
}

/**
 * Returns all enharmonic names for the Chord, if any exists. If none exists, 
 * an empty result is returned.
 */
SILENCE_PUBLIC std::vector<std::string> namesForChord(const Chord &chord) {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initializeNames();
    }
    std::multimap<Chord, std::string, ChordTickLess> &namesForChords_ = namesForChords();
    std::vector<std::string> result;
    auto matches = namesForChords_.equal_range(chord);
    for (auto it = matches.first; it != matches.second; ++it) {
        result.push_back(it->second);
    }
    return result;
}

/**
 * Returns the first valid name for the Chord. If none exists, an empty result 
 * is returned.
 */
SILENCE_PUBLIC std::string nameForChord(const Chord &chord) {
    auto result = namesForChord(chord);
    if (result.size() == 0) {
        return "";
    } else {
        return result[0];
    }
}

SILENCE_PUBLIC const Chord &chordForName(std::string name) {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initializeNames();
    }
    const std::map<std::string, Chord> &chordsForNames_ = chordsForNames();
    std::map<std::string, Chord>::const_iterator it = chordsForNames_.find(name);
    if (it == chordsForNames_.end()) {
        static Chord chord;
        chord.resize(0);
        return chord;
    } else {
        return it->second;
    }
}

/**
 * Returns all enharmonic names for the Scale, if any exists. If none exists, 
 * an empty result is returned.
 */
SILENCE_PUBLIC std::vector<std::string> namesForScale(const Scale &scale) {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initializeNames();
    }
    std::multimap<Scale, std::string, ChordTickLess> &namesForScales_ = namesForScales();
    std::vector<std::string> result;
    auto matches = namesForScales_.equal_range(scale);
    for (auto it = matches.first; it != matches.second; ++it) {
        result.push_back(it->second);
    }
    return result;
}

/**
 * Returns the first valid name for the Scale.
 */
SILENCE_PUBLIC std::string nameForScale(const Scale &scale) {
    auto result = namesForScale(scale);
    if (result.size() == 0) {
        return "";
    } else {
        return result[0];
    }
}

SILENCE_PUBLIC const Scale &scaleForName(std::string name) {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initializeNames();
    }
    const std::map<std::string, Scale> &scalesForNames_ = scalesForNames();
    std::map<std::string, Scale>::const_iterator it = scalesForNames_.find(name);
    if (it == scalesForNames_.end()) {
        static Scale scale;
        scale.resize(0);
        return scale;
    } else {
        return it->second;
    }
}

SILENCE_PUBLIC std::string print_chord(const Chord &chord) {
    std::string result;
    static char buffer[0x8000];
    snprintf(buffer, sizeof(buffer), "%s   ", chord.toString().c_str());
    result.append(buffer);
    for (int sector = 0; sector < chord.voices(); ++sector) {
        bool belongs = chord.is_in_rpt_sector(sector);
        std::string membership;
        if (belongs == true) {  
            bool minor = chord.is_in_minor_rpti_sector(sector);
            if (minor == true) {
                membership = "m";
            } else {                
                membership = "M";     
            }
        } else {            
            membership = " ";
        }
        snprintf(buffer, sizeof(buffer), "[OPT %2d %s]", sector, membership.c_str());
        result.append(buffer);    
    }
    return result;
}

SILENCE_PUBLIC std::string print_opti_sectors(const Chord &chord) {
    std::string result;
    char buffer[0x8000];
    for (int sector = 0; sector < chord.voices(); ++sector) {
        bool belongs = chord.is_in_rpt_sector(sector);
        std::string membership;
        if (belongs == true) {  
            bool minor = chord.is_in_minor_rpti_sector(sector);
            if (minor == true) {
                membership = "minor";
            } else {
                membership = "major";     
            }
        } else {
            membership = "     ";
        }
        snprintf(buffer, sizeof(buffer), "RPT sector %3d: %s\n", sector, membership.c_str());
        result.append(buffer);
    }
    return result;    
}

std::string Chord::information_debug(int sector_) const {
    SCOPED_DEBUGGING debugging;
    return information_sector(sector_);
}

std::string Chord::information() const {
    return information_sector(-1);
}

std::string Chord::information_sector(int opt_sector_) const {
    std::string result;
    char buffer[0x10000];
    if (voices() < 1) {
        return "Empty chord.";
    }
    snprintf(buffer, sizeof(buffer), "\nCHORD:\n");
    result.append(buffer);
    int opt_sector = 0;
    if (opt_sector_ == -1) {
        auto sectors = opt_domain_sectors();
        if (sectors.empty() == false)
        {
            opt_sector = sectors.front();
        }
        else
        {
            opt_sector = 0; // or -1, but 0 avoids later indexing crashes in info printing
        }    
    }
    snprintf(buffer, sizeof(buffer), "%-19s %s\n", name().c_str(), print_chord(*this).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "Pitch-class set:    %s\n", epcs().eP().toString().c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "Normal order:       %s\n", normal_order().toString().c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "Normal form:        %s\n", normal_form().toString().c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "Prime form:         %s\n", prime_form().toString().c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "Inverse prime form: %s\n", inverse_prime_form().toString().c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "Sum:                %12.7f\n", layer());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "O:           %3d => %s\n", iseO(), print_chord(eO()).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "P:           %3d => %s\n", iseP(), print_chord(eP()).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "T:           %3d => %s\n", iseT(), print_chord(eT()).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "TT:          %3d => %s\n", iseTT(), print_chord(eTT()).c_str());
    result.append(buffer);
    auto isei = iseI(opt_sector);
    auto ei = eI(opt_sector);
    snprintf(buffer, sizeof(buffer), "I:           %3d => %s\n", isei, print_chord(ei).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OP:          %3d => %s\n", iseOP(), print_chord(eOP()).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OT:          %3d => %s\n", iseOT(), print_chord(eOT()).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OTT:         %3d => %s\n", iseOTT(1.0), print_chord(eOTT()).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OPT:         %3d => %s\n", iseOPT(opt_sector), print_chord(eOPT(opt_sector)).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OPTT:        %3d => %s\n", iseOPTT(1.0, opt_sector), print_chord(eOPTT(opt_sector)).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OPI:         %3d => %s\n", iseOPI(opt_sector), print_chord(eOPI(opt_sector)).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OPTI:        %3d => %s\n", iseOPTI(opt_sector), print_chord(eOPTI(opt_sector)).c_str());
    result.append(buffer);
    snprintf(buffer, sizeof(buffer), "OPTTI:       %3d => %s\n", iseOPTTI(1.0, opt_sector), print_chord(eOPTTI(opt_sector)).c_str());
    result.append(buffer);
    auto &hyperplane_equations = hyperplane_equations_for_dimensionalities()[voices()];
    auto &opt_sectors = opt_sectors_for_dimensionalities()[voices()];
    for (int sector = 0, n = opt_sectors.size(); sector < n; ++sector) {
        snprintf(buffer, sizeof(buffer), "OPT sector:  %3d\n", sector);
        result.append(buffer);
        for (int vertex = 0; vertex < voices(); ++vertex) {
            snprintf(buffer, sizeof(buffer), "    vertex:  %3d:   %s\n", vertex, opt_sectors[sector][vertex].toString().c_str());
            result.append(buffer);
        }    
        const auto &hyperplane_equation_ = hyperplane_equation(sector);
        auto text = hyperplane_equation_.toString();
        snprintf(buffer, sizeof(buffer), "    hyperplane equation:\n       %s\n", text.c_str());
        result.append(buffer);
        auto sector_text = print_opti_sectors(*this);
        snprintf(buffer, sizeof(buffer), "    this:           %s\n", print_chord(*this).c_str());
        result.append(buffer);      
        auto reflected = reflect_in_inversion_flat(*this, sector);
        sector_text = print_opti_sectors(reflected);
        snprintf(buffer, sizeof(buffer), "    reflected:      %s\n", print_chord(reflected).c_str());
        result.append(buffer);      
        auto rereflected = reflect_in_inversion_flat(reflected, sector);
        sector_text = print_opti_sectors(rereflected);
        snprintf(buffer, sizeof(buffer), "    re-reflected:   %s\n", print_chord(rereflected).c_str());
        result.append(buffer);      
        result.append("\n");
    }
    return result;
}

SILENCE_PUBLIC int indexForOctavewiseRevoicing(const Chord &origin, const Chord &chord, double range) {
    int revoicingN = octavewiseRevoicings(chord, range);
    Chord revoicing = origin;
    int revoicingI = 0;
    while (true) {
        CHORD_SPACE_DEBUG("indexForOctavewiseRevoicing of %s in range %7.3f: %5d of %5d: %s\n",
              chord.toString().c_str(),
              range,
              revoicingI,
              revoicingN,
              revoicing.toString().c_str());
        if (revoicing == chord) {
            return revoicingI;
        }
        (void) next(revoicing, origin, range, OCTAVE());
        revoicingI++;
        if (revoicingI > revoicingN) {
            return -1;
        }
    }
}

SILENCE_PUBLIC int indexForOctavewiseRevoicing(const Chord &chord, double range) {
    const auto origin = csound::equate<EQUIVALENCE_RELATION_RP>(chord, OCTAVE(), 1.0, 0);
    auto result = indexForOctavewiseRevoicing(origin, chord, range);
    return result;
}

SILENCE_PUBLIC bool operator == (const Chord &a, const Chord &b) {
    if (&a == &b) {
        return true;
    }
    if (a.voices() != b.voices()) {
        return false;
    }
    for (size_t voice = 0; voice < a.voices(); ++voice) {
        if (!eq_tolerance(a.getPitch(voice), b.getPitch(voice))) {
            return false;
        }
    }
    return true;
}

SILENCE_PUBLIC bool operator < (const Chord &a, const Chord &b) {
    size_t n = std::min(a.voices(), b.voices());
    for (size_t voice = 0; voice < n; voice++) {
        if (lt_tolerance(a.getPitch(voice), b.getPitch(voice))) {
            return true;
        }
        if (gt_tolerance(a.getPitch(voice), b.getPitch(voice))) {
            return false;
        }
    }
    if (a.voices() < b.voices()) {
        return true;
    }
    return false;
}

SILENCE_PUBLIC bool operator < (const Scale &a, const Scale &b) {
    size_t n = std::min(a.voices(), b.voices());
    for (size_t voice = 0; voice < n; voice++) {
        if (lt_tolerance(a.getPitch(voice), b.getPitch(voice))) {
            return true;
        }
        if (gt_tolerance(a.getPitch(voice), b.getPitch(voice))) {
            return false;
        }
    }
    if (a.voices() < b.voices()) {
        return true;
    }
    return false;
}

SILENCE_PUBLIC bool operator <= (const Chord &a, const Chord &b) {
    if (a == b) {
        return true;
    }
    return (a < b);
}

SILENCE_PUBLIC bool operator > (const Chord &a, const Chord &b) {
    size_t n = std::min(a.voices(), b.voices());
    for (size_t voice = 0; voice < n; voice++) {
        if (gt_tolerance(a.getPitch(voice), b.getPitch(voice))) {
            return true;
        }
        if (lt_tolerance(a.getPitch(voice), b.getPitch(voice))) {
            return false;
        }
    }
    if (a.voices() > b.voices()) {
        return true;
    }
    return false;
}

SILENCE_PUBLIC bool operator >= (const Chord &a, const Chord &b) {
    if (a == b) {
        return true;
    }
    return (a > b);
}

SILENCE_PUBLIC double C4() {
    return MIDDLE_C();
}

SILENCE_PUBLIC Chord chord(const Chord &scale, int scale_degree, int chord_voices, int interval) {
    int scale_index = scale_degree - 1;
    int scale_interval = interval - 1;
    Chord result;
    result.resize(chord_voices);
    double octave = 0.;
    for (int chord_voice = 0; chord_voice < chord_voices; ++chord_voice) {
        if (scale_index >= scale.voices()) {
            scale_index = scale_index - scale.voices();
            octave = octave + OCTAVE();
        }
        auto pitch = scale.getPitch(scale_index) + octave;
        result.setPitch(chord_voice, pitch);
        scale_index = scale_index + scale_interval;
    }
    return result;
}

void Chord::ensure_sectors_initialized()
{
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
        initialize_sectors();
    }
}

Chord::Chord() {
    resize(0);
}

Chord::Chord(int voices_) {
    resize(voices_);    
}

Chord::Chord(const Chord &other) = default;

Chord::Chord(const std::vector<double> &other) {
    *this = other;
}

Chord &Chord::operator = (const Chord &other) {
    Matrix::operator=(other);
    return *this;
}

Chord &Chord::operator = (const std::vector<double> &other) {
    auto voices_n = other.size();
    resize(voices_n);
    for (size_t voice = 0; voice < voices_n; ++voice) {
        setPitch(voice, other[voice]);
    }
    return *this;
}

Chord::operator std::vector<double>() const {
    std::vector<double> result;
    result.resize(voices());
    for (size_t voice = 0; voice < voices(); ++voice) {
        result.push_back(getPitch(voice));
    }
    return result;
}
#if __cpplusplus >= 201103L
Chord &Chord::operator = (Chord &&other) = default;
#endif
Chord::~Chord() {
}

size_t Chord::voices() const {
    return rows();
}

void Chord::resize(size_t voiceN) {
    Matrix::resize(voiceN, COUNT);
}

bool Chord::test(const char *label) const {
    std::fprintf(stderr, "\nTESTING %s %s\n\n", toString().c_str(), label);
    bool passed = true;
    // For some of these we need to know the OPT sector.
    auto opt_sector = opt_domain_sectors().front();
    // Test idempotency of transformations: 
    // equate<R>(equate<R>(chord, sector), sector) == equate<R>(chord, sector)
    if (eOP().eOP().iseOP() == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOP is not idempotent.\n");
    } else {
        std::fprintf(stderr, "        Chord::eOP is idempotent.\n");
    }
    if (eOPT(opt_sector).eOPT(opt_sector).iseOPT(opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOPT is not idempotent.\n");
    } else {
        std::fprintf(stderr, "        Chord::eOPT is idempotent.\n");
    }
    if (eOPTT(1.0, opt_sector).eOPTT(1.0, opt_sector).iseOPTT(1.0, opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOPTT is not idempotent.\n");
    } else {
        std::fprintf(stderr, "        Chord::eOPTT is idempotent.\n");
    }
    if (eOPTI(opt_sector).eOPTI(opt_sector).iseOPTI(opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOPTI is not idempotent.\n");
    } else {
        std::fprintf(stderr, "        Chord::eOPTI is idempotent.\n");
    }
    if (eOPTTI(1.0, opt_sector).eOPTTI(1.0, opt_sector).iseOPTTI(1.0, opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOPTTI is not idempotent.\n");
    } else {
        std::fprintf(stderr, "        Chord::eOPTTI is idempotent.\n");
    }
    // Test the consistency of the transformations:
    // predicate<R>(equate<R>(chord, sector), sector) == true
    if (eO().iseO() == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eO is not consistent with Chord::iseO.\n");
    } else {
        std::fprintf(stderr, "        Chord::eO is consistent with Chord::iseO.\n");
    }
    if (eP().iseP() == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eP is not consistent with Chord::iseP.\n");
    } else {
        std::fprintf(stderr, "        Chord::eP is consistent with Chord::iseP.\n");
    }
    if (eT().iseT() == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eT is not consistent with Chord::iseT.\n");
    } else {
        std::fprintf(stderr, "        Chord::eT is consistent with Chord::iseT.\n");
    }
    if (eTT(1.0).iseTT(1.0) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eTT is not consistent with Chord::iseTT.\n");
    } else {
        std::fprintf(stderr, "        Chord::eTT is consistent with Chord::iseTT.\n");
    }
    auto ei = eI(opt_sector);
    if (ei.iseI(opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eI is not consistent with eI::iseI.\n");
    } else {
        std::fprintf(stderr, "        Chord::eI is consistent with eI::iseI.\n");
    }
    if (eOP().iseOP() == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOP is not consistent with Chord::iseOP.\n");
    } else {
        std::fprintf(stderr, "        Chord::eOP is consistent with Chord::iseOP.\n");
    }
    if (eOPT(opt_sector).is_in_rpt_sector(opt_sector, 12.) == false) {
        passed = false;
        auto opt_chord = eOPT(opt_sector);
        std::fprintf(stderr, "Failed: Chord::eOPT is not consistent with Chord::iseOPT (%s => %s).\n", toString().c_str(), opt_chord.toString().c_str());
    } else {
        std::fprintf(stderr, "        Chord::eOPT is consistent with Chord::iseOPT.\n");
    }
    if (eOPTT(1.0, opt_sector).iseOPTT(1.0, opt_sector) == false) {
        passed = false;
        auto optt_chord = eOPTT(1.0, opt_sector);
        std::fprintf(stderr, "Failed: Chord::eOPTT is not consistent with Chord::iseOPTT (%s => %s).\n", toString().c_str(), optt_chord.toString().c_str());
    } else {
        std::fprintf(stderr, "        Chord::eOPTT is consistent with Chord::iseOPTT.\n");
    }
    auto opti_chord = eOPTI(opt_sector);
    if (opti_chord.iseOPTI(opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOPTI is not consistent with Chord::iseOPTI (%s => %s).\n", toString().c_str(), opti_chord.toString().c_str());
    } else {
        std::fprintf(stderr, "        Chord::eOPTI is consistent with Chord::iseOPTI.\n");
    }
    auto optti_chord = eOPTTI(1.0, opt_sector);
    if (optti_chord.iseOPTTI(1.0, opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOPTTI is not consistent with Chord::iseOPTTI  (%s => %s).\n", toString().c_str(), optti_chord.toString().c_str());
    } else {
        std::fprintf(stderr, "        Chord::eOPTTI is consistent with Chord::iseOPTTI.\n");
    }
    // Test the decomposability of the predicates. The predicate for a 
    // compound equivalence relation must return the same truth value as the 
    // conjunction of predicates for each elementary relation.
    if (iseOP() == true) {
        if (iseO() == false ||
            iseP() == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOP is not decomposable.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOP is decomposable.\n");
        }
    }
    if (iseOPT(opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseT() == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPT is not decomposable.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPT is decomposable.\n");
        }
    }
    // If it is transformed to T, is it OPT? 
    // After that, is it Tg?
    if (iseOPTT(1.0, opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseTT(1.0) == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPTT is not decomposable.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPTT is decomposable.\n");
        }
    }
    if (iseOPTI(opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseT() == false || 
            iseI(opt_sector) == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPTI is not decomposable.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPTI is decomposable.\n");
        }
    }
    if (iseOPTTI(1.0, opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseTT(1.0) == false || 
            iseI(opt_sector) == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPTTI is not decomposable.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPTTI is decomposable.\n");
        }
    }
    std::fprintf(stderr, "\n");
    std::fprintf(stderr, "%s", information().c_str());
    return passed;
}

/**
 * Returns a string representation of the chord's pitches (only).
 * Quadratic complexity, but short enough not to matter.
 */
std::string Chord::toString() const {
    char buffer[0x1000];
    std::stringstream stream;
    for (size_t voice = 0; voice < voices(); ++voice) {
        std::snprintf(buffer, 0x100, "%12.7f", getPitch(voice));
        if (voice > 0) {
            stream << " ";
        }
        stream << buffer;
    }
    auto text = stream.str();
    // This string representation may be used as a key. In such cases, ensure 
    // that -0 == 0 (as it does for numeric comparisons).
    boost::replace_all(text, " -0.", "  0.");
    return text;
}

/**
 * Rebuilds the chord's pitches (only) from a line of text.
 */
void Chord::fromString(std::string text) {
    double scalar;
    std::vector<double> vector_;
    std::stringstream stream(text);
    while (stream >> scalar) {
        vector_.push_back(scalar);
    }
    resize(vector_.size());
    for (int i = 0, n = vector_.size(); i < n; ++i) {
        setPitch(i, vector_[i]);
    }
}

double Chord::getPitch(int voice) const {
    return coeff(voice, PITCH);
}

double &Chord::getPitchReference(int voice) {
    return coeffRef(voice, PITCH);
}

void Chord::setPitch(int voice, double value) {
    coeffRef(voice, PITCH) = value;
}

double Chord::getDuration(int voice) const {
    return coeff(voice, DURATION);
}

void Chord::setDuration(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, DURATION) = value;
        }
    } else {
        coeffRef(voice, DURATION) = value;
    }
}

double Chord::getLoudness(int voice) const {
    return coeff(voice, LOUDNESS);
}

void Chord::setLoudness(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, LOUDNESS) = value;
        }
    } else {
        coeffRef(voice, LOUDNESS) = value;
    }
}

double Chord::getInstrument(int voice) const {
    return coeff(voice, INSTRUMENT);
}

void Chord::setInstrument(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, INSTRUMENT) = value;
        }
    } else {
        coeffRef(voice, INSTRUMENT) = value;
    }
}

double Chord::getPan(int voice) const {
    return coeff(voice, PAN);
}

void Chord::setPan(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, PAN) = value;
        }
    } else {
        coeffRef(voice, PAN) = value;
    }
}

size_t Chord::count(double pitch) const {
    size_t n = 0;
    for (size_t voice = 0; voice < voices(); ++voice) {
        if (eq_tolerance(getPitch(voice), pitch)) {
            n++;
        }
    }
    return n;
}

bool Chord::contains(double pitch_) const {
    for (size_t voice = 0; voice < voices(); voice++) {
        if (eq_tolerance(getPitch(voice), pitch_)) {
            return true;
        }
    }
    return false;
}

std::vector<double> Chord::min() const {
    std::vector<double> result(2);
    result[0] = getPitch(0);;
    result[1] = 0;
    for (int voice = 1; voice < voices(); ++voice) {
        auto pitch = getPitch(voice);
        if (lt_tolerance(pitch, result[0]) == true) {
            result[0] = pitch;
            result[1] = voice;
        }
    }
    return result;}

std::vector<double> Chord::max() const {
    std::vector<double> result(2);
    result[0] = getPitch(0);;
    result[1] = 0;
    for (int voice = 1; voice < voices(); ++voice) {
        auto pitch = getPitch(voice);
        if (gt_tolerance(pitch, result[0]) == true) {
            result[0] = pitch;
            result[1] = voice;
        }
    }
    return result;
}

double Chord::minimumInterval() const {
    double minimumInterval_ = std::abs(getPitch(0) - getPitch(1));
    for (size_t v1 = 0; v1 < voices(); v1++) {
        for (size_t v2 = 0; v2 < voices(); v2++) {
            double interval = std::abs(getPitch(v1) - getPitch(v2));
            if (lt_tolerance(interval, minimumInterval_)) {
                minimumInterval_ = interval;
            }
        }
    }
    return minimumInterval_;
}

double Chord::maximumInterval() const {
    double maximumInterval_ = std::abs(getPitch(0) - getPitch(1));
    for (size_t v1 = 0; v1 < voices(); v1++) {
        for (size_t v2 = 0; v2 < voices(); v2++) {
            double interval = std::abs(getPitch(v1) - getPitch(v2));
            if (gt_tolerance(interval, maximumInterval_)) {
                maximumInterval_ = interval;
            }
        }
    }
    return maximumInterval_;
}

Chord Chord::floor() const {
    Chord clone = *this;
    for (size_t voice = 0; voice  < voices(); voice++) {
        clone.setPitch(voice, std::floor(getPitch(voice)));
    }
    return clone;
}

#if 0
Chord Chord::ceiling(double g) const {
    SCOPED_DEBUGGING debugging;
    Chord result = *this;
    for (size_t voice = 0; voice  < voices(); voice++) {
        result.setPitch(voice, std::ceil(getPitch(voice)));
    }
    CHORD_SPACE_DEBUG("Chord::ceiling: %s\n", print_chord(*this).c_str());
    CHORD_SPACE_DEBUG("             => %s\n", print_chord(result).c_str());
    result.clamp(g);
    return result;
}
#endif

// TODO: Keep track of this.

Chord Chord::ceiling(double g) const
{
    Chord out = *this;

    for (int v = 0; v < voices(); ++v)
    {
        const double x = out.getPitch(v);
        const double q = x / g;

        const double qi = std::round(q);
        if (eq_tolerance(q, qi))
        {
            out.setPitch(v, qi * g);
            continue;
        }

        const double eps = 1e-12;
        out.setPitch(v, std::ceil(q - eps) * g);
    }

    return out;
}


Chord Chord::origin() const {
    Chord origin_;
    origin_.resize(voices());
    return origin_;
}

double Chord::distanceToOrigin() const {
    Chord origin_ = origin();
    return euclidean(*this, origin_);
}

double Chord::layer() const {
    double sum = 0.0;
    for (size_t voice = 0; voice < voices(); ++voice) {
        sum += getPitch(voice);
    }
    return sum;
}

double Chord::distanceToUnisonDiagonal() const {
    Chord unison = origin();
    double pitch = layer() / double(voices());
    for (size_t voice = 0; voice < voices(); voice ++) {
        unison.setPitch(voice, pitch);
    }
    return euclidean(*this, unison);
}

Chord Chord::center() const {
    Chord clone = *this;
    double even_ = OCTAVE() / voices();
    for (size_t voice = 0; voice < voices(); voice++) {
        clone.setPitch(voice, voice * even_);
    }
    return clone;
}

Chord Chord::T(double interval) const {
    Chord clone = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        clone.setPitch(voice, csound::T(getPitch(voice), interval));
    }
    return clone;
}

Chord Chord::T_voiceleading(const Chord &voiceleading) {
    Chord clone = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        clone.setPitch(voice, getPitch(voice) + voiceleading.getPitch(voice));
    }
    return clone;
}

Chord Chord::voiceleading(const Chord &destination) const {
    Chord voiceleading_ = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        voiceleading_.setPitch(voice, destination.getPitch(voice) - getPitch(voice));
    }
    return voiceleading_;
}

Chord Chord::I(double center) const {
    Chord inverse = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        inverse.setPitch(voice, csound::I(getPitch(voice), center));
    }
    return inverse;
}

Chord Chord::cycle(int stride) const {
    Chord permuted = *this;
    int voicesToPopAndPush = std::abs(stride) % voices();
    int voicesToShift = voices() - voicesToPopAndPush;
    if (stride < 0) {
        permuted.bottomRows(voicesToShift) = topRows(voicesToShift);
        permuted.topRows(voicesToPopAndPush) = bottomRows(voicesToPopAndPush);
    }
    if (stride > 0) {
        permuted.topRows(voicesToShift) = bottomRows(voicesToShift);
        permuted.bottomRows(voicesToPopAndPush) = topRows(voicesToPopAndPush);
    }
    return permuted;
}

std::vector<Chord> Chord::permutations() const {
    std::vector<Chord> permutations_;
    Chord permutation = *this;
    permutations_.push_back(permutation);
    for (size_t i = 1; i < voices(); i++) {
        permutation = permutation.cycle();
        permutations_.push_back(permutation);
    }
    std::sort(permutations_.begin(), permutations_.end(), ChordTickLess());
    return permutations_;
}

Chord Chord::v(int direction, double interval) const {
    Chord chord = *this;
    int head = voices() - 1;
    while (direction > 0) {
        chord = chord.cycle(1);
        chord.setPitch(head, chord.getPitch(head) + interval);
        direction--;
    }
    while (direction < 0) {
        chord = chord.cycle(-1);
        chord.setPitch(0, chord.getPitch(0) - interval);
        direction++;
    }
    return chord;
}

std::vector<Chord> Chord::voicings(int direction, double interval) const {
    Chord chord = *this;
    std::vector<Chord> voicings;
    voicings.push_back(chord);
    for (size_t voicing = 1; voicing < voices(); voicing++) {
        chord = chord.v(direction, interval);
        voicings.push_back(chord);
    }
    return voicings;
}

bool Chord::isepcs() const {
    for (size_t voice = 0; voice < voices(); voice++) {
        if (!eq_tolerance(getPitch(voice), epc(getPitch(voice)))) {
            return false;
        }
    }
    return true;
}

Chord Chord::epcs() const {
    Chord chord = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        chord.setPitch(voice, epc(getPitch(voice)));
    }
    return chord;
}

Chord Chord::eppcs() const {
    Chord chord = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        chord.setPitch(voice, epc(getPitch(voice)));
    }
    return chord.eP();
}

bool Chord::iset() const {
    Chord et_ = et();
    if (!(*this == et_)) {
        return false;
    }
    return true;
}

Chord Chord::et() const {
    double min_ = min()[0];
    return T(-min_);
}

bool Chord::iseO() const {
    return iseR(OCTAVE());
}

Chord Chord::eO() const {
    return eR(OCTAVE());
}

bool Chord::iseI(int opt_sector) const {
    return iseI_chord(nullptr, opt_sector);
}

bool Chord::iseOP() const {
    return iseRP(OCTAVE());
}

Chord Chord::eOP() const {
    return eRP(OCTAVE());
}

bool Chord::iseOPT(int opt_sector) const {
    return iseRPT(OCTAVE(), opt_sector);
}

bool Chord::iseOPTT(double g, int opt_sector) const {
    return iseRPTT(OCTAVE(), g, opt_sector);
}

Chord Chord::eOPT(int opt_sector) const {
    return eRPT(OCTAVE(), opt_sector);
}

Chord Chord::eOPTT(double g, int opt_sector) const {
    return eRPTT(OCTAVE(), g, opt_sector);
}

bool Chord::iseOPI(int opt_sector) const {
    return iseRPI(OCTAVE(), opt_sector);
}

Chord Chord::eOPI(int opt_sector) const {
    return eRPI(OCTAVE(), opt_sector);
}

bool Chord::iseOPTI(int opt_sector) const {
    return iseRPTI(OCTAVE(), opt_sector);
}

bool Chord::iseOPTTI(double g, int opt_sector) const {
    return iseRPTTI(OCTAVE(), g, opt_sector);
}

Chord Chord::eOPTI(int opt_sector) const {
    return eRPTI(OCTAVE(), opt_sector);
}

Chord Chord::eOPTTI(double g, int opt_sector) const {
    return eRPTTI(OCTAVE(), g, opt_sector);
}

std::string Chord::name() const {
    std::string name_ = nameForChord(*this);
    return name_;
}

Chord Chord::move(int voice, double interval) const {
    Chord chord = *this;
    chord.setPitch(voice, csound::T(getPitch(voice), interval));
    return chord;
}

Chord Chord::nrR() const {
    //System::message("Chord::nrR: this:                               %s\n", toString().c_str());
    auto normal_form_ = normal_form();
    //System::message("Chord::nrR: normal_form_:                       %s\n", normal_form_.toString().c_str());
    auto normal_form_transformed = normal_form_;
    auto transposition_of_normal_form = (normal_form_.layer() - layer()) / voices();
    //System::message("Chord::nrR: transposition_of_normal_form:       %9.4f\n", transposition_of_normal_form);
    if (normal_form_.getPitch(1) == 4.0) {
        normal_form_transformed.setPitch(2, normal_form_.getPitch(2) + 2.0);
    } else if (normal_form_.getPitch(1) == 3.0) {
        normal_form_transformed.setPitch(0, normal_form_.getPitch(0) - 2.0);
    }
    //System::message("Chord::nrR: normal_form_transformed:            %s\n", normal_form_transformed.toString().c_str());
    auto normal_form_transformed_transposed = normal_form_transformed.T(-transposition_of_normal_form);
    //System::message("Chord::nrR: normal_form_transformed_transposed: %s\n", normal_form_transformed_transposed.toString().c_str());
    auto result = normal_form_transformed_transposed.eOP();
    //System::message("Chord::nrR: result:                             %s\n", result.toString().c_str());
    return result;
}

Chord Chord::nrP() const {
    //System::message("Chord::nrP: this:                               %s\n", toString().c_str());
    auto normal_form_ = normal_form();
    //System::message("Chord::nrP: normal_form_:                       %s\n", normal_form_.toString().c_str());
    auto normal_form_transformed = normal_form_;
    auto transposition_of_normal_form = (normal_form_.layer() - layer()) / voices();
    //System::message("Chord::nrP: transposition_of_normal_form:       %9.4f\n", transposition_of_normal_form);
    if (normal_form_.getPitch(1) == 4.0) {
        normal_form_transformed.setPitch(1, normal_form_.getPitch(1) - 1.0);
    } else if (normal_form_.getPitch(1) == 3.0) {
        normal_form_transformed.setPitch(1, normal_form_.getPitch(1) + 1.0);
    }
    //System::message("Chord::nrP: normal_form_transformed:            %s\n", normal_form_transformed.toString().c_str());
    auto normal_form_transformed_transposed = normal_form_transformed.T(-transposition_of_normal_form);
    //System::message("Chord::nrP: normal_form_transformed_transposed: %s\n", normal_form_transformed_transposed.toString().c_str());
    auto result = normal_form_transformed_transposed.eOP();
    //System::message("Chord::nrP: result:                             %s\n", result.toString().c_str());
    return result;
}

Chord Chord::nrL() const {
    //System::message("Chord::nrL: this:                               %s\n", toString().c_str());
    auto normal_form_ = normal_form();
    //System::message("Chord::nrL: normal_form_:                       %s\n", normal_form_.toString().c_str());
    auto normal_form_transformed = normal_form_;
    auto transposition_of_normal_form = (normal_form_.layer() - layer()) / voices();
    //System::message("Chord::nrL: normal_form_transformed:            %s\n", normal_form_transformed.toString().c_str());
    if (normal_form_.getPitch(1) == 4.0) {
        normal_form_transformed.setPitch(0, normal_form_.getPitch(0) - 1.0);
    } else if (normal_form_.getPitch(1) == 3.0) {
        normal_form_transformed.setPitch(2, normal_form_.getPitch(2) + 1.0);
        transposition_of_normal_form += 8.;
    }
    //System::message("Chord::nrL: transposition_of_normal_form:       %9.4f\n", transposition_of_normal_form);
    auto normal_form_transformed_transposed = normal_form_transformed.T(-transposition_of_normal_form);
    //System::message("Chord::nrL: normal_form_transformed_transposed: %s\n", normal_form_transformed_transposed.toString().c_str());
    auto result = normal_form_transformed_transposed.eOP();
    //System::message("Chord::nrL: result:                             %s\n", result.toString().c_str());
    return result;
}

Chord Chord::nrN() const {
    return nrR().nrL().nrP();
}

Chord Chord::nrS() const {
    return nrL().nrP().nrR();
}

Chord Chord::nrH() const {
    return nrL().nrP().nrL();
}

Chord Chord::nrD() const {
    return T(-7.0). eOP();
}

Chord Chord::K() const {
    Chord chord = *this;
    if (chord.voices() < 2) {
        return chord;
    }
    // A _pitch-class segment_ is an _unordered_ collection of pitch-classes, 
    // which can contain repeated pitch-classes (Morris); this is Chord::epcs 
    // in CsoundAC. Everything here is done in octave equivalence.
    Chord epcs_ = epcs();
    double center = epc(epcs_.getPitch(0) + epcs_.getPitch(1));
    for (size_t voice = 0; voice < voices(); voice++) {
        auto pitch = epcs_.getPitch(voice);
        auto pitch_inverted = center - pitch;
        auto pitch_inverted_pc = epc(pitch_inverted);
        chord.setPitch(voice, epc(pitch_inverted_pc));
        //std::fprintf(stderr, "Chord::K: %3d center %9.4f p %9.4f p_i %9.4f p_i_pc: %9.4f\n", voice, center, pitch, pitch_inverted, pitch_inverted_pc
    }
    return chord;
}

Chord Chord::K_range(double range) const {
    Chord chord = K();
    return chord.eRP(range);
}

bool Chord::Tform(const Chord &Y, double g) const {
    Chord eopx = epcs().eP();
    double i = 0.0;
    while (i < OCTAVE()) {
        Chord ty = Y.T(i);
        Chord eopty = ty.epcs().eP();
        if (eopx == eopty) {
            return true;
        }
        i = i + g;
    }
    return false;
}

bool Chord::Iform(const Chord &Y, double g) const {
    Chord eopx = epcs().eP();
    double i = 0.0;
    while (i < OCTAVE()) {
        Chord iy = Y.I(i);
        Chord eopiy = iy.epcs().eP();
        if (eopx == eopiy) {
            return true;
        }
        i = i + g;
    }
    return false;
}

Chord Chord::Q(double x, const Chord &m, double g) const {
    if (Tform(m, g)) {
        return T(x);
    }
    if (Iform(m, g)) {
        return T(-x);
    }
    return *this;
}

Chord Chord::a(int arpeggiation, double &resultPitch, int &resultVoice) const {
    Chord resultChord = v(arpeggiation);
    if (arpeggiation < 0) {
        resultVoice = resultChord.voices() - 1;
    } else {
        resultVoice = 0;
    }
    resultPitch = resultChord.getPitch(resultVoice);
    return resultChord;
}

bool Chord::equals(const Chord &other) const {
    return *this == other;
}

bool Chord::greater(const Chord &other) const {
    return *this > other;
}

bool Chord::greater_equals(const Chord &other) const {
    return *this >= other;
}

bool Chord::lesser(const Chord &other) const {
    return *this < other;
}

bool Chord::lesser_equals(const Chord &other) const {
    return *this <= other;
}

SILENCE_PUBLIC double closestPitch(double pitch, const Chord &chord) {
    std::map<double, double> pitchesForDistances;
    for (int voice = 0; voice < chord.voices(); ++voice) {
        double chordPitch = chord.getPitch(voice);
        double distance = std::fabs(chordPitch - pitch);
        pitchesForDistances[distance] = chordPitch;
    }
    return pitchesForDistances.begin()->second;
}

SILENCE_PUBLIC double conformToPitchClassSet(double pitch, const Chord &pcs) {
    double pc_ = epc(pitch);
    double closestPc = closestPitch(pc_, pcs);
    double register_ = std::floor(pitch / OCTAVE()) * OCTAVE();
    double closestPitch = register_ + closestPc;
    return closestPitch;
}

SILENCE_PUBLIC double epc(double pitch) {
    double pc = modulo(pitch, OCTAVE());
    return pc;
}

// See: 
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// https://bitbashing.io/comparing-floats.html
// https://www.boost.org/doc/libs/1_72_0/libs/test/doc/html/boost_test/testing_tools/extended_comparison/floating_point/floating_points_comparison_theory.html
//
// A floating point number can be in the following states:
//
// (1) Normal - OK to compare.
// (2) Denormal - OK to compare.
// (3) Max - Automatically unequal.
// (4) Inf - Automatically unequal.
// (5) NaN - Automatically unequal.
//
// If a and b differ in sign, they are not equal, unless one is zero and the other is negative zero.
// If a or b are max, inf, or nan, they are not equal.
// If difference = fabs(a - b) is max, inf, or nan, the numbers are not equal; difference is subject to catastrophic cancellation near 0.
// If a or b is near zero, the tolerance is multiples of machine epsilon, and the result is difference <= tolerance.
// If a and b are not near zero, the tolerance is multiples of ulps, and the result is difference <= tolerance.

SILENCE_PUBLIC bool eq_tolerance(double a, double b, int epsilons, int ulps) {
    static const double machine_epsilon = std::numeric_limits<double>::epsilon();
    static const double double_max_ = std::numeric_limits<double>::max();
    if (a == b) {
        CHORD_SPACE_DEBUG("eq_tolerance: a and b are strictly equal:\n    => return true.\n");
        return true;
    }
    if ((a == double_max_ || b == double_max_) == true) {
        CHORD_SPACE_DEBUG("eq_tolerance: a or b is double_max_:\n    => return false.\n");
        return false;
    }
    if ((std::isinf(a) || std::isinf(b)) == true) {
        CHORD_SPACE_DEBUG("eq_tolerance: a or b is inf:\n    => return false.\n");
        return false;
    }
    if ((std::isnan(a) || std::isnan(b)) == true) {
        CHORD_SPACE_DEBUG("eq_tolerance: a or b is nan:\n    => return false.\n");
        return false;
    }
    double difference = a - b;
    // True means the number is negative.
    if (std::signbit(difference) == true) {
        difference = -difference;
    }
    if ((difference == double_max_) == true) {
        CHORD_SPACE_DEBUG("eq_tolerance: difference of and b is double_max_:\n    => return false.\n");
        return false;
    }
    if (std::isinf(difference) == true) {
         CHORD_SPACE_DEBUG("eq_tolerance: difference of and b is inf:\n    => return false.\n");
        return false;
    }
    if (std::isnan(difference) == true) {
         CHORD_SPACE_DEBUG("eq_tolerance: difference of and b is nan:\n    => return false.\n");
        return false;
    }
    double tolerance = epsilons * machine_epsilon;
    if ((a == 0. || b == 0.) == true || difference <= tolerance) {
        if (difference <= tolerance) {
            CHORD_SPACE_DEBUG("eq_tolerance: a or b is strictly equal to 0 and difference of a and b <= epsilons tolerance:\n    => return true.\n");
            return true;
        } else {
            CHORD_SPACE_DEBUG("eq_tolerance: a or b is strictly equal to 0 and difference of a and b > epsilons tolerance:\n    => return false.\n");
            return false;
        }
    } else {
        double difference_ulp = boost::math::ulp(difference);
        tolerance = difference_ulp * ulps;
        if (difference <= tolerance) {
            CHORD_SPACE_DEBUG("eq_tolerance: a or b are not strictly equal to 0 and difference of a and b <= ulps tolerance:\n    => return true.\n");
            return true;
        } else {
            CHORD_SPACE_DEBUG("eq_tolerance: a or b are not strictly equal to 0 and difference of a and b > ulps tolerance:\n    => return false.\n");
            return false;
        }
    }
}

SILENCE_PUBLIC double euclidean(const Chord &a, const Chord &b) {
    double sumOfSquaredDifferences = 0.0;
    const size_t voices = a.voices();
    for (size_t voice = 0; voice < voices; ++voice) {
        sumOfSquaredDifferences += std::pow((a.getPitch(voice) - b.getPitch(voice)), 2.0);
    }
    return std::sqrt(sumOfSquaredDifferences);
}

SILENCE_PUBLIC double factorial(double n) {
    double result = 1.0;
    for (int i = 0; i <= n; ++i) {
        result = result * i;
    }
    return result;
}

SILENCE_PUBLIC bool ge_tolerance(double a, double b, int epsilons, int ulps) {
    if (eq_tolerance(a, b, epsilons, ulps)) {
        return true;
    } else {
       if (a > b) {
           return true;
       } else {
           return false;
       }
    }
}

SILENCE_PUBLIC bool gt_tolerance(double a, double b, int epsilons, int ulps) {
    if (eq_tolerance(a, b, epsilons, ulps)) {
        return false;
    } else {
       if (a > b) {
           return true;
       } else {
           return false;
       }
    }
}

SILENCE_PUBLIC double I(double pitch, double center) {
    return center - pitch;
}

/**
 * Returns the sum of the distances of the chord to each of one or more chords.
 */
SILENCE_PUBLIC double distance_to_points(const Chord &chord, const std::vector<Chord> &sector_vertices) {
    double sum = 0;
    for (auto vertex : sector_vertices) {
        auto distance = euclidean(chord, vertex);
        sum = sum + distance;
    }
    return sum;
}

template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC bool predicate(const Chord &chord, double range, int sector) {
    bool result = predicate<EQUIVALENCE_RELATION>(chord, range, 1.0);
    return result;
}

template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC bool predicate(const Chord &chord, double range) {
    bool result = predicate<EQUIVALENCE_RELATION>(chord, range, 1.0);
    return result;
}

template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC bool predicate(const Chord &chord) {
    bool result = predicate<EQUIVALENCE_RELATION>(chord, OCTAVE());
    return result;
}

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_r>(const Chord &chord, double range, double g, int opt_sector) {
    for (int voice = 0; voice < chord.voices(); ++voice) {
        double pitch = chord.getPitch(voice);
        if (le_tolerance(0., pitch) == false) {
            return false;
        }
        if (lt_tolerance(pitch, range) == false) {
            return false;
        }
    }
    return true;
}

SILENCE_PUBLIC Chord iterator(int voiceN, double first) {
    Chord odometer;
    odometer.resize(voiceN);
    for (int voice = 0; voice < voiceN; ++voice) {
        odometer.setPitch(voice, first);
    }
    return odometer;
}

SILENCE_PUBLIC bool le_tolerance(double a, double b, int epsilons, int ulps) {
    if (eq_tolerance(a, b, epsilons, ulps)) {
        return true;
    } else {
        if (a < b) {
            return true;
        } else {
            return false;
        }
    }
}

SILENCE_PUBLIC bool lt_tolerance(double a, double b, int epsilons, int ulps) {
    if (eq_tolerance(a, b, epsilons, ulps)) {
        return false;
    } else {
        if (a < b) {
            return true;
        } else {
            return false;
        }
    }
}

SILENCE_PUBLIC Chord midpoint(const Chord &a, const Chord &b) {
    Chord midpoint_ = a;
    for (int voice = 0, voices = a.voices(); voice < voices; ++voice) {
        double voiceSum = a.getPitch(voice) + b.getPitch(voice);
        double voiceMidpoint = voiceSum / 2.0;
        midpoint_.setPitch(voice, voiceMidpoint);
    }
    /// CHORD_SPACE_DEBUG("a: %s  b: %s  mid: %s\n", a.toString().c_str(), b.toString().c_str(), midpoint_.toString().c_str());
    return midpoint_;
}

SILENCE_PUBLIC double MIDDLE_C() {
    return 60.0;
}

SILENCE_PUBLIC double modulo(double dividend, double divisor) {
    double quotient = 0.0;
    if (le_tolerance(divisor, 0.) == true) {
        quotient = std::ceil(dividend / divisor);
    }
    if (gt_tolerance(divisor, 0.) == true) {
        quotient = std::floor(dividend / divisor);
    }
    double remainder = dividend - (quotient * divisor);
    return remainder;
}

SILENCE_PUBLIC bool next(Chord &iterator_, const Chord &origin, double range_, double increment) {
    // The actual range of iteration is from the least pitch of the origin, to 
    // the least pitch of the origin plus the range.
    double minimum_pitch = origin.min().front();
    double maximum_pitch = minimum_pitch + range_;
    // We are treating the chord as an odometer, with voices as "places", but 
    // the least significant place is the leftmost voice, not the rightmost 
    // digit as it would be with a decimal numeral. This reflects common 
    // musical practice.
    for (int voice = 0, voice_n = iterator_.voices(); voice < voice_n; ) {
        double pitch = iterator_.getPitch(voice);
        double incremented_pitch = pitch + increment;
        if (le_tolerance(incremented_pitch, maximum_pitch) == true) {
            // If there is no need to carry, just increment the pitch of the 
            // current voice, and return true to indicate that iteration can 
            // continue.
            iterator_.setPitch(voice, incremented_pitch);
            return true;
        } else {
            // Otherwise, reset this "place" and carry over to the next.
            iterator_.setPitch(voice, origin.getPitch(voice));
            voice++;
        }
    }
    // We have used up all our "places," so return false to indicate that 
    // iteration has finished.
    return false;
}

template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC Chord equate(const Chord &chord, double range) {
    return equate<EQUIVALENCE_RELATION>(chord, range, 1.0);
}

template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC Chord equate(const Chord &chord) {
    return equate<EQUIVALENCE_RELATION>(chord, OCTAVE());
}

SILENCE_PUBLIC double OCTAVE() {
    return 12.0;
}

SILENCE_PUBLIC Chord octavewiseRevoicing(const Chord &chord, int revoicingNumber_, double range) {
    int revoicingN = octavewiseRevoicings(chord, range);
    if (revoicingN == 0) {
        revoicingN = 1;
    }
    int revoicingNumber = revoicingNumber_ % revoicingN;
    Chord origin = csound::equate<EQUIVALENCE_RELATION_RP>(chord, OCTAVE(), 1.0, 0);
    Chord revoicing = origin;
    int revoicingI = 0;
    while (true) {
        CHORD_SPACE_DEBUG("octavewiseRevoicing %d (%d) of %s in range %7.3f: %5d: %s\n",
              revoicingNumber,
              revoicingNumber_,
              chord.toString().c_str(),
              range,
              revoicingI,
              revoicing.toString().c_str());
         if (revoicingI == revoicingNumber) {
            return revoicing;
        }
        (void) next(revoicing, origin, range, OCTAVE());
        revoicingI++;
    }
    return origin;
}

SILENCE_PUBLIC int octavewiseRevoicings(const Chord &chord,
        double range) {
    Chord origin = chord.eOP();
    Chord odometer = origin;
    // Enumerate the permutations.
    int voicings = 0;
    while (next(odometer, origin, range, OCTAVE())) {
        voicings = voicings + 1;
    }
    CHORD_SPACE_DEBUG("octavewiseRevoicings: chord:    %s\n", chord.toString().c_str());
    CHORD_SPACE_DEBUG("octavewiseRevoicings: eop:      %s\n", chord.eOP().toString().c_str());
    CHORD_SPACE_DEBUG("octavewiseRevoicings: odometer: %s\n", odometer.toString().c_str());
    CHORD_SPACE_DEBUG("octavewiseRevoicings: voicings: %5d\n", voicings);
    return voicings;
}

SILENCE_PUBLIC bool parallelFifth(const Chord &a, const Chord &b) {
    Chord voiceleading_ = voiceleading(a, b);
    if (voiceleading_.count(7) > 1) {
        return true;
    } else {
        return false;
    }
}

SILENCE_PUBLIC Vector reflect_vector(const Vector &v, const Vector &u, double c) {
    ///SCOPED_DEBUGGING debugging;
    CHORD_SPACE_DEBUG("reflect_vector: v: \n%s\nu: \n%s\nc: %g\n", toString(v).c_str(), toString(u).c_str(), c);
    auto v_dot_u = v.dot(u);
    CHORD_SPACE_DEBUG("reflect_vector: v_dot_u: %g\n", double(v_dot_u));
    auto v_dot_u_minus_c = v_dot_u - c;
    CHORD_SPACE_DEBUG("reflect_vector: v_dot_u_minus_c: %g\n", double(v_dot_u_minus_c));
    auto u_dot_u = u.dot(u);
    CHORD_SPACE_DEBUG("reflect_vector: u_dot_u: %g\n", double(u_dot_u));
    auto quotient = v_dot_u_minus_c / u_dot_u;
    CHORD_SPACE_DEBUG("reflect_vector: quotient: %g\n", double(quotient));
    auto subtrahend = u * (2. * quotient);
    //CHORD_SPACE_DEBUG("reflect_vector: subtrahend: %g\n", double(subtrahend));
    auto reflection = v - subtrahend;
    CHORD_SPACE_DEBUG("reflect_vector: reflection:\n%s\n \n", toString(reflection).c_str());
    return reflection;
}

SILENCE_PUBLIC Vector reflect_vectorx(const Vector &v, const Vector &u, double c) {
    auto v_dot_u = v.dot(u);
    auto v_dot_u_minus_c = v_dot_u - c;
    auto u_dot_u = u.dot(u);
    auto quotient = v_dot_u_minus_c / u_dot_u;
    auto subtrahend = u * (2. * quotient);
    auto reflection = v - subtrahend;
    return reflection;
}

/**
 * Computes the Householder reflector matrix and applies it to the chord.
 * The transformation is: H(p) = p - 2 * u * (u^T * p).
 * The corresponding matrix is: I - 2 * u * u^T.
 */
SILENCE_PUBLIC Chord reflect_by_householder(const Chord &chord) {
    auto opt_domain_sectors_ = chord.opt_domain_sectors().front();
    auto hyperplane_equation = chord.hyperplane_equation(opt_domain_sectors_);
    CHORD_SPACE_DEBUG("reflect_by_householder: chord:              %s\n", chord.toString().c_str());
    CHORD_SPACE_DEBUG("reflect_by_householder: unit normal vector: \n%s\n", toString(hyperplane_equation.unit_normal).c_str());
    auto center_ = chord.center().eT();
    CHORD_SPACE_DEBUG("reflect_by_householder: center:             %s\n", center_.toString().c_str());
    auto tensor = hyperplane_equation.unit_normal.col(0) * hyperplane_equation.unit_normal.col(0).transpose();
    CHORD_SPACE_DEBUG("reflect_by_householder: tensor: \n%s\n", toString(tensor).c_str());
    auto product = 2. * tensor;
    CHORD_SPACE_DEBUG("reflect_by_householder: product:  \n%s\n", toString(product).c_str());
    auto identity = Matrix::Identity(center_.voices(), center_.voices());
    CHORD_SPACE_DEBUG("reflect_by_householder: identity:  \n%s\n", toString(identity).c_str());
    auto householder = identity - product;
    CHORD_SPACE_DEBUG("reflect_by_householder: householder:  \n%s\n", toString(householder).c_str());
    auto moved_to_origin = chord.col(0) - center_.col(0);
    CHORD_SPACE_DEBUG("reflect_by_householder: moved_to_origin:  \n%s\n", toString(moved_to_origin).c_str());
    auto reflected = householder * moved_to_origin;
    CHORD_SPACE_DEBUG("reflect_by_householder: reflected:  \n%s\n", toString(reflected).c_str());
    auto moved_from_origin = reflected + center_.col(0);
    CHORD_SPACE_DEBUG("reflect_by_householder: moved_from_origin:  \n%s\n", toString(moved_from_origin).c_str());
    Chord reflection_ = chord;
    for (int voice = 0, n = chord.voices(); voice < n; ++voice) {
        reflection_.setPitch(voice, moved_from_origin(voice, 0));
    }
    CHORD_SPACE_DEBUG("reflect_by_householder: reflection_:        %s\n\n", reflection_.toString().c_str());
    return reflection_;
}

SILENCE_PUBLIC Chord reflect_in_central_diagonal(const Chord &chord) {
    auto sum = chord.layer();
    auto transposition = sum / chord.voices();
    auto inversion_point = chord.center().T(transposition);
    auto reflection = chord;
    for (auto voice = 0; voice < chord.voices(); ++voice) {
        auto chord_voice = reflection.getPitch(voice);
        auto center_voice = inversion_point.getPitch(voice);
        auto reflected_voice = (2. * center_voice) - chord_voice;
        reflection.setPitch(voice, reflected_voice);
    }
    return reflection;
}

SILENCE_PUBLIC Chord reflect_in_central_point(const Chord &chord) {
    auto inversion_point = chord.center();
    auto reflection = chord;
    for (auto voice = 0; voice < chord.voices(); ++voice) {
        auto chord_voice = reflection.getPitch(voice);
        auto center_voice = inversion_point.getPitch(voice);
        auto reflected_voice = (2. * center_voice) - chord_voice;
        reflection.setPitch(voice, reflected_voice);
    }
    return reflection;
}

SILENCE_PUBLIC Chord reflect_in_unison_diagonal(const Chord &chord) {
    auto sum = chord.layer();
    auto transposition = sum / chord.voices();
    auto inversion_point = chord.origin().T(transposition);
    auto reflection = chord;
    for (auto voice = 0; voice < chord.voices(); ++voice) {
        auto chord_voice = reflection.getPitch(voice);
        auto center_voice = inversion_point.getPitch(voice);
        auto reflected_voice = (2. * center_voice) - chord_voice;
        reflection.setPitch(voice, reflected_voice);
    }
    return reflection;
}

SILENCE_PUBLIC Chord reflect_in_inversion_flat(const Chord &chord, int opt_sector) {
    // Preserve non-pitch data in the chord.
    Chord result = chord;
    int dimensions = chord.voices();
    HyperplaneEquation hyperplane = chord.hyperplane_equation(opt_sector);
    auto reflected = hyperplane.reflect(chord);
    for (int voice = 0; voice < dimensions; ++voice) {
        result.setPitch(voice, reflected.getPitch(voice));
    }
    return result;
}

SILENCE_PUBLIC Chord scale(std::string name) {
    CHORD_SPACE_DEBUG("scale: for name: %s\n", name.c_str());
    auto scale = chordForName(name);
    if (scale.size() == 0) {
        return scale;
    }
    auto parts = split(name);
    auto tonic = pitchClassForName(parts.front());
    CHORD_SPACE_DEBUG("scale: tonic: %9.4f\n", tonic);
    CHORD_SPACE_DEBUG("scale: initially: %s\n", scale.toString().c_str());
    while (eq_tolerance(scale.getPitch(0), tonic) == false) {
        scale = scale.v();
        CHORD_SPACE_DEBUG("scale: revoicing: %s\n", scale.toString().c_str());
    }
    return scale;
}

SILENCE_PUBLIC Scale::Scale() {
    resize(0);
}

SILENCE_PUBLIC Scale::Scale(std::string name) {
    const Chord temporary = csound::scale(name);
    Matrix::operator=(temporary);
    if (temporary.voices() > 0) {
        auto space_at = name.find(' ');
        type_name = name.substr(space_at + 1);
    }
}

SILENCE_PUBLIC Scale::Scale(std::string name, const Chord &scale_pitches) {
    Scale temporary(name);
    if (temporary.voices() > 0) 
    {
        *this = temporary;
        return;
    } 
    resize(scale_pitches.size());
    for (int index = 0; index < voices(); ++index) {
        setPitch(index, scale_pitches.getPitch(index));
    }
    add_scale(name, *this);
}

SILENCE_PUBLIC Scale::Scale(std::string name, const std::vector<double> &scale_pitches) {
    resize(scale_pitches.size());
    for (int index = 0; index < voices(); ++index) {
        setPitch(index, scale_pitches[index]);
    }
    add_scale(name, *this);
}

SILENCE_PUBLIC Scale::~Scale() = default;

SILENCE_PUBLIC Scale &Scale::operator = (const Scale &other) {
    Chord::operator=(other);
    type_name = other.getTypeName();
    return *this;
}

SILENCE_PUBLIC Chord Scale::chord(int scale_degree, int voices, int interval) const {
    return csound::chord(*this, scale_degree, voices, interval);
}

SILENCE_PUBLIC void Scale::conform_with_alteration(Event &event, int scale_degree, int voices, int interval, int alteration) const {
    Chord pcs = chord(scale_degree, voices, interval);
    double pitch = event.getKey_tempered(12);
    pitch = conformToPitchClassSet(pitch, pcs);
    if (alteration != 0) {
        pitch = epc(pitch + alteration);
    }
    event.setKey(pitch);
}

SILENCE_PUBLIC void Scale::conform_with_interval(Event &event, int scale_degree, int voices, int interval) const {
    conform_with_alteration(event, scale_degree, voices, interval, 0);
}

SILENCE_PUBLIC void Scale::conform(Event &event, int scale_degree, int voices) const {
    conform_with_alteration(event, scale_degree, voices, 3, 0);
}

SILENCE_PUBLIC Chord Scale::transpose_degrees(const Chord &chord, int scale_degrees, int interval) const {
    return csound::transpose_degrees(*this, chord, scale_degrees, interval);
}

SILENCE_PUBLIC double Scale::semitones_for_degree(int scale_degree) const {
    int scale_degrees = voices();
    while(scale_degree < 1) {
        scale_degree = scale_degree + scale_degrees;
    }
    while (scale_degree > scale_degrees) {
        scale_degree = scale_degree - scale_degrees;
    }
    double pitch_of_tonic = tonic();
    double pitch_of_scale_degree = getPitch(scale_degree - 1);
    double semitones = pitch_of_scale_degree - pitch_of_tonic;
    return semitones;
}

SILENCE_PUBLIC Scale Scale::transpose_to_degree(int degrees) const {
    CHORD_SPACE_DEBUG("Scale::transpose_to_degree(%9.4f)...\n", degrees);
    double semitones = semitones_for_degree(degrees);
    return transpose(semitones);
}

SILENCE_PUBLIC Scale Scale::transpose(double semitones) const {
    Chord transposed_pitches = T(semitones);
    // Make sure the copy starts in octave 0.
    while (lt_tolerance(transposed_pitches.getPitch(0), 0) == true) {
        transposed_pitches = transposed_pitches.T(OCTAVE());
    }
    while (ge_tolerance(transposed_pitches.getPitch(0), OCTAVE()) == true) {
        transposed_pitches = transposed_pitches.T( - OCTAVE());
    }
    CHORD_SPACE_DEBUG("Scale::transpose: transposed_pitches(%f): %s\n", semitones, transposed_pitches.toString().c_str());
    // Create the copy with the name of the new tonic.
    CHORD_SPACE_DEBUG("Scale::transpose: original name: %s\n", name().c_str());
    auto tonic_name = nameForPitchClass(transposed_pitches.getPitch(0));
    Scale transposed_scale;
    transposed_scale.type_name = getTypeName();
    transposed_scale.resize(voices());
    for (int voice = 0; voice < voices(); ++voice) {
        transposed_scale.setPitch(voice, transposed_pitches.getPitch(voice));
    }
    CHORD_SPACE_DEBUG("Scale::transpose: new name: %s\n", transposed_scale.name().c_str());
    CHORD_SPACE_DEBUG("Scale::transpose: result: %s\n", transposed_scale.information().c_str());
    return transposed_scale;
}

SILENCE_PUBLIC std::string Scale::name() const {
    return nameForPitchClass(tonic()) + " " + type_name;
}

SILENCE_PUBLIC std::string Scale::getTypeName() const {
    return type_name;
}

SILENCE_PUBLIC double Scale::tonic() const {
    return getPitch(0);
}

SILENCE_PUBLIC int Scale::degree(const Chord &chord_, int interval) const {
    int chord_voices = chord_.voices();
    int scale_degrees = voices();
    Chord eop = chord_.eOP();
    for (int scale_degree = 1; scale_degree <= scale_degrees; ++scale_degree) {
        Chord chord_for_degree_eop = chord(scale_degree, chord_voices, interval).eOP();
        if (eop == chord_for_degree_eop) {
            return scale_degree;
        }
    }
    return -1;
}

SILENCE_PUBLIC void Scale::modulations_for_scale_types(std::vector<Scale> &result, const Chord &current_chord, int voices_, const std::vector<std::string> &type_names) const {
    result.clear();
    int current_degree = degree(current_chord);
    if (current_degree == -1) {
        return;
    }
    if (voices_ == -1) {
        voices_ = current_chord.voices();
    }
    Chord chord_ = chord(current_degree, voices_);
    for (auto scale : unique_scales()) {
        if (scale.degree(chord_) != -1) {
            if (std::find(type_names.begin(), type_names.end(), scale.getTypeName()) != type_names.end()) {
                if (std::find(result.begin(), result.end(), scale) == result.end()) {
                    result.push_back(scale);
                }
            }
        }
    }
}

SILENCE_PUBLIC std::vector<Scale> Scale::modulations_for_voices(const Chord &chord, int voices) const {
    std::vector<Scale> result;
    std::vector<std::string> type_names;
    type_names.push_back("major");
    type_names.push_back("harmonic minor");
    modulations_for_scale_types(result, chord, voices, type_names);
    return result;
}

SILENCE_PUBLIC std::vector<Scale> Scale::modulations(const Chord &chord) const {
    std::vector<Scale> result;
    std::vector<std::string> type_names;
    type_names.push_back("major");
    type_names.push_back("harmonic minor");
    modulations_for_scale_types(result, chord, chord.voices(), type_names);
    return result;
}

SILENCE_PUBLIC void Scale::relative_tonicizations_for_scale_types(std::vector<Scale> &result, const Chord &current_chord, int secondary_function, int voices, const std::vector<std::string> &type_names) const {
    result.clear();
    int current_degree = degree(current_chord);
    CHORD_SPACE_DEBUG("Scale::relative_tonicizations: chord: %.20s (%s) degree: %3d\n", current_chord.name().c_str(), current_chord.toString().c_str(), current_degree);
    if (current_degree == -1) {
        return;
    }
     if (voices == -1) {
        voices = current_chord.voices();
    }
    Chord chord_ = chord(current_degree, voices);
    CHORD_SPACE_DEBUG("Scale::relative_tonicizations: resized: %.20s (%s) degree: %3d\n", chord_.name().c_str(), chord_.toString().c_str(), current_degree);
    std::vector<Scale> modulations_ = modulations(chord_);
    for (auto modulation : modulations_) {
        int degree_in_modulation = modulation.degree(chord_);
        if (degree_in_modulation == secondary_function) {
            if (std::find(result.begin(), result.end(), modulation) == result.end()) {
                CHORD_SPACE_DEBUG("Scale::relative_tonicizations: modulation: %.20s (%s) degree of chord in modulation: %3d\n", modulation.name().c_str(), modulation.toString().c_str(), degree_in_modulation);
                result.push_back(modulation);
            }
        }
    }
}

SILENCE_PUBLIC std::vector<Scale> Scale::relative_tonicizations(const Chord &current_chord, int secondary_function, int voices) const {
    std::vector<Scale> result;
    std::vector<std::string> scale_types = {"major", "harmonic minor"};
    relative_tonicizations_for_scale_types(result, current_chord, secondary_function, voices, scale_types);
    return result;
}

SILENCE_PUBLIC std::vector<Chord> Scale::secondary(const Chord &current_chord, int secondary_function, int voices_) const {
    if (voices_ == -1) {
        voices_ = current_chord.voices();
    }
    std::vector<Scale> relative_tonicizations_ = relative_tonicizations(current_chord, secondary_function);
    std::vector<Chord> result;
    for (auto tonicization : relative_tonicizations_) {
        Chord mutation = tonicization.chord(secondary_function, voices_);
            if (std::find(result.begin(), result.end(), mutation) == result.end()) {
                result.push_back(mutation);
            }
    }
    return result;
}

SILENCE_PUBLIC std::vector<Scale> Scale::tonicizations(const Chord &current_chord, int voices) const {
    std::vector<Scale> result;
    int current_degree = degree(current_chord);
    CHORD_SPACE_DEBUG("Scale::tonicizations: chord: %.20s (%s) degree: %3d\n", current_chord.name().c_str(), current_chord.toString().c_str(), current_degree);
    if (current_degree == -1) {
        return result;
    }
    if (voices == -1) {
        voices = current_chord.voices();
    }
    Chord chord_ = chord(current_degree, voices);
    CHORD_SPACE_DEBUG("Scale::tonicizations: resized: %.20s (%s) degree: %3d\n", chord_.name().c_str(), chord_.toString().c_str(), current_degree);
    std::vector<Scale> modulations_ = modulations(chord_);
    for (auto modulation : modulations_) {
        int degree_in_modulation = modulation.degree(chord_);
        if (degree_in_modulation == 1) {
            if (std::find(result.begin(), result.end(), modulation) == result.end()) {
                CHORD_SPACE_DEBUG("Scale::tonicizations: modulation: %.20s (%s) degree of chord in modulation: %3d\n", modulation.name().c_str(), modulation.toString().c_str(), degree_in_modulation);
                result.push_back(modulation);
            }
        }
    }
    return result;
}

SILENCE_PUBLIC double T(double pitch, double semitones) {
    return pitch + semitones;
}

SILENCE_PUBLIC Chord transpose_degrees(const Chord &scale, const Chord &original_chord, int transposition_degrees, int interval) {
    int scale_degrees = scale.voices();
    int chord_voices = original_chord.voices();
    Chord original_eop = original_chord.eOP();
    for (int original_chord_index = 0; original_chord_index < scale_degrees; ++original_chord_index) {
        CHORD_SPACE_DEBUG("transpose_degrees: original_chord_index: %d scale_degrees: %d\n", original_chord_index, scale_degrees);
        Chord transposed = csound::chord(scale, original_chord_index + 1, chord_voices, interval);
        Chord transposed_eop = transposed.eOP();
        CHORD_SPACE_DEBUG("original_eop: %s\ntransposed_eop: %s\n", original_eop.information().c_str(), transposed_eop.information().c_str());
        if (original_eop == transposed_eop) {
            // Found the scale index of the original chord, now get the transposed chord.
            int target_index = original_chord_index + transposition_degrees;
            CHORD_SPACE_DEBUG("found chord, target_index: %d original_chord_index: %d transposition_degrees: %d\n", target_index, original_chord_index, transposition_degrees);
            // Transposition has sign. If negative, wrap.
            while (target_index < 0) {
                target_index = target_index + scale_degrees;
            }
            CHORD_SPACE_DEBUG("wrapped target_index: %d original_chord_index: %d transposition_degrees: %d\n", target_index, original_chord_index, transposition_degrees);
            Chord transposed_chord = csound::chord(scale, target_index + 1, chord_voices, interval);
            CHORD_SPACE_DEBUG("transposed_chord: %s\n", transposed_chord.toString().c_str());
            return transposed_chord;
        }
    }
    Chord empty_chord;
    empty_chord.resize(0);
    return empty_chord;
}

SILENCE_PUBLIC Chord voiceleading(const Chord &a, const Chord &b) {
    Chord voiceleading_ = a;
    for (int voice = 0; voice < a.voices(); ++voice) {
        voiceleading_.setPitch(voice, b.getPitch(voice) - a.getPitch(voice));
    }
    return voiceleading_;
}

SILENCE_PUBLIC Chord voiceleadingCloser(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels) {
    if (avoidParallels) {
        if (parallelFifth(source, d1)) {
            return d2;
        }
        if (parallelFifth(source, d2)) {
            return d1;
        }
    }
    double s1 = voiceleadingSmoothness(source, d1);
    double s2 = voiceleadingSmoothness(source, d2);
    if (s1 < s2) {
        return d1;
    }
    if (s2 > s1) {
        return d2;
    }
    return voiceleadingSimpler(source, d1, d2, avoidParallels);
}

SILENCE_PUBLIC Chord voiceleadingClosestRange(const Chord &source, const Chord &destination, double range, bool avoidParallels) {
    Chord destinationOP = destination.eOP();
    Chord d = destinationOP;
    Chord origin = source.eOP();
    Chord odometer = origin;
    while (next(odometer, origin, range, OCTAVE())) {
        Chord revoicing = odometer;
        for (int voice = 0; voice < revoicing.voices(); ++voice) {
            revoicing.setPitch(voice, revoicing.getPitch(voice) + destinationOP.getPitch(voice));
        }
        d = voiceleadingCloser(source, d, revoicing, avoidParallels);
    }
    return d;
}

SILENCE_PUBLIC Chord voiceleadingSimpler(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels) {
    if (avoidParallels) {
        if (parallelFifth(source, d1)) {
            return d2;
        }
        if (parallelFifth(source, d2)) {
            return d1;
        }
    }
    // TODO: Verify this.
    int s1 = voiceleading(source, d1).count(0.0);
    int s2 = voiceleading(source, d2).count(0.0);
    if (s1 > s2) {
        return d1;
    }
    if (s2 > s1) {
        return d2;
    }
    return d1;
}

SILENCE_PUBLIC Chord voiceleadingSmoother(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels, double range) {
    if (avoidParallels) {
        if (parallelFifth(source, d1)) {
            return d2;
        }
        if (parallelFifth(source, d2)) {
            return d1;
        }
    }
    double s1 = voiceleadingSmoothness(source, d1);
    double s2 = voiceleadingSmoothness(source, d2);
    if (s1 <= s2) {
        return d1;
    } else {
        return d2;
    }
}

SILENCE_PUBLIC double voiceleadingSmoothness(const Chord &a, const Chord &b) {
    double L1 = 0.0;
    for (int voice = 0; voice < a.voices(); ++voice) {
        L1 = L1 + std::abs(b.getPitch(voice) - a.getPitch(voice));
    }
    return L1;
}

std::map<int, std::vector<Chord>> &Chord::cyclical_regions_for_dimensionalities() {
    static std::map<int, std::vector<Chord>> cyclical_regions_for_dimensionalities_;
    Chord().ensure_sectors_initialized();
    return cyclical_regions_for_dimensionalities_;
}

std::map<int, std::vector<std::vector<Chord>>> &Chord::opt_sectors_for_dimensionalities() {
    static std::map<int, std::vector<std::vector<Chord>>> opt_sectors_for_dimensionalities_;
    Chord().ensure_sectors_initialized();
    return opt_sectors_for_dimensionalities_;
}

std::map<int, std::vector<std::vector<Chord>>> &Chord::opt_simplexes_for_dimensionalities() {
    static std::map<int, std::vector<std::vector<Chord>>> opt_simplexes_for_dimensionalities_;
    Chord().ensure_sectors_initialized();
    return opt_simplexes_for_dimensionalities_;
}

std::map<int, std::vector<HyperplaneEquation>> &Chord::hyperplane_equations_for_dimensionalities() {
    static std::map<int, std::vector<HyperplaneEquation>> hyperplane_equations_for_dimensionalities_;
    Chord().ensure_sectors_initialized();
    return hyperplane_equations_for_dimensionalities_;
}

const HyperplaneEquation &Chord::hyperplane_equation(int opt_sector) const
{
    auto &by_dim = hyperplane_equations_for_dimensionalities();
    auto it = by_dim.find(voices());
    if (it == by_dim.end() || opt_sector < 0 || opt_sector >= int(it->second.size()))
    {
        System::error("hyperplane_equation: missing dim=%d or bad sector=%d.\n", voices(), opt_sector);
        static HyperplaneEquation dummy;
        return dummy;
    }
    return it->second[size_t(opt_sector)];
}

// What we want is:
//
// 1) OPT fundamental domain (the OP/T base):
//    In OT-reduced coordinates (mod transposition), the OPT fundamental domain
//    is the (N−1)-dimensional simplex that is the convex hull of the N vertices
//    of the cyclical region. This simplex is the base of the OP hyperprism.
//
// 2) OPT sector decomposition:
//    The OPT fundamental domain tiles into N OPT sector polytopes. Each OPT
//    sector is the convex hull of:
//      - the OPT-domain center, and
//      - one (N−2)-dimensional facet of the cyclical-region simplex.
//    (For N=3 the facet has 2 vertices; for general N it has N−1 vertices.)
//
// 3) Extrusion to OP geometry:
//    Extruding the OPT base along the unison-diagonal direction by height 12/N
//    yields the OP fundamental hyperprism. Extruding each OPT sector yields the
//    corresponding OP sector hyperprism.
//
// 4) Inversion flats:
//    For each OPT sector there is an associated inversion hyperplane (“inversion
//    flat”) that bisects the sector hyperprism into minor and major halves.
//    In the base, this hyperplane passes through the OPT-domain center and the
//    midpoint of the sector’s base facet; in the full OP geometry it is the
//    extrusion of that bisector along the unison-diagonal direction.
//    Each inversion flat is represented computationally by a hyperplane equation
//    n·x = d (unit normal n and constant d).
//
// 5) OPTI representative domain:
//    Each OPT sector is split by its inversion flat into two OPTI half-sector
//    regions (minor and major). A fundamental domain for inversional equivalence
//    OPTI can be chosen as the union of the N minor half-sector regions.
//    (Equivalently, one may choose the union of the N major halves, but we take
//    the minor union as the representative.)
//
// 6) Extrusion to OPI geometry:
//    Extruding the OPTI half-sector regions by 12/N yields the corresponding
//    OPI half-sector hyperprisms, and extruding the inversion flats yields the
//    full inversion flats in OP space.
void Chord::initialize_sectors()
{
    static bool initialized = false;
    if (initialized)
    {
        return;
    }
    initialized = true;

    SCOPED_DEBUGGING scoped_debugging;

    auto cyclical_regions = cyclical_regions_for_dimensionalities();
    auto &opt_sectors_for_dimensionalities_ = opt_sectors_for_dimensionalities();
    auto &opt_simplexes_for_dimensionalities_ = opt_simplexes_for_dimensionalities();
    auto &hyperplane_equations_for_dimensions = hyperplane_equations_for_dimensionalities();

    for (int n = 3; n < 12; ++n)
    {
        CHORD_SPACE_DEBUG("initialize_sectors for %d dimensions:\n", n);

        const auto cyclical_region = cyclical_regions[n];

        std::vector<std::vector<Chord>> opt_domains;
        std::vector<std::vector<Chord>> opt_simplexes;
        std::vector<HyperplaneEquation> hyperplane_equations;

        // ------------------------------------------------------------
        // OPT base simplex vertices from octavewise revoicings of origin.
        // These remain distinct after eT().
        // ------------------------------------------------------------
        Chord origin(n);

        std::vector<Chord> base_vertices;
        base_vertices.reserve(size_t(n));

        for (int i = 0; i < n; ++i)
        {
            base_vertices.push_back(origin.v(i, OCTAVE()).eT());
        }

        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                if (base_vertices[i] == base_vertices[j])
                {
                    System::error("initialize_sectors: duplicate base vertices for n=%d (i=%d j=%d).\n",
                                  n, i, j);
                }
            }
        }

        // Common apex: centroid of base vertices (center of OPT base).
        Chord sum(n);
        for (int i = 0; i < n; ++i)
        {
            sum += base_vertices[i];
            CHORD_SPACE_DEBUG("  base vertex %d: %s\n", i, base_vertices[i].toString().c_str());
        }

        Chord apex(n);
        /// Crashes in WASM: apex.col(0) = sum / double(n);
        for (int voice = 0; voice < n; ++voice) {
            apex.setPitch(voice, sum.getPitch(voice) / double(n));
        }   
        CHORD_SPACE_DEBUG("  apex (centroid): %s\n", apex.toString().c_str());

        // ------------------------------------------------------------
        // Sector k: { apex, all base vertices except base_vertices[k] }.
        // vertices[0] is apex as required by HyperplaneEquation::create(vertices).
        // ------------------------------------------------------------
        for (int k = 0; k < n; ++k)
        {
            std::vector<Chord> sector_vertices;
            sector_vertices.reserve(size_t(n));

            sector_vertices.push_back(apex);

            for (int v = 0; v < n; ++v)
            {
                if (v == k)
                {
                    continue;
                }
                sector_vertices.push_back(base_vertices[v]);
            }

            if (int(sector_vertices.size()) != n)
            {
                System::error("initialize_sectors: sector %d in n=%d has %d vertices (expected %d).\n",
                              k, n, int(sector_vertices.size()), n);
            }

            auto opt_domain = cyclical_region;
            for (const auto &vx : sector_vertices)
            {
                opt_domain.push_back(vx);
            }
            opt_domains.push_back(opt_domain);
            opt_simplexes.push_back(sector_vertices);

            for (int vi = 0; vi < int(sector_vertices.size()); ++vi)
            {
                CHORD_SPACE_DEBUG("  OPT sector %2d vertex %2d: %s\n",
                                  k, vi, sector_vertices[size_t(vi)].toString().c_str());
            }

            HyperplaneEquation hp;
            hp.create(sector_vertices);
            hyperplane_equations.push_back(hp);

            CHORD_SPACE_DEBUG("  OPT sector %2d hyperplane: %s\n", k, hp.toString().c_str());
        }

        opt_sectors_for_dimensionalities_[n] = opt_domains;
        opt_simplexes_for_dimensionalities_[n] = opt_simplexes;
        hyperplane_equations_for_dimensions[n] = hyperplane_equations;
    }
}

double Chord::rownd(double x, int places)  
{
    double power_of_10 = std::pow(double(10), double(places));
    double multiplied = x * power_of_10;
    double rounded = std::round(multiplied);
    double divided = rounded / power_of_10;
    return divided;
}    

bool Chord::is_compact(double range) const {
    double outer_interval = getPitch(0) + range - getPitch(voices() - 1);
    for (size_t voice_i = 0, voice_n = voices() - 2; voice_i < voice_n; ++voice_i) {
        double inner_interval = getPitch(voice_i + 1) - getPitch(voice_i);
        if (le_tolerance(outer_interval, inner_interval) == false) {
            return false;
        }
    }
    return true;
}

void Chord::clamp(double g) {
    double divisions_per_octave = std::round(OCTAVE() / g);
    for (int voice_i = 0, voice_n = voices(); voice_i < voice_n; ++voice_i) {
        auto pitch = getPitch(voice_i);
        auto divisions = std::round(pitch * divisions_per_octave);
        pitch = divisions / divisions_per_octave;
        setPitch(voice_i, pitch);
    }
}

Chord Chord::normal_order() const
{
    const auto ppcs = eppcs();

    // Trivial cases.
    if (ppcs.voices() == 0)
    {
        return ppcs;
    }
    if (ppcs.voices() == 1)
    {
        return ppcs;
    }

    auto permutations_ = ppcs.permutations();

    // Iterate upper_voice from voices()-1 down to 1 safely.
    for (size_t upper_voice = ppcs.voices() - 1; upper_voice >= 1; --upper_voice)
    {
        double least_interval = std::numeric_limits<double>::max();
        std::multimap<double, Chord> permutations_for_intervals;

        for (const auto &permutation : permutations_)
        {
            const auto lower_pc = permutation.getPitch(0);
            auto upper_pc = permutation.getPitch(upper_voice);
            auto interval = upper_pc - lower_pc;

            if (lt_tolerance(interval, 0.0))
            {
                interval += OCTAVE();
            }

            interval = rownd(interval, 9);

            if (lt_tolerance(interval, least_interval))
            {
                least_interval = interval;
            }

            permutations_for_intervals.insert({interval, permutation});
        }

        // No candidates: bail out deterministically (avoids UB).
        if (permutations_for_intervals.empty())
        {
            break;
        }

        if (permutations_for_intervals.count(least_interval) == 1)
        {
            return permutations_for_intervals.begin()->second;
        }

        // Filter to only those with the least interval.
        std::vector<Chord> filtered;
        filtered.reserve(permutations_for_intervals.count(least_interval));

        auto range = permutations_for_intervals.equal_range(least_interval);
        for (auto it = range.first; it != range.second; ++it)
        {
            filtered.push_back(it->second);
        }

        permutations_.swap(filtered);

        // Stop if we're down to one candidate.
        if (permutations_.size() == 1)
        {
            return permutations_.front();
        }

        // Prevent size_t underflow in the loop condition.
        if (upper_voice == 1)
        {
            break;
        }
    }

    // Final deterministic choice.
    if (permutations_.empty())
    {
        return ppcs;
    }

    std::sort(permutations_.begin(), permutations_.end(), ChordTickLess());
    return permutations_.front();
}

Chord Chord::normal_form() const {
    auto &cache = normal_forms_for_chords();
    auto it = cache.find(*this);
    if (it == cache.end()) {
        auto normal_order_ = normal_order();
        auto normal_order_t0 = normal_order_.T(-normal_order_.getPitch(0));
        auto normal_form_ = normal_order_t0.normal_order();
        cache.insert({*this, normal_form_});
        return normal_form_;
    } else {
        return it->second;
    }
}

Chord Chord::prime_form() const {
    auto &cache = prime_forms_for_chords();
    auto it = cache.find(*this);
    if (it == cache.end()) {
        auto normal_form_ = normal_form();
        auto inverse = normal_form_. I(0.);
        auto inverse_normal_form_ = inverse.normal_form();
        if (normal_form_ < inverse_normal_form_) {
            cache.insert({*this, normal_form_});
            return normal_form_;
        } else if (inverse_normal_form_ < normal_form_) {
            cache.insert({*this, inverse_normal_form_});
            return inverse_normal_form_;
        }
        return normal_form_;
    } else {
        return it->second;
    }
}

Chord Chord::inverse_prime_form() const {
    auto &cache = inverse_prime_forms_for_chords();
    auto it = cache.find(*this);
    if (it == cache.end()) {
        auto normal_form_ = normal_form();
        auto inverse = normal_form_. I(0.);
        auto inverse_normal_form_ = inverse.normal_form();
        if (normal_form_ > inverse_normal_form_) {
            cache.insert({*this, normal_form_});
            return normal_form_;
        } else if (inverse_normal_form_ > normal_form_) {
            cache.insert({*this, inverse_normal_form_});
            return inverse_normal_form_;
        }
        return normal_form_;
    } else {
        return it->second;
    }
}

bool Chord::is_minor() const {
    int lowerVoice = 1;
    int upperVoice = voices() - 1;
    // Compare the intervals in a chord with those in its inverse,
    // starting with the "first." This is NOT the same as
    // whether the chord is less than or equal to its inverse, and NOT 
    // the same as OPTI "minor" sector.
    while (lowerVoice < upperVoice) {
        double lowerInterval = getPitch(lowerVoice) - getPitch(lowerVoice - 1);
        double upperInterval = getPitch(upperVoice) - getPitch(upperVoice - 1);
        if (lt_tolerance(lowerInterval, upperInterval)) {
            return true;
        }
        if (gt_tolerance(lowerInterval, upperInterval)) {
            return false;
        }
        lowerVoice = lowerVoice + 1;
        upperVoice = upperVoice - 1;
    }
    return true;    
}

SILENCE_PUBLIC bool is_in_full_simplex(const Chord &point,
    const std::vector<Chord> &simplex_vertices,
    double tolerance = 1e-8)
{
    const int n = point.voices();

    if (static_cast<int>(simplex_vertices.size()) != n + 1)
    {
        return false;
    }

    const Chord &v0 = simplex_vertices[0];

    Eigen::MatrixXd A(n, n);
    for (int i = 1; i < n + 1; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            A(j, i - 1) = simplex_vertices[i].getPitch(j) - v0.getPitch(j);
        }
    }

    Eigen::VectorXd b(n);
    for (int j = 0; j < n; ++j)
    {
        b(j) = point.getPitch(j) - v0.getPitch(j);
    }

    // Solve A w = b
    Eigen::VectorXd w = A.colPivHouseholderQr().solve(b);

    // Optional residual check (loose, relative)
    double residual = (A * w - b).norm();
    double scale = std::max(1.0, b.norm());
    if (residual > tolerance * 100.0 * scale)
    {
        // Keep this loose or omit entirely.
        // Returning false here can reintroduce "no sector" brittleness.
        // I'd normally omit this for classification.
    }

    double sum_w = 0.0;
    for (int i = 0; i < w.size(); ++i)
    {
        sum_w += w(i);
    }

    double lambda0 = 1.0 - sum_w;
    if (lambda0 < -tolerance)
    {
        return false;
    }

    for (int i = 0; i < w.size(); ++i)
    {
        if (w(i) < -tolerance)
        {
            return false;
        }
    }

    return true;
}

std::vector<int> Chord::opt_domain_sectors() const
{
    auto &opt_simplexes_for_dimensionalities_ = opt_simplexes_for_dimensionalities();
    if (voices() < 0 || static_cast<size_t>(voices()) >= opt_simplexes_for_dimensionalities_.size())
    {
        std::fprintf(stderr, "opt_domain_sectors: voices()=%d out of range (size=%zu)\n",
            voices(), opt_simplexes_for_dimensionalities_.size());
        std::abort();
    }
    auto &opt_simplexes = opt_simplexes_for_dimensionalities_[voices()];

    std::vector<int> result;
    auto ot = eOT();

    for (int sector = 0, n = static_cast<int>(opt_simplexes.size()); sector < n; ++sector)
    {
        if (is_in_full_simplex(ot, opt_simplexes[sector]) == true)
        {
            result.push_back(sector);
        }
    }

    if (result.empty() == true)
    {
        double best = std::numeric_limits<double>::infinity();
        int best_sector = 0;

        for (int sector = 0, n = static_cast<int>(opt_simplexes.size()); sector < n; ++sector)
        {
            double d = distance_to_points(ot, opt_simplexes[sector]);
            if (d < best)
            {
                best = d;
                best_sector = sector;
            }
        }

        result.push_back(best_sector);
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

std::vector<Chord> Chord::opt_domain(int sector) const {
    auto opt_sectors_for_dimensions = opt_sectors_for_dimensionalities();
    auto opt_sectors = opt_sectors_for_dimensions[voices()];
    return opt_sectors[sector];
}

Chord Chord::eOT() const {
    auto o = eO();
    auto o_t = o.eT();
    return o_t;
}

Chord Chord::eOTT(double g) const {
    auto o = eO();
    auto o_tt = o.eTT(g);
    return o_tt;
}

Chord Chord::reflect(int opt_sector) const {
    auto reflection = reflect_in_inversion_flat(*this, opt_sector);
    return reflection;
}

SILENCE_PUBLIC PITV::~PITV() {};

SILENCE_PUBLIC int PITV::getN() const {
    return N;
}

SILENCE_PUBLIC int PITV::getG() const {
    return g;
}

SILENCE_PUBLIC int PITV::getRange() const {
    return range;
}

SILENCE_PUBLIC int PITV::getCountP() const {
    return countP;
}

SILENCE_PUBLIC int PITV::getCountI() const {
    return countI;
}

SILENCE_PUBLIC int PITV::getCountT() const {
    return countT;
}

SILENCE_PUBLIC int PITV::getCountV() const {
    return countV;
}

SILENCE_PUBLIC void PITV::preinitialize(int N_, double range_, double g_) {
    System::inform("PITV::preinitialize...\n");
    PsForIndexes.clear();
    indexesForPs.clear();
    normal_forms.clear();
    N = N_;
    range = range_;
    g = g_;
    countP = 0;
    countI = 2;
    countT = (OCTAVE() / g);
    Chord chord;
    chord.resize(N);
    countV = octavewiseRevoicings(chord, range);
}

SILENCE_PUBLIC void PITV::initialize(int N_, double range_, double g_, bool printme) {
    System::message("PITV::initialize: N_: %d range_: %f g_: %f\n", N_, range_, g_);
    preinitialize(N_, range_, g_);
    // Collect all prime forms.
    int upperI = 24.;
    int lowerI = 0.;
    Chord iterator_ = iterator(N, lowerI);
    Chord origin = iterator_;
    int chords = 0;
    int index = 0;
    int normal_form_n = 0;
    while (next(iterator_, origin, upperI, g) == true) {
        chords++;
        Chord clamped = iterator_;
        clamped.clamp();
        auto normal_form_ = clamped.normal_form();
        auto prime_form_ = clamped.prime_form();
        // Make a collection to map prime forms to their indices.
        auto inserted_prime_form = indexesForPs.insert({prime_form_, index});
        if (inserted_prime_form.second == true) {
            // Make an inverse collection to map indices to prime forms.
            PsForIndexes.insert({index, prime_form_});
            ++index;
        }
        // If prime != normal then I == 1.
        auto inserted_normal_form = normal_forms.insert(normal_form_);
        if (inserted_normal_form.second == true && printme == true) {
            System::message("%3d chord:               %s\n", normal_form_n, print_chord(iterator_).c_str());
            System::message("    normal form:         %s\n", print_chord(normal_form_).c_str());
            System::message("    prime form:          %s\n", print_chord(iterator_.prime_form()).c_str());
            System::message("    inverse prime form:  %s\n", print_chord(iterator_.inverse_prime_form()).c_str());
            ++normal_form_n;
        }
    }    
    countP = indexesForPs.size();
    System::message("PITV::initialize: finished with countP: %d.\n", countP);
}

SILENCE_PUBLIC void PITV::list(bool listheader, bool listps, bool listvoicings) const {
    if (listheader) {
        System::message("PITV::list...\n");
        System::message("PITV.voices:     %8d\n", N);
        System::message("PITV.range:      %13.4f\n", range);
        System::message("PITV.g:          %13.4f\n", g);
        System::message("PITV.countP:     %8d\n", countP);
        System::message("PITV.countI:     %8d\n", countI);
        System::message("PITV.countT:     %8d\n", countT);
        System::message("PITV.countV:     %8d\n", countV);
    }
    if (listps) {
        std::map<std::string, std::string> opttis_for_prime_forms;
        std::map<std::string, std::string> optts_for_normal_forms;
        std::set<std::string> prime_forms_from_PsForIndexes;
        std::set<std::string> prime_forms_from_indexesForPs;
        std::multimap<int, std::string> opttis_for_sectors;
        std::multimap<int, std::string> optts_for_sectors;
        for (const auto &entry: PsForIndexes) {
            const auto &chord = entry.second;
            prime_forms_from_PsForIndexes.insert(chord.toString());
            const auto &index = entry.first;
            const auto prime_form_ = chord.prime_form();
            const auto inverse_prime_form_ = prime_form_.inverse_prime_form();
            Chord normal_form_;
            if (prime_form_ <= inverse_prime_form_) {
                normal_form_ = prime_form_;
            } else {
                normal_form_ = inverse_prime_form_;
            }
            const auto optti = chord.eOPTTI(g);
            opttis_for_prime_forms.insert({prime_form_.toString(), optti.toString()});
            const auto optt = chord.eOPTT(g);
            optts_for_normal_forms.insert({normal_form_.toString(), optt.toString()});
            const auto op = chord.eOP();
            const auto opt_sectors = optt.opt_domain_sectors();
            for(auto opt_sector : opt_sectors) {
                optts_for_sectors.insert({opt_sector, optt.toString()});
            }
            System::message("PsForIndexes[%5d]: chord:         %s\n", index, print_chord(chord).c_str());
            System::message("PsForIndexes[%5d]: prime:         %s\n", index, print_chord(prime_form_).c_str());
            System::message("PsForIndexes[%5d]: normal:        %s\n", index, print_chord(normal_form_).c_str());
            System::message("PsForIndexes[%5d]: inverse prime: %s\n", index, print_chord(inverse_prime_form_).c_str());
        }
        for (const auto &entry : indexesForPs) {
            const auto &chord = entry.first;
            prime_forms_from_indexesForPs.insert(chord.toString());
            const auto &index = entry.second;
            const auto prime_form_ = chord.prime_form();
            const auto inverse_prime_form_ = prime_form_.inverse_prime_form();
            Chord normal_form_;
            if (prime_form_ <= inverse_prime_form_) {
                normal_form_ = prime_form_;
            } else {
                normal_form_ = inverse_prime_form_;
            }
            const auto optti = chord.eOPTTI(g);
            const auto optt = chord.eOPTT(g);
            const auto op = chord.eOP();
            const auto opt_sectors = optt.opt_domain_sectors();
            auto key = chord.toString();
            System::message("indexesForPs[%s]: index:   %5d %s\n", key.c_str(), index, print_chord(chord).c_str());
            System::message("indexesForPs[%s]: prime:         %s\n", key.c_str(), print_chord(prime_form_).c_str());
            System::message("indexesForPs[%s]: normal:        %s\n", key.c_str(), print_chord(normal_form_).c_str());
            System::message("indexesForPs[%s]: inverse prime: %s\n", key.c_str(), print_chord(inverse_prime_form_).c_str());
        }
        System::message("PsForIndexes size:           %6d\n", PsForIndexes.size());
        System::message("indexesForPs size:           %6d\n", indexesForPs.size());
        System::message("opttis_for_prime_forms size: %6d\n", opttis_for_prime_forms.size());
        System::message("optts_for_normal_forms size: %6d\n", optts_for_normal_forms.size());
        for (int i = 0; i < (N * 2); ++i) {
            System::message("opti sector: %3d opttis: %6d\n", i, opttis_for_sectors.count(i));
        }
        for (const auto &key : prime_forms_from_PsForIndexes) {
            if (prime_forms_from_indexesForPs.find(key) == prime_forms_from_indexesForPs.end()) {
                System::error("%s not found in indexesForPs.\n", key.c_str());
            }
        }
        for (const auto &key : prime_forms_from_indexesForPs) {
            if (prime_forms_from_PsForIndexes.find(key) == prime_forms_from_indexesForPs.end()) {
                System::error("%s not found in PsForIndexes.\n", key.c_str());
            }
        }
     }
}

Eigen::VectorXi PITV::fromChord(const Chord &chord, bool printme) const {
    if (printme) {
        System::message("PITV::fromChord:          chord:         %s\n", print_chord(chord).c_str());
    }
    Eigen::VectorXi pitv(4);
    const auto ppcs = chord.eppcs();
    const auto prime_form_ = chord.prime_form();
    int P = indexesForPs.at(prime_form_);
    pitv.coeffRef(0) = P;
    const auto normal_form_ = chord.normal_form();
    const auto inverse_prime = chord.inverse_prime_form();
    int I = 0;
    if (prime_form_ != normal_form_) {
        I = 1;
    }
    pitv.coeffRef(1) = I;
    if (printme) {
        System::message("PITV::fromChord: I: %5d normal_form:   %s\n", pitv(1), print_chord(normal_form_).c_str());
        System::message("PITV::fromChord: P: %5d prime_form:    %s\n", pitv(0), print_chord(prime_form_).c_str());
        System::message("PITV::fromChord:          inverse_prime: %s\n", print_chord(inverse_prime).c_str());
    }
    auto normal_form_t = normal_form_;
    int T;
    for (T = 0; T < 12; ++T) {
        if (normal_form_t.eppcs() == ppcs) {
            break;
        }
        normal_form_t = normal_form_t.T(g);
    }
    pitv.coeffRef(2) = T;
    if (printme) {
        System::message("PITV::fromChord: T: %5d normal_form_t: %s\n", pitv(2), print_chord(normal_form_t).c_str());
    }
    auto op = normal_form_t.eOP();
    auto op_from_chord = chord.eOP();
    if (printme) {
        System::message("PITV::fromChord:          op from PIT:   %s\n", print_chord(op).c_str());
        System::message("PITV::fromChord:          op from chord: %s\n", print_chord(op_from_chord).c_str());
    }
    int V;
    if (chord < op) {
        V = -1;
        System::error("PITV::fromChord: Error: Chord is below OP.\n");
    } else {
        V = indexForOctavewiseRevoicing(op, chord, range); 
    }        
    pitv.coeffRef(3) = V;
    auto op_v = op;
    if (V >= 0) {
        op_v = octavewiseRevoicing(op, V, range);
    }
    if (op_v != chord) {
        System::error("PITV::fromChord: Error: revoiced OP  (%s)\n", print_chord(op_v).c_str());
        System::error("                 doesn't match chord (%s)\n", print_chord(chord).c_str());
    }
    if (printme) {
        System::message("PITV::fromChord: V: %5d op_v:          %s\n", pitv(3), print_chord(op_v).c_str());
    }
    if (printme) {
        System::message("PITV::fromChord: PITV:               %8d     %8d     %8d     %8d\n\n", pitv(0), pitv(1), pitv(2), pitv(3));
    }
    return pitv;
}

/**
 * Returns the chord for the indices of prime form, inversion,
 * transposition, and voicing. The chord is not in RP; rather, each voice of
 * the chord's OP may have zero or more octaves added to it.
 */
std::vector<Chord> PITV::toChord(int P, int I, int T, int V, bool printme) const {
    if (printme) {
        System::message("PITV::toChord:   PITV:               %8d     %8d     %8d     %8d\n", P, I, T, V);
    }
    P = P % countP;
    I = I % countI;
    T = T % countT;
    V = V % countV;
    if (printme) {
        System::message("PITV::toChord:   PITV (modulus):     %8d     %8d     %8d     %8d\n", P, I, T, V);
    }
    auto prime_form_ = PsForIndexes.at(P);
    auto normal_form_ = prime_form_;
    auto inverse_prime_form_ = prime_form_.inverse_prime_form();
    if (I == 1) {
        normal_form_ = inverse_prime_form_;
    }
    if (printme) {
        System::message("PITV::toChord:   I: %5d normal_form:   %s\n", I, print_chord(normal_form_).c_str());
        System::message("PITV::toChord:   P: %5d prime_form_:   %s\n", P, print_chord(prime_form_).c_str());
        System::message("PITV::toChord:            inv prime form:%s\n",  print_chord(inverse_prime_form_).c_str());
   }
    auto normal_form_t = normal_form_;
    for (int t = 0; t < T; ++t) {
        normal_form_t = normal_form_t.T(g);
    }
    if (printme) {
        System::message("PITV::toChord:   T: %5d normal_form_t: %s\n", T, print_chord(normal_form_t).c_str());
    }
    auto op = normal_form_t.eOP();
    if (printme) {
        System::message("PITV::toChord:            op:            %s\n",  print_chord(op).c_str());
    }
    auto op_v = octavewiseRevoicing(op, V, range);
    std::vector<Chord> result(3);
    result[0] = op_v;
    result[1] = op;
    result[2] = normal_form_;
    if (printme) {
        System::message("PITV::toChord:            result:        %s\n\n", result[0].toString().c_str());
    }
    return result;
}

SILENCE_PUBLIC std::vector<Chord> PITV::toChord_vector(const Eigen::VectorXi &pitv, bool printme) const {
    return toChord(pitv(0), pitv(1), pitv(2), pitv(3), printme);
}

SILENCE_PUBLIC std::map<Chord, Chord, ChordTickLess> &normal_forms_for_chords() {
    static std::map<Chord, Chord, ChordTickLess> cache;
    return cache;
}

SILENCE_PUBLIC std::map<Chord, Chord, ChordTickLess> &prime_forms_for_chords() {
    static std::map<Chord, Chord, ChordTickLess> cache;
    return cache;
}

SILENCE_PUBLIC std::map<Chord, Chord, ChordTickLess> &inverse_prime_forms_for_chords() {
    static std::map<Chord, Chord, ChordTickLess> cache;
    return cache;
}

SILENCE_PUBLIC void Scale::set_ratio(const int voice, const double numerator, const double denominator) {
    const double ratio = numerator / denominator;
    const double midi_key = std::log2(ratio) * 12.0;
    setPitch(voice, midi_key);
}

SILENCE_PUBLIC void Scale::from_scala(const std::string &name, const std::string &scala_text) {
    std::istringstream stream(scala_text);
    std::string line;
    std::vector<std::string> lines;
    // Collect non-empty, non-comment lines
    while (std::getline(stream, line)) {
        // Remove leading/trailing whitespace
        line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");
        if (line.empty() || line[0] == '!') continue;
        lines.push_back(line);
    }
    if (lines.size() < 2) {
        throw std::runtime_error("Invalid Scala file: not enough data.");
    }
    int num_pitches = std::stoi(lines[1]);
    if ((int)lines.size() < 2 + num_pitches) {
        throw std::runtime_error("Invalid Scala file: fewer pitches than declared.");
    }
    resize(num_pitches);
    // Scala scale always starts from 0.0 (the fundamental).
    int voice = 0;
    setPitch(voice, 0);
    for (int i = 2; i < 2 + num_pitches; ++i) {
        const std::string& pitch_str = lines[i];
        double midi_key;
        // Check if it's cents.
        if (std::regex_match(pitch_str, std::regex("^[0-9\\.]+$"))) {
            double cents = std::stod(pitch_str);
            midi_key = cents / 100.0;
            setPitch(voice, midi_key);
        } else if (std::regex_match(pitch_str, std::regex("^[0-9]+/[0-9]+$"))) {
            // Check if it's a ratio like 3/2.
            size_t slash = pitch_str.find('/');
            double numerator = std::stod(pitch_str.substr(0, slash));
            double denominator = std::stod(pitch_str.substr(slash + 1));
            set_ratio(voice, numerator, denominator);
        } else {
            throw std::runtime_error("Invalid pitch format: " + pitch_str);
        }
        voice++;
    }
    add_scale(name, *this);
}

} // End of namespace csound.

#pragma GCC diagnostic push
