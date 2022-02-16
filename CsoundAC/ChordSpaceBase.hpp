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

#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#include "Platform.hpp"
#include "System.hpp"
#ifdef SWIG
%module CsoundAC
%{
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
%}
%include "std_string.i"
%include "std_vector.i"
#else
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

inline SILENCE_PUBLIC std::string chord_space_version() {
    return "ChordSpaceBase version 2.0.3.";
}

/**
 * Returns the current state of the chord space debugging flag as a 
 * reference, which can be an lvalue or an rvalue.
 */
inline SILENCE_PUBLIC bool &CHORD_SPACE_DEBUGGING() {
    static bool CHORD_SPACE_DEBUGGING_ = false;
    return CHORD_SPACE_DEBUGGING_;
}

struct SILENCE_PUBLIC SCOPED_DEBUGGING {
    int prior_state = false;
    SCOPED_DEBUGGING() {
        prior_state = CHORD_SPACE_DEBUGGING();
        CHORD_SPACE_DEBUGGING() = true;
    }
    ~SCOPED_DEBUGGING() {
        CHORD_SPACE_DEBUGGING() = prior_state;
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

struct SILENCE_PUBLIC HyperplaneEquation {
    Matrix unit_normal_vector;
    double constant_term;
};

class SILENCE_PUBLIC PITV;

class SILENCE_PUBLIC Scale;

SILENCE_PUBLIC double distance_to_points(const Chord &chord, const std::vector<Chord> &points);

SILENCE_PUBLIC double epc(double pitch);

SILENCE_PUBLIC bool gt_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC HyperplaneEquation hyperplane_equation_from_singular_value_decomposition(const std::vector<Chord> &points_, bool make_eT);

SILENCE_PUBLIC bool le_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC bool lt_tolerance(double a, double b, int epsilons=20, int ulps=200);

SILENCE_PUBLIC Chord midpoint(const Chord &a, const Chord &b);

/**
 * Cache prime forms for chords for speed.
 */
SILENCE_PUBLIC std::map<Chord, Chord> &normal_forms_for_chords();

/**
 * Cache normal forms for chords for speed.
 */
SILENCE_PUBLIC std::map<Chord, Chord> &prime_forms_for_chords();

/**
 * Cache inverse prime forms for chords for speed.
 */
SILENCE_PUBLIC std::map<Chord, Chord> &inverse_prime_forms_for_chords();

/**
 * Returns a string representation of the pitches in the chord, along with the 
 * sectors of the cyclical regions of the OPT and OPTI fundamental domains to 
 * which the chord belongs.
 */
SILENCE_PUBLIC const char *print_chord(const Chord &chord);


// End of forward declarations needed by other forward declarations!

/**
 * The size of the octave, defined to be consistent with
 * 12 tone equal temperament and MIDI.
 */
SILENCE_PUBLIC double OCTAVE();

SILENCE_PUBLIC bool operator == (const Chord &a, const Chord &b);

SILENCE_PUBLIC bool operator < (const Chord &a, const Chord &b);

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
#if __cpplusplus >= 201103L
    Chord &operator = (Chord &&other) = default;
#endif
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
     * region of n fundamental domains of OPT equivalence. The vertices of the
     * cyclical region consist of the n octavewise revoicings of the origin. 
     * This function returns a global collection of these cyclical regions.
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
    virtual HyperplaneEquation hyperplane_equation(int opt_sector) const;
    /**
     * For each chord space of dimensions 3 <= n <= 12, there are n 
     * fundamental domains (sectors) of OPT equivalence. For each OPT fundamental domain,
     * there is a inversion flat that evenly divides the OPT fundamental domain into 2 OPTI 
     * fundamental domains. This function returns a global collection of the hyperplane 
     * equations that define these inversion flats.
     */
    static std::map<int, std::vector<HyperplaneEquation>> &hyperplane_equations_for_opt_sectors();
    /**
     * Inverts the chord by another chord that is on the unison diagonal, by
     * default the origin.
     * 
     * NOTE: Does NOT return an equivalent under any requivalence relation.
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
     * The reason for starting with C[n-1] is to include the origin in the 0th 
     * fundamental domain, because we regard OPT sector 0 as the 
     * _representative_ fundamental  domain of OPT.
     *
     * This code is based on the construction of Noam Elkies described in the 
     * _Generalized Chord Spaces_ draft by Callender, Quinn, and Tymoczko.
     */
    virtual void initialize_sectors();
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
     * the cyclical region of OPT fundamental domains.
     */
    virtual bool is_opt_sector(int opt_sector = 0) const;
    /**
     * Returns whether or not this chord lies within the indicated sector of 
     * the cyclical region of OPTI fundamental domains.
     */
    virtual bool is_opti_sector(int opti_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domain of inversional 
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
    virtual bool iseRT(double range) const {
        if (iseR(range) == false) {
            return false;
        }
        if (iseT() == false) {
            return false;
        }
        return true;
    }
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
     * 
     * NOTE: Does NOT return an equivalent under any requivalence relation.
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
     * Returns the vertices of the OPTI fundamental domain for the indicated
     * sector of the cyclical region.
     */
    virtual std::vector<Chord> opti_domain(int sector) const;
    /**
     * Returns the zero-based index(s) of the sector(s) within the cyclical 
     * region of OPTI fundamental domains to which the chord belongs. A chord 
     * on a vertex, edge, or facet shared by more than one sector belongs to 
     * each them; the center of the cyclical region belongs to all of the 
     * sectors. Sectors are generated by rotation of a fundamental domain 
     * (equivalently, by the octavewise revoicing of chords) and correspond to 
     * "chord inversion" in the musician's ordinary sense.
     */
    virtual std::vector<int> opti_domain_sectors() const;
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
     * For each chord space of dimensions 3 <= n <= 12, there are n 
     * fundamental domains (sectors) of OPTI equivalence. This function returns a global 
     * collection of these sectors. 
     */
    static std::map<int, std::vector<std::vector<Chord>>> &opti_sectors_for_dimensionalities();
     /**
     * Returns a collection of vertices for the OPTI fundamental domains that 
     * have an added  vertex to make a simplex for chord location. 
     */
    static std::map<int, std::vector<std::vector<Chord>>> &opti_simplexes_for_dimensionalities();
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
    virtual Chord reflect(int opt_sector) const;
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
     * an octave to the highest (or lowest) voice. The revoicing will move the 
     * chord up or down in pitch. A positive direction is the same as a 
     * musician's first inversion, second inversion, etc.
     */
    virtual Chord v(int direction = 1) const;
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
     * Returns all the 'inversions' (in the musician's sense) or octavewise 
     * revoicings of the chord. The first voice is transposed up by one octave, 
     * and all voices are then rotated "left" so the transposed voice becomes 
     * the last voice.
     */
    virtual std::vector<Chord> voicings() const;
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

/**
 * This is the basis of all other numeric comparisons that take floating-point 
 * limits into account. It is a "close enough" comparison. If a or b equals 0,
 * the indicated number of machine epsilons is used as the tolerance; if 
 * neither a nor b equals 0, the indicated number of units in the last place
 * (ULPs) is used as the tolerance. These tolerances should be set to 
 * appropriate values based on the use case.
 */
SILENCE_PUBLIC bool eq_tolerance(double a, double b, int epsilons=20, int ulps=200);

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

SILENCE_PUBLIC double factorial(double n);

void fill(std::string rootName, double rootPitch, std::string typeName, std::string typePitches, bool is_scale = false);
 
/**
 * Returns a set of chords in sector 0 of the cyclical region, sorted by 
 * normal order, for the indicated equivalence relation. If there are 
 * duplicate chords for the same equivalence, only the one closest to the 
 * origin is returned.
 */
template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC std::vector<Chord> fundamentalDomainByPredicate(int voiceN, double range, double g = 1., int sector=0, bool printme=false);

/**
 * Returns a set of chords in sector 0 of the cyclical region, sorted by 
 * normal order, for the indicated equivalence relation. All duplicate chords 
 * for the same equivalence are returned, ordered by distance from the origin.
 */
template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC std::vector<Chord> fundamentalDomainByTransformation(int voiceN, double range, double g = 1., int sector=0);

SILENCE_PUBLIC bool ge_tolerance(double a, double b, int epsilons=20,int ulps=200);

//~ /**
 //~ * Given a set of points sufficient to define a hyperplane, computes the 
 //~ * scalar equation of the hyperplane. The algorithm derives vectors from the 
 //~ * points and solves for the scalar equation using the singular value 
 //~ * decomposition. The equation is returned in the form of a unit normal vector 
 //~ * of the hyperplane and a constant factor.
 //~ */
//~ SILENCE_PUBLIC HyperplaneEquation hyperplane_equation(const std::vector<Chord> &points_in_hyperplane, bool make_eT = true);

//~ SILENCE_PUBLIC HyperplaneEquation hyperplane_equation_from_random_inversion_flat(int dimensions, bool transpositional_equivalence = true, int opt_sector = 1);

SILENCE_PUBLIC bool gt_tolerance(double a, double b, int epsilons, int ulps);

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

SILENCE_PUBLIC std::multimap<Chord, std::string> &namesForChords();

SILENCE_PUBLIC std::multimap<Scale, std::string> &namesForScales();

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
    std::map<Chord, int> indexesForPs;
    std::map<int, Chord> PsForIndexes;
    std::set<Chord> normal_forms;
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
SILENCE_PUBLIC Vector reflect_vector(const Vector &point, const Vector &unit_normal_vector, double constant_term);

SILENCE_PUBLIC Chord reflect_by_householder(const Chord &chord);

SILENCE_PUBLIC Chord reflect_in_central_diagonal(const Chord &chord);

SILENCE_PUBLIC Chord reflect_in_central_point(const Chord &chord);

SILENCE_PUBLIC Chord reflect_in_inversion_flat(const Chord &chord, int opt_sector);

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
         * Returns the chord for the indicated scale degree, number of voices
         * in the chord, and interval in scale degrees of the chord (defaults 
         * to thirds, or 3; the actual number of scale steps between chord 
         * pitches is interval - 1).
         */
        virtual Chord chord(int scale_degree, int voices, int interval = 3) const;
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

SILENCE_PUBLIC std::set<Chord> &unique_chords();

SILENCE_PUBLIC std::set<Scale> &unique_scales();

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

//////////////////////////////////////////////////
// ONLY DEFINITIONS BELOW HERE -- NO DECLARATIONS.
//////////////////////////////////////////////////

static std::mt19937 mersenne_twister;

inline SILENCE_PUBLIC std::string toString(const Matrix& mat){
    std::stringstream ss;
    ss << mat;
    return ss.str();
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_r>(const Chord &chord, double range, double g, int opt_sector) {
    Chord normal = chord;
    for (int voice = 0; voice < chord.voices(); ++voice) {
        double pitch = chord.getPitch(voice);
        pitch = modulo(pitch, range);
        normal.setPitch(voice, pitch);
    }
    return normal;
}

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_R>(const Chord &chord, double range, double g, int opt_sector) {
    double max = chord.max()[0];
    double min = chord.min()[0];
    double min_plus_range = min + range;
    if (le_tolerance(max, min_plus_range) == false) {
        return false;
    }
    double sum_ = chord.layer();
    if (le_tolerance(0., sum_) == false) {
        return false;
    }
    if (le_tolerance(sum_, range) == false) {
        return false;
    }
    return true;
}

inline bool Chord::self_inverse(int opt_sector) const {
    auto inverse = reflect_in_inversion_flat(*this, opt_sector);
    if (*this == inverse) {
        return true;
    } else {
        return false;
    }
}

inline bool Chord::is_opt_sector(int index) const {
    auto sectors = opt_domain_sectors();
    for (auto sector : sectors) {
        if (sector == index) {
            return true;
        }
    }
    return false;
}

inline bool Chord::is_opti_sector(int index) const {
    auto sectors = opti_domain_sectors();
    for (auto sector : sectors) {
        if (sector == index) {
            return true;
        }
    }
    return false;
}


inline bool Chord::iseR(double range_) const {
    return predicate<EQUIVALENCE_RELATION_R>(*this, range_, 1.0, 0);
}

/*
// The clue here is that at least one voice must be >= 0,
// but no voice can be > range.
// First, move all pitches inside the interval [0,  range).
let er = this.er(range);
let most_compact_er = er;
// Then, reflect voices that are outside of the fundamental domain
// back into it, which will revoice the chord, i.e.
// the sum of pitches will then be in [0,  range].
// There may actually be more than one chord in the fundamental 
// domain that meets this criterion.
while (ChordSpace.le_tolerance(er.sum(), range) === false) {
    let max_ = er.max();
    let maximum_pitch = max_[0];
    let maximum_voice = max_[1];
    // Because no voice is above the range,
    // any voices that need to be revoiced will now be negative.
    er.voices[maximum_voice] = maximum_pitch - range;
    if (ChordSpace.le_tolerance(er.span(), most_compact_er.span()) == true) {
        most_compact_er = er;
    }
}
return most_compact_er;
*/
template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_R>(const Chord &chord, double range_, double g, int opt_sector) {
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

inline Chord Chord::eR(double range) const {
    return csound::equate<EQUIVALENCE_RELATION_R>(*this, range, 1.0, 0);
}

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_P>(const Chord &chord, double range, double g, int opt_sector) {
    for (size_t voice = 1; voice < chord.voices(); voice++) {
        double a = chord.getPitch(voice - 1);
        double b = chord.getPitch(voice);
        if (le_tolerance(a, b) == false) {
            return false;
        }
    }
    return true;
}

inline bool Chord::iseP() const {
    return predicate<EQUIVALENCE_RELATION_P>(*this, OCTAVE(), 1.0, 0);
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_P>(const Chord &chord, double range, double g, int opt_sector) {
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

inline Chord Chord::eP() const {
    return csound::equate<EQUIVALENCE_RELATION_P>(*this, OCTAVE(), 1.0, 0);
}

//	EQUIVALENCE_RELATION_T

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_T>(const Chord &chord, double range, double g, int opt_sector) {
    auto layer_ = chord.layer();
    CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_T>: chord: %s sector: %s layer: %12.7f\n", chord.toString().c_str(), opt_sector, layer_);
    if (eq_tolerance(layer_, 0.) == false) {
        return false;
    } else {
        return true;
    }
}

inline bool Chord::iseT() const {
    return predicate<EQUIVALENCE_RELATION_T>(*this, OCTAVE(), 1., 0);
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_T>(const Chord &chord, double range, double g, int opt_sector) {
    Chord result = chord;
    double sum = chord.layer();
    double sum_per_voice = sum / chord.voices();
    result = result.T(-sum_per_voice);
    return result;
}

inline Chord Chord::eT() const {
    return csound::equate<EQUIVALENCE_RELATION_T>(*this, OCTAVE(), 1.0, 0);
}

//	EQUIVALENCE_RELATION_Tg

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_Tg>(const Chord &chord, double range, double g, int opt_sector) {
    auto sum = chord.layer();
    auto t = chord.eT();
    auto t_ceiling = t.ceiling();
    while (lt_tolerance(t_ceiling.layer(), 0.) == true) {
        t_ceiling = t_ceiling.T(g);
    }
    auto tt_sum = t_ceiling.sum();
    if (eq_tolerance(sum, tt_sum) == true) {
        return true;
    } else {
        return false;
    }
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_Tg>(const Chord &chord, double range, double g, int opt_sector) {
    auto self_t = chord.eT();
    auto self_t_ceiling = self_t.ceiling();
    while (lt_tolerance(self_t_ceiling.layer(), 0.) == true) {
        self_t_ceiling = self_t_ceiling.T(g);
    }
    return self_t_ceiling;
}

inline Chord Chord::eTT(double g) const {
    return csound::equate<EQUIVALENCE_RELATION_Tg>(*this, OCTAVE(), g, 0);
}

inline bool Chord::iseTT(double g) const {
    return predicate<EQUIVALENCE_RELATION_Tg>(*this, OCTAVE(), g, 0);
}

//	EQUIVALENCE_RELATION_I

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_I>(const Chord &chord, double range, double g, int opt_sector) {
    // Chords that are inversionally equivalent automatically are normal.
    CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_I>: %s opt_sector: %d\n", chord.toString().c_str(), opt_sector);
    if (chord.self_inverse(opt_sector) == true) {
        CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_I>: %s returning self_inverse: %d\n", chord.toString().c_str(), true);
        return true;
    }
    // Otherwise, if the chord is in a "minor" OPTI sector in the current OPT sector,
    // the chord is normal.
    int minor_opti_sector = opt_sector * 2;
    CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_I>: %s minor_opti_sector: %d\n", chord.toString().c_str(), minor_opti_sector);
    if (chord.is_opti_sector(minor_opti_sector) == true) {
        CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_I>: %s is in minor opti sector: %d\n", chord.toString().c_str(), true);
       return true;
    } else {
        CHORD_SPACE_DEBUG("predicate<EQUIVALENCE_RELATION_I>: %s is in major opti sector: %d\n", chord.toString().c_str(), false);
        return false;
    }
}

inline bool Chord::iseI_chord(Chord *inverse, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_I>(*this, OCTAVE(), 1.0, opt_sector);
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_I>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_I>(chord, range, g, opt_sector) == true) {
        return chord;
    } else {
        return reflect_in_inversion_flat(chord, opt_sector);
    }
}

inline Chord Chord::eI(int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_I>(*this, OCTAVE(), 1.0, opt_sector);
}


//  EQUIVALENCE_RELATION_RP

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RP>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    return true;
}

inline bool Chord::iseRP(double range) const {
    return predicate<EQUIVALENCE_RELATION_RP>(*this, range, 1.0, 0);
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RP>(const Chord &chord, double range, double g, int opt_sector) {
    Chord normal = equate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector);
    normal = equate<EQUIVALENCE_RELATION_P>(normal, range, g, opt_sector);
    return normal;
}

inline Chord Chord::eRP(double range) const {
    return csound::equate<EQUIVALENCE_RELATION_RP>(*this, range, 1.0, 0);
}

//  EQUIVALENCE_RELATION_RT

//	EQUIVALENCE_RELATION_RI

//	EQUIVALENCE_RELATION_PT

//	EQUIVALENCE_RELATION_PTg

//	EQUIVALENCE_RELATION_PI

//	EQUIVALENCE_RELATION_TI

//	EQUIVALENCE_RELATION_RPT

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPT>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (chord.is_opt_sector(opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_T>(chord, range, g, opt_sector) == false) {
        return false;
    }
    return true;
}

inline bool Chord::iseRPT(double range, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPT>(*this, range, 1.0, opt_sector);
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPT>(const Chord &chord, double range, double g, int opt_sector) {
    auto rpts = chord.eRPTs();
    for (auto rpt : rpts) {
        if (rpt.is_opt_sector(opt_sector) == true) {
            return rpt;
        }
    }
    System::error("Error: Chord equate<EQUIVALENCE_RELATION_RPT>: no RPT in sector %d.\n", opt_sector);
    ///CHORD_SPACE_DEBUGGING() = true;
    ///std::raise(SIGINT);
    for (auto rpt : rpts) {
        System::message("equate<EQUIVALENCE_RELATION_RPT>: chord %s rpt: %s opt_sector: %d\n", print_chord(chord), print_chord(rpt), opt_sector);
        if (rpt.is_opt_sector(opt_sector) == true) {
            return rpt;
        }
    }
    return rpts.front();
}

inline Chord Chord::eRPT(double range, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPT>(*this, range, 1.0, opt_sector);
}

inline std::vector<Chord> Chord::eRPTs(double range) const {
    std::vector<Chord> rpts;
    auto rp = eRP(range);
    auto rp_vs = rp.voicings();
    for (auto rp_v : rp_vs) {
        auto rp_v_t = rp_v.eT();
        rpts.push_back(rp_v_t);
    }
    return rpts;
}

//	EQUIVALENCE_RELATION_RPTg

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTg>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (chord.is_opt_sector(opt_sector) == false) {
    //~ if (chord.iseRPT(range, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_Tg>(chord, range, g, opt_sector) == false) {
        return false;
    }
    return true;
}

inline bool Chord::iseRPTT(double range, double g, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPTg>(*this, range, g, opt_sector);
}
// TODO: Fix this with eRPTs instead of eRPTTs?

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTg>(const Chord &chord, double range, double g, int opt_sector) {
    auto rptts = chord.eRPTTs(range, g);
    for (auto &rptt : rptts) {
        if (rptt.is_opt_sector(opt_sector) == true) {
            return rptt;
        }
    }
    System::error("Error: Chord equate<EQUIVALENCE_RELATION_RPTg>: no RPTg in sector %d.\n", opt_sector);
    ///CHORD_SPACE_DEBUGGING() = true;
    ///std::raise(SIGINT);
    for (auto rptt : rptts) {
        System::inform("equate<EQUIVALENCE_RELATION_RPTg: chord %s rptt: %s opt_sector: %d\n", print_chord(chord), print_chord(rptt), opt_sector);
        if (rptt.is_opt_sector(opt_sector) == true) {
            return rptt;
        }
    }
    return rptts.front();
}

inline Chord Chord::eRPTT(double range, double g, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPTg>(*this, range, g, opt_sector);
}

inline std::vector<Chord> Chord::eRPTTs(double range, double g) const {
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

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPI>(const Chord &chord, double range, double g, int opt_sector) {
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

inline bool Chord::iseRPI(double range, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPI>(*this, range, 1.0, opt_sector);
}

// TODO: Verify.

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPI>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_RPI>(chord, range, g, opt_sector) == true) {
        return chord;
    }
    return chord.eI(opt_sector).eRP(range);
}

inline Chord Chord::eRPI(double range, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPI>(*this, range, 1.0, opt_sector);
}

//	EQUIVALENCE_RELATION_RTI

//	EQUIVALENCE_RELATION_RTgI

//	EQUIVALENCE_RELATION_RPTI

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (chord.is_opt_sector(opt_sector) == false) {
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

inline bool Chord::iseRPTI(double range, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPTI>(*this, range, 1.0, opt_sector);
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord, double range, double g, int opt_sector) {
    auto rpt = equate<EQUIVALENCE_RELATION_RPT>(chord, range, g, opt_sector);
    if (predicate<EQUIVALENCE_RELATION_I>(rpt, range, g, opt_sector) == true) {
        return rpt;
    } else {
        auto rpt_i = equate<EQUIVALENCE_RELATION_I>(rpt, range, g, opt_sector);
        auto rpt_i_rpt = equate<EQUIVALENCE_RELATION_RPT>(rpt_i, range, g, opt_sector);
        return rpt_i_rpt;
    }
}

inline Chord Chord::eRPTI(double range, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPTI>(*this, range, 1.0, opt_sector);
}

//	EQUIVALENCE_RELATION_RPTgI

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTgI>(const Chord &chord, double range, double g, int opt_sector) {
    if (predicate<EQUIVALENCE_RELATION_R>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_P>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (chord.is_opt_sector(opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_Tg>(chord, range, g, opt_sector) == false) {
        return false;
    }
    if (predicate<EQUIVALENCE_RELATION_I>(chord, range, g, opt_sector) == false) {
        return false;
    }
    return true;
}

inline bool Chord::iseRPTTI(double range, double g, int opt_sector) const {
    return predicate<EQUIVALENCE_RELATION_RPTgI>(*this, range, g, opt_sector);
}

template<> inline SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTgI>(const Chord &chord, double range, double g, int opt_sector) {
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

inline Chord Chord::eRPTTI(double range, double g, int opt_sector) const {
    return csound::equate<EQUIVALENCE_RELATION_RPTgI>(*this, range, 1.0, opt_sector);
}

inline SILENCE_PUBLIC const std::map<std::string, double> &pitchClassesForNames() {
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

inline SILENCE_PUBLIC double pitchClassForName(std::string name) {
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
inline SILENCE_PUBLIC std::string nameForPitchClass(double pitch) {
    auto pc = epc(pitch);
    const std::map<std::string, double> &pitchClassesForNames_ = pitchClassesForNames();
    for (auto it = pitchClassesForNames_.begin(); it != pitchClassesForNames_.end(); ++it) {
        if (eq_tolerance(it->second, pc) == true) {
            return it->first;
        }
    }
    return "";
}

inline SILENCE_PUBLIC std::multimap<Chord, std::string> &namesForChords() {
    static std::multimap<Chord, std::string> namesForChords_;
    return namesForChords_;
}

inline SILENCE_PUBLIC std::map<std::string, Chord> &chordsForNames() {
    static std::map<std::string, Chord> chordsForNames_;
    return chordsForNames_;
}

inline SILENCE_PUBLIC std::multimap<Scale, std::string> &namesForScales() {
    static std::multimap<Scale, std::string> namesForScales_;
    return namesForScales_;
}

inline SILENCE_PUBLIC std::map<std::string, Scale> &scalesForNames() {
    static std::map<std::string, Scale> scalesForNames_;
    return scalesForNames_;
}

inline SILENCE_PUBLIC std::set<Chord> &unique_chords() {
    static std::set<Chord> unique_chords_;
    return unique_chords_;
}

inline SILENCE_PUBLIC std::set<Scale> &unique_scales() {
    static std::set<Scale> unique_scales_;
    return unique_scales_;
}

inline SILENCE_PUBLIC void add_chord(std::string name, const Chord &chord) {
    unique_chords().insert(chord);
    chordsForNames().insert(std::make_pair(name, chord));
    namesForChords().insert(std::make_pair(chord, name));
}

inline SILENCE_PUBLIC void add_scale(std::string name, const Scale &scale) {
    unique_scales().insert(scale);
    scalesForNames().insert(std::make_pair(name, scale));
    namesForScales().insert(std::make_pair(scale, name));
}

inline SILENCE_PUBLIC std::vector<std::string> split(std::string string_) {
    std::vector<std::string> tokens;
    std::istringstream iss(string_);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter<std::vector<std::string> >(tokens));
    return tokens;
}

inline void fill(std::string rootName, double rootPitch, std::string typeName, std::string typePitches, bool is_scale) {
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

inline void initializeNames() {
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
inline SILENCE_PUBLIC std::vector<std::string> namesForChord(const Chord &chord) {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initializeNames();
    }
    std::multimap<Chord, std::string> &namesForChords_ = namesForChords();
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
inline SILENCE_PUBLIC std::string nameForChord(const Chord &chord) {
    auto result = namesForChord(chord);
    if (result.size() == 0) {
        return "";
    } else {
        return result[0];
    }
}

inline SILENCE_PUBLIC const Chord &chordForName(std::string name) {
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
inline SILENCE_PUBLIC std::vector<std::string> namesForScale(const Scale &scale) {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        initializeNames();
    }
    std::multimap<Scale, std::string> &namesForScales_ = namesForScales();
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
inline SILENCE_PUBLIC std::string nameForScale(const Scale &scale) {
    auto result = namesForScale(scale);
    if (result.size() == 0) {
        return "";
    } else {
        return result[0];
    }
}

inline SILENCE_PUBLIC const Scale &scaleForName(std::string name) {
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

static std::string print_opti_sectors(const Chord &chord) {
    std::string result;
    char buffer[0x1000];
    auto opti_sectors = chord.opti_domain_sectors();
    for (auto opti_sector : opti_sectors) {
        std::sprintf(buffer, "[opt:%2d  opti:%2d]", opti_sector / 2, opti_sector);
        result.append(buffer);
    }
    return result;    
}

inline const char *print_chord(const Chord &chord) {
    static char buffer[0x1000];
    std::sprintf(buffer, "%s   ", chord.toString().c_str());
    auto opti_sectors = chord.opti_domain_sectors();
    for (auto opti_sector : opti_sectors) {
        std::sprintf(buffer + std::strlen(buffer), "[OPT %2d  OPTI %2d]  ", opti_sector / 2, opti_sector);
    }
    return buffer;
}

inline std::string Chord::information_debug(int sector_) const {
    SCOPED_DEBUGGING debugging;
    return information_sector(sector_);
}

inline std::string Chord::information() const {
    return information_sector(-1);
}

inline std::string Chord::information_sector(int opt_sector_) const {
    std::string result;
    char buffer[0x4000];
    if (voices() < 1) {
        return "Empty chord.";
    }
    std::sprintf(buffer, "CHORD:\n");
    result.append(buffer);
    int opt_sector = 0;
    if (opt_sector_ == -1) {
        opt_sector = opt_domain_sectors().front();
    }
    std::sprintf(buffer, "%-19s %s\n", name().c_str(), print_chord(*this));
    result.append(buffer);
    std::sprintf(buffer, "Pitch-class set:    %s\n", epcs().eP().toString().c_str());
    result.append(buffer);
    std::sprintf(buffer, "Normal order:       %s\n", normal_order().toString().c_str());
    result.append(buffer);
    std::sprintf(buffer, "Normal form:        %s\n", normal_form().toString().c_str());
    result.append(buffer);
    std::sprintf(buffer, "Prime form:         %s\n", prime_form().toString().c_str());
    result.append(buffer);
    std::sprintf(buffer, "Inverse prime form: %s\n", inverse_prime_form().toString().c_str());
    result.append(buffer);
    std::sprintf(buffer, "Sum:                %12.7f\n", layer());
    result.append(buffer);
    std::sprintf(buffer, "O:           %3d => %s\n", iseO(), print_chord(eO()));
    result.append(buffer);
    std::sprintf(buffer, "P:           %3d => %s\n", iseP(), print_chord(eP()));
    result.append(buffer);
    std::sprintf(buffer, "T:           %3d => %s\n", iseT(), print_chord(eT()));
    result.append(buffer);
    std::sprintf(buffer, "TT:          %3d => %s\n", iseTT(), print_chord(eTT()));
    result.append(buffer);
    auto isei = iseI(opt_sector);
    auto ei = eI(opt_sector);
    std::sprintf(buffer, "I:           %3d => %s\n", isei, print_chord(ei));
    result.append(buffer);
    std::sprintf(buffer, "OP:          %3d => %s\n", iseOP(), print_chord(eOP()));
    result.append(buffer);
    std::sprintf(buffer, "OT:          %3d => %s\n", iseOT(), print_chord(eOT()));
    result.append(buffer);
    std::sprintf(buffer, "OTT:         %3d => %s\n", iseOTT(), print_chord(eOTT()));
    result.append(buffer);
    std::sprintf(buffer, "OPT:         %3d => %s\n", iseOPT(opt_sector), print_chord(eOPT(opt_sector)));
    result.append(buffer);
    std::sprintf(buffer, "OPTT:        %3d => %s\n", iseOPTT(opt_sector), print_chord(eOPTT(opt_sector)));
    result.append(buffer);
    std::sprintf(buffer, "OPI:         %3d => %s\n", iseOPI(opt_sector), print_chord(eOPI(opt_sector)));
    result.append(buffer);
    std::sprintf(buffer, "OPTI:        %3d => %s\n", iseOPTI(opt_sector), print_chord(eOPTI(opt_sector)));
    result.append(buffer);
    std::sprintf(buffer, "OPTTI:       %3d => %s\n", iseOPTTI(opt_sector), print_chord(eOPTTI(opt_sector)));
    result.append(buffer);
    std::sprintf(buffer, "               OPT sectors:\n");
    result.append(buffer);
    auto rpts = eRPTs(); 
    auto &hyperplane_equations = hyperplane_equations_for_opt_sectors()[voices()];
    for (auto i = 0; i < rpts.size(); ++i) {
        auto rpt = rpts[i];
        auto sector_text = print_opti_sectors(rpt);
        std::sprintf(buffer, "                    %s\n", print_chord(rpt));
        result.append(buffer);
    }
    std::sprintf(buffer, "               OPTT sectors:\n");
    result.append(buffer);
    auto rptts = eRPTTs(12.);
    for (auto i = 0; i < rptts.size(); ++i) {
        auto rptt = rptts[i];
        auto sector_text = print_opti_sectors(rptt);
        std::sprintf(buffer, "                    %s\n", print_chord(rptt));
        result.append(buffer);
    }
    std::sprintf(buffer, "               Inversion flats (vector equations) and corresponding reflections of this:\n");
    result.append(buffer);
    auto sectors = opt_domain_sectors();
    for (int i = 0, n = voices(); i < n; ++i) {
        auto &hyperplane_equation = hyperplane_equations[i];
        std::sprintf(buffer, "OPT[%2d]  Normal:  [", i);
        result.append(buffer);
        for (int j = 0, m = hyperplane_equation.unit_normal_vector.rows(); j < m; ++j) {
            std::sprintf(buffer, " %12.7f", hyperplane_equation.unit_normal_vector(j, 0));
            result.append(buffer);
        }
        auto reflected = reflect_in_inversion_flat(*this, i);
        std::sprintf(buffer, " ] Constant: %11.7f\n", hyperplane_equation.constant_term);
        result.append(buffer);
        auto sector_text = print_opti_sectors(reflected);
        std::sprintf(buffer, "         Reflection:%s\n", print_chord(reflected));
        result.append(buffer);   
    }    
    return result;
}

inline SILENCE_PUBLIC int indexForOctavewiseRevoicing(const Chord &origin, const Chord &chord, double range) {
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

inline SILENCE_PUBLIC int indexForOctavewiseRevoicing(const Chord &chord, double range) {
    const auto origin = csound::equate<EQUIVALENCE_RELATION_RP>(chord, OCTAVE(), 1.0, 0);
    auto result = indexForOctavewiseRevoicing(origin, chord, range);
    return result;
}

inline SILENCE_PUBLIC bool operator == (const Chord &a, const Chord &b) {
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

inline SILENCE_PUBLIC bool operator < (const Chord &a, const Chord &b) {
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

inline SILENCE_PUBLIC bool operator <= (const Chord &a, const Chord &b) {
    if (a == b) {
        return true;
    }
    return (a < b);
}

inline SILENCE_PUBLIC bool operator > (const Chord &a, const Chord &b) {
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

inline SILENCE_PUBLIC bool operator >= (const Chord &a, const Chord &b) {
    if (a == b) {
        return true;
    }
    return (a > b);
}

inline SILENCE_PUBLIC double C4() {
    return MIDDLE_C();
}

inline SILENCE_PUBLIC Chord chord(const Chord &scale, int scale_degree, int chord_voices, int interval) {
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

inline Chord::Chord() {
    initialize_sectors();
    resize(0);
}

inline Chord::Chord(int voices_) {
    resize(voices_);    
}

inline Chord::Chord(const Chord &other) {
    *this = other;
}

inline Chord::Chord(const std::vector<double> &other) {
    *this = other;
}

inline Chord &Chord::operator = (const Chord &other) {
    Matrix::operator=(other);
    return *this;
}

inline Chord &Chord::operator = (const std::vector<double> &other) {
    auto voices_n = other.size();
    resize(voices_n);
    for (size_t voice = 0; voice < voices_n; ++voice) {
        setPitch(voice, other[voice]);
    }
    return *this;
}

inline Chord::operator std::vector<double>() const {
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
inline Chord::~Chord() {
}

inline size_t Chord::voices() const {
    return rows();
}

inline void Chord::resize(size_t voiceN) {
    Matrix::resize(voiceN, COUNT);
}

inline bool Chord::test(const char *label) const {
    std::fprintf(stderr, "TESTING %s %s\n\n", toString().c_str(), label);
    bool passed = true;
    // For some of these we need to know the OPT sector, and if the chord 
    // belongs to more than one sector, we choose the first.
    auto opt_sector = opt_domain_sectors().front();
    // Test the consistency of the predicates.
    if (iseOP() == true) {
        if (iseO() == false ||
            iseP() == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOP is not consistent.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOP is consistent.\n");
        }
    }
    if (iseOPT(opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseT() == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPT is not consistent.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPT is consistent.\n");
        }
    }
    // If it is transformed to T, is it OPT? 
    // After that, is it Tg?
    if (iseOPTT(opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseTT() == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPTT is not consistent.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPTT is consistent.\n");
        }
    }
    if (iseOPTI(opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseT() == false || 
            iseI(opt_sector) == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPTI is not consistent.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPTI is consistent.\n");
        }
    }
    if (iseOPTTI(opt_sector) == true) {
        if (iseO() == false ||
            iseP() == false || 
            iseTT() == false || 
            iseI(opt_sector) == false) {
            passed = false;
            std::fprintf(stderr, "Failed: Chord::iseOPTTI is not consistent.\n");
        } else {
            std::fprintf(stderr, "        Chord::iseOPTTI is consistent.\n");
        }
    }
    // Test the consistency of the transformations.
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
    if (eTT().iseTT() == false) {
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
    if (eOPT(opt_sector).is_opt_sector(opt_sector) == false) {
        passed = false;
        auto opt_chord = eOPT(opt_sector);
        std::fprintf(stderr, "Failed: Chord::eOPT is not consistent with Chord::iseOPT (%s => %s).\n", toString().c_str(), opt_chord.toString().c_str());
    } else {
        std::fprintf(stderr, "        Chord::eOPT is consistent with Chord::iseOPT.\n");
    }
    if (eOPTT(opt_sector).iseOPTT(opt_sector) == false) {
        passed = false;
        auto optt_chord = eOPTT(opt_sector);
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
    auto optti_chord = eOPTTI(opt_sector);
    if (optti_chord.iseOPTTI(opt_sector) == false) {
        passed = false;
        std::fprintf(stderr, "Failed: Chord::eOPTTI is not consistent with Chord::iseOPTTI  (%s => %s).\n", toString().c_str(), optti_chord.toString().c_str());
    } else {
        std::fprintf(stderr, "        Chord::eOPTTI is consistent with Chord::iseOPTTI.\n");
    }
    std::fprintf(stderr, "\n");
    std::fprintf(stderr, "%s", information().c_str());
    return passed;
}

/**
 * Returns a string representation of the chord's pitches (only).
 * Quadratic complexity, but short enough not to matter.
 */
inline std::string Chord::toString() const {
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
inline void Chord::fromString(std::string text) {
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

inline double Chord::getPitch(int voice) const {
    return coeff(voice, PITCH);
}

inline double &Chord::getPitchReference(int voice) {
    return coeffRef(voice, PITCH);
}

inline void Chord::setPitch(int voice, double value) {
    coeffRef(voice, PITCH) = value;
}

inline double Chord::getDuration(int voice) const {
    return coeff(voice, DURATION);
}

inline void Chord::setDuration(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, DURATION) = value;
        }
    } else {
        coeffRef(voice, DURATION) = value;
    }
}

inline double Chord::getLoudness(int voice) const {
    return coeff(voice, LOUDNESS);
}

inline void Chord::setLoudness(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, LOUDNESS) = value;
        }
    } else {
        coeffRef(voice, LOUDNESS) = value;
    }
}

inline double Chord::getInstrument(int voice) const {
    return coeff(voice, INSTRUMENT);
}

inline void Chord::setInstrument(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, INSTRUMENT) = value;
        }
    } else {
        coeffRef(voice, INSTRUMENT) = value;
    }
}

inline double Chord::getPan(int voice) const {
    return coeff(voice, PAN);
}

inline void Chord::setPan(double value, int voice) {
    if (voice == -1) {
        for (voice = 0; voice < rows(); ++voice) {
            coeffRef(voice, PAN) = value;
        }
    } else {
        coeffRef(voice, PAN) = value;
    }
}

inline size_t Chord::count(double pitch) const {
    size_t n = 0;
    for (size_t voice = 0; voice < voices(); ++voice) {
        if (eq_tolerance(getPitch(voice), pitch)) {
            n++;
        }
    }
    return n;
}

inline bool Chord::contains(double pitch_) const {
    for (size_t voice = 0; voice < voices(); voice++) {
        if (eq_tolerance(getPitch(voice), pitch_)) {
            return true;
        }
    }
    return false;
}

inline std::vector<double> Chord::min() const {
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

inline std::vector<double> Chord::max() const {
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

inline double Chord::minimumInterval() const {
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

inline double Chord::maximumInterval() const {
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

inline Chord Chord::floor() const {
    Chord clone = *this;
    for (size_t voice = 0; voice  < voices(); voice++) {
        clone.setPitch(voice, std::floor(getPitch(voice)));
    }
    return clone;
}

inline Chord Chord::ceiling(double g) const {
    Chord result = *this;
    for (size_t voice = 0; voice  < voices(); voice++) {
        result.setPitch(voice, std::ceil(getPitch(voice)));
    }
    CHORD_SPACE_DEBUG("Chord::ceiling: %s\n", print_chord(*this));
    CHORD_SPACE_DEBUG("             => %s\n", print_chord(result));
    return result;
}

inline Chord Chord::origin() const {
    Chord origin_;
    origin_.resize(voices());
    return origin_;
}

inline double Chord::distanceToOrigin() const {
    Chord origin_ = origin();
    return euclidean(*this, origin_);
}

inline double Chord::layer() const {
    double sum = 0.0;
    for (size_t voice = 0; voice < voices(); ++voice) {
        sum += getPitch(voice);
    }
    return sum;
}

inline double Chord::distanceToUnisonDiagonal() const {
    Chord unison = origin();
    double pitch = layer() / double(voices());
    for (size_t voice = 0; voice < voices(); voice ++) {
        unison.setPitch(voice, pitch);
    }
    return euclidean(*this, unison);
}

inline Chord Chord::center() const {
    Chord clone = *this;
    double even_ = OCTAVE() / voices();
    for (size_t voice = 0; voice < voices(); voice++) {
        clone.setPitch(voice, voice * even_);
    }
    return clone;
}

inline Chord Chord::T(double interval) const {
    Chord clone = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        clone.setPitch(voice, csound::T(getPitch(voice), interval));
    }
    return clone;
}

inline Chord Chord::T_voiceleading(const Chord &voiceleading) {
    Chord clone = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        clone.setPitch(voice, getPitch(voice) + voiceleading.getPitch(voice));
    }
    return clone;
}

inline Chord Chord::voiceleading(const Chord &destination) const {
    Chord voiceleading_ = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        voiceleading_.setPitch(voice, destination.getPitch(voice) - getPitch(voice));
    }
    return voiceleading_;
}

inline Chord Chord::I(double center) const {
    Chord inverse = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        inverse.setPitch(voice, csound::I(getPitch(voice), center));
    }
    return inverse;
}

inline Chord Chord::cycle(int stride) const {
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

inline std::vector<Chord> Chord::permutations() const {
    std::vector<Chord> permutations_;
    Chord permutation = *this;
    permutations_.push_back(permutation);
    for (size_t i = 1; i < voices(); i++) {
        permutation = permutation.cycle();
        permutations_.push_back(permutation);
    }
    std::sort(permutations_.begin(), permutations_.end());
    return permutations_;
}

inline Chord Chord::v(int direction) const {
    Chord chord = *this;
    int head = voices() - 1;
    while (direction > 0) {
        chord = chord.cycle(1);
        chord.setPitch(head, chord.getPitch(head) + OCTAVE());
        direction--;
    }
    while (direction < 0) {
        chord = chord.cycle(-1);
        chord.setPitch(0, chord.getPitch(0) + OCTAVE());
        direction++;
    }
    return chord;
}

inline std::vector<Chord> Chord::voicings() const {
    Chord chord = *this;
    std::vector<Chord> voicings;
    voicings.push_back(chord);
    for (size_t voicing = 1; voicing < voices(); voicing++) {
        chord = chord.v();
        voicings.push_back(chord);
    }
    return voicings;
}

inline bool Chord::isepcs() const {
    for (size_t voice = 0; voice < voices(); voice++) {
        if (!eq_tolerance(getPitch(voice), epc(getPitch(voice)))) {
            return false;
        }
    }
    return true;
}

inline Chord Chord::epcs() const {
    Chord chord = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        chord.setPitch(voice, epc(getPitch(voice)));
    }
    return chord;
}

inline Chord Chord::eppcs() const {
    Chord chord = *this;
    for (size_t voice = 0; voice < voices(); voice++) {
        chord.setPitch(voice, epc(getPitch(voice)));
    }
    return chord.eP();
}

inline bool Chord::iset() const {
    Chord et_ = et();
    if (!(*this == et_)) {
        return false;
    }
    return true;
}

inline Chord Chord::et() const {
    double min_ = min()[0];
    return T(-min_);
}

inline bool Chord::iseO() const {
    return iseR(OCTAVE());
}

inline Chord Chord::eO() const {
    return eR(OCTAVE());
}

inline bool Chord::iseI(int opt_sector) const {
    return iseI_chord(nullptr, opt_sector);
}

inline bool Chord::iseOP() const {
    return iseRP(OCTAVE());
}

inline Chord Chord::eOP() const {
    return eRP(OCTAVE());
}

inline bool Chord::iseOPT(int opt_sector) const {
    return iseRPT(OCTAVE(), opt_sector);
}

inline bool Chord::iseOPTT(double g, int opt_sector) const {
    return iseRPTT(OCTAVE(), g, opt_sector);
}

inline Chord Chord::eOPT(int opt_sector) const {
    return eRPT(OCTAVE(), opt_sector);
}

inline Chord Chord::eOPTT(double g, int opt_sector) const {
    return eRPTT(OCTAVE(), g, opt_sector);
}

inline bool Chord::iseOPI(int opt_sector) const {
    return iseRPI(OCTAVE(), opt_sector);
}

inline Chord Chord::eOPI(int opt_sector) const {
    return eRPI(OCTAVE(), opt_sector);
}

inline bool Chord::iseOPTI(int opt_sector) const {
    return iseRPTI(OCTAVE(), opt_sector);
}

inline bool Chord::iseOPTTI(double g, int opt_sector) const {
    return iseRPTTI(OCTAVE(), g, opt_sector);
}

inline Chord Chord::eOPTI(int opt_sector) const {
    return eRPTI(OCTAVE(), opt_sector);
}

inline Chord Chord::eOPTTI(double g, int opt_sector) const {
    return eRPTTI(OCTAVE(), g, opt_sector);
}

inline std::string Chord::name() const {
    std::string name_ = nameForChord(*this);
    return name_;
}

inline Chord Chord::move(int voice, double interval) const {
    Chord chord = *this;
    chord.setPitch(voice, csound::T(getPitch(voice), interval));
    return chord;
}

inline Chord Chord::nrR() const {
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

inline Chord Chord::nrP() const {
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

inline Chord Chord::nrL() const {
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

inline Chord Chord::nrN() const {
    return nrR().nrL().nrP();
}

inline Chord Chord::nrS() const {
    return nrL().nrP().nrR();
}

inline Chord Chord::nrH() const {
    return nrL().nrP().nrL();
}

inline Chord Chord::nrD() const {
    return T(-7.0). eOP();
}

inline Chord Chord::K() const {
    Chord chord = *this;
    if (chord.voices() < 2) {
        return chord;
    }
    // Unordered and in [0, 12).
    Chord epc = epcs();
    double center = epc.getPitch(0) + epc.getPitch(1);
    return I(center);
}

inline Chord Chord::K_range(double range) const {
    Chord chord = K();
    return chord.eRP(range);
}

inline bool Chord::Tform(const Chord &Y, double g) const {
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

inline bool Chord::Iform(const Chord &Y, double g) const {
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

inline Chord Chord::Q(double x, const Chord &m, double g) const {
    if (Tform(m, g)) {
        return T(x);
    }
    if (Iform(m, g)) {
        return T(-x);
    }
    return *this;
}

inline Chord Chord::a(int arpeggiation, double &resultPitch, int &resultVoice) const {
    Chord resultChord = v(arpeggiation);
    if (arpeggiation < 0) {
        resultVoice = resultChord.voices() - 1;
    } else {
        resultVoice = 0;
    }
    resultPitch = resultChord.getPitch(resultVoice);
    return resultChord;
}

inline bool Chord::equals(const Chord &other) const {
    return *this == other;
}

inline bool Chord::greater(const Chord &other) const {
    return *this > other;
}

inline bool Chord::greater_equals(const Chord &other) const {
    return *this >= other;
}

inline bool Chord::lesser(const Chord &other) const {
    return *this < other;
}

inline bool Chord::lesser_equals(const Chord &other) const {
    return *this <= other;
}

inline SILENCE_PUBLIC double closestPitch(double pitch, const Chord &chord) {
    std::map<double, double> pitchesForDistances;
    for (int voice = 0; voice < chord.voices(); ++voice) {
        double chordPitch = chord.getPitch(voice);
        double distance = std::fabs(chordPitch - pitch);
        pitchesForDistances[distance] = chordPitch;
    }
    return pitchesForDistances.begin()->second;
}

inline SILENCE_PUBLIC double conformToPitchClassSet(double pitch, const Chord &pcs) {
    double pc_ = epc(pitch);
    double closestPc = closestPitch(pc_, pcs);
    double register_ = std::floor(pitch / OCTAVE()) * OCTAVE();
    double closestPitch = register_ + closestPc;
    return closestPitch;
}

inline SILENCE_PUBLIC double epc(double pitch) {
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

inline SILENCE_PUBLIC bool eq_tolerance(double a, double b, int epsilons, int ulps) {
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

inline SILENCE_PUBLIC double euclidean(const Chord &a, const Chord &b) {
    double sumOfSquaredDifferences = 0.0;
    const size_t voices = a.voices();
    for (size_t voice = 0; voice < voices; ++voice) {
        sumOfSquaredDifferences += std::pow((a.getPitch(voice) - b.getPitch(voice)), 2.0);
    }
    return std::sqrt(sumOfSquaredDifferences);
}

inline SILENCE_PUBLIC double factorial(double n) {
    double result = 1.0;
    for (int i = 0; i <= n; ++i) {
        result = result * i;
    }
    return result;
}

struct SILENCE_PUBLIC compare_by_normal_order {
    bool operator ()(const Chord &a, const Chord &b) const {
        if (a.normal_order() < b.normal_order()) {
            return true;
        } else {
            return false;
        }
    }
};

struct SILENCE_PUBLIC compare_by_op {
    bool operator ()(const Chord &a, const Chord &b) const {
        if (a.eOP() < b.eOP()) {
            return true;
        } else {
            return false;
        }
    }
};

struct SILENCE_PUBLIC compare_by_normal_form {
    bool operator ()(const Chord &a, const Chord &b) const {
        if (a.normal_form() < b.normal_form()) {
            return true;
        } else {
            return false;
        }
    }
};

template<int EQUIVALENCE_RELATION> inline SILENCE_PUBLIC std::vector<csound::Chord> fundamentalDomainByPredicate(int voiceN, double range, double g, int sector, bool printme)
{
    ///SCOPED_DEBUGGING debugging;
    auto name_ = namesForEquivalenceRelations[EQUIVALENCE_RELATION];
    System::message("fundamentalDomainByPredicate<%s>: voiceN: %d range: %f g: %f: sector: %d\n", name_, voiceN, range, g, sector);
    std::set<Chord, compare_by_normal_order> fundamentalDomainSet;
    std::vector<Chord> fundamentalDomainVector;
    int upperI = 3 * (range + 1);
    int lowerI = - (1 * (range + 1));
    Chord iterator_ = iterator(voiceN, lowerI);
    Chord origin = iterator_;
    int chords = 0;
    int normals = 0;
    //~ static auto target = csound::Chord({-3.0000000,   -1.0000000,    2.0000000,    5.0000000});
    //~ static bool target_found = false;
    while (next(iterator_, origin, upperI, g) == true) {
        chords++;
        bool iterator_is_normal = predicate<EQUIVALENCE_RELATION>(iterator_, range, g, sector);
        //~ if (target_found == false) {
            //~ if (target.toString() == iterator_.toString()) {
                //~ target_found = true;
                //~ System::message("fundamentalDomainByPredicate<%s>: iterator_is_normal: %d\n    found:  %s\n    target: %s\n\n", name_, iterator_is_normal, print_chord(target), print_chord(iterator_));
                //~ ///std::raise(SIGINT);
            //~ }
        //~ }
        if (iterator_is_normal == true) {
            normals++;
            fundamentalDomainVector.push_back(iterator_);
            auto result = fundamentalDomainSet.insert(iterator_);
            if (CHORD_SPACE_DEBUGGING() && result.second == true) {
                Chord normalized = equate<EQUIVALENCE_RELATION>(iterator_, range, g, sector);
                bool normalized_is_normal = predicate<EQUIVALENCE_RELATION>(normalized, range, g, 0);
                CHORD_SPACE_DEBUG("%s By predicate  %-8s: chord: %6d  domain: %6d  range: %7.2f  g: %7.2f  iterator: %s  predicate: %d  normalized: %s  predicate: %d\n",
                    (normalized_is_normal ? "      " : "WRONG "),
                    name_,
                    chords,
                    fundamentalDomainSet.size(),
                    range,
                    g,
                    iterator_.toString().c_str(),
                    iterator_is_normal,
                    normalized.toString().c_str(),
                    normalized_is_normal);
            }
        }
        if (printme == true) {
            System::message("fundamentalDomainByPredicate<%s>: %s normal: %6d set: %6d iterator: %12d %s\n", 
                name_,
                (iterator_is_normal ? "NORMAL " : "       "), 
                normals, 
                fundamentalDomainSet.size(), 
                chords, 
                print_chord(iterator_));
        }
    }
    std::sort(fundamentalDomainVector.begin(), fundamentalDomainVector.end());
    System::message("fundamentalDomainByPredicate<%s>: count: %d unique: %d\n", namesForEquivalenceRelations[EQUIVALENCE_RELATION], fundamentalDomainVector.size(), fundamentalDomainSet.size());
    //~ return std::vector<Chord>(fundamentalDomainSet.begin(), fundamentalDomainSet.end());
    return fundamentalDomainVector;
 }

template<int EQUIVALENCE_RELATION> inline SILENCE_PUBLIC std::vector<csound::Chord> fundamentalDomainByTransformation(int voiceN, double range, double g, int sector)
{
    std::set<Chord> fundamentalDomain;
    int upperI = 2 * (range + 1);
    int lowerI = - (range + 1);
    Chord iterator_ = iterator(voiceN, lowerI);
    Chord origin = iterator_;
    int chords = 0;
    while (next(iterator_, origin, upperI, g) == true) {
        chords++;
        CHORD_SPACE_DEBUG("fundamentalDomainByTransformation: %6d %s\n", chords, iterator_.toString().c_str());
        bool iterator_is_normal = predicate<EQUIVALENCE_RELATION>(iterator_, range, g, sector);
        CHORD_SPACE_DEBUG("fundamentalDomainByTransformation: %6d is_normal: %d\n", chords, iterator_is_normal);
        Chord normalized = equate<EQUIVALENCE_RELATION>(iterator_, range, g, sector);
        CHORD_SPACE_DEBUG("fundamentalDomainByTransformation: %6d normalized: %s\n", chords, normalized.toString().c_str());
        bool normalized_is_normal = predicate<EQUIVALENCE_RELATION>(normalized, range, g, sector);
        CHORD_SPACE_DEBUG("fundamentalDomainByTransformation: %6d normalized_is_normal: %d\n", chords, normalized_is_normal);
        auto result = fundamentalDomain.insert(normalized);
        if (CHORD_SPACE_DEBUGGING() && result.second == true) {
            CHORD_SPACE_DEBUG("%s By equate %-8s: chord: %6d  domain: %6d  range: %7.2f  g: %7.2f  iterator: %s  predicate: %d  normalized: %s  predicate: %d\n",
                (normalized_is_normal ? "      " : "WRONG "),
                namesForEquivalenceRelations[EQUIVALENCE_RELATION],
                chords,
                fundamentalDomain.size(),
                range,
                g,
                iterator_.toString().c_str(),
                iterator_is_normal,
                normalized.toString().c_str(),
                normalized_is_normal);
        }
    }
    std::vector<Chord> result(fundamentalDomain.begin(), fundamentalDomain.end());
    return result;
}

inline SILENCE_PUBLIC bool ge_tolerance(double a, double b, int epsilons, int ulps) {
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

inline SILENCE_PUBLIC bool gt_tolerance(double a, double b, int epsilons, int ulps) {
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

inline SILENCE_PUBLIC double I(double pitch, double center) {
    return 2. * center - pitch;
}

inline SILENCE_PUBLIC HyperplaneEquation hyperplane_equation_from_singular_value_decomposition(const std::vector<Chord> &points_, bool make_eT) {
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: original points:" << std::endl;
    std::vector<Chord> points;
    if (make_eT == true) {
        for (auto point : points_) {
            points.push_back(point.eT());
        }
    } else {
        points = points_;
    }
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: points:" << std::endl;
    auto opt = "";
    if (make_eT == true) {
        opt = "T: ";
    }
    for (auto point: points) {
        std::cerr << opt <<  point.col(0).transpose() << std::endl;
    }
    auto subtrahend = points.back().col(0);
    Matrix matrix(subtrahend.rows(), points.size() - 1);
    for (int i = 0, n = points.size() - 1; i < n; ++i) {
        Vector difference = points[i].col(0) - subtrahend;
        matrix.col(i) = difference;
    }
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: vectors:" << std::endl << matrix << std::endl;
    matrix.transposeInPlace();
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: vectors transposed:" << std::endl << matrix << std::endl;
    Eigen::JacobiSVD<Matrix, Eigen::FullPivHouseholderQRPreconditioner> svd(matrix, Eigen::ComputeFullU | Eigen::ComputeFullV);
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: U:" << std::endl << svd.matrixU() << std::endl;
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: singular values:" << std::endl << svd.singularValues() << std::endl;
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: V:" << std::endl << svd.matrixV() << std::endl;
    //~ auto rhs = Matrix::Zero(svd.singularValues().rows(), 1);
    //~ auto solution = svd.solve(rhs);
    //~ std::cerr << "solution:\n";
    //~ std::cerr << solution << std::endl;
    HyperplaneEquation hyperplane_equation_;
    hyperplane_equation_.unit_normal_vector = svd.matrixV().rightCols(1);
    auto norm = hyperplane_equation_.unit_normal_vector.norm();
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: norm:" << std::endl << norm << std::endl;
    hyperplane_equation_.unit_normal_vector = hyperplane_equation_.unit_normal_vector / norm;
    auto constant_term = hyperplane_equation_.unit_normal_vector.adjoint() * subtrahend;
    hyperplane_equation_.constant_term = constant_term(0, 0);
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: unit normal vector: " << std::endl << hyperplane_equation_.unit_normal_vector << std::endl;
    std::cerr << "hyperplane_equation_from_singular_value_decomposition: constant term: " << std::endl << hyperplane_equation_.constant_term << std::endl;
    return hyperplane_equation_;
}

/**
 * Returns the sum of the distances of the chord to each of one or more chords.
 */
inline SILENCE_PUBLIC double distance_to_points(const Chord &chord, const std::vector<Chord> &sector_vertices) {
    double sum = 0;
    for (auto vertex : sector_vertices) {
        auto distance = euclidean(chord, vertex);
        sum = sum + distance;
    }
    return sum;
}

inline SILENCE_PUBLIC HyperplaneEquation hyperplane_equation_from_random_inversion_flat(int dimensions, bool transpositional_equivalence, int opt_sector) {
    std::uniform_real_distribution<> uniform(-1., 1.);
    std::vector<Chord> inversion_flat;
    Chord center = Chord(dimensions).center();
    for (int i = 0; i < 100; ++i) {
        Chord chord(dimensions);
        if (i == 0) {
            inversion_flat.push_back(center);
        } else {
            int side = std::floor(dimensions / 2.);
            int lower_voice = 0;
            int upper_voice = dimensions - 1;
            for (lower_voice = 0; lower_voice < side; ++lower_voice, --upper_voice) {
                double random_pitch = uniform(mersenne_twister);
                chord.setPitch(lower_voice, -random_pitch);
                chord.setPitch(upper_voice,  random_pitch);
            }
            if (transpositional_equivalence == true) {
                chord = chord.eT().eP();
            } else {
                chord = chord.eP();
            }
            inversion_flat.push_back(chord);
        }
    }
    HyperplaneEquation hyperplane_equation_ = hyperplane_equation_from_singular_value_decomposition(inversion_flat, true);
    CHORD_SPACE_DEBUG("hyperplane_equation_from_random_inversion_flat: sector: %d\n", opt_sector);
    CHORD_SPACE_DEBUG("hyperplane_equation_from_random_inversion_flat: center:\n");
    for(int i = 0; i < dimensions; i++) {
        CHORD_SPACE_DEBUG("  %9.4f\n", center.getPitch(i));
    }
    CHORD_SPACE_DEBUG("hyperplane_equation_from_random_inversion_flat: unit_normal_vector:\n");
    for(int i = 0; i < dimensions; i++) {
        CHORD_SPACE_DEBUG("  %9.4f\n", hyperplane_equation_.unit_normal_vector(i, 0));
    }
    CHORD_SPACE_DEBUG("hyperplane_equation_from_random_inversion_flat: constant_term: %9.4f\n", hyperplane_equation_.constant_term);
    return hyperplane_equation_;
}

template<int EQUIVALENCE_RELATION> inline SILENCE_PUBLIC bool predicate(const Chord &chord, double range, int sector) {
    bool result = predicate<EQUIVALENCE_RELATION>(chord, range, 1.0);
    return result;
}

template<int EQUIVALENCE_RELATION> inline SILENCE_PUBLIC bool predicate(const Chord &chord, double range) {
    bool result = predicate<EQUIVALENCE_RELATION>(chord, range, 1.0);
    return result;
}

template<int EQUIVALENCE_RELATION> inline SILENCE_PUBLIC bool predicate(const Chord &chord) {
    bool result = predicate<EQUIVALENCE_RELATION>(chord, OCTAVE());
    return result;
}

template<> inline SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_r>(const Chord &chord, double range, double g, int opt_sector) {
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

inline SILENCE_PUBLIC Chord iterator(int voiceN, double first) {
    Chord odometer;
    odometer.resize(voiceN);
    for (int voice = 0; voice < voiceN; ++voice) {
        odometer.setPitch(voice, first);
    }
    return odometer;
}

inline SILENCE_PUBLIC bool le_tolerance(double a, double b, int epsilons, int ulps) {
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

inline SILENCE_PUBLIC bool lt_tolerance(double a, double b, int epsilons, int ulps) {
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

inline SILENCE_PUBLIC Chord midpoint(const Chord &a, const Chord &b) {
    Chord midpoint_ = a;
    for (int voice = 0, voices = a.voices(); voice < voices; ++voice) {
        double voiceSum = a.getPitch(voice) + b.getPitch(voice);
        double voiceMidpoint = voiceSum / 2.0;
        midpoint_.setPitch(voice, voiceMidpoint);
    }
    /// CHORD_SPACE_DEBUG("a: %s  b: %s  mid: %s\n", a.toString().c_str(), b.toString().c_str(), midpoint_.toString().c_str());
    return midpoint_;
}

inline SILENCE_PUBLIC double MIDDLE_C() {
    return 60.0;
}

inline SILENCE_PUBLIC double modulo(double dividend, double divisor) {
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

inline SILENCE_PUBLIC bool next(Chord &iterator_, const Chord &origin, double range_, double increment) {
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

template<int EQUIVALENCE_RELATION> inline SILENCE_PUBLIC Chord equate(const Chord &chord, double range) {
    return equate<EQUIVALENCE_RELATION>(chord, range, 1.0);
}

template<int EQUIVALENCE_RELATION> inline SILENCE_PUBLIC Chord equate(const Chord &chord) {
    return equate<EQUIVALENCE_RELATION>(chord, OCTAVE());
}

inline SILENCE_PUBLIC double OCTAVE() {
    return 12.0;
}

inline SILENCE_PUBLIC Chord octavewiseRevoicing(const Chord &chord, int revoicingNumber_, double range) {
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

inline SILENCE_PUBLIC int octavewiseRevoicings(const Chord &chord,
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

inline SILENCE_PUBLIC bool parallelFifth(const Chord &a, const Chord &b) {
    Chord voiceleading_ = voiceleading(a, b);
    if (voiceleading_.count(7) > 1) {
        return true;
    } else {
        return false;
    }
}

inline SILENCE_PUBLIC Vector reflect_vector(const Vector &v, const Vector &u, double c) {
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

inline SILENCE_PUBLIC Vector reflect_vectorx(const Vector &v, const Vector &u, double c) {
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
inline SILENCE_PUBLIC Chord reflect_by_householder(const Chord &chord) {
    auto opt_domain_sectors_ = chord.opt_domain_sectors().front();
    auto hyperplane_equation = chord.hyperplane_equation(opt_domain_sectors_);
    CHORD_SPACE_DEBUG("reflect_by_householder: chord:              %s\n", chord.toString().c_str());
    CHORD_SPACE_DEBUG("reflect_by_householder: unit normal vector: \n%s\n", toString(hyperplane_equation.unit_normal_vector).c_str());
    auto center_ = chord.center().eT();
    CHORD_SPACE_DEBUG("reflect_by_householder: center:             %s\n", center_.toString().c_str());
    auto tensor = hyperplane_equation.unit_normal_vector.col(0) * hyperplane_equation.unit_normal_vector.col(0).transpose();
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

inline SILENCE_PUBLIC Chord reflect_in_central_diagonal(const Chord &chord) {
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

inline SILENCE_PUBLIC Chord reflect_in_central_point(const Chord &chord) {
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

inline SILENCE_PUBLIC Chord reflect_in_unison_diagonal(const Chord &chord) {
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


inline SILENCE_PUBLIC Chord reflect_in_inversion_flat(const Chord &chord, int opt_sector) {
    // Preserve non-pitch data in the chord.
    Chord result = chord;
    int dimensions = chord.voices();
    HyperplaneEquation hyperplane = chord.hyperplane_equation(opt_sector);
    auto reflected = reflect_vector(chord.col(0), hyperplane.unit_normal_vector, hyperplane.constant_term);
    for (int voice = 0; voice < dimensions; ++voice) {
        result.setPitch(voice, reflected(voice, 0));
    }
    return result;
}

inline SILENCE_PUBLIC Chord scale(std::string name) {
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

inline SILENCE_PUBLIC Scale::Scale() {
    resize(0);
}

inline SILENCE_PUBLIC Scale::Scale(std::string name) {
    const Chord temporary = csound::scale(name);
    Matrix::operator=(temporary);
    if (temporary.voices() > 0) {
        auto space_at = name.find(' ');
        type_name = name.substr(space_at + 1);
    }
}

inline SILENCE_PUBLIC Scale::Scale(std::string name, const Chord &scale_pitches) {
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

inline SILENCE_PUBLIC Scale::Scale(std::string name, const std::vector<double> &scale_pitches) {
    resize(scale_pitches.size());
    for (int index = 0; index < voices(); ++index) {
        setPitch(index, scale_pitches[index]);
    }
    add_scale(name, *this);
}

inline SILENCE_PUBLIC Scale::~Scale() {};

inline SILENCE_PUBLIC Scale &Scale::operator = (const Scale &other) {
   Matrix::operator=(dynamic_cast<const Chord &>(other));
    type_name = other.getTypeName();
    return *this;
}

inline SILENCE_PUBLIC Chord Scale::chord(int scale_degree, int voices, int interval) const {
    return csound::chord(*this, scale_degree, voices, interval);
}

inline SILENCE_PUBLIC Chord Scale::transpose_degrees(const Chord &chord, int scale_degrees, int interval) const {
    return csound::transpose_degrees(*this, chord, scale_degrees, interval);
}

inline SILENCE_PUBLIC double Scale::semitones_for_degree(int scale_degree) const {
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

inline SILENCE_PUBLIC Scale Scale::transpose_to_degree(int degrees) const {
    CHORD_SPACE_DEBUG("Scale::transpose_to_degree(%9.4f)...\n", degrees);
    double semitones = semitones_for_degree(degrees);
    return transpose(semitones);
}

inline SILENCE_PUBLIC Scale Scale::transpose(double semitones) const {
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

inline SILENCE_PUBLIC std::string Scale::name() const {
    return nameForPitchClass(tonic()) + " " + type_name;
}

inline SILENCE_PUBLIC std::string Scale::getTypeName() const {
    return type_name;
}

inline SILENCE_PUBLIC double Scale::tonic() const {
    return getPitch(0);
}

inline SILENCE_PUBLIC int Scale::degree(const Chord &chord_, int interval) const {
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

inline SILENCE_PUBLIC void Scale::modulations_for_scale_types(std::vector<Scale> &result, const Chord &current_chord, int voices_, const std::vector<std::string> &type_names) const {
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

inline SILENCE_PUBLIC std::vector<Scale> Scale::modulations_for_voices(const Chord &chord, int voices) const {
    std::vector<Scale> result;
    std::vector<std::string> type_names;
    type_names.push_back("major");
    type_names.push_back("harmonic minor");
    modulations_for_scale_types(result, chord, voices, type_names);
    return result;
}

inline SILENCE_PUBLIC std::vector<Scale> Scale::modulations(const Chord &chord) const {
    std::vector<Scale> result;
    std::vector<std::string> type_names;
    type_names.push_back("major");
    type_names.push_back("harmonic minor");
    modulations_for_scale_types(result, chord, chord.voices(), type_names);
    return result;
}

inline SILENCE_PUBLIC void Scale::relative_tonicizations_for_scale_types(std::vector<Scale> &result, const Chord &current_chord, int secondary_function, int voices, const std::vector<std::string> &type_names) const {
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

inline SILENCE_PUBLIC std::vector<Scale> Scale::relative_tonicizations(const Chord &current_chord, int secondary_function, int voices) const {
    std::vector<Scale> result;
    std::vector<std::string> scale_types = {"major", "harmonic minor"};
    relative_tonicizations_for_scale_types(result, current_chord, secondary_function, voices, scale_types);
    return result;
}

inline SILENCE_PUBLIC std::vector<Chord> Scale::secondary(const Chord &current_chord, int secondary_function, int voices_) const {
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

inline SILENCE_PUBLIC std::vector<Scale> Scale::tonicizations(const Chord &current_chord, int voices) const {
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

inline SILENCE_PUBLIC double T(double pitch, double semitones) {
    return pitch + semitones;
}

inline SILENCE_PUBLIC Chord transpose_degrees(const Chord &scale, const Chord &original_chord, int transposition_degrees, int interval) {
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

inline SILENCE_PUBLIC Chord voiceleading(const Chord &a, const Chord &b) {
    Chord voiceleading_ = a;
    for (int voice = 0; voice < a.voices(); ++voice) {
        voiceleading_.setPitch(voice, b.getPitch(voice) - a.getPitch(voice));
    }
    return voiceleading_;
}

inline SILENCE_PUBLIC Chord voiceleadingCloser(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels) {
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

inline SILENCE_PUBLIC Chord voiceleadingClosestRange(const Chord &source, const Chord &destination, double range, bool avoidParallels) {
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

inline SILENCE_PUBLIC Chord voiceleadingSimpler(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels) {
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

inline SILENCE_PUBLIC Chord voiceleadingSmoother(const Chord &source, const Chord &d1, const Chord &d2, bool avoidParallels, double range) {
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

inline SILENCE_PUBLIC double voiceleadingSmoothness(const Chord &a, const Chord &b) {
    double L1 = 0.0;
    for (int voice = 0; voice < a.voices(); ++voice) {
        L1 = L1 + std::abs(b.getPitch(voice) - a.getPitch(voice));
    }
    return L1;
}

inline std::map<int, std::vector<Chord>> &Chord::cyclical_regions_for_dimensionalities() {
    static std::map<int, std::vector<Chord>> cyclical_regions_for_dimensionalities_;
    return cyclical_regions_for_dimensionalities_;
}

inline std::map<int, std::vector<std::vector<Chord>>> &Chord::opt_sectors_for_dimensionalities() {
    static std::map<int, std::vector<std::vector<Chord>>> opt_sectors_for_dimensionalities_;
    return opt_sectors_for_dimensionalities_;
}

inline std::map<int, std::vector<std::vector<Chord>>> &Chord::opti_sectors_for_dimensionalities() {
    static std::map<int, std::vector<std::vector<Chord>>> opti_sectors_for_dimensionalities_;
    return opti_sectors_for_dimensionalities_;
}

inline std::map<int, std::vector<std::vector<Chord>>> &Chord::opt_simplexes_for_dimensionalities() {
    static std::map<int, std::vector<std::vector<Chord>>> opt_simplexes_for_dimensionalities_;
    return opt_simplexes_for_dimensionalities_;
}

inline std::map<int, std::vector<std::vector<Chord>>> &Chord::opti_simplexes_for_dimensionalities() {
    static std::map<int, std::vector<std::vector<Chord>>> opti_simplexes_for_dimensionalities_;
    return opti_simplexes_for_dimensionalities_;
}

inline std::map<int, std::vector<HyperplaneEquation>> &Chord::hyperplane_equations_for_opt_sectors() {
    static std::map<int, std::vector<HyperplaneEquation>> hyperplane_equations_for_opt_sectors_;
    return hyperplane_equations_for_opt_sectors_;
}

inline HyperplaneEquation Chord::hyperplane_equation(int opt_sector) const {
    auto hyperplane_equations_for_dimensions = hyperplane_equations_for_opt_sectors();
    auto hyperplane_equations = hyperplane_equations_for_dimensions[voices()];
    return hyperplane_equations[opt_sector];
}

inline void Chord::initialize_sectors() {
    static bool initialized = false;
    if (initialized == false) {
        initialized = true;
        //SCOPED_DEBUGGING scoped_debugging;
        auto cyclical_regions = cyclical_regions_for_dimensionalities();
        auto &opt_domains_for_dimensions = opt_sectors_for_dimensionalities();
        auto &opti_domains_for_dimensions = opti_sectors_for_dimensionalities();
        auto &opt_simplexes_for_dimensions = opt_simplexes_for_dimensionalities();
        auto &opti_simplexes_for_dimensions = opti_simplexes_for_dimensionalities();
        auto &hyperplane_equations_for_dimensions = hyperplane_equations_for_opt_sectors();
        for (int dimensions_i = 3; dimensions_i < 12; ++dimensions_i) {
            CHORD_SPACE_DEBUG("cyclical region for %d dimensions:\n", dimensions_i);
            auto cyclical_region = cyclical_regions[dimensions_i];
            std::vector<Chord> original;
            std::vector<Chord> transposed;
            auto center_ = Chord(dimensions_i).center();
            CHORD_SPACE_DEBUG("  center:            %s\n", center_.toString().c_str());
            auto center_t = center_.eT();
            CHORD_SPACE_DEBUG("  center_t:          %s\n", center_t.toString().c_str());
            for (int dimension_i = 0; dimension_i < dimensions_i; ++dimension_i) {
                Chord vertex(dimensions_i);
                for (int voice_i = 0, start = dimensions_i - dimension_i; voice_i < dimensions_i; ++voice_i) {
                    if (voice_i >= start) {
                        vertex.setPitch(voice_i, 12.);
                    }
                }
                original.push_back(vertex);
                auto vertex_t = vertex.eT();
                transposed.push_back(vertex_t);
                cyclical_region.push_back(vertex_t);
            }
            // Print in order to check.
            for (int dimension_i = 0; dimension_i < dimensions_i; ++dimension_i) {
                CHORD_SPACE_DEBUG("  original[%2d][%2d]   %s\n", dimensions_i, dimension_i, original[dimension_i].toString().c_str());
            }
            for (int dimension_i = 0; dimension_i < dimensions_i; ++dimension_i) {
                CHORD_SPACE_DEBUG("  transposed[%2d][%2d] %s\n", dimensions_i, dimension_i, transposed[dimension_i].toString().c_str());
            }
            for (int dimension_i = 0; dimension_i < dimensions_i; ++dimension_i) {
                CHORD_SPACE_DEBUG("  cyclical[%2d][%2d]   %s\n", dimensions_i, dimension_i, cyclical_region[dimension_i].toString().c_str());
            }
            cyclical_regions[dimensions_i] = cyclical_region;
            auto opt_domains = opt_domains_for_dimensions[dimensions_i];
            auto opti_domains = opti_domains_for_dimensions[dimensions_i];
            auto opt_simplexes = opt_simplexes_for_dimensions[dimensions_i];
            auto opti_simplexes = opti_simplexes_for_dimensions[dimensions_i];
            auto hyperplane_equations = hyperplane_equations_for_dimensions[dimensions_i];
            for (int dimension_i = 0; dimension_i < dimensions_i; ++dimension_i) {
                auto opt_domain = cyclical_regions[dimensions_i];
                opt_domain[(dimension_i + dimensions_i - 1) % dimensions_i] = center_t;
                opt_domains.push_back(opt_domain);
                CHORD_SPACE_DEBUG("  center:            %s\n", center_t.toString().c_str());
                Chord extra_vertex = center_t.T(1.);
                std::vector<Chord> opt_simplex = opt_domain;
                opt_simplex.push_back(extra_vertex);
                opt_simplexes.push_back(opt_simplex);
                auto opti_midpoint = midpoint(opt_domain[(dimension_i + dimensions_i) % dimensions_i], opt_domain[(dimension_i + dimensions_i - 2) % dimensions_i]);
                CHORD_SPACE_DEBUG("  midpoint:          %s\n", opti_midpoint.toString().c_str());
                CHORD_SPACE_DEBUG("  midpoint_t0:       %s\n", opti_midpoint.et().toString().c_str());
                int vertex_i = 0;
                for (auto vertex : opt_domains[dimension_i]) {
                    CHORD_SPACE_DEBUG("  OPT [%2d][%2d]       %s\n", opt_domains.size() - 1, vertex_i++, vertex.toString().c_str());
                }
                CHORD_SPACE_DEBUG("  extra vertex:      %s\n", extra_vertex.toString().c_str());
                auto opti_domain_0 = opt_domain;
                opti_domain_0[(dimension_i + dimensions_i - 2) % dimensions_i] = opti_midpoint;
                opti_domains.push_back(opti_domain_0);
                vertex_i = 0;
                for (auto vertex : opti_domain_0) {
                    CHORD_SPACE_DEBUG("  OPTI[%2d][%2d]       %s\n", opti_domains.size() - 1, vertex_i++, vertex.toString().c_str());
                }
                CHORD_SPACE_DEBUG("  extra vertex:      %s\n", extra_vertex.toString().c_str());
                std::vector<Chord> opti_simplex_0 = opti_domain_0;
                opti_simplex_0.push_back(extra_vertex);
                opti_simplexes.push_back(opti_simplex_0);
                auto opti_domain_1 = opt_domain;
                opti_domain_1[(dimension_i + dimensions_i) % dimensions_i] = opti_midpoint;
                opti_domains.push_back(opti_domain_1);
                std::vector<Chord> opti_simplex_1 = opti_domain_1;
                opti_simplex_1.push_back(extra_vertex);
                opti_simplexes.push_back(opti_simplex_1);
                vertex_i = 0;
                for (auto vertex : opti_domain_1) {
                    CHORD_SPACE_DEBUG("  OPTI[%2d][%2d]       %s\n", opti_domains.size() - 1, vertex_i++, vertex.toString().c_str());
                }
                CHORD_SPACE_DEBUG("  extra vertex:      %s\n", extra_vertex.toString().c_str());
                auto lower_point = opt_domain[(dimensions_i + dimension_i) % dimensions_i];
                auto upper_point = opt_domain[(dimensions_i + dimension_i - 2) % dimensions_i];
                CHORD_SPACE_DEBUG("  hyperplane_equation: upper_point: %s\n", upper_point.toString().c_str());
                CHORD_SPACE_DEBUG("  hyperplane_equation: lower point: %s\n", lower_point.toString().c_str());
                auto normal_vector = upper_point.col(0) - lower_point.col(0);
                auto norm = normal_vector.norm();
                HyperplaneEquation hyperplane_equation_;
                hyperplane_equation_.unit_normal_vector = normal_vector / norm;
                auto temp = center_.col(0).adjoint() * hyperplane_equation_.unit_normal_vector;    
                hyperplane_equation_.constant_term = temp(0, 0);
                CHORD_SPACE_DEBUG("  hyperplane_equation: sector: %d\n", dimension_i);
                CHORD_SPACE_DEBUG("  hyperplane_equation: center:\n");
                for (int dimension_j = 0; dimension_j < dimensions_i; dimension_j++) {
                    CHORD_SPACE_DEBUG("    %9.4f\n", center_.getPitch(dimension_j));
                }
                CHORD_SPACE_DEBUG("  hyperplane_equation: normal_vector:\n");
                for (int dimension_j = 0; dimension_j < dimensions_i; dimension_j++) {
                    CHORD_SPACE_DEBUG("    %9.4f\n", normal_vector(dimension_j, 0));
                }
                CHORD_SPACE_DEBUG("  hyperplane_equation: norm: %9.4f\n", norm);
                CHORD_SPACE_DEBUG("  hyperplane_equation: unit_normal_vector:\n");
                for (int dimension_j = 0; dimension_j < dimensions_i; dimension_j++) {
                    CHORD_SPACE_DEBUG("    %9.4f\n", hyperplane_equation_.unit_normal_vector(dimension_j, 0));
                }
                CHORD_SPACE_DEBUG("  hyperplane_equation: constant_term: %9.4f\n", hyperplane_equation_.constant_term);
                hyperplane_equations.push_back(hyperplane_equation_);
            }
            opt_domains_for_dimensions[dimensions_i] = opt_domains;
            opti_domains_for_dimensions[dimensions_i] = opti_domains;
            opt_simplexes_for_dimensions[dimensions_i] = opt_simplexes;
            opti_simplexes_for_dimensions[dimensions_i] = opti_simplexes;
            hyperplane_equations_for_dimensions[dimensions_i] = hyperplane_equations;
        }
    }
}

inline double Chord::rownd(double x, int places)  
{
    double power_of_10 = std::pow(double(10), double(places));
    double multiplied = x * power_of_10;
    double rounded = std::round(multiplied);
    double divided = rounded / power_of_10;
    return divided;
}    

inline bool Chord::is_compact(double range) const {
    double outer_interval = getPitch(0) + range - getPitch(voices() - 1);
    for (size_t voice_i = 0, voice_n = voices() - 2; voice_i < voice_n; ++voice_i) {
        double inner_interval = getPitch(voice_i + 1) - getPitch(voice_i);
        if (le_tolerance(outer_interval, inner_interval) == false) {
            return false;
        }
    }
    return true;
}

inline void Chord::clamp(double g) {
    double divisions_per_octave = std::round(OCTAVE() / g);
    for (int voice_i = 0, voice_n = voices(); voice_i < voice_n; ++voice_i) {
        auto pitch = getPitch(voice_i);
        auto divisions = std::round(pitch * divisions_per_octave);
        pitch = divisions / divisions_per_octave;
        setPitch(voice_i, pitch);
    }
}

inline Chord Chord::normal_order() const {
    // This chord as a pitch-class set in ascending order.
    const auto ppcs = eppcs();
    // All cyclic permutations.
    auto permutations_ = ppcs.permutations();
    // We need to keep track of intervals.
    double least_interval = std::numeric_limits<double>::max();
    std::multimap<double, Chord> permutations_for_intervals;
    // Store the permutations keyed by ordered pitch-class interval from the 
    // bottom  voice to the top voice.
    for (auto upper_voice = voices() - 1; upper_voice > 0; --upper_voice) {
        for (auto &permutation : permutations_) {
            auto lower_pc = permutation.getPitch(0);
            auto upper_pc = permutation.getPitch(upper_voice);
            auto interval = upper_pc - lower_pc;
            // Tricky! This is arithmetic modulo the octave.
            // This is _ordered pitch-class interval_.
            // Count up from the left pc to the right pc modulo octaves.
            if (lt_tolerance(interval, 0.) == true) {
                interval = interval + OCTAVE();
            }
            interval = rownd(interval, 9);
            if (lt_tolerance(interval, least_interval) == true) {
                least_interval = interval;
            }
            permutations_for_intervals.insert({interval, permutation});
        }
        // If only one permutation has the least interval, that permutation is 
        // the normal order.
        if (permutations_for_intervals.count(least_interval) == 1) {
            return permutations_for_intervals.begin()->second;
        // Otherwise, replace the list of permutations with only the 
        // permutations having the least interval, and decrement the 
        // upper voice.
        } else {
            permutations_.clear();
            auto range = permutations_for_intervals.equal_range(least_interval);
            for (auto &it = range.first; it != range.second; ++it) {
                permutations_.push_back(it->second);
            }
            permutations_for_intervals.clear();
        }
    }
    std::sort(permutations_.begin(), permutations_.end());
    return permutations_.front();    
}

inline Chord Chord::normal_form() const {
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

inline Chord Chord::prime_form() const {
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

inline Chord Chord::inverse_prime_form() const {
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

inline bool Chord::is_minor() const {
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

inline std::vector<int> Chord::opt_domain_sectors() const {
    //~ auto &opti_sectors_for_dimensions = opti_sectors_for_dimensionalities();
    //~ auto &opti_sectors = opti_sectors_for_dimensions[voices()];
    //~ std::multimap<double, int> sectors_for_distances;
    //~ double minimum_distance = std::numeric_limits<double>::max();
    //~ auto ot = eOT();
    //~ for (int sector = 0, n = opti_sectors.size(); sector < n; ++sector) {
        //~ auto opt_sector = sector / 2;
        //~ auto distance_ = distance_to_points(ot, opti_sectors[sector]);
        //~ auto distance = rownd(distance_);
        //~ sectors_for_distances.insert({distance, opt_sector});
        //~ if (lt_tolerance(distance, minimum_distance, 1000, 10000) == true) {
            //~ minimum_distance = distance;
        //~ }
        //~ auto delta = minimum_distance - distance;
        //~ CHORD_SPACE_DEBUG("Chord::opt_domain_sectors: %s sector: %3d distance: %.20g minimum distance: %.20g delta: %.20g\n", toString().c_str(), opt_sector, distance_, minimum_distance, delta);
    //~ }
    //~ std::vector<int> result;
    //~ auto range = sectors_for_distances.equal_range(minimum_distance);
    //~ for (auto it = range.first; it != range.second; ++it) {
        //~ CHORD_SPACE_DEBUG("Chord::opt_domain_sectors: result for: %s sector: %3d distance: %.20g\n", toString().c_str(), it->second, it->first);
        //~ result.push_back(it->second);
    //~ }
    //~ std::sort(result.begin(), result.end());
    
    // Counting ukp from OPTI sector 0, every two OPTI sectors is one OPT sector.
    auto opti_sectors = opti_domain_sectors();
    std::set<int> opt_sectors;
    for (auto opti_sector : opti_sectors) {
        int opt_sector = std::floor(opti_sector / 2.);
        opt_sectors.insert(opt_sector);
    }
    std::vector<int> result;
    for (auto opt_sector : opt_sectors) {
        result.push_back(opt_sector);
    }
    std::sort(result.begin(), result.end());
    return result;
}

inline std::vector<int> Chord::opti_domain_sectors() const {
    ///SCOPED_DEBUGGING debug;
    auto &opti_sectors_for_dimensions = opti_sectors_for_dimensionalities();
    auto &opti_sectors = opti_sectors_for_dimensions[voices()];
    std::multimap<double, int> sectors_for_distances;
    double minimum_distance = std::numeric_limits<double>::max();
    auto ot = eOT();
    for (int sector = 0, n = opti_sectors.size(); sector < n; ++sector) {
        auto distance_ = distance_to_points(ot, opti_sectors[sector]);
        auto distance = rownd(distance_);
        sectors_for_distances.insert({distance, sector});
        if (lt_tolerance(distance, minimum_distance, 1000, 10000) == true) {
            minimum_distance = distance;
        }
        auto delta = minimum_distance - distance;
        CHORD_SPACE_DEBUG("Chord::opti_domain_sectors: %s sector: %3d distance: %.20g minimum distance: %.20g delta: %.20g\n", toString().c_str(), sector, distance_, minimum_distance, delta);
    }
    std::vector<int> result;
    auto range = sectors_for_distances.equal_range(minimum_distance);
    for (auto it = range.first; it != range.second; ++it) {
        CHORD_SPACE_DEBUG("Chord::opti_domain_sectors: result for: %s sector: %3d distance: %.20g\n", toString().c_str(), it->second, it->first);
        result.push_back(it->second);
    }
    std::sort(result.begin(), result.end());
    return result;
}

inline std::vector<Chord> Chord::opt_domain(int sector) const {
    auto opt_sectors_for_dimensions = opt_sectors_for_dimensionalities();
    auto opt_sectors = opt_sectors_for_dimensions[voices()];
    return opt_sectors[sector];
}

inline std::vector<Chord> Chord::opti_domain(int sector) const {
    auto opti_sectors_for_dimensions = opti_sectors_for_dimensionalities();
    auto opti_sectors = opti_sectors_for_dimensions[voices()];
    return opti_sectors[sector];
}    

inline Chord Chord::eOT() const {
    auto o = eO();
    auto o_t = o.eT();
    return o_t;
}

inline Chord Chord::eOTT(double g) const {
    auto o = eO();
    auto o_tt = o.eTT(g);
    return o_tt;
}

inline Chord Chord::reflect(int opt_sector) const {
    auto reflection = reflect_in_inversion_flat(*this, opt_sector);
    return reflection;
}

inline SILENCE_PUBLIC PITV::~PITV() {};

inline SILENCE_PUBLIC int PITV::getN() const {
    return N;
}

inline SILENCE_PUBLIC int PITV::getG() const {
    return g;
}

inline SILENCE_PUBLIC int PITV::getRange() const {
    return range;
}

inline SILENCE_PUBLIC int PITV::getCountP() const {
    return countP;
}

inline SILENCE_PUBLIC int PITV::getCountI() const {
    return countI;
}

inline SILENCE_PUBLIC int PITV::getCountT() const {
    return countT;
}

inline SILENCE_PUBLIC int PITV::getCountV() const {
    return countV;
}

inline SILENCE_PUBLIC void PITV::preinitialize(int N_, double range_, double g_) {
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

inline SILENCE_PUBLIC void PITV::initialize(int N_, double range_, double g_, bool printme) {
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
            System::message("%3d chord:               %s\n", normal_form_n, print_chord(iterator_));
            System::message("    normal form:         %s\n", print_chord(normal_form_));
            System::message("    prime form:          %s\n", print_chord(iterator_.prime_form()));
            System::message("    inverse prime form:  %s\n", print_chord(iterator_.inverse_prime_form()));
            ++normal_form_n;
        }
    }    
    countP = indexesForPs.size();
    System::message("PITV::initialize: finished with countP: %d.\n", countP);
}

inline SILENCE_PUBLIC void PITV::list(bool listheader, bool listps, bool listvoicings) const {
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
            const auto opti_sectors = optti.opti_domain_sectors();
            for(auto opti_sector : opti_sectors) {
                opttis_for_sectors.insert({opti_sector, optti.toString()});
            }
            System::message("PsForIndexes[%5d]: chord:         %s\n", index, print_chord(chord));
            System::message("PsForIndexes[%5d]: prime:         %s\n", index, print_chord(prime_form_));
            System::message("PsForIndexes[%5d]: normal:        %s\n", index, print_chord(normal_form_));
            System::message("PsForIndexes[%5d]: inverse prime: %s\n", index, print_chord(inverse_prime_form_));
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
            const auto opti_sectors = optti.opti_domain_sectors();
            auto key = chord.toString();
            System::message("indexesForPs[%s]: index:   %5d %s\n", key.c_str(), index, print_chord(chord));
            System::message("indexesForPs[%s]: prime:         %s\n", key.c_str(), print_chord(prime_form_));
            System::message("indexesForPs[%s]: normal:        %s\n", key.c_str(), print_chord(normal_form_));
            System::message("indexesForPs[%s]: inverse prime: %s\n", key.c_str(), print_chord(inverse_prime_form_));
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

inline Eigen::VectorXi PITV::fromChord(const Chord &chord, bool printme) const {
    if (printme) {
        System::message("PITV::fromChord:          chord:         %s\n", print_chord(chord));
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
        System::message("PITV::fromChord: I: %5d normal_form:   %s\n", pitv(1), print_chord(normal_form_));
        System::message("PITV::fromChord: P: %5d prime_form:    %s\n", pitv(0), print_chord(prime_form_));
        System::message("PITV::fromChord:          inverse_prime: %s\n", print_chord(inverse_prime));
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
        System::message("PITV::fromChord: T: %5d normal_form_t: %s\n", pitv(2), print_chord(normal_form_t));
    }
    auto op = normal_form_t.eOP();
    auto op_from_chord = chord.eOP();
    if (printme) {
        System::message("PITV::fromChord:          op from PIT:   %s\n", print_chord(op));
        System::message("PITV::fromChord:          op from chord: %s\n", print_chord(op_from_chord));
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
        System::error("PITV::fromChord: Error: revoiced OP  (%s)\n", print_chord(op_v));
        System::error("                 doesn't match chord (%s)\n", print_chord(chord));
    }
    if (printme) {
        System::message("PITV::fromChord: V: %5d op_v:          %s\n", pitv(3), print_chord(op_v));
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
inline std::vector<Chord> PITV::toChord(int P, int I, int T, int V, bool printme) const {
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
        System::message("PITV::toChord:   I: %5d normal_form:   %s\n", I, print_chord(normal_form_));
        System::message("PITV::toChord:   P: %5d prime_form_:   %s\n", P, print_chord(prime_form_));
        System::message("PITV::toChord:            inv prime form:%s\n",  print_chord(inverse_prime_form_));
   }
    auto normal_form_t = normal_form_;
    for (int t = 0; t < T; ++t) {
        normal_form_t = normal_form_t.T(g);
    }
    if (printme) {
        System::message("PITV::toChord:   T: %5d normal_form_t: %s\n", T, print_chord(normal_form_t));
    }
    auto op = normal_form_t.eOP();
    if (printme) {
        System::message("PITV::toChord:            op:            %s\n",  print_chord(op));
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

inline SILENCE_PUBLIC std::vector<Chord> PITV::toChord_vector(const Eigen::VectorXi &pitv, bool printme) const {
    return toChord(pitv(0), pitv(1), pitv(2), pitv(3), printme);
}

inline SILENCE_PUBLIC std::map<Chord, Chord> &normal_forms_for_chords() {
    static std::map<Chord, Chord> cache;
    return cache;
}

inline SILENCE_PUBLIC std::map<Chord, Chord> &prime_forms_for_chords() {
    static std::map<Chord, Chord> cache;
    return cache;
}

inline SILENCE_PUBLIC std::map<Chord, Chord> &inverse_prime_forms_for_chords() {
    static std::map<Chord, Chord> cache;
    return cache;
}

} // End of namespace csound.

#pragma GCC diagnostic push
  
