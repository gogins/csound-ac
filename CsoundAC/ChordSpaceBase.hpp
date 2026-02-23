#ifndef CHORDSPACEBASE_HPP_INCLUDED
#define CHORDSPACEBASE_HPP_INCLUDED
/*
 * C S O U N D  A C
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
 
#pragma SWIG nowarn=302
#pragma SWIG nowarn=509

#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#include "Platform.hpp"
#include "System.hpp"
#ifdef SWIG
%module CsoundAC
%{
#include "Event.hpp"
#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <Eigen/Dense>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <regex>
%}
%include "std_string.i"
%include "std_vector.i"
#else
#include "Event.hpp"
#include "Platform.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/math/special_functions/ulp.hpp>
#include <cfloat>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <Eigen/Dense>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <regex>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

namespace csound {
/** \file ChordSpaceBase.hpp
This library implements a geometric approach to some common operations on 
chords in neo-Riemannian music theory for use in score generating procedures:

 -  Identifying whether a chord belongs to some equivalence class of music
    theory, or sending a chord to its equivalent within a representative
    ("normal") fundamental domain of some equivalence relation. The
    equivalence relations are octave (O), permutational (P), transpositional,
    (T), inversional (I), and their compounds OP, OPT (set-class or chord
    type), and OPTI (similar to prime form), among others.

 -  Causing chord progressions to move strictly within an orbifold that
    reoresents some equivalence class.

 -  Implementing chord progressions based on the L, P, R, D, K, and Q
    operations of neo-Riemannian theory (thus implementing some aspects of
    "harmony").

 -  Implementing chord progressions performed within a more abstract
    equivalence class by means of the closest voice-leading within a less
    abstract equivalence class (thus implementing some fundamentals of
    "counterpoint").
    
 -  Implementing "functional" or "Roman numeral" operations performed 
    using scales and scale degrees (thus implementing many fundamentals of 
    "pragmatic music theory").
    
# Definitions

Pitch is the perception of a distinct sound frequency. It is a logarithmic
perception; octaves, which sound 'equivalent' in some sense, represent
doublings or halvings of frequency.

Pitches and intervals are represented as real numbers. Middle C is 60 and the
octave is 12. Our usual system of 12-tone equal temperament, as well as MIDI
key numbers, are completely represented by the whole numbers; any and all
other pitches can be represented simply by using fractions.

A voice is a distinct sound that is heard as having a pitch.

A chord is simply a set of voices heard at the same time, represented here
as a point in a chord space having one dimension of pitch for each voice
in the chord.

A scale is a chord with a tonic pitch-class as its first and lowest voice, 
all other voices being pitch-classes sorted in ascending order.

For the purposes of algorithmic composition, a score can be considered to be a 
sequence of more or less fleeting chords.

# Equivalence Relations and Classes

An equivalence relation identifies different elements of a set as belonging to
the same class. For example the octave is an equivalence relation that 
identifies C1, C2, and C3 as belonging to the equivalence class C. Operations 
that send elements to their equivalents induce quotient spaces or orbifolds, 
where the equivalence operation identifies points on one facet of the orbifold 
with points on an opposing facet. The fundamental domain of the equivalence 
relation is the space consisting of the orbifold and its surface.

Plain chord space has no equivalence relation. Ordered chords are represented
as vectors in parentheses (p1, ..., pN). Unordered chords are represented as
sorted vectors in braces {p1, ..., pN}. Unordering is itself an equivalence
relation -- permutational equivalence.

The following equivalence relations apply to pitches and chords, and exist in
different orbifolds. Equivalence relations can be combined (Callendar, Quinn,
and Tymoczko, "Generalized Voice-Leading Spaces," _Science_ 320, 2008), and
the more equivalence relations are combined, the more abstract is the
resulting orbifold compared to the parent space.

In most cases, a chord space can be divided into a number, possibly
infinite, of geometrically equivalent fundamental domains for the same
equivalence relation. Therefore, here we use the notion of 'representative'
or 'normal' fundamental domain. For example, the representative fundamental
domain of unordered sequences, out of all possible orderings, consists of all
sequences in their ordinary sorted order. It is important, in the following,
to identify representative fundamental domains that combine properly, e.g.
such that the representative fundamental domain of OP / the representative
fundamental domain of PI equals the representative fundamental domain of OPI.
And this in turn may require accounting for duplicate elements of the
representative fundamental domain caused by reflections or singularities in
the orbifold (e.g. on vertices, edges, or facets shared by fundamental domains 
with a cyclical structure), or by doubled pitches in a chord.

<dl>
<dt>C   <dd>Cardinality equivalence, e.g. {1, 1, 2} == {1, 2}. _Not_ assuming
        cardinality equivalence ensures that there is a proto-metric in plain
        chord space that is inherited by all child chord spaces. Cardinality
        equivalence is never assumed here, because we are working in chord
        spaces of fixed dimensionality; e.g. we represent the note middle C
        not only as {60}, but also as {60, 60, ..., 60}.
        
<dt>O   <dd>Octave equivalence. The fundamental domain is defined by the pitches
        in a chord spanning the range of an octave or less, and summing to
        an octave or less.

<dt>P   <dd>Permutational equivalence. The fundamental domain is defined by a
        "wedge" of plain chord space in which the voices of a chord are always
        sorted by pitch.

<dt>T   <dd>Transpositional equivalence, e.g. {1, 2} == {7, 8}. The fundamental
        domain is defined as a hyperplane in chord space at right angles to the
        diagonal of unison chords. Represented by the chord always having a
        sum of pitches equal to 0.

<dt>Tg  <dd>Transpositional equivalence; the pitches of the chord are sent to 
        the ceilings of the pitches in the first chord whose sum is equal 
        to or greater than 0, i.e., rounded up to equal temperament.

<dt>I   <dd>Inversional equivalence. Care is needed to distinguish the
        mathematician's sense of 'invert', which means 'pitch-space inversion'
        or 'reflect in a point', from the musician's sense of 'invert', which
        varies according to context but in practice often means 'registral
        inversion' or 'revoice by adding an octave to the lowest tone of a
        chord.' Here, we use 'invert' and 'inversion' in the mathematician's
        sense, and we use the terms 'revoice' and 'voicing' for the musician's
        'invert' and 'inversion'. Here, the inversion of a chord is its 
        reflection in a hyperplane (the inversion flat) that divides a 
        fundamental domain of pitch.

<dt>PI  <dd>Inversional equivalence with permutational equivalence. The
        'inversion flat' of unordered chord space is a hyperplane consisting
        of all those unordered chords that are invariant under inversion. A
        fundamental domain is defined by any half space bounded by a
        hyperplane containing the inversion flat.

<dt>OP  <dd>Octave equivalence with permutational equivalence. Tymoczko's 
        orbifold for chords; i.e. chords with a fixed number of voices in a 
        harmonic context. The fundamental domain is defined as a hyperprism 
        one octave long with as many sides as voices and the ends identified 
        by octave equivalence and one cyclical permutation of voices, modulo 
        the unordering. In OP for trichords in 12TET, the augmented triads run 
        up the middle of the prism, the major and minor triads are in 6
        alternating columns around the augmented triads, the two-pitch chords
        form the 3 sides, and the one-pitch chords form the 3 edges that join
        the sides.
        
<dt>OPT  <dd>The layer of the OP prism as close as possible to the origin, modulo
        the number of voices. Chord type. Note that CM and Cm are different
        OPT. Because the OP prism is canted down from the origin, at least one
        pitch in each OPT chord (excepting the origin itself) is negative. 
        For n dimensions there are n OPT fundamental domains centering on the 
        maximally even chord and generated by rotation about the maximally 
        even chord, equivalently octavewise revoicing, more or less the same 
        as the musician's sense of "chord inversion."

<dt>OPTT  <dd>The same as OPT, but with chords rounded up within equal 
        temperament; equivalent to "chord type."

<dt>OPI  <dd>The OP prism modulo inversion, i.e. 1/2 of the OP prism. The
        representative fundamental consits of those chords having inversional 
        equivalence.

<dt>OPTI  <dd>The OPT layer modulo inversion, i.e. 1/2 of the OPT layer.
        Set-class. Note that minor and major triads are are the same OPTI.

<dt>OPTTI  <dd>The same as OPTI, but with chords rounded up within equal 
        temperament; equivalent to "set class."
</dl>

# Operations

Each of the above equivalence relations is, of course, an operation that sends
chords outside some fundamental domain to chords inside that fundamental 
domain. We define the following additional operations:

<dl>
<dt>T(p, x)     <dd>Translate p by x.

<dt>I(p [, x])  <dd>Reflect p in x, by default the origin.

<dt>P           <dd>Send a major triad to the minor triad with the same root,
                or vice versa (Riemann's parallel transformation).

<dt>L           <dd>Send a major triad to the minor triad one major third higher,
                or vice versa (Riemann's Leittonwechsel or leading-tone
                exchange transformation).

<dt>R           <dd>Send a major triad to the minor triad one minor third lower,
                or vice versa (Riemann's relative transformation).

<dt>D            <dd>Send a triad to the next triad a perfect fifth lower
                (dominant transformation).
</dl>

P, L, and R have been extended as follows, see Fiore and Satyendra,
"Generalized Contextual Groups", _Music Theory Online_ 11, August 2008:

<dl>
<dt>K(c)        <dd>Interchange by inversion;
                `K(c) := I(c, c[1] + c[2])`.
                This is a generalized form of P; for major and minor triads,
                it is exactly the same as P, but it also works with other
                chord types.

<dt>Q(c, n, m)  <dd>Contexual transposition;
                `Q(c, n, m) := T(c, n)` if c is a T-form of m,
                or `T(c, -n)` if c is an I-form of M. Not a generalized form
                of L or R; but, like them, K and Q generate the T-I group.
</dl>
                
*/

static SILENCE_PUBLIC std::string chord_space_version() {
    char buffer[0x500];
    std::snprintf(buffer, sizeof(buffer), "ChordSpaceBase version 2.1.0. Compiled from %s on %s at %s.", __FILE__, __DATE__, __TIME__);
    return buffer;
}

/**
 * Returns the current state of the chord space debugging flag as a 
 * reference, which can be an lvalue or an rvalue.
 */
static SILENCE_PUBLIC bool &CHORD_SPACE_DEBUGGING() {
    static bool CHORD_SPACE_DEBUGGING_ = false;
    return CHORD_SPACE_DEBUGGING_;
}

static SILENCE_PUBLIC bool SET_CHORD_SPACE_DEBUGGING(bool enabled) {
    bool prior_value = CHORD_SPACE_DEBUGGING();
    CHORD_SPACE_DEBUGGING() = enabled;
    return prior_value;
}

/**
 * Returns the current state of the chord space _scoped_ debugging flag as a 
 * reference, which can be an lvalue or an rvalue.
 */
static SILENCE_PUBLIC bool &SCOPED_DEBUGGING_FLAG() {
    static bool SCOPED_DEBUGGING_ = false;
    return SCOPED_DEBUGGING_;
}

static SILENCE_PUBLIC bool SET_SCOPED_DEBUGGING(bool enabled) {
    bool prior_value = SCOPED_DEBUGGING_FLAG();
    SCOPED_DEBUGGING_FLAG() = enabled;
    return prior_value;
}

struct SILENCE_PUBLIC SCOPED_DEBUGGING {
    int prior_state = false;
    SCOPED_DEBUGGING() {
        if (SCOPED_DEBUGGING_FLAG()) {
            prior_state = CHORD_SPACE_DEBUGGING();
            CHORD_SPACE_DEBUGGING() = true;
        }
    }
    ~SCOPED_DEBUGGING() {
        if (SCOPED_DEBUGGING_FLAG()) {
            CHORD_SPACE_DEBUGGING() = prior_state;
        }
    }
};

#define CHORD_SPACE_DEBUG if (CHORD_SPACE_DEBUGGING() == true) csound::System::message

/////////////////////////////////////////////////////////////////////////////////////////
// ALL DECLARATIONS BELOW HERE MORE OR LESS IN ALPHABETICAL ORDER -- NO DEFINITIONS HERE.
/////////////////////////////////////////////////////////////////////////////////////////

// But a few forward declarations come first.

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Matrix;

typedef Eigen::Matrix<double, Eigen::Dynamic, 1> Vector;

class SILENCE_PUBLIC Chord;

struct SILENCE_PUBLIC ChordTickLess;

struct SILENCE_PUBLIC HyperplaneEquation;

class SILENCE_PUBLIC PITV;

class SILENCE_PUBLIC Scale;

SILENCE_PUBLIC double distance_to_points(const Chord &chord, const std::vector<Chord> &points);

SILENCE_PUBLIC double epc(double pitch);

/**
 * This is the basis of all other numeric comparisons that take floating-point 
 * limits into account. It is a "close enough" comparison. If a or b equals 0,
 * the indicated number of machine epsilons is used as the tolerance; if 
 * neither a nor b equals 0, the indicated number of units in the last place
 * (ULPs) is used as the tolerance. These tolerances should be set to 
 * appropriate values based on the use case.
 */
SILENCE_PUBLIC bool eq_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC bool ge_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC bool gt_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC bool le_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC bool lt_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC Chord midpoint(const Chord &a, const Chord &b);

/**
 * Cache prime forms for chords for speed.
 */
SILENCE_PUBLIC std::map<Chord, Chord, ChordTickLess> &normal_forms_for_chords();

/**
 * Cache normal forms for chords for speed.
 */
SILENCE_PUBLIC std::map<Chord, Chord, ChordTickLess> &prime_forms_for_chords();

/**
 * Cache inverse prime forms for chords for speed.
 */
SILENCE_PUBLIC std::map<Chord, Chord, ChordTickLess> &inverse_prime_forms_for_chords();

/**
 * Returns a string representation of the pitches in the chord, along with the 
 * sectors of the cyclical regions of the OPT and OPTI fundamental domains to 
 * which the chord belongs.
 */
SILENCE_PUBLIC std::string print_chord(const Chord &chord);

SILENCE_PUBLIC std::string print_opti_sectors(const Chord &chord);

// End of forward declarations needed by other forward declarations!

/**
 * The size of the octave, defined to be consistent with
 * 12 tone equal temperament and MIDI.
 */
SILENCE_PUBLIC double OCTAVE();

SILENCE_PUBLIC bool operator == (const Chord &a, const Chord &b);

SILENCE_PUBLIC bool operator < (const Chord &a, const Chord &b);

SILENCE_PUBLIC bool operator < (const Scale &a, const Scale &b);

SILENCE_PUBLIC bool operator <= (const Chord &a, const Chord &b);

SILENCE_PUBLIC bool operator > (const Chord &a, const Chord &b);

SILENCE_PUBLIC bool operator >= (const Chord &a, const Chord &b);

SILENCE_PUBLIC void add_chord(std::string, const Chord &chord);

SILENCE_PUBLIC void add_scale(std::string, const Scale &scale);

SILENCE_PUBLIC double C4();

/**
 * Returns the chord, in scale order, for the specified degree of the scale.
 * The chord can be composed of seconds, thirds, or larger intervals, and 
 * can have two or more voices. The scale can have any number of pitch-classes  
 * and any interval content; it simply has to consists of pitch-classes sorted 
 * from the tonic pitch-class on up.
 * 
 * PLEASE NOTE: Scale degree is 1-based. A "third" is denoted "3" but is two 
 * scale degrees, and so on.
 */
SILENCE_PUBLIC Chord chord(const Chord &scale, int scale_degree, int chord_voices, int interval = 3);

/**
 * Chords consist of simultaneously sounding pitches. The pitches are
 * represented as semitones with 0 at the origin and middle C as 60.
 * Each voice also has a duration, velocity, channel, and pan.
 * Eigen matrices are accessed (row, column) and stored as column
 * vectors, so a Chord is accessed (voice (same as row), attribute).
 */
class SILENCE_PUBLIC Chord : public Matrix  {
public:
    enum {
        PITCH = 0,
        DURATION = 1,
        LOUDNESS = 2,
        INSTRUMENT = 3,
        PAN = 4,
        COUNT = 5
    };
    Chord();
    Chord(int size);
    Chord(const Chord &other);
    Chord(const std::vector<double> &other);
    virtual ~Chord();
    virtual Chord &operator = (const Chord &other);
    Chord& operator=(Chord&& other) noexcept = default;
    Chord(Chord&& other) noexcept = default;
    virtual Chord clone() const {
        Chord clone_ = *this;
        return clone_;
    }
    virtual Chord &operator = (const std::vector<double> &other);
    virtual operator std::vector<double>() const;
    /**
     * Returns the ith arpeggiation, current voice, and corresponding revoicing
     * of the chord. Positive arpeggiations start with the lowest voice of the
     * chord and revoice up; negative arpeggiations start with the highest voice
     * of the chord and revoice down.
     */
    virtual Chord a(int arpeggiation, double &resultPitch, int &resultVoice) const;
    /**
     * Returns a new chord whose pitches are the ceilings of this chord's 
     * pitches, with respect to the generator of transposition g, which 
     * defaults to 1 semitone.
     */
    virtual Chord ceiling(double g = 1.) const;
    /**
     * Rounds the pitches in this chord to the nearest integer multiple of g,
     * the generator of transposition. This is valid only if g goes evenly 
     * into 12 (the octave), i.e. in 12/g tone equal temperament.
     */
    virtual void clamp(double g=1.);
    /**
     * Returns whether or not the chord contains the pitch.
     */
    virtual bool contains(double pitch_) const;
    /**
     * Returns the number of voices in this chord, the same as the number of 
     * dimensions in this chord space.
     */
    virtual size_t count(double pitch) const;
    /**
     * Returns a copy of the chord cyclically permuted by a stride, by default 1.
     * The direction of rotation is by default the same as musicians' first
     * inversion, second inversion, and so on; but negative sign will reverse
     * the direction of rotation.
     * + 1 is pop the front and push it on the back, shifting the middle down.
     * 0 1 2 3 4 => 1 2 3 4 0
     * - 1 is pop the back and push it on the front, shifting the middle up.
     * 0 1 2 3 4 => 4 0 1 2 3
     */
    virtual Chord cycle(int stride = 1) const;
    /**
     * For each chord space of dimensions 3 <= n <= 12, there is one cyclical 
     * region in the domain OPT equivalence, consisting of n sectors. The vertices of the
     * cyclical region consist of n transpositions of the origin by 12 / n, 
     * under OP equivalence. This function returns a global collection of these 
     * cyclical regions. Transposing this base by 12 / n defines a simplex 
     * containing n fundamental domains of OPT.
     */
    static std::map<int, std::vector<Chord>> &cyclical_regions_for_dimensionalities();
    /**
     * Returns the Euclidean distance of this chord from its space's
     * origin.
     */
    virtual double distanceToOrigin() const;
    /**
     * Returns the Euclidean distance from this chord
     * to the unison diagonal of its chord space.
     */
    virtual double distanceToUnisonDiagonal() const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of inversional equivalence.
     */
    virtual Chord eI(int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within the representative 
     * fundamental domain of octave equivalence.
     */
    virtual Chord eO() const;
    /**
     * Returns the equivalent of the chord within the representative 
     * fundamental domain of octave and permutational equivalence.
     */
    virtual Chord eOP() const;
    /**
     * Returns the equivalent of the chord within a fundamental domain of 
     * octave, permutational, and inversional equivalence.
     */
    virtual Chord eOPI(int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of octave, permutational, and transpositional equivalence.
     */
    virtual Chord eOPT(int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of octave, permutational, and transpositional equivalence but 
     * in the equal temperament generated by g.
     */
    virtual Chord eOPTT(double g = 1., int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range, permutational, transpositional, and inversional
     * equivalence.
     */
    virtual Chord eOPTI(int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range, permutational, transpositional, and inversional
     * equivalence but in the equal temperament generated by g.
     */
    virtual Chord eOPTTI(double g = 1., int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within the representative 
     * fundamental domain of octave and transpositional equivalence.
     */
    virtual Chord eOT() const;
    /**
     * Returns the equivalent of the chord within a fundamental domain of 
     * octave and transpositional equivalence but in the equal temperament 
     * generated by g.
     */
    virtual Chord eOTT(double g = 1.) const;
    /**
     * Returns the equivalent of the chord within the representative
     * fundamental domain of permutational equivalence.
     */
    virtual Chord eP() const;
    /**
     * Returns the equivalent of the chord under pitch-class equivalence,
     * i.e. the pitch-class set of the chord.
     */
    virtual Chord epcs() const;
    /**
     * Returns the equivalent of the chord under pitch-class equivalence, i.e. 
     * the pitch-class set of the chord, sorted by pitch-class.
     */
    virtual Chord eppcs() const;
    /**
     * Returns whether the voices of this chord equal the voices of the other.
     */
    virtual bool equals(const Chord &other) const;
    /**
     * Returns the equivalent of the chord within the representative
     * fundamental domain of a range equivalence.
     */
    virtual Chord eR(double range) const;
    /**
     * Returns the equivalent of the chord within the representative fundamental
     * domain of range and permutational equivalence.
     */
    virtual Chord eRP(double range) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range, permutational, and inversional equivalence.
     */
    virtual Chord eRPI(double range, int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range, permutational, and transpositional equivalence.
     */
    virtual Chord eRPT(double range, int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range, permutational, and transpositional equivalence, in the 
     * equal temperament generated by g; the same as chord type.
     */
    virtual Chord eRPTT(double range, double g = 1., int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range and transpositional equivalence.
     */
    virtual Chord eRT(double range) const;
    /**
     * Returns all equivalents of the chord within all fundamental 
     * domains of range, permutational, and transpositional equivalence.
     */
    virtual std::vector<Chord> eRPTs(double range = OCTAVE()) const;
     /**
     * Returns all equivalents of the chord within all fundamental 
     * domains of range, permutational, and transpositional equivalence in 
     * the equal temperament generated by g; equivalent to all inversions of 
     * the chord in the musician's sense.
     */
    virtual std::vector<Chord> eRPTTs(double range, double g = 1.) const;
    /**
     * Returns the equivalent of the chord within the representative fundamental
     * domain of range, permutational, transpositional, and inversional
     * equivalence.
     */
    virtual Chord eRPTI(double range, int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range, permutational, transpositional, and inversional
     * equivalence.
     */
    virtual Chord eRPTTI(double range, double g = 1., int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within a fundamental
     * domain of range, permutational, transpositional, and inversional
     * equivalence in the equal temperament generated by g; the same as set 
     * class.
     */
    virtual Chord eT() const;
    /**
     * Returns the equivalent of the chord within the representative fundamental
     * domain of transpositonal equivalence and the equal temperament generated
     * by g, i.e., returns the chord transposed such that its layer is 0 or, under
     * transposition, the positive layer closest to 0. NOTE: Does NOT return the
     * result under any other equivalence class.
     */
    virtual Chord eTT(double g = 1.) const;
    /**
     * Returns the equivalent of the chord within the fundamental domain of
     * transposition to 0.
     */
    virtual Chord et() const;
    /**
     * Returns a new chord whose pitches are the floors of this chord's pitches.
     */
    virtual Chord floor() const;
    virtual double getDuration(int voice = 0) const;
    virtual double getInstrument(int voice = 0) const;
    virtual double getLoudness(int voice = 0) const ;
    virtual double getPan(int voice = 0) const;
    virtual double getPitch(int voice) const;
    virtual double &getPitchReference(int voice);
    /**
     * Returns whether the voices of this chord are greater than the voices of 
     * the other.
     */
    virtual bool greater(const Chord &other) const;
    /**
     * Returns whether the voices of this chord are greater than or equal to
     * the voices of the other.
     */
    virtual bool greater_equals(const Chord &other) const;
    /**
     * Rebuilds the chord's pitches (only) from a line of text.
     */
    virtual void fromString(std::string text);
    /**
     * Returns the hyperplane equation for the inversion flat that evenly 
     * divides the fundamental domain in the indicated sector of the OPT 
     * cyclical region.
     */
    virtual const HyperplaneEquation &hyperplane_equation(int opt_sector) const;
    /**
     * For each chord space of dimensions 3 <= n <= 12, there are n 
     * fundamental domains (sectors) of OPT equivalence. For each OPT fundamental domain,
     * there is a inversion flat that evenly divides the OPT fundamental domain into 2 OPTI 
     * fundamental domains. This function returns a global collection of the hyperplane 
     * equations that define these inversion flats.
     */
    static std::map<int, std::vector<HyperplaneEquation>> &hyperplane_equations_for_dimensionalities();
    /**
     * Inverts the chord by another chord that is on the unison diagonal, by
     * default the origin.
     * 
     * NOTE: Does NOT return an equivalent under any requivalence relation; 
     * not to be confused with reflection in the inversion flat.
     */
    virtual Chord I(double center = 0.0) const;
    /**
     * Returns whether the chord is an inversional form of Y with interval size g.
     * Only works in equal temperament.
     */
    virtual bool Iform(const Chord &Y, double g = 1.) const;
    /**
     * Print much information about the chord including whether it is within 
     * important equivalence classes, or what its equivalents would be. The 
     * printout first enables then restores debugging diagnostics.
     */
    virtual std::string information_debug(int opt_sector) const;
    /**
     * Print much information about the chord including whether it is within 
     * important equivalence classes, or what its equivalents would be.
     */
    virtual std::string information_sector(int opt_sector) const;
    /**
     * Print much information about the chord including whether it is within 
     * important equivalence classes, or what its equivalents would be.
     */
    virtual std::string information() const;
    /**
     * Initializes the fundamental domains (sectors) of the cyclical regions 
     * of OPT equivalence and OPTI equivalence, as well as the hyperplane 
     * equations that define the inversion flat in each OPT sector.
     * 
     * The cyclical region C of OPT for n voices is the (n-1)-simplicial 
     * region of R^n / T with n vertices at A_i = [0^(n - i), 12^n)]_T, for 
     * 0 <= i < n. These are the n octavewise revoicings of the origin. 
     *
     * (1) To obtain the fundamental regions of OPT in C, for dimensions 
     *     0 <= d < n, replace C[(d+n-1)%n] with the center of C c to give 
     *     OPT_d.
     * 
     * (2) To obtain the fundamental regions for OPTI in C for dimensions 
     *     0 <= d < n, replace OPT_d[(d+n-2)%n] with the midpoint of 
     *     OPT_d[(d+n)%n] => OPT_d[(d+n-2)%n] to give OPTI_d_0, and replace 
     *     OPT_d[(d+n)%n] with the midpoint of OPT_d[(d+n)%n] => 
     *     OPT_d[(d+n-2)%n] to give OPTI_d_1.
     *
     * (3) A vector that is normal to the inversion flat in OPT_d is then 
     *     OPT_d[(d+n)%n] => OPT_d[d+n-2)%n]. Normalizing this vector gives 
     *     the unit normal vector u for the inversion flat. Then the 
     *     hyperplane equation for the inversion flat is u and its constant 
     *     term is u dot c.
     *
     * NOTE: 
     *
     * In this code, sector vertices are NOT permuted.
     *
     * The reason for starting with C[n-1] is to include the origin in each 
     * OPT sector, which is important for the code that identifies whether a 
     * chord is in an OPT sector. If we started with C[0], then the origin 
     * would be on the boundary of all OPT sectors, and the code that 
     * identifies whether a chord is in an OPT sector would have to identify 
     * whether the chord is on the boundary of the sector, which is more 
     * complicated than identifying whether the chord is in the interior of 
     * the sector. 
     *
     * This code is based on the construction of Noam Elkies described in the 
     * _Generalized Chord Spaces_ draft by Callender, Quinn, and Tymoczko.
     */
    virtual void initialize_sectors();

    virtual void ensure_sectors_initialized();
    /**
     * Returns this chord as the inverse standard "prime form."
     *
     * NOTE: The code here does NOT remove duplicate pitch-classes.    
     */
    virtual Chord inverse_prime_form() const;
    /**
     * Returns whether this chord has a compact voicing. This identifies 
     * whether the chord belongs to the representative fundamental domain of 
     * the OPT equivalence class. In Tymoczko's 1-based notation:
     * x[1] + 12 - x[N] <= x[i + 1] - x[i], 1 <= i < N - 1
     * In 0-based notation:
     * x[0] + 12 - x[N-1] <= x[i + 1] - x[i], 0 <= i < N - 2
     */
    virtual bool is_compact(double range=12.) const;
    /**
     * Returns whether this chord is "minor" in the sense of having the 
     * smallest "wrapround interval" of all its voicings.
     */
    virtual bool is_minor() const;
    /**
     * Returns whether or not this chord lies within the indicated sector of 
     * the cyclical region of the OPT (or RPT) fundamental domain.
     */
    virtual bool is_in_rpt_sector(int opt_sector = 0, double range = 12.) const;
    /**
     * Raw sector membership test.
     *
     * This assumes *this* is already expressed in the RP/T base (i.e. already
     * reduced by eRP(range).eT() or an equivalent normalization).
     *
     * Unlike is_in_rpt_sector(), this function performs no internal reduction,
     * which makes it suitable for decomposability checks.
     */
    virtual bool is_in_rpt_sector_base(int opt_sector = 0, double range = 12.) const;
    /**
     * Returns whether this chord is in the "minor" half of the OPTI 
     * fundamental domain. This function is only valid when called with 
     * an OPT sector to which the chord belongs.
     */
    virtual bool is_in_minor_rpti_sector(int opt_sector) const;
    /**
     * Returns whether the chord is within the fundamental domain of inversional 
     * equivalence.
     */
    virtual bool iseI_chord(Chord *inverse, int opt_sector = 0) const;
    virtual bool iseI(int opt_sector = 0) const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of octave equivalence.
     */
    virtual bool iseO() const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of octave and permutational equivalence.
     */
    virtual bool iseOP() const;
    /**
     * Returns whether the chord is within a fundamental domain of octave, 
     * permutational, and inversional equivalence.
     */
    virtual bool iseOPI(int opt_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domainof octave, 
     * permutational, and transpositional equivalence.
     */
    virtual bool iseOPT(int opt_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domain of octave, 
     * permutational, and transpositional equivalence in the equal temperament 
     * generated by g.
     */
    virtual bool iseOPTT(double g = 1., int opt_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domain of octave, 
     * permutational, transpositional, and inversional equivalence.
     */
    virtual bool iseOPTI(int opt_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domain of octave, 
     * permutational, transpositional, and inversional equivalence in the 
     * equal temperament generated by g.
     */
    virtual bool iseOPTTI(double g  = 1., int opt_sector = 0) const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of octave and transpositional equivalence.
     */
    virtual bool iseOT() const {
        if (iseO() == false) {
            return false;
        }
        if (iseT() == false) {
            return false;
        }
        return true;
    }
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of octave and translational equivalence in the equal temperament 
     * generated by g.
     */
    virtual bool iseOTT(double g = 1.) const {
        if (iseO() == false) {
            return false;
        }
        if (iseTT(g) == false) {
            return false;
        }
        return true;
    }
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of permutational equivalence.
     */
    virtual bool iseP() const;
    /**
     * Returns whether the chord is within the fundamental domain of
     * pitch-class equivalence, i.e. is a pitch-class set.
     */
    virtual bool isepcs() const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of the indicated range equivalence.
     */
    virtual bool iseR(double range_) const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of range and permutational equivalence.
     */
    virtual bool iseRP(double range) const;
    /**
     * Returns whether the chord is within a fundamental domain of range, 
     * permutational, and inversional equivalence.
     */
    virtual bool iseRPI(double range, int opt_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domain of range, 
     * permutational, and transpositional equivalence.
     */
    virtual bool iseRPT(double range, int opt_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domain of range, 
     * permutational, and transpositional equivalence in the equal temperament 
     * generated by g.
     */
    virtual bool iseRPTT(double range, double g = 1., int opt_sector = 0) const;
    /** 
     * Returns whether the chord is within a fundamental domain of range, 
     * permutational, transpositional, and inversional equivalence.
     */
    virtual bool iseRPTI(double range, int opt_sector = 0) const;
     /** 
     * Returns whether the chord is within a fundamental domain of range, 
     * permutational, transpositional, and inversional equivalence in the '
     * equal temperament generated by g.
     */
   virtual bool iseRPTTI(double range, double g = 1., int opt_sector = 0) const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of range and transpositional equivalence.
     */
    virtual bool iseRT(double range) const;
    /**
     * Returns whether the chord is within a fundamental domain of range and 
     * transpositional equivalence in the equal temperament generated by g.
     */
    virtual bool iseRTT(double range, double g = 1.) const {
        if (iseR(range) == false) {
            return false;
        }
        if (iseTT(g) == false) {
            return false;
        }
        return true;
    }
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of transpositional equivalence.
     */
    virtual bool iseT() const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of transpositional equivalence in the equal temperament 
     * generated by g.
     */
    virtual bool iseTT(double g = 1.) const;

    /**
     * Returns whether the chord is within the fundamental domain of
     * transposition to 0.
     */
    virtual bool iset() const;
    /**
     * Returns whether or not this chord is invariant under reflection in the 
     * inversion flat of the indicated OPT sector. Such are the shared 
     * vertices, edges, and facets of those fundamental domains that involve 
     * inversional equivalence.
     */
    virtual bool self_inverse(int opt_sector = 0) const;
    /**
     * Returns the chord inverted by the sum of its first two voices.
     */
    virtual Chord K() const;
    virtual Chord K_range(double range) const;
    /**
     * Returns the sum of the pitches in the chord.
     */
    virtual double layer() const;
    /**
     * Returns whether the voices of this chord are less than the voices of 
     * the other.
     */
    virtual bool lesser(const Chord &other) const;
    /**
     * Returns whether the voices of this chord are less than or equal to
     * the voices of the other.
     */
    virtual bool lesser_equals(const Chord &other) const;
    /**
    * Returns the highest pitch in the chord, and also the voice index of that
    * pitch.
    */
    virtual std::vector<double> max() const;
    /**
     * Returns the maximally even chord in the chord's space,
     * e.g. the augmented triad for 3 dimensions.
     */
    virtual Chord center() const;
    /**
     * Returns the maximum interval within the chord.
     */
    virtual double maximumInterval() const;
    /**
    * Returns the lowest pitch in the chord, and also the voice index of that 
    * pitch.
    */
    virtual std::vector<double> min() const;
    /**
     * Returns the minimum interval within the chord.
     */
    virtual double minimumInterval() const;
    /**
     * Move 1 voice of the chord.
     * 
     * NOTE: Does NOT return an equivalent under any requivalence relation.
     */
    virtual Chord move(int voice, double interval) const;
    /**
     * Return the jazz-style name of the chord, if possible, or else a 
     * human-readable list of the voices in the chord.
     */
    virtual std::string name() const;
    /**
     * Returns this chord as its standard "normal form."
     *
     * NOTE: The code here does NOT remove duplicate pitch-classes.
     */
    virtual Chord normal_form() const;
    /**
     * Returns this chord in standard "normal order." For a very clear 
     * explanation, see: 
     * https://www.mta.ca/pc-set/pc-set_new/pages/page04/page04.html and 
     * http://openmusictheory.com/normalOrder.html/
     * 
     * NOTE: The code here does NOT remove duplicate pitch-classes.
     * "Normal order" is the most compact ordering to the left of 
     * pitch-classes in a chord, measured by pitch-class interval.
     */
    virtual Chord normal_order() const;
    /**
     * Performs the dominant transformation (which is not a neo-Reimannian 
     * transformation). The result is returned in OP.
     */
    virtual Chord nrD() const;
    /**
     * Performs the neo-Riemannian hexatonic pole transformation. The result
     * is returned in OP.
     */
    virtual Chord nrH() const;
    /**
     * Performs the neo-Riemannian Lettonwechsel transformation. The result 
     * is returned in OP.
     */
    virtual Chord nrL() const;
    /**
     * Performs the neo-Riemannian Nebenverwandt transformation. The result 
     * is returned in NP.
     */
    virtual Chord nrN() const;
    /**
     * Performs the neo-Riemannian parallel transformation. The result is 
     * returned in OP.
     */
    virtual Chord nrP() const;
    /**
     * Performs the neo-Riemannian parallel transformation.
     */
    virtual Chord nrR() const;
    /**
     * Performs the neo-Riemannian Slide transformation. The result is 
     * returned in OP.
     */
    virtual Chord nrS() const;
    /**
     * Returns the vertices of the OPT fundamental domain for the indicated
     * sector of the cyclical region.
     */
    virtual std::vector<Chord> opt_domain(int sector) const;
    /**
     * Returns the zero-based index(s) of the sector(s) within the cyclical 
     * region of OPT fundamental domains to which the chord belongs. A chord 
     * on a vertex, edge, or facet shared by more than one sector belongs to 
     * each of them; the center of the cyclical region belongs to all of the 
     * sectors. Sectors are generated by rotation of a fundamental domain 
     * around the central axis (equivalently, by the octavewise revoicing of 
     * chords) and correspond to "chord inversion" in the musician's sense.
     */
    virtual std::vector<int> opt_domain_sectors() const;
    /**
     * For each chord space of dimensions 3 <= n <= 12, there are n 
     * fundamental domains (sectors) of OPT equivalence. This function returns a global 
     * collection of these sectors. 
     */
    static std::map<int, std::vector<std::vector<Chord>>> &opt_sectors_for_dimensionalities();
    /**
     * Returns a collection of vertices for the OPT fundamental domains; each 
     * has an added vertex to make a simplex for chord location. 
     */
    static std::map<int, std::vector<std::vector<Chord>>> &opt_simplexes_for_dimensionalities();
    /**
     * Returns the origin of the chord's space.
     */
    virtual Chord origin() const;
    /**
     * Returns the permutations of the pitches in a chord. The permutations 
     * are always returned in the same order.
     */
    virtual std::vector<Chord> permutations() const;
    /**
     * Returns this chord as its standard "prime form." 
     *
     * NOTE: The code here does NOT remove duplicate pitch-classes.    
     */
    virtual Chord prime_form() const;
    /**
     * Returns the contextual transposition of the chord by x with respect to m
     * with minimum interval size g.
     *
     * NOTE: Does NOT return an equivalent under any requivalence relation.
     */
    virtual Chord Q(double x, const Chord &m, double g = 1.) const;
    /**
     * Reflects the chord in the inversion flat of the indicated OPT domain sector.
     */
    virtual Chord reflect(int opt_sector, double g = OCTAVE()) const;

    virtual void resize(size_t voiceN);
    /**
     * Rounds the value of x to the specified number of decimal places.
     */
    static double rownd(double x, int places=12);
    virtual void setDuration(double value, int voice = -1);
    virtual void setInstrument(double value, int voice = -1);
    virtual void setLoudness(double value, int voice = -1);
    virtual void setPan(double value, int voice = -1);
    virtual void setPitch(int voice, double value);
    /**
     * Transposes the chord by the indicated interval (may be a fraction).
     *
     * NOTE: Does NOT return an equivalent under any requivalence relation.
     */
    virtual Chord T(double interval) const;
    /**
     * Tests the internal consistency of the predicates ("iseX") and 
     * transformations ("eX") of this chord, and prints a report.
     */
    virtual bool test(const char *caption="") const;
    /**
     * Returns whether the chord is a transpositional form of Y with interval size g.
     * Only works in equal temperament.
     */
    virtual bool Tform(const Chord &Y, double g = 1.) const;
    /**
     * Transposes the chord by the indicated voiceleading (passed as a Chord 
     * of directed intervals). 
     *
     * NOTE: Does NOT return an equivalent under any equivalence relation.
     */
    virtual Chord T_voiceleading(const Chord &voiceleading);
    /**
     * Returns a string representation of the chord's pitches (only).
     * Quadratic complexity, but short enough not to matter.
     */
    virtual std::string toString() const;
    /**
     * Returns a copy of the chord 'inverted' in the musician's sense, i.e. 
     * revoiced by cyclically permuting the chord and adding (or subtracting) 
     * an octave (or other interval) to the highest (or lowest) voice. The 
     * revoicing will move the chord up or down in pitch. A positive direction 
     * is the same as a  musician's first inversion, second inversion, etc.
     */
    virtual Chord v(int direction = 1, double interval = OCTAVE()) const;
    /**
     * Returns the transpositions (as a Chord of directed intervals) that 
     * takes this chord to the destination chord.
     *
     * NOTE: Makes no assumption that both chords are in the same equivalence 
     * class.
     */
    virtual Chord voiceleading(const Chord &destination) const;
    /**
     * Returns the number of voices in this chord; that is, the number of 
     * dimensions in the chord space for this chord.
     */
    virtual size_t voices() const;
    /**
     * Returns all the 'inversions' (in the musician's sense) of this chord. 
     * The first voice is transposed up (or down) by an interval (defaulting 
     * to the octave), and all voices are then rotated "left" so the 
     * transposed voice becomes the last voice. This procedure is iterated 
     * until all 'inversions' have been generated.
     */
    virtual std::vector<Chord> voicings(int direction = 1, double interval = OCTAVE()) const;
};

/**
 * Provides a strict weak ordering of chords by their pitches, independent of 
 * g. This must be used in all std:: containers that require ordering (e.g. 
 * std::set) to ensure that the ordering is stable and not affected by tiny 
 * floating point noise. The ordering is determined first by number of voices, 
 * then by pitches in voice order. NaNs are considered greater than any number, 
 * and equal to each other.
 */
struct ChordTickLess
{
    // Fixed resolution, independent of g.
    // 1e6 ticks per semitone gives a stable ordering while tolerating tiny 
    // floating point noise.
    static constexpr double ticks_per_semitone = 1000000.0;

    bool operator()(const Chord &a, const Chord &b) const
    {
        const int a_n = a.voices();
        const int b_n = b.voices();

        if (a_n != b_n)
        {
            return a_n < b_n;
        }

        for (int i = 0; i < a_n; ++i)
        {
            const double ap = a.getPitch(i);
            const double bp = b.getPitch(i);

            // Deterministic NaN handling so ordering can't go undefined.
            const bool a_nan = std::isnan(ap);
            const bool b_nan = std::isnan(bp);

            if (a_nan || b_nan)
            {
                if (a_nan != b_nan)
                {
                    // Put NaNs after numbers.
                    return b_nan;
                }
                // Both NaN at this position, continue.
                continue;
            }

            const int64_t at = static_cast<int64_t>(std::llround(ap * ticks_per_semitone));
            const int64_t bt = static_cast<int64_t>(std::llround(bp * ticks_per_semitone));

            if (at < bt)
            {
                return true;
            }
            if (at > bt)
            {
                return false;
            }
        }

        return false;
    }
};

struct SILENCE_PUBLIC HyperplaneEquation
{
    HyperplaneEquation()
        : apex_a()
        , base_midpoint()
        , unit_normal()
    {
    }

    /**
     * Create hyperplane for one OPT sector.
     *
     * vertices[0]  = apex
     * vertices[1..] = vertices of the opposite base facet
     *
     * Works in all dimensions.
     */
    void create(const std::vector<Chord> &vertices)
    {
        if (vertices.size() < 3)
        {
            System::error("HyperplaneEquation::create: need at least 3 vertices (apex + two base vertices).\n");
            return;
        }

        const int dimensions = vertices[0].rows();

        // Extract apex (must be vertices[0]).
        apex_a = chord_point_column(vertices[0]);

        // Compute centroid of the base facet (all vertices except apex).
        base_midpoint = Vector::Zero(dimensions, 1);
        const std::size_t base_count = vertices.size() - 1;
        for (std::size_t i = 1; i < vertices.size(); ++i)
        {
            base_midpoint += chord_point_column(vertices[i]);
        }
        base_midpoint /= double(base_count);

        // --------------------------------------------------------------------
        // Inversion-flat normal:
        // For sector k, initialize_sectors() must order the base vertices so that:
        //   vertices[1] = base_vertices[(k + 1) % n]  (v_plus)
        //   vertices[2] = base_vertices[(k - 1 + n) % n] (v_minus)
        //
        // Then the OPT inversion-flat (extruded along unison) has a normal
        // proportional to (v_plus - v_minus), projected off unison.
        // --------------------------------------------------------------------
        Vector normal_vector = chord_point_column(vertices[1]) - chord_point_column(vertices[2]);

        // Remove unison component (extrusion compatibility).
        Vector unison = Vector::Ones(dimensions, 1);
        const double unison_norm = unison.norm();
        if (unison_norm > 0.0)
        {
            const Vector unison_hat = unison / unison_norm;
            const double projection = normal_vector.dot(unison_hat);
            normal_vector = normal_vector - (projection * unison_hat);
        }

        const double norm_ = normal_vector.norm();
        if (norm_ <= 0.0)
        {
            System::error("HyperplaneEquation::create: degenerate normal.\n");
            unit_normal = Vector::Zero(dimensions, 1);
            return;
        }

        unit_normal = normal_vector / norm_;

        // Keep your existing sign convention: base facet should lie on "minor" side.
        const double signed_at_base = (base_midpoint - apex_a).dot(unit_normal);
        if (signed_at_base > 0.0)
        {
            unit_normal = -unit_normal;
        }
    }

    Chord reflect(const Chord &chord) const
    {
        Chord reflection = chord;

        const Vector chord_v = chord_point_column(chord);
        const Vector a_to_chord = chord_v - apex_a;

        const double signed_distance = a_to_chord.dot(unit_normal);

        const Vector reflected =
            chord_v - (2.0 * signed_distance * unit_normal);

        for (int voice = 0; voice < chord.voices(); ++voice)
        {
            reflection.setPitch(voice, reflected(voice, 0));
        }

        return reflection;
    }

    bool is_minor(const Chord &chord) const
    {
        const Vector chord_v = chord_point_column(chord);
        const Vector a_to_chord = chord_v - apex_a;
        const double signed_distance = a_to_chord.dot(unit_normal);

        return le_tolerance(signed_distance, 0.0, 64, 512);
    }

    bool is_invariant(const Chord &chord) const
    {
        const Vector chord_v = chord_point_column(chord);
        const Vector a_to_chord = chord_v - apex_a;
        const double signed_distance = a_to_chord.dot(unit_normal);

        return eq_tolerance(std::abs(signed_distance), 0.0, 64, 512);
    }

    Chord midpoint() const
    {
        Chord midpoint_chord;
        midpoint_chord.resize(apex_a.rows());

        for (int voice = 0; voice < midpoint_chord.voices(); ++voice)
        {
            midpoint_chord.setPitch(voice, base_midpoint(voice, 0));
        }

        return midpoint_chord;
    }

    std::string toString() const
    {
        std::string result;

        auto append_vector = [&](const char *label, const Vector &v)
        {
            result += label;
            const int rows = static_cast<int>(v.rows());
            for (int i = 0; i < rows; ++i)
            {
                char buffer[64];
                std::snprintf(buffer, sizeof(buffer), "%12.7f", v(i, 0));
                result += buffer;
                if (i < rows - 1)
                {
                    result += " ";
                }
            }
        };

        append_vector("unit_normal: ", unit_normal);
        result += " ";
        append_vector(" apex: ", apex_a);
        return result;
    }

    Vector apex_a;
    Vector base_midpoint;
    Vector unit_normal;

    static Vector chord_point_column(const Chord &chord)
    {
        const Eigen::Index rows = chord.rows();
        const Eigen::Index cols = chord.cols();

        if (rows <= 0)
        {
            return Vector();
        }

        if (cols <= 0)
        {
            System::error(
                "HyperplaneEquation: chord has no columns.\n");
            return Vector::Zero(rows, 1);
        }

        return chord.col(0);
    }
};

SILENCE_PUBLIC const Chord &chordForName(std::string name);

/**
 * Returns the pitch in the chord that is closest to the indicated pitch.
 */
SILENCE_PUBLIC double closestPitch(double pitch, const Chord &chord);

/**
 * Conforms the pitch to the pitch-class set, but in its original register.
 */
SILENCE_PUBLIC double conformToPitchClassSet(double pitch, const Chord &pitch_class_set);

/**
 * Returns the sum of the distances of the chord to each of the vertices 
 * of the indicated sector of a cyclical region.
 */
SILENCE_PUBLIC double distance_to_points(const Chord &chord, const std::vector<Chord> &sector_vertices);

/**
 * Returns the equivalent of the pitch under pitch-class equivalence, i.e.
 * the pitch is in the interval [0, OCTAVE). Implemented using the Euclidean
 * definition.
 */
SILENCE_PUBLIC double epc(double pitch);


inline SILENCE_PUBLIC bool is_in_affine_simplex(const Chord &point,
    const std::vector<Chord> &vertices,
    double tolerance = 1e-9)
{
    const int n = point.voices();
    const int k = static_cast<int>(vertices.size());

    if (k < 2)
    {
        return false;
    }
    if (vertices[0].voices() != n)
    {
        return false;
    }

    // We solve: point - v0 = B * w, where B columns are (vi - v0), i=1..k-1.
    // This is a least-squares solve in R^n for an (k-1)-dimensional affine simplex.
    const Chord &v0 = vertices[0];

    Eigen::MatrixXd B(n, k - 1);
    for (int i = 1; i < k; ++i)
    {
        Eigen::VectorXd col(n);
        for (int j = 0; j < n; ++j)
        {
            col(j) = vertices[i].getPitch(j) - v0.getPitch(j);
        }
        B.col(i - 1) = col;
    }

    Eigen::VectorXd rhs(n);
    for (int j = 0; j < n; ++j)
    {
        rhs(j) = point.getPitch(j) - v0.getPitch(j);
    }

    // Least-squares (works for rectangular B).
    Eigen::VectorXd w = B.colPivHouseholderQr().solve(rhs);

    // Check that point is actually close to the affine span (numerical sanity).
    Eigen::VectorXd recon = B * w;
    double residual = (rhs - recon).norm();
    double scale = std::max(1.0, rhs.norm());
    if (residual > tolerance * scale)
    {
        return false;
    }

    // Barycentric coordinates in the (k-1)-simplex:
    // lambda0 = 1 - sum(w), lambdai = w[i-1]
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

/**
 * Returns the Euclidean distance between the two chords.
 */
SILENCE_PUBLIC double euclidean(const csound::Chord &a, const csound::Chord &b);

/**
 * Enums for all defined equivalence relations, used to specialize template 
 * functions. If relation R takes no range argument, it defaults to a range of 
 * one octave. T is transposition to layer 0, Tg is transposition to the next 
 * chord higher than layer 0 in the equal temperament generated by g.
 *
 * NOTE: Not all of these are currently implemented.
 */
typedef enum {
    EQUIVALENCE_RELATION_r = 0,
    EQUIVALENCE_RELATION_R,
    EQUIVALENCE_RELATION_P,
    EQUIVALENCE_RELATION_T,
    EQUIVALENCE_RELATION_Tg,
    EQUIVALENCE_RELATION_I,
    EQUIVALENCE_RELATION_RP,
    EQUIVALENCE_RELATION_RT,
    //~ EQUIVALENCE_RELATION_RTg,
    //~ EQUIVALENCE_RELATION_RI,
    //~ EQUIVALENCE_RELATION_PT,
    //~ EQUIVALENCE_RELATION_PTg,
    //~ EQUIVALENCE_RELATION_PI,
    //~ EQUIVALENCE_RELATION_TI,
    EQUIVALENCE_RELATION_RPT,
    EQUIVALENCE_RELATION_RPTg,
    EQUIVALENCE_RELATION_RPI,
    EQUIVALENCE_RELATION_RTI,
    EQUIVALENCE_RELATION_RTgI,
    EQUIVALENCE_RELATION_RPTI,
    EQUIVALENCE_RELATION_RPTgI,
} EQUIVALENCE_RELATIONS;

#if !defined(SWIG)
static const char* namesForEquivalenceRelations[] = {
    "r",
    "R",
    "P",
    "T",
    "Tg",
    "I",
    "RP",
    "RT",
    //~ "RTg",
    //~ "RI",
    //~ "PT",
    //~ "PTg",
    //~ "PI",
    //~ "TI",
    "RPT",
    "RPTg",
    "RPI",
    "RTI",
    "RTgI",
    "RPTI",
    "RPTgI",
};
#endif

SILENCE_PUBLIC double factorial(double n);

void fill(std::string rootName, double rootPitch, std::string typeName, std::string typePitches, bool is_scale = false);

/**
 * Returns a set of chords in one sector of the cyclical region.
 */
template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC std::vector<Chord> fundamentalDomainByPredicate(int voiceN, double range, double g = 1., int sector=0, bool printme=false);

/**
 * Returns a set of chords in the union of all sectors of the cyclical region,
 * generated directly from the RP lattice (nondecreasing "shape" plus scalar
 * translation) and filtered by the indicated predicate.
 *
 * This avoids scanning an ambient box and therefore prevents pathological
 * growth in candidate counts for higher voice counts.
 */
template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC std::vector<Chord> fundamentalDomainByGeneration(int voiceN, double range, double g = 1., int sector=0, bool printme=false);

/**
 * Returns the pitch reflected in the center, which may be any pitch.
 * 
 * NOTE: Does NOT return an equivalent under any requivalence relation.
 */
SILENCE_PUBLIC double I(double pitch, double center = 0.0);

/**
 * Returns the index of the octavewise revoicing that this chord is,
 * counting up from the origin, within the indicated range. Returns
 * -1 if there is no such chord within the range.
 */
SILENCE_PUBLIC int indexForOctavewiseRevoicing(const Chord &origin, const Chord &chord, double range);

/**
 * Returns the index of the octavewise revoicing that this chord is,
 * relative to its OP equivalent, within the indicated range. Returns
 * -1 if there is no such chord within the range.
 */
SILENCE_PUBLIC int indexForOctavewiseRevoicing(const Chord &chord, double range);

void initializeNames();

/**
 * Template function returning whether or not the chord is within the 
 * specialized fundamental domain, which may in some cases be defined by the 
 * indicated range, generator of transposition g, and sector of the cyclical 
 * region of OPT fundamental domains.
 */
template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC bool predicate(const Chord &chord,
        double range, double g, int opt_sector);

// Declarations of specializations of the above template.

template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_r>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_R>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_P>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_T>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_Tg>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_I>(const Chord &chord,
        double range, double g, int opt_sector);            
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RP>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RT>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPT>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTg>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RTI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RTgI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTgI>(const Chord &chord,
        double range, double g, int opt_sector);            

/**
 * Returns a chord with the specified number of voices all set to a first
 * pitch, useful as an iterator.
 */
SILENCE_PUBLIC Chord iterator(int voiceN, double first);

SILENCE_PUBLIC bool le_tolerance(double a, double b, int epsilons, int ulps);

SILENCE_PUBLIC bool lt_tolerance(double a, double b, int epsilons, int ulps);

/**
 * Returns the chord that is the midpoint between two chords,
 * which must have the same number of voices.
 */
SILENCE_PUBLIC Chord midpoint(const Chord &a, const Chord &b);

SILENCE_PUBLIC double MIDDLE_C();

/**
 * Returns the remainder of the dividend divided by the divisor,
 * according to the Euclidean definition.
 */
SILENCE_PUBLIC double modulo(double dividend, double divisor);

SILENCE_PUBLIC std::string nameForChord(const Chord &chord);

SILENCE_PUBLIC std::string nameForPitchClass(double pitch);

SILENCE_PUBLIC std::string nameForScale(const Scale &scale);

SILENCE_PUBLIC std::multimap<Chord, std::string, ChordTickLess> &namesForChords();

SILENCE_PUBLIC std::multimap<Scale, std::string, ChordTickLess> &namesForScales();

/**
 * Increment a chord voicewise through chord space, from a low point on the 
 * unison diagonal through a high point on the unison diagonal. g is the 
 * generator of transposition. Before iterating the iterator must be set to 
 * the low point of iteration.
 */
SILENCE_PUBLIC bool next(Chord &iterator_, const Chord &minimum, double range, double g = 1.);

/**
 * Template function that returns the chord sent to a fundamental domain of 
 * specialized equivalence relation, which in some cases may be defined by the 
 * indicated range, generator of transposition g, and sector of the cyclical 
 * region of OPT fundamental domains.
 */
template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC Chord equate(const Chord &chord,
        double range, double g, int opt_sector);

// Forward declarations of specializations of the above template.

template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_r>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_R>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_P>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_T>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_Tg>(const Chord &chord,
        double range, double g, int opt_sector);
template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_I>(const Chord &chord,
        double range, double g, int opt_sector);            
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RP>(const Chord &chord,
        double range, double g, int opt_sector); 
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RT>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPT>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTg>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RTI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RTgI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTgI>(const Chord &chord,
        double range, double g, int opt_sector);           

/**
 * Returns the nth octavewise revoicing of the chord that is generated by iterating 
 * revoicings within the indicated range.
 */
SILENCE_PUBLIC Chord octavewiseRevoicing(const Chord &chord, int revoicingNumber_, double range);

/**
 * Returns the full set of octavewise revoicings of the chord within the indicated range.
 */

SILENCE_PUBLIC int octavewiseRevoicings(const Chord &chord, double range = OCTAVE());
/**
 * Returns whether the voiceleading between chords a and b contains a parallel fifth.
 */
SILENCE_PUBLIC bool parallelFifth(const Chord &a, const Chord &b);

SILENCE_PUBLIC double pitchClassForName(std::string name);

/**
 * This class implements a cyclic additive group for all chords under 
 * cardinality, permutational, and range equivalence. It is formed by the 
 * direct product of prime form equivalence or P, inversional equivalence or 
 * I, transpositional equivalence or T, and equivalence under octavewise 
 * revoicing within range R or V. The group is thus PITV = P x I x T x V. 
 * Therefore, operations on the P, I, T, or V subgroups may be used to 
 * independently and orthogonally transform the respective symmetry of any 
 * chord. Some of these operations will reflect in RP. 
 *
 * NOTE:  Prime form rather than OPTI is used because prime form abstracts 
 * from voicings (i.e. from the sectors of the OPT cyclical region).
 */
class SILENCE_PUBLIC PITV {
public:
    virtual ~PITV();
    int countI;
    int countP;
    int countT;
    int countV;
    /**
     * Returns the indices of prime form, inversion, transposition,
     * and voicing for a chord, as the first 4 elements, respectively,
     * of a homogeneous vector.
     *
     * NOTE: Where there are singularities in the quotient spaces for chords, 
     * there may be several chords that belong to the same equivalence class. 
     * In such cases, any of several chords at a singular point of the 
     * fundamental domain will return the same P.
     */
    Eigen::VectorXi fromChord(const Chord &chord, bool printme = false) const;
    /**
     * The generator of transposition.
     */
    double g;
    virtual int getCountI() const;
    virtual int getCountP() const;
    virtual int getCountT() const;
    virtual int getCountV() const;
    virtual int getG() const;
    virtual int getN() const;
    virtual int getRange() const;
    std::map<Chord, int, ChordTickLess> indexesForPs;
    std::map<int, Chord> PsForIndexes;
    std::set<Chord, ChordTickLess> normal_forms;
    virtual void initialize(int N_, double range_, double g_ = 1., bool printme=false);
    virtual void list(bool listheader = true, bool listps = false, bool listvoicings = false) const;
    int N;
    virtual void preinitialize(int N_, double range_, double g_ = 1.0);
    /**
     * The 0-based range of the chord space.
     */
    double range;
    /**
     * Returns the chord for the indices of prime form, inversion, 
     * transposition, and voicing.
     */
    std::vector<Chord> toChord(int P, int I, int T, int V, bool printme = false) const;
    std::vector<Chord> toChord_vector(const Eigen::VectorXi &pitv, bool printme = false) const;
};

SILENCE_PUBLIC const std::map<std::string, double> &pitchClassesForNames();

/**
 * Returns the point reflected in the hyperplane defined by the unit normal 
 * vector and constant term.
 */
SILENCE_PUBLIC Vector reflect_vector(const Vector &point, const Vector &unit_normal, double constant_term);

SILENCE_PUBLIC Chord reflect_by_householder(const Chord &chord);

SILENCE_PUBLIC Chord reflect_in_central_diagonal(const Chord &chord);

SILENCE_PUBLIC Chord reflect_in_central_point(const Chord &chord);

SILENCE_PUBLIC Chord reflect_in_inversion_flat(const Chord &chord, int opt_sector, double g = 1.0);

SILENCE_PUBLIC Chord reflect_in_unison_diagonal(const Chord &chord);
    
/**
 * Returns the named chord as a scale, that is, starting with the chord in OP, 
 * and sorting it from the tonic pitch-class on up. This enables 
 * transformations in tonal harmony such as transposing by scale degree. If no 
 * Chord exists for the name, an empty Chord is returned.
 */
SILENCE_PUBLIC Chord scale(std::string name);

/**
 * Scale as a class; must be created with the name of the scale. Inherits 
 * from Chord. Note that inherited Chord member functions such as T and I 
 * return Chords, not Scales.
 */
class SILENCE_PUBLIC Scale : public Chord {
    public:
         /**
         * Default constructor, an empty Scale.
         */
        Scale();
        Scale &operator=(Scale &&other) noexcept = default;
        Scale(const Scale &other) = default;
        Scale(Scale &&other) noexcept = default;
        /**
         * Creates a Scale by name, e.g. 'C major'. If the named Scale does 
         * not already exist, an empty Scale without a name is created.
         */
        Scale(std::string name);
        /** 
         * Creates a Scale with a new name as a set of pitches. These must 
         * start in octave 0 and be in ascending order, but otherwise may have 
         * any value in semitones or fractions of semitones; this permits the 
         * construction of new scales with any temperament and with any 
         * interval content. If a Scale with the proposed name already exists, 
         * that Scale is returned. New Scales are also stored as new named 
         * Scales.
         */
        Scale(std::string name, const Chord &scale_pitches);
        /** 
         * Creates a Scale with a new name as a set of pitches. These must 
         * start in octave 0 and be in ascending order, but otherwise may have 
         * any value in semitones or fractions of semitones; this permits the 
         * construction of new scales with any temperament and with any 
         * interval content. If a Scale with the proposed name already exists, 
         * that Scale is replaced. New Scales are also stored as new named 
         * Scales.
         */
        Scale(std::string name, const std::vector<double> &scale_pitches);
        virtual ~Scale();
        virtual Scale &operator = (const Scale &other);
        /** 
         * Sets the pitch of the specified voice of this as a (possibly 
         * fractional) MIDI key number, derived from the specified frequency 
         * ratio. The Scale must first have enough voices to hold all pitches 
         * set in this way.
         */
        virtual void set_ratio(const int voice, const double numerator, const double denominator);
        /** 
         * Recreates this Scale with a new name as a set of pitches from the 
         * text of a Scala file. Note that the tonic of the scale is always 
         * MIDI key 0 (C-1)! Thus to have, e.g., a D-1 just intonation scale, 
         * one would create the default C-1 just intonation scale, then add 2 
         * to each pitch.
         */
        virtual void from_scala(const std::string &name, const std::string &scala);
        /** 
         * Returns the chord for the indicated scale degree, number of voices
         * in the chord, and interval in scale degrees of the chord (defaults 
         * to thirds, or 3; the actual number of scale steps between chord 
         * pitches is interval - 1).
         */
        virtual Chord chord(int scale_degree, int voices, int interval = 3) const;
        /**
         * Moves the pitch (MIDI key number in 12TET) of the event to the 
         * nearest pitch-class of the chord implied by the scale degree, 
         * number of voices, and interval in chord steps between chord tones 
         * (defaulting to 3). If the alteration parameter is used, the pitch 
         * of the event is first moved to the nearest pitch-class of the chord 
         * implied by the scale degree, number of voices, and interval; this 
         * tone is then altered (plus or minus) by the number of semitones 
         * specified by the alteration parameter (default 0). All operations 
         * are performed under octave equivalence.
         */
       virtual void conform_with_alteration(Event &event, int scale_degree, int voices, int interval, int alteration) const;
       virtual void conform_with_interval(Event &event, int scale_degree, int voices, int interval) const;
       virtual void conform(Event &event, int scale_degree, int voices) const;
        /**
         * Returns the scale degree of the Chord in this Scale; if the 
         * Chord does not belong to this Scale, -1 is returned.
         */
        virtual int degree(const Chord &chord_, int interval = 3) const;
        /**
         * Returns the type name, e.g. "major" or "whole tone," of this.
         * This name will probably be invalid if the interval structure of 
         * this has been changed, e.g. by inversion.  
         */
        virtual std::string getTypeName() const;
        /**
         * Returns a list of common modulations, that is, other major or 
         * harmonic minor Scales to which the Chord belongs; optionally the 
         * Chord can first be resized (e.g. from a 9th chord to a triad) in 
         * order to find more or fewer possible modulations.
         */
        virtual std::vector<Scale> modulations(const Chord &chord) const;
        virtual std::vector<Scale> modulations_for_voices(const Chord &chord, int voices) const;
        /**
         * For any Chord belonging to this Scale, returns in the argument a 
         * list of other Scales to which that Chord also belongs. Switching to 
         * one of these Scales will perform some sort of modulation. The list 
         * of scale type names restricts the types of Scale that will be 
         * returned.
         */
        virtual void modulations_for_scale_types(std::vector<Scale> &result, const Chord &current_chord, int voices_, const std::vector<std::string> &type_names) const;
        /**
         * Returns the name of this Scale.
         */
        virtual std::string name() const;
        /**
         * Returns a list of common relative tonicizations for the Chord, that 
         * is, the other major or harmonic minor Scales for which that Chord 
         * could be mutated to have the secondary function. If that is not 
         * possible, an empty result is returned.
         */
        virtual std::vector<Scale> relative_tonicizations(const Chord &current_chord, int secondary_function = 5, int voices = -1) const;
        /**
         * Returns all major or minor Scales for which the current Chord is 
         * the tonic (scale degree 1). The number of voices defaults to that 
         * of the current Chord, but may be larger or smaller.
         *
         * NOTE: Here, tonicizations are modulations in which the Chord has 
         * degree 1, i.e. is the tonic chord.
         */
        virtual std::vector<Scale> tonicizations(const Chord &current_chord, int voices = -1) const;
        /**
         * Returns the _relative_ tonicizations of the Chord, that is, the 
         * scales for which that Chord could be mutated to have the secondary 
         * function, if that is possible. The list of scale types is used to 
         * restrict the types of Scales that are returned.
         */
        virtual void relative_tonicizations_for_scale_types(std::vector<Scale> &result, const Chord &current_chord, int secondary_function, int voices, const std::vector<std::string> &type_names) const;
        /**
         * Returns the current Chord mutated, if possible, to one or more 
         * function(s) with respect to another Chord in its Scale. Not 
         * "secondary function of this chord," but "this chord as secondary 
         * function of another (tonicized) chord." If that is not 
         * possible, an empty Chord is returned. The number of voices 
         * defaults to that of the current Chord. Can be used to generate 
         * secondary dominants (function = 5), secondary supertonics 
         * (function = 2), secondary subtonics (function = 6), and so on.
         * It is then up to the user to perform an appropriate progression 
         * by number of scale degrees in the original Scale.
         */
        virtual std::vector<Chord> secondary(const Chord &current_chord, int secondary_function = 5, int voices_ = -1) const;
        /**
         * Returns the number of semitones (may be whole or fractional) from 
         * the tonic (as 0) of this Scale to the indicated scale degree, which 
         * is wrapped around by octave equivalence.
         */
        virtual double semitones_for_degree(int scale_degree) const;
        /**
         * Returns the pitch-class that is the tonic or root of this Scale.
         */
        virtual double tonic() const;
        /** 
         * Returns a copy of this Scale transposed by the indicated number of 
         * _semitones_.
         */
        virtual Scale transpose(double semitones) const;
        /**
         * Returns a Chord transposed by the indicated number of _scale 
         * degrees_; the chord as passed must belong to this Scale, and the
         * interval must be the same as that used to generate the Chord; 
         * (defaults to thirds, or 3; the actual number of scale steps between 
         * chord pitches is interval - 1).
         */
        virtual Chord transpose_degrees(const Chord &chord, int scale_degrees, int interval = 3) const;
        /** 
         * Returns a copy of this Scale transposed to the indicated 
         * _scale degree_. 
         */
        virtual Scale transpose_to_degree(int degrees) const;
    protected:
        std::string type_name;
};

SILENCE_PUBLIC const Scale &scaleForName(std::string name);

SILENCE_PUBLIC std::map<std::string, Scale> &scalesForNames();

SILENCE_PUBLIC std::vector<std::string> split(std::string);

/**
 * Returns the pitch transposed by semitones, which may be any scalar.
 *
 * NOTE: Does NOT return an equivalent under any requivalence relation.
 */
SILENCE_PUBLIC double T(double pitch, double semitones);

/**
 * Returns the chord, in scale order, transposed within the scale by the 
 * indicated number of scale degrees, which can be positive or negative.
 * The original chord may be in any order or voicing. By default,
 * chords are generated by thirds, but they can be at any interval in scale 
 * degrees. If the original chord does not belong to the scale, an empty 
 * Chord is returned.
 */
SILENCE_PUBLIC Chord transpose_degrees(const Chord &scale, const Chord &original_chord, int transposition_degrees, int interval = 3);

SILENCE_PUBLIC std::set<Scale, ChordTickLess> &unique_scales();

/**
 * Returns the voice-leading between chords a and b,
 * i.e. what you have to add to a to get b, as a
 * chord of directed intervals.
 */
SILENCE_PUBLIC Chord voiceleading(const Chord &a, const Chord &b);

/**
 * Returns which of the voiceleadings (source to d1, source to d2)
 * is the closer (first smoother, then simpler), optionally avoiding parallel fifths.
 */
SILENCE_PUBLIC Chord voiceleadingCloser(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels = false);

/**
 * Returns the voicing of the destination which has the closest voice-leading
 * from the source within the range, optionally avoiding parallel fifths.
 */
SILENCE_PUBLIC Chord voiceleadingClosestRange(const Chord &source, const Chord &destination, double range, bool avoidParallels);

/**
 * Returns the smoothness of the voiceleading between chords a and b by L1 
 * norm.
 */
SILENCE_PUBLIC double voiceleadingSmoothness(const Chord &a, const Chord &b);

/**
 * Returns which of the voiceleadings (source to d1, source to d2) is the 
 * smoother (shortest moves), optionally avoiding parallel fifths.
 */
SILENCE_PUBLIC Chord voiceleadingSmoother(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels = false, double range = OCTAVE());

/**
 * Returns which of the voiceleadings (source to d1, source to d2) is the 
 * simpler (fewest moves), optionally avoiding parallel fifths.
 */
SILENCE_PUBLIC Chord voiceleadingSimpler(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels = false);

//////////////////////////////////////////////////////////////////////////////
// Definitions have been moved if possible to ChordSpace.cpp. The following 
// are template definitions that must be in the header.
//////////////////////////////////////////////////////////////////////////////

template<int EQUIVALENCE_RELATION>
SILENCE_PUBLIC std::vector<csound::Chord>
fundamentalDomainByPredicate(int voiceN, double range, double g, int sector, bool printme)
{
    const char *name_ = namesForEquivalenceRelations[EQUIVALENCE_RELATION];
    System::message(
        "fundamentalDomainByPredicate<%s>: voiceN: %d range: %f g: %f sector: %d\n",
        name_, voiceN, range, g, sector
    );

    if (g <= 0.0)
    {
        g = 1.0;
    }

    // Iterator bounds (same scheme you had).
    const int upperI = static_cast<int>(3 * (range + 1));
    const int lowerI = static_cast<int>(-(1 * (range + 1)));

    Chord it = iterator(voiceN, lowerI);
    const Chord origin = it;

    int scanned = 0;
    int accepted = 0;
    int in_sector0 = 0;

    std::vector<Chord> chords_in_domain;

    while (next(it, origin, upperI, g))
    {
        ++scanned;
        // This is the union predicate.
        for (auto sector = 0; sector < voiceN; ++sector) {
            if (predicate<EQUIVALENCE_RELATION>(it, range, g, sector))
            {
                ++accepted;
                chords_in_domain.push_back(it);
                if (sector == 0) {
                    ++in_sector0;
                }   
                break;
            }
        }

        if (printme)
        {
            System::message(
                "fundamentalDomainByPredicate<%s>: %s accepted: %6d unique: %6d scanned: %12d %s\n",
                name_,
                predicate<EQUIVALENCE_RELATION>(it, range, g, sector) ? "NORMAL " : "       ",
                accepted,
                static_cast<int>(chords_in_domain.size()),
                scanned,
                print_chord(it).c_str()
            );
        }
    }

     System::message(
        "fundamentalDomainByPredicate<%s>: scanned: %d accepted: %d size: %d size in sector 0: %d\n",
        name_, scanned, accepted, static_cast<int>(chords_in_domain.size()), in_sector0
    );

    return chords_in_domain;
}   

template<int EQUIVALENCE_RELATION>
SILENCE_PUBLIC std::vector<csound::Chord>
fundamentalDomainByGeneration(int voiceN, double range, double g, int sector, bool printme)
{
    const char *name_ = namesForEquivalenceRelations[EQUIVALENCE_RELATION];
    System::message(
        "fundamentalDomainByGeneration<%s>: voiceN: %d range: %f g: %f sector: %d\n",
        name_, voiceN, range, g, sector
    );

    if (voiceN <= 0)
    {
        return {};
    }

    if (g <= 0.0)
    {
        g = 1.0;
    }

    // We generate candidates in the RP lattice parameterization:
    //   chord[v] = m + s[v], with s[0]=0 and nondecreasing integer s[v] in [0, range].
    // This guarantees P and max-min <= range. We then choose integer m so that
    // layer(chord) is within [0, range], which satisfies R.
    const int range_i = static_cast<int>(std::floor(range + 0.0000001));

    auto floor_div = [](int a, int b) -> int
    {
        // b > 0
        if (a >= 0)
        {
            return a / b;
        }
        return -(((-a) + b - 1) / b);
    };

    auto ceil_div = [](int a, int b) -> int
    {
        // b > 0
        if (a >= 0)
        {
            return (a + b - 1) / b;
        }
        return -(((-a) / b));
    };

    std::vector<Chord> chords_in_domain;
    chords_in_domain.reserve(1024);

    std::vector<int> shape(static_cast<size_t>(voiceN), 0);

    long long generated = 0;
    long long accepted = 0;
    long long in_sector0 = 0;

    // Recursive enumeration of nondecreasing shapes with shape[0]=0.
    std::function<void(int, int)> enumerate_shapes = [&](int voice, int last_value)
    {
        if (voice >= voiceN)
        {
            long long sum_s = 0;
            for (int v = 0; v < voiceN; ++v)
            {
                sum_s += shape[static_cast<size_t>(v)];
            }

            const int n = voiceN;
            // m such that 0 <= n*m + sum_s <= range_i
            const int m_min = ceil_div(static_cast<int>(-sum_s), n);
            const int m_max = floor_div(static_cast<int>(range_i - sum_s), n);

            for (int m = m_min; m <= m_max; ++m)
            {
                Chord c(voiceN);
                for (int v = 0; v < voiceN; ++v)
                {
                    c.setPitch(v, static_cast<double>(m + shape[static_cast<size_t>(v)]));
                }

                ++generated;

                // Union over sectors, as in fundamentalDomainByPredicate.
                for (int s = 0; s < voiceN; ++s)
                {
                    if (predicate<EQUIVALENCE_RELATION>(c, range, g, s))
                    {
                        ++accepted;
                        chords_in_domain.push_back(c);
                        if (s == 0)
                        {
                            ++in_sector0;
                        }
                        break;
                    }
                }

                if (printme)
                {
                    System::message(
                        "fundamentalDomainByGeneration<%s>: accepted: %6lld size: %6d generated: %12lld %s\n",
                        name_,
                        accepted,
                        static_cast<int>(chords_in_domain.size()),
                        generated,
                        print_chord(c).c_str()
                    );
                }
            }
            return;
        }

        for (int v = last_value; v <= range_i; ++v)
        {
            shape[static_cast<size_t>(voice)] = v;
            enumerate_shapes(voice + 1, v);
        }
    };

    // shape[0] fixed at 0; enumerate remaining voices.
    enumerate_shapes(1, 0);

    System::message(
        "fundamentalDomainByGeneration<%s>: generated: %lld accepted: %lld size: %d size in sector 0: %lld\n",
        name_,
        generated,
        accepted,
        static_cast<int>(chords_in_domain.size()),
        in_sector0
    );

    return chords_in_domain;
}

} // End of namespace csound.

#pragma GCC diagnostic push
  
#endif
