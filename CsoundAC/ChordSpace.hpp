#ifndef CHORD_SPACE_HPP_INCLUDED
#define CHORD_SPACE_HPP_INCLUDED
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
#include <limits>
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
#include "Score.hpp"
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
#include <limits>
#include <map>
#include <mutex>
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
/** \file ChordSpace.hpp
 *
 * Geometric operations on chords for algorithmic composition: equivalence
 * classes (orbifolds), voice-leading, neo-Riemannian transformations,
 * scales, and functional harmony.
 *
 * The design follows Callender, Quinn, and Tymoczko, "Generalized
 * Voice-Leading Spaces," _Science_ 320, 2008. Continuous geometry is
 * implemented first; a discrete layer (suffix \c g) expresses how musicians
 * usually think in equal temperament.
 *
 * # What this library does
 *
 *  - Decide whether a chord lies in a representative fundamental domain for
 *    an equivalence relation, and map arbitrary chords to canonical
 *    representatives (\c eOP, \c eOPT, \c eOPTIg, and related functions).
 *
 *  - Enumerate equivalence classes (e.g. all trichord set-classes in 12-TET
 *    via \c fundamentalDomainByEquate).
 *
 *  - Move progressions within an orbifold, apply neo-Riemannian \c P, \c L,
 *    \c R, \c D, \c K, \c Q, and related contextual operations.
 *
 *  - Voice-lead between abstract equivalence classes using closest
 *    voice-leading in a less abstract space.
 *
 *  - Work with scales, scale degrees, and Roman-numeral-style operations.
 *
 * # Pitches, voices, and chords
 *
 * Pitch is a logarithmic quantity: octaves are doublings of frequency. Pitches
 * and intervals are \b real numbers. Middle C is \c 60; one octave is \c 12,
 * matching MIDI and 12-tone equal temperament (12-TET). Whole numbers are
 * semitone steps; fractions allow microtonal or continuous values.
 *
 * A \b voice is one sounding line. A \b chord is an ordered tuple of pitches,
 * a point in \f$ \mathbb{R}^N \f$ with one dimension per voice. A \b scale
 * is a chord whose lowest voice is a tonic pitch-class and whose other voices
 * are pitch-classes in ascending order. For composition, a score can be read
 * as a sequence of fleeting chords.
 *
 * # Continuous vs discrete (\c g)
 *
 * Two related geometries share the same \c Chord type:
 *
 *  - \b Continuous space: pitches may be any reals. Equivalence relations
 *    \c O, \c P, \c T, \c I and their compounds (\c OP, \c OPT, \c OPTI,
 *    \c RPT, \c RPTI, …) use exact transposition (sum of pitches \c 0 after
 *    \c eT) and hyperplane reflection where appropriate.
 *
 *  - \b Discrete (\c g-lattice) space: pitches are projected to an equally
 *    spaced grid with step \c g > 0. For musical use, \c g should divide the
 *    octave evenly: \f$ 12/g \in \mathbb{N} \f$. Examples: \c g = 1 (12-TET),
 *    \c g = 0.5 (24-TET), \c g = 2 (whole-tone lattice). Relations with suffix
 *    \c g (\c Tg, \c Ig, \c OPTg, \c OPTIg, \c RPTg, \c RPTIg, …) first
 *    compute the continuous representative, then snap to the lattice (\c eET).
 *
 * For \b algorithmic composition in 12-TET, prefer the \c g APIs with
 * \c g = 1 (\c eOPTIg, \c eRPTIg, \c iseRPTIg, \c fundamentalDomainByEquate
 * for \c RPTIg). They match “chord type” and “set class” as musicians use
 * those terms.
 *
 * # Equivalence relations
 *
 * An equivalence relation groups different chords that are considered “the
 * same” under some symmetry. Quotienting by equivalences yields orbifolds:
 * facets of a fundamental domain are identified (e.g. octave wrap-around).
 * Many relations admit several geometrically identical copies of a fundamental
 * domain; this API picks \b representative domains (normal forms).
 *
 * Chords on \b boundaries (shared facets, edges, or vertices of sectors)
 * may belong to more than one sector; predicates are closed-set tests.
 * Sector indices select which inversion flat or cyclical region to use—they
 * do not always assign unique ownership.
 *
 * Cardinality equivalence is \b not assumed: spaces have fixed dimension \c N
 * (e.g. a single note may be \c {60, 60, 60} in trichord space).
 *
 * <dl>
 *
 * <dt>O</dt><dd>Octave equivalence: pitches differing by whole octaves (\c 12)
 * are equivalent. Fundamental domain: span at most one octave.</dd>
 *
 * <dt>P</dt><dd>Permutational equivalence: reordering voices does not change
 * the chord. Fundamental domain: voices sorted ascending.</dd>
 *
 * <dt>T</dt><dd>Continuous transpositional equivalence: translate all voices
 * by the same amount. Fundamental domain: hyperplane with pitch sum \c 0
 * (mean zero). Pitches need not be integers after \c eT.</dd>
 *
 * <dt>Tg</dt><dd>Transpositional equivalence on the \c g-lattice: same idea as
 * \c T, but the representative is chosen so voices lie on the tempered grid
 * (\c eTg, \c eET).</dd>
 *
 * <dt>I</dt><dd>Continuous inversion: reflection in a hyperplane (inversion
 * flat), not registral revoicing. “Invert” here is the mathematician’s sense;
 * octave shifts of individual voices are \b revoicings.</dd>
 *
 * <dt>Ig</dt><dd>Inversion on the \c g-lattice (\c eIg).</dd>
 *
 * <dt>OP</dt><dd>Octave + permutational equivalence (Tymoczko’s chord space).
 * An \c N-voice space is an equilateral hyperprism, one octave high, with \c N
 * cyclical sectors from octavewise revoicing. Trichords in 12-TET: augmented
 * triads along the center, major/minor columns around them.</dd>
 *
 * <dt>OPT</dt><dd>Chord \b type: \c OP further modulo continuous translation
 * (\c T). The representative layer is the base of the \c OP prism (normal
 * form). There are \c N sector copies related by revoicing.</dd>
 *
 * <dt>OPTg</dt><dd>As \c OPT, with representatives on the \c g-lattice.</dd>
 *
 * <dt>OPTI</dt><dd>\b Set class: \c OPT modulo continuous inversion (\c I).
 * Major and minor triads share a class. Canonical inversion pairs use direct
 * musical inversion (negate and reverse T-normal pitches, then reduce), not
 * only hyperplane reflection—required for correctness when \c N \c > \c 4.</dd>
 *
 * <dt>OPTIg</dt><dd>As \c OPTI on the \c g-lattice. Inversion pairs in the
 * \c RPTIg fundamental domain are resolved by lexicographic tie-break
 * (smaller representative of each pair), equivalent to picking one side of
 * each inversion pair without relying on the inversion flat after revoicing.</dd>
 *
 * <dt>R, RP, RPT, RPTI, RPTg, RPTIg</dt><dd>Range (\c R) and related
 * compounds combine octavewise revoicing within a range with the above
 * relations. Names ending in \c g use the discrete layer.</dd>
 *
 * </dl>
 *
 * # Neo-Riemannian and contextual operations
 *
 * Beyond equivalence maps (\c eOP, \c eOPT, …):
 *
 * <dl>
 *
 * <dt>T(p, x)</dt><dd>Translate chord \c p by \c x semitones.</dd>
 *
 * <dt>I(p [, x])</dt><dd>Reflect \c p in point \c x (default origin).</dd>
 *
 * <dt>P, L, R</dt><dd>Parallel, Leittonwechsel, Relative (major/minor
 * neighbors).</dd>
 *
 * <dt>D</dt><dd>Dominant (down a perfect fifth).</dd>
 *
 * <dt>K(c), Q(c, n, m)</dt><dd>Contextual inversion and transposition
 * (Fiore and Satyendra, "Generalized Contextual Groups," _MTO_ 11, 2008).</dd>
 *
 * <dt>R(s), Rg(s, g)</dt><dd>Reflection in the inversion flat of OPT sector
 * \c s; \c Rg uses a cached involution on \c g-lattice representatives.</dd>
 *
 * </dl>
 *
 * # Known limitations and test coverage
 *
 *  - \b Continuous \c eOPT / \c eRPT: One known failure in \c ChordSpaceTests
 *    for \c N = 6 voices: some representatives in the \c RPTI catalogue have
 *    pitch sums not divisible by \c 6, so continuous \c eT yields non-integer
 *    pitches and \c eOPT is not idempotent. This does \b not affect the
 *    discrete path \c eOPTg / \c eOPTIg / \c RPTIg at \c g = 1, which is the
 *    recommended API for 12-TET composition. The continuous failure is
 *    intentionally left unfixed for now.
 *
 *  - \b Other values of \c g: The implementation is parameterized by \c g,
 *    but the regression suite (\c ChordSpaceTests) currently exercises only
 *    \c g = 1 with \c range = 12. Domain-size checks for \c RPTg and \c RPTIg
 *    are hardcoded for 12-TET (\c 19, \c 72, \c 196, \c 561 set-classes for
 *    \c N = 3…6). Using \c g \c != \c 1 (e.g. 24-TET) should be treated as
 *    untested until dedicated tests are added.
 *
 *  - \b CQT catalogues: Illustrations in Callender–Quinn–Tymoczko use integer
 *    pitch-classes in continuous OPTI geometry; this library’s \c RPTIg
 *    domains at \c g = 1 use a slightly different convention (exact lattice,
 *    lex inversion tie-break). Counts may differ from manual tallies of those
 *    figures (e.g. 72 vs 84 tetrachord classes).
 *
 * See also \c CHORDSPACE.md in the repository for a longer reference.
 * 
 * Debugging, testing, and design changes were done by Michael Gogins using 
 * ChatGPT and Cursor with Sonnet 3.5.
 */

static std::string chord_space_version() {
    char buffer[0x500];
    std::snprintf(buffer, sizeof(buffer), "ChordSpace version 3.0.0. Compiled from %s on %s at %s.", __FILE__, __DATE__, __TIME__);
    return buffer;
}

/**
 * Returns the current state of the chord space debugging flag as a 
 * reference, which can be an lvalue or an rvalue.
 */
static bool &CHORD_SPACE_DEBUGGING() {
    static bool CHORD_SPACE_DEBUGGING_ = false;
    return CHORD_SPACE_DEBUGGING_;
}

static bool SET_CHORD_SPACE_DEBUGGING(bool enabled) {
    bool prior_value = CHORD_SPACE_DEBUGGING();
    CHORD_SPACE_DEBUGGING() = enabled;
    return prior_value;
}

/**
 * Returns the current state of the chord space _scoped_ debugging flag as a 
 * reference, which can be an lvalue or an rvalue.
 */
static bool &SCOPED_DEBUGGING_FLAG() {
    static bool SCOPED_DEBUGGING_ = false;
    return SCOPED_DEBUGGING_;
}

static bool SET_SCOPED_DEBUGGING(bool enabled) {
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

SILENCE_PUBLIC bool equals_in_rpt(const Chord& a, const Chord& b, double range, double g);

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

template<int EQUIVALENCE_RELATION> SILENCE_PUBLIC Chord equate(const Chord &chord,
        double range, double g, int opt_sector);

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
    template<int EQUIVALENCE_RELATION>
    Chord key(
        double range,
        double g,
        int sector) const
    {
        if (!(range > 0.0))
        {
            range = OCTAVE();
        }

        if (!(g > 0.0))
        {
            g = 1.0;
        }

        if (sector < 0 || sector >= voices())
        {
            sector = 0;
        }

        return equate<EQUIVALENCE_RELATION>(
            *this,
            range,
            g,
            sector).eET(g);
    }
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
     * Returns a new Chord with the pitches in this chord rounded to the nearest 
     * integer multiple of g, the generator of transposition. This is valid only 
     * if g goes evenly into 12 (the octave), i.e. in 12/g tone equal 
     * temperament.
     */
    virtual Chord eET(double g=1.) const;
    /**
     * Returns whether or not the chord lies on the g-lattice.
     */
    virtual bool iseET(double g=1.) const;
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
     * Returns the equivalent of the chord within a fundamental domain of
     * inversional equivalence in the equal temperament generated by g.
     */
    virtual Chord eIg(double range = OCTAVE(), double g = 1., int opt_sector = 0) const;
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
    virtual Chord eOPg(double g = 1.) const;
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
    virtual Chord eOPTg(double g = 1., int opt_sector = 0) const;
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
    virtual Chord eOPTIg(double g = 1., int opt_sector = 0) const;
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
    virtual Chord eOTg(double g = 1.) const;
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
    virtual Chord eRPTg(double range, double g = 1., int opt_sector = 0) const;
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
     * domains of range, permutational, and transpositional equivalence
     * in the equal temperament generated by g.
     */
     virtual std::vector<Chord> eRPTgs(double range = OCTAVE(), double g = 1.) const;
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
    virtual Chord eRPTIg(double range, double g = 1., int opt_sector = 0) const;
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
    virtual Chord eTg(double g = 1.) const;
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


    virtual bool is_in_rpt_sector_g(
        int opt_sector = 0,
        double range = 12.0,
        double g = 1.0) const;
    
    virtual bool is_in_rpt_sector_base_g(
        int opt_sector = 0,
        double range = 12.0,
        double g = 1.0) const;
    
    virtual bool is_in_minor_rpti_sector_g(
        int opt_sector,
        double range = 12.0,
        double g = 1.0) const;

    /**
     * Returns whether the chord is within the fundamental domain of inversional 
     * equivalence.
     */
    virtual bool iseI_chord(Chord *inverse, int opt_sector = 0) const;
    virtual bool iseI(int opt_sector = 0) const;
    /**
     * Returns whether the chord is within a fundamental domain of inversional
     * equivalence in the equal temperament generated by g.
     */
    virtual bool iseIg(double range = OCTAVE(), double g = 1., int opt_sector = 0) const;
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
     * Returns whether the chord is within the representative fundamental 
     * domain of octave and permutational equivalence in the equal temperament 
     * generated by g.
     */
     virtual bool iseOPg(double g = 1.) const;
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
    virtual bool iseOPTg(double g = 1., int opt_sector = 0) const;
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
    virtual bool iseOPTIg(double g  = 1., int opt_sector = 0) const;
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
    virtual bool iseOTg(double g = 1.) const {
        if (iseO() == false) {
            return false;
        }
        if (iseTg(g) == false) {
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
    virtual bool iseRPTg(double range, double g = 1., int opt_sector = 0) const;
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
   virtual bool iseRPTIg(double range, double g = 1., int opt_sector = 0) const;
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of range and transpositional equivalence.
     */
    virtual bool iseRT(double range) const;
    /**
     * Returns whether the chord is within a fundamental domain of range and 
     * transpositional equivalence in the equal temperament generated by g.
     */
    virtual bool iseRTg(double range, double g = 1.) const {
        if (iseR(range) == false) {
            return false;
        }
        if (iseTg(g) == false) {
            return false;
        }
        return true;
    }
    /**
     * Returns whether the chord is within the representative fundamental 
     * domain of range and permutational equivalence in the equal temperament 
     * generated by g.
     */
    virtual bool iseRPg(double range, double g = 1.0, int opt_sector = 0) const;
    /**
     * Returns the equivalent of the chord within the representative fundamental 
     * domain of range and permutational equivalence in the equal temperament 
     * generated by g.
     */
    virtual Chord eRPg(double range, double g = 1.0, int opt_sector = 0) const;     
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
    virtual bool iseTg(double g = 1.) const;

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
     * Reflects the chord in the inversion flat of the indicated OPT sector.
     */
    virtual Chord R(int opt_sector) const;
    /**
     * Reflects the chord in the inversion flat of the indicated OPT sector,
     * on the g-lattice.
     */
    virtual Chord Rg(int opt_sector, double g = 1.0) const;
    /**
     * Returns the chord inverted by the sum of its first two voices.
     */
    virtual Chord K() const;
    virtual Chord K_range(double range) const;
    /**
     * Returns the sum of the pitches in the chord.
     */
    virtual double layer() const;
    virtual int tg_layer(double g) const;

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

    /**
     * Returns all the 'inversions' (in the musician's sense) of this chord, in the 
     * equal temperament generated by g, but also including permutations; for use 
     * with the equivalence class RPTg.
     */
    virtual std::vector<Chord> permuted_voicings_g(double range=OCTAVE(), double g = 1.0) const;
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
        : apex()
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
        apex = chord_point_column(vertices[0]);

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
        const double signed_at_base = (base_midpoint - apex).dot(unit_normal);
        if (signed_at_base > 0.0)
        {
            unit_normal = -unit_normal;
        }
    }

    Chord reflect(const Chord &chord) const
    {
        Chord reflection = chord;

        const Vector chord_v = chord_point_column(chord);
        const Vector a_to_chord = chord_v - apex;

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
        const Vector a_to_chord = chord_v - apex;
        const double signed_distance = a_to_chord.dot(unit_normal);

        return le_tolerance(signed_distance, 0.0, 64, 512);
    }

    bool is_invariant(const Chord &chord) const
    {
        const Vector chord_v = chord_point_column(chord);
        const Vector a_to_chord = chord_v - apex;
        const double signed_distance = a_to_chord.dot(unit_normal);

        return eq_tolerance(std::abs(signed_distance), 0.0, 64, 512);
    }

    Chord midpoint() const
    {
        Chord midpoint_chord;
        midpoint_chord.resize(apex.rows());

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
        append_vector(" apex: ", apex);
        return result;
    }

    Vector apex;
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

/**
 * Induced discrete involution helper: Given a continuous involution R (e.g., 
 * reflection in inversion-flat), produce a lattice map R_g that is an 
 * involution on the g-lattice.
 */ 
template<typename ContinuousMap, typename SectorPredicate>
inline SILENCE_PUBLIC Chord discrete_involutive_map(
    const Chord& x,
    double g,
    ContinuousMap R,
    SectorPredicate sector_ok)
{
    const int n = x.voices();

    // Continuous image.
    const Chord y_cont = R(x);

    // Reference lattice snap near the continuous image.
    Chord y_lattice = y_cont;
    y_lattice.eET(g);
    y_lattice = y_lattice.eOP();

    // Neighborhood around y_lattice (radius 1 in each coordinate).
    std::vector<Chord> candidates;
    candidates.reserve(size_t(1 + 2 * n));
    candidates.push_back(y_lattice);

    for (int v = 0; v < n; ++v)
    {
        for (int dir : {-1, 1})
        {
            Chord c = y_lattice;
            c.setPitch(v, c.getPitch(v) + dir * g);
            c = c.eOP();
            c.eET(g);
            c = c.eOP();
            candidates.push_back(c);
        }
    }

    // Optional sector-preserving filter.
    std::vector<Chord> filtered;
    filtered.reserve(candidates.size());
    for (const auto& c : candidates)
    {
        if (sector_ok(x, c))
        {
            filtered.push_back(c);
        }
    }
    if (filtered.empty())
    {
        filtered = candidates;
    }

    // Symmetric pairing selection: choose c such that R_g(c) == x.
    Chord best;
    bool found = false;
    double best_dist = std::numeric_limits<double>::infinity();

    for (const auto& c : filtered)
    {
        Chord back = R(c);
        back.eET(g);
        back = back.eOP();

        if ((back - x).norm() < 1e-9)
        {
            const double d = (c - y_cont).norm();
            if (!found || d < best_dist)
            {
                best = c;
                best_dist = d;
                found = true;
            }
        }
    }

    if (!found)
    {
        // Fallback: nearest filtered lattice point to y_cont.
        for (const auto& c : filtered)
        {
            const double d = (c - y_cont).norm();
            if (!found || d < best_dist)
            {
                best = c;
                best_dist = d;
                found = true;
            }
        }
    }

    return best;
}

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
    // Continuous equivalence relations.
    EQUIVALENCE_RELATION_r = 0,
    EQUIVALENCE_RELATION_R,
    EQUIVALENCE_RELATION_P,
    EQUIVALENCE_RELATION_T,
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
    EQUIVALENCE_RELATION_RPI,
    EQUIVALENCE_RELATION_RTI,
    EQUIVALENCE_RELATION_RPTI,
    // Discrete equivalence relations.
    EQUIVALENCE_RELATION_Tg,
    EQUIVALENCE_RELATION_Ig,
    EQUIVALENCE_RELATION_RPg,
    EQUIVALENCE_RELATION_RPTg,
    EQUIVALENCE_RELATION_RTIg,
    EQUIVALENCE_RELATION_RPTIg,
} EQUIVALENCE_RELATIONS;

/*
typedef enum {
    DISCRETE_EQUIVALENCE_RELATION_dT,
    DISCRETE_EQUIVALENCE_RELATION_dRP,
    DISCRETE_EQUIVALENCE_RELATION_dRPT,
    DISCRETE_EQUIVALENCE_RELATION_dI,
    DISCRETE_EQUIVALENCE_RELATION_dRPTI,
} DISCRETE_EQUIVALENCE_RELATIONS;
*/

#if !defined(SWIG)
static const char* namesForEquivalenceRelations[] = {
    "r",
    "R",
    "P",
    "T",
    "I",
    "RP",
    "RT",
    "RPT",
    "RPI",
    "RTI",
    "RPTI",
    "Tg",
    "Ig",
    "RPg",
    "RPTg",
    "RTIg",
    "RPTIg",
};
#endif

SILENCE_PUBLIC double factorial(double n);

SILENCE_PUBLIC void fill(std::string rootName, double rootPitch, std::string typeName, std::string typePitches, bool is_scale = false);

/**
 * Returns a set of chords in one sector of the cyclical region.
 */
template<int EQUIVALENCE_RELATION> const std::vector<Chord> &fundamentalDomainByPredicate(int voiceN, double range, double g = 1., int sector=0, bool printme=false);

/**
 * Returns a set of chords in the union of all sectors of the cyclical region,
 * generated directly from the RP lattice (nondecreasing "shape" plus scalar
 * translation) and filtered by the indicated predicate.
 *
 * This avoids scanning an ambient box and therefore prevents pathological
 * growth in candidate counts for higher voice counts.
 */
template<int EQUIVALENCE_RELATION> const std::vector<Chord> &fundamentalDomainByEquate(int voiceN, double range, double g = 1., int sector=0, bool printme=false);

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

SILENCE_PUBLIC void initializeNames();

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
template<> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_Ig>(const Chord &chord,
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
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RTIg>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC bool predicate<EQUIVALENCE_RELATION_RPTIg>(const Chord &chord,
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
template<> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_Ig>(const Chord &chord,
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
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RTIg>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTI>(const Chord &chord,
        double range, double g, int opt_sector);
template <> SILENCE_PUBLIC Chord equate<EQUIVALENCE_RELATION_RPTIg>(const Chord &chord,
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

// Continuous geometric reflection only.
// Not lattice-aware. No rounding. No discrete involution repair.
SILENCE_PUBLIC Chord reflect_in_inversion_flat(
    const Chord &chord,
    int opt_sector);

// Discrete lattice operation only.
// Projects to g-lattice, reflects geometrically, then chooses the nearest
// lattice point in a way that preserves involution.
SILENCE_PUBLIC Chord reflect_in_inversion_flat_g(
    const Chord &chord,
    int opt_sector,
    double g);

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
         * (function = 2), secondary submediants (function = 6), and so on.
         * It is then up to the user to perform an appropriate progression 
         * by number of scale degrees in the original Scale.
         */
        virtual std::vector<Chord> secondary(const Chord &current_chord, int secondary_function = 5, int voices_ = -1) const;
        /**
         * Returns the current Chord mutated, if possible, have the specified 
         * function with respect to the target scale degree of the Scale. Not 
         * "secondary function of this chord," but "this chord as secondary 
         * function of another (tonicized) chord." The number of voices 
         * defaults to that of the current Chord. Can be used to generate 
         * secondary dominants (function = 5), secondary supertonics 
         * (function = 2), secondary submediants (function = 6), and so on.
         * It is then up to the user to perform an appropriate progression 
         * by number of scale degrees in the original Scale. If no such mutation 
         * can be found, nothing is done; i.e., the current Chord is returned.
         */
        virtual Chord secondary_to_degree(const Chord &current_chord, int secondary_function, int target_degree, int chord_voices = -1) const;
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

/*
    ChordSpaceFundamentalDomains.hpp

    Fast cached construction of fundamental domains for ChordSpace equivalence
    relations.

    Header-only implementation intended to be pasted into ChordSpace.hpp.

    Public cached API:

        fundamentalDomainByPredicate<EQUIVALENCE_RELATION>
        fundamentalDomainByEquate<EQUIVALENCE_RELATION>

    Uncached computation API:

        fundamentalDomainByPredicate_<EQUIVALENCE_RELATION>
        fundamentalDomainByEquate_<EQUIVALENCE_RELATION>

    The implementation enumerates unordered pitch-class multisets directly,
    applies the requested equivalence relation, uniquifies the representatives,
    caches the resulting domain, and exposes lookup helpers.
*/

#pragma region fundamental_domain_cache

/*
 * Header-only fundamental-domain cache and fast domain enumerators.
 *
 * Intended placement: paste into ChordSpace.hpp after Chord, predicate<ER>,
 * equate<ER>, and namesForEquivalenceRelations are declared.
 *
 * The public fundamentalDomainByPredicate<ER> and
 * fundamentalDomainByEquate<ER> functions return const references to cached
 * vectors. The underscore-suffixed functions do the actual uncached
 * computation and return vectors by value.
 */

 enum class fundamental_domain_generation_mode
 {
     by_predicate,
     by_equate
 };
 
 struct fundamental_domain_cache_key
 {
     int equivalence_relation = 0;
     int voices = 0;
     int sector = 0;
     int range_key = 0;
     int g_key = 0;
     fundamental_domain_generation_mode mode =
         fundamental_domain_generation_mode::by_equate;
 
     bool operator<(const fundamental_domain_cache_key &other) const
     {
         if (equivalence_relation != other.equivalence_relation)
         {
             return equivalence_relation < other.equivalence_relation;
         }
         if (voices != other.voices)
         {
             return voices < other.voices;
         }
         if (sector != other.sector)
         {
             return sector < other.sector;
         }
         if (range_key != other.range_key)
         {
             return range_key < other.range_key;
         }
         if (g_key != other.g_key)
         {
             return g_key < other.g_key;
         }
         return static_cast<int>(mode) < static_cast<int>(other.mode);
     }
 };
 
 namespace fundamental_domain_detail
 {
 
 inline int double_key(double value)
 {
     return static_cast<int>(std::llround(value * 1000000.0));
 }
 
 inline long long binomial_ll(int n, int k)
 {
     if (k < 0 || k > n)
     {
         return 0;
     }
 
     if (k > n - k)
     {
         k = n - k;
     }
 
     long long result = 1;
 
     for (int i = 1; i <= k; ++i)
     {
         result =
             (result * static_cast<long long>(n - k + i)) /
             static_cast<long long>(i);
     }
 
     return result;
 }
 
 inline int temperament_steps(double range, double g)
 {
     if (!(range > 0.0))
     {
         range = OCTAVE();
     }
 
     if (!(g > 0.0))
     {
         g = 1.0;
     }
 
    const double raw_steps = range / g;
    const int steps = static_cast<int>(std::llround(raw_steps));

    if (steps <= 0)
    {
        return 12;
    }

    if (std::abs(raw_steps - static_cast<double>(steps)) > 1.0e-7)
    {
        System::message(
            "Warning: range/g is not integral in fundamental-domain enumeration "
            "(range: %f g: %f range/g: %.12f rounded steps: %d).\n",
            range,
            g,
            raw_steps,
            steps);
    }
 
     return steps;
 }
 
 inline std::vector<long long> lattice_key_for_chord(
     const Chord &chord,
     double g)
 {
     if (!(g > 0.0))
     {
         g = 1.0;
     }
 
     std::vector<long long> key;
     key.reserve(static_cast<std::size_t>(chord.voices()));
 
     for (int voice = 0; voice < chord.voices(); ++voice)
     {
         key.push_back(
             static_cast<long long>(
                 std::llround(chord.getPitch(voice) / g)));
     }
 
     return key;
 }
 
 template<typename Callback>
 void enumerate_pitch_class_multisets(
     int voices,
     int steps,
     Callback callback)
 {
     if (voices <= 0 || steps <= 0)
     {
         return;
     }
 
     std::vector<int> pcs(static_cast<std::size_t>(voices), 0);
 
     std::function<void(int, int)> enumerate =
         [&](int voice, int last_pc)
         {
             if (voice >= voices)
             {
                 callback(pcs);
                 return;
             }
 
             for (int pc = last_pc; pc < steps; ++pc)
             {
                 pcs[static_cast<std::size_t>(voice)] = pc;
                 enumerate(voice + 1, pc);
             }
         };
 
     enumerate(0, 0);
 }
 
 inline Chord chord_from_pitch_class_multiset(
     const std::vector<int> &pcs,
     double g)
 {
     Chord chord(static_cast<int>(pcs.size()));
 
     for (int voice = 0; voice < static_cast<int>(pcs.size()); ++voice)
     {
         chord.setPitch(
             voice,
             static_cast<double>(pcs[static_cast<std::size_t>(voice)]) * g);
     }
 
     return chord.eET(g);
 }
 
 inline bool add_unique_chord(
     std::vector<Chord> &chords,
     std::map<std::vector<long long>, std::size_t> &index_by_lattice_key,
     const Chord &candidate,
     double g)
 {
     const std::vector<long long> key =
         lattice_key_for_chord(candidate, g);
 
     if (index_by_lattice_key.find(key) != index_by_lattice_key.end())
     {
         return false;
     }
 
     index_by_lattice_key.emplace(key, chords.size());
     chords.push_back(candidate);
     return true;
 }
 
 inline void sort_unique_domain(
     std::vector<Chord> &chords,
     double g)
 {
     std::sort(chords.begin(), chords.end());
 
     std::vector<Chord> unique_chords;
     unique_chords.reserve(chords.size());
 
     std::map<std::vector<long long>, std::size_t> index_by_lattice_key;
 
     for (const Chord &chord : chords)
     {
         add_unique_chord(
             unique_chords,
             index_by_lattice_key,
             chord,
             g);
     }
 
     chords.swap(unique_chords);
 }
 
 /*
  * Expected sizes are intentionally cheap.
  *
  * For arbitrary equal temperaments, expected transposition/TI sizes can be
  * computed by Burnside or by multiset enumeration, but doing so in this hot
  * path defeats the point of the cache. Keep exact known values for the common
  * 12TET cases and exact OPg counts for all ETs.
  */
 template<int EQUIVALENCE_RELATION>
 int expected_domain_size(
     int voices,
     int steps)
 {
     if (voices <= 0 || steps <= 0)
     {
         return -1;
     }
 
     if (EQUIVALENCE_RELATION == EQUIVALENCE_RELATION_RPg)
     {
         const long long count =
             binomial_ll(steps + voices - 1, voices);
 
         if (count <= static_cast<long long>(std::numeric_limits<int>::max()))
         {
             return static_cast<int>(count);
         }
 
         return -1;
     }
 
     if (steps == 12)
     {
         if (EQUIVALENCE_RELATION == EQUIVALENCE_RELATION_RPTg)
         {
             switch (voices)
             {
             case 3:
                 return 31;
             case 4:
                 return 116;
             case 5:
                 return 364;
             case 6:
                 return 1038;
             default:
                 return -1;
             }
         }
 
         if (EQUIVALENCE_RELATION == EQUIVALENCE_RELATION_RPTIg)
         {
             switch (voices)
             {
             case 3:
                 return 19;
             case 4:
                 return 72;
             case 5:
                 return 196;
             case 6:
                 return 561;
             default:
                 return -1;
             }
         }
     }
 
     return -1;
 }
 
 inline fundamental_domain_cache_key make_cache_key(
     int equivalence_relation,
     int voices,
     double range,
     double g,
     int sector,
     fundamental_domain_generation_mode mode)
 {
     fundamental_domain_cache_key key;
     key.equivalence_relation = equivalence_relation;
     key.voices = voices;
     key.sector = sector;
     key.range_key = double_key(range);
     key.g_key = double_key(g);
     key.mode = mode;
     return key;
 }
 
 } // namespace fundamental_domain_detail
 
 struct fundamental_domain
 {
     std::vector<Chord> chords;
     std::unordered_map<std::string, std::size_t> index_by_to_string;
     std::map<std::vector<long long>, std::size_t> index_by_lattice_key;
     mutable std::unordered_map<std::string, std::size_t> index_by_name;
     mutable bool name_index_is_ready = false;
 
     long long generated = 0;
     long long accepted = 0;
     int expected_size = -1;
     bool size_is_expected = true;
     double g = 1.0;
 
     void rebuild_indices(double g_)
     {
         g = (g_ > 0.0) ? g_ : 1.0;
         index_by_to_string.clear();
         index_by_lattice_key.clear();
         index_by_name.clear();
         name_index_is_ready = false;
 
         for (std::size_t index = 0; index < chords.size(); ++index)
         {
             const Chord &chord = chords[index];
 
             index_by_to_string.emplace(
                 chord.toString(),
                 index);
 
             index_by_lattice_key.emplace(
                 fundamental_domain_detail::lattice_key_for_chord(chord, g),
                 index);
         }
     }
 
     void rebuild_name_index() const
     {
         if (name_index_is_ready)
         {
             return;
         }
 
         index_by_name.clear();
 
         for (std::size_t index = 0; index < chords.size(); ++index)
         {
             index_by_name.emplace(
                 print_chord(chords[index]),
                 index);
         }
 
         name_index_is_ready = true;
     }
 
     std::size_t size() const
     {
         return chords.size();
     }
 
     bool empty() const
     {
         return chords.empty();
     }
 
     const Chord *chord_for_index(std::size_t index) const
     {
         if (index >= chords.size())
         {
             return nullptr;
         }
 
         return &chords[index];
     }
 
     const Chord *chord_for_to_string(const std::string &text) const
     {
         const auto it = index_by_to_string.find(text);
 
         if (it == index_by_to_string.end())
         {
             return nullptr;
         }
 
         return chord_for_index(it->second);
     }
 
     const Chord *chord_for_name(const std::string &name) const
     {
         const Chord *by_to_string =
             chord_for_to_string(name);
 
         if (by_to_string != nullptr)
         {
             return by_to_string;
         }
 
         rebuild_name_index();
 
         const auto name_it = index_by_name.find(name);
 
         if (name_it == index_by_name.end())
         {
             return nullptr;
         }
 
         return chord_for_index(name_it->second);
     }
 
     int index_for_to_string(const std::string &text) const
     {
         const auto it = index_by_to_string.find(text);
 
         if (it == index_by_to_string.end())
         {
             return -1;
         }
 
         return static_cast<int>(it->second);
     }
 
     int index_for_name(const std::string &name) const
     {
         const int string_index =
             index_for_to_string(name);
 
         if (string_index >= 0)
         {
             return string_index;
         }
 
         rebuild_name_index();
 
         const auto name_it = index_by_name.find(name);
 
         if (name_it == index_by_name.end())
         {
             return -1;
         }
 
         return static_cast<int>(name_it->second);
     }
 
     int index_for_chord(const Chord &chord) const
     {
         const auto key =
             fundamental_domain_detail::lattice_key_for_chord(chord, g);
 
         const auto key_it =
             index_by_lattice_key.find(key);
 
         if (key_it != index_by_lattice_key.end())
         {
             return static_cast<int>(key_it->second);
         }
 
         const int string_index =
             index_for_to_string(chord.toString());
 
         if (string_index >= 0)
         {
             return string_index;
         }
 
         for (std::size_t index = 0; index < chords.size(); ++index)
         {
             if (chords[index] == chord)
             {
                 return static_cast<int>(index);
             }
         }
 
         return -1;
     }
 };
 
 namespace fundamental_domain_detail
 {
 
 inline std::map<fundamental_domain_cache_key, fundamental_domain> &domain_cache()
 {
     static std::map<fundamental_domain_cache_key, fundamental_domain> cache;
     return cache;
 }
 
 inline std::mutex &domain_cache_mutex()
 {
     static std::mutex mutex;
     return mutex;
 }
 
 inline void log_domain_summary(
     const char *function_name,
     const char *relation_name,
     const fundamental_domain &domain,
     int voices,
     double range,
     double g,
     int sector,
     bool from_cache)
 {
     System::message(
         "%s<%s>: voices: %d range: %f g: %f sector: %d generated: %lld accepted: %lld size: %d%s\n",
         function_name,
         relation_name,
         voices,
         range,
         g,
         sector,
         domain.generated,
         domain.accepted,
         static_cast<int>(domain.chords.size()),
         from_cache ? " cached" : "");
 
     if (domain.expected_size >= 0 &&
         domain.expected_size != static_cast<int>(domain.chords.size()))
     {
         System::message(
             "Warning: %s<%s>: unexpected domain size: expected %d but got %d "
             "(voices: %d range: %f g: %f sector: %d).\n",
             function_name,
             relation_name,
             domain.expected_size,
             static_cast<int>(domain.chords.size()),
             voices,
             range,
             g,
             sector);
     }
 }
 
 inline fundamental_domain make_domain_from_chords(
     std::vector<Chord> chords,
     long long generated,
     long long accepted,
     int expected_size,
     double g)
 {
     fundamental_domain domain;
     domain.chords = std::move(chords);
     domain.generated = generated;
     domain.accepted = accepted;
     domain.expected_size = expected_size;
     domain.size_is_expected =
         expected_size < 0 ||
         expected_size == static_cast<int>(domain.chords.size());
     domain.rebuild_indices(g);
     return domain;
 }
 
 } // namespace fundamental_domain_detail
 
 template<int EQUIVALENCE_RELATION>
 const fundamental_domain &fundamentalDomainByEquateDomain(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme);
 
 template<int EQUIVALENCE_RELATION>
 const fundamental_domain &fundamentalDomainByPredicateDomain(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme);
 
 template<int EQUIVALENCE_RELATION>
 std::vector<Chord> fundamentalDomainByEquate_(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     const char *name =
         namesForEquivalenceRelations[EQUIVALENCE_RELATION];
 
     if (!(range > 0.0))
     {
         range = OCTAVE();
     }
 
     if (!(g > 0.0))
     {
         g = 1.0;
     }
 
     if (voiceN <= 0)
     {
         return {};
     }
 
     if (sector < 0 || sector >= voiceN)
     {
         System::message(
             "fundamentalDomainByEquate_<%s>: invalid sector %d for voices %d; using sector 0.\n",
             name,
             sector,
             voiceN);
 
         sector = 0;
     }
 
     const int steps =
         fundamental_domain_detail::temperament_steps(range, g);
 
     const int expected_size =
         fundamental_domain_detail::expected_domain_size<EQUIVALENCE_RELATION>(
             voiceN,
             steps);
 
     std::vector<Chord> chords;
     chords.reserve(
         expected_size > 0 ?
             static_cast<std::size_t>(expected_size) :
             static_cast<std::size_t>(1024));
 
     std::map<std::vector<long long>, std::size_t> index_by_lattice_key;
 
     long long generated = 0;
     long long accepted = 0;
 
     fundamental_domain_detail::enumerate_pitch_class_multisets(
         voiceN,
         steps,
         [&](const std::vector<int> &pcs)
         {
             Chord chord =
                 fundamental_domain_detail::chord_from_pitch_class_multiset(
                     pcs,
                     g);
 
             ++generated;
 
             Chord representative =
             chord.key<EQUIVALENCE_RELATION>(
                 range,
                 g,
                 sector);
 
             if (fundamental_domain_detail::add_unique_chord(
                     chords,
                     index_by_lattice_key,
                     representative,
                     g))
             {
                 ++accepted;
 
                 if (printme)
                 {
                     System::message(
                         "fundamentalDomainByEquate_<%s>: accepted: %6lld unique: %6d generated: %12lld sector: %d layer: %d input: %s output: %s\n",
                         name,
                         accepted,
                         static_cast<int>(chords.size()),
                         generated,
                         sector,
                         representative.tg_layer(g),
                         print_chord(chord).c_str(),
                         print_chord(representative).c_str());
                 }
             }
         });
 
     fundamental_domain_detail::sort_unique_domain(chords, g);
 
     System::message(
         "fundamentalDomainByEquate_<%s>: voices: %d range: %f g: %f sector: %d generated: %lld accepted: %lld size: %d\n",
         name,
         voiceN,
         range,
         g,
         sector,
         generated,
         accepted,
         static_cast<int>(chords.size()));
 
     if (expected_size >= 0 &&
         expected_size != static_cast<int>(chords.size()))
     {
         System::message(
             "Warning: fundamentalDomainByEquate_<%s>: unexpected domain size: expected %d but got %d "
             "(voices: %d range: %f g: %f sector: %d).\n",
             name,
             expected_size,
             static_cast<int>(chords.size()),
             voiceN,
             range,
             g,
             sector);
     }
 
     return chords;
 }
 
 /*
  * RPTIg / OPTIg fast path:
  *
  * The inversional domain is now defined as the minor half of the already
  * computed RPTg / OPTg domain. This avoids running geometric reflection for
  * every OPg pitch-class multiset.
  */
 template<>
 inline std::vector<Chord> fundamentalDomainByEquate_<EQUIVALENCE_RELATION_RPTIg>(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     const char *name =
         namesForEquivalenceRelations[EQUIVALENCE_RELATION_RPTIg];
 
     if (!(range > 0.0))
     {
         range = OCTAVE();
     }
 
     if (!(g > 0.0))
     {
         g = 1.0;
     }
 
     if (voiceN <= 0)
     {
         return {};
     }
 
     if (sector < 0 || sector >= voiceN)
     {
         System::message(
             "fundamentalDomainByEquate_<%s>: invalid sector %d for voices %d; using sector 0.\n",
             name,
             sector,
             voiceN);
 
         sector = 0;
     }
 
     const fundamental_domain &rptg_domain =
         fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION_RPTg>(
             voiceN,
             range,
             g,
             sector,
             false);
 
     std::vector<Chord> chords;
     chords.reserve(rptg_domain.chords.size());
 
     std::map<std::vector<long long>, std::size_t> index_by_lattice_key;
 
     for (const Chord &chord : rptg_domain.chords)
     {
         // Build the musical inversion in T-space: negate pitches and reverse
         // voice order so that the sorted T-normal form of a chord {c0...cn-1}
         // maps to the T-normal form of its negative {-cn-1...-c0}.
         const int n = chord.voices();
         Chord inv_chord(n);
         for (int v = 0; v < n; ++v)
         {
             inv_chord.setPitch(v, -chord.getPitch(n - 1 - v));
         }
 
         // Get the RPTg canonical representative of the inversion.
         const Chord inv_rptg =
             equate<EQUIVALENCE_RELATION_RPTg>(inv_chord, range, g, sector);
 
         // Include this chord iff it is the lexicographically smaller member
         // of the pair {chord, inv_rptg}.  Self-inverse chords (chord == inv_rptg)
         // are always included.  This selection is equivalent to the OPTI
         // convention "keep the minor representative" but avoids dependence on
         // the inversion-flat hyperplane, which can misclassify some chords
         // after cyclic revoicing canonicalization.
         if (chord <= inv_rptg)
         {
             fundamental_domain_detail::add_unique_chord(
                 chords,
                 index_by_lattice_key,
                 chord.eET(g),
                 g);
         }
     }
 
     fundamental_domain_detail::sort_unique_domain(chords, g);
 
     const int steps =
         fundamental_domain_detail::temperament_steps(range, g);
 
     const int expected_size =
         fundamental_domain_detail::expected_domain_size<EQUIVALENCE_RELATION_RPTIg>(
             voiceN,
             steps);
 
     System::message(
         "fundamentalDomainByEquate_<%s>: voices: %d range: %f g: %f sector: %d generated: %lld accepted: %lld size: %d\n",
         name,
         voiceN,
         range,
         g,
         sector,
         static_cast<long long>(rptg_domain.chords.size()),
         static_cast<long long>(chords.size()),
         static_cast<int>(chords.size()));
 
     if (expected_size >= 0 &&
         expected_size != static_cast<int>(chords.size()))
     {
         System::message(
             "Warning: fundamentalDomainByEquate_<%s>: unexpected domain size: expected %d but got %d "
             "(voices: %d range: %f g: %f sector: %d).\n",
             name,
             expected_size,
             static_cast<int>(chords.size()),
             voiceN,
             range,
             g,
             sector);
     }
 
     if (printme)
     {
         for (std::size_t index = 0; index < chords.size(); ++index)
         {
             System::message(
                 "fundamentalDomainByEquate_<%s>: chord[%6d]: %s\n",
                 name,
                 static_cast<int>(index),
                 print_chord(chords[index]).c_str());
         }
     }
 
     return chords;
 }
 
 template<int EQUIVALENCE_RELATION>
 std::vector<Chord> fundamentalDomainByPredicate_(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     const char *name =
         namesForEquivalenceRelations[EQUIVALENCE_RELATION];
 
     /*
      * Fast path: the predicate domain should be the cached equate-domain
      * image. Validate only these representatives, not all OPg seeds.
      */
     const fundamental_domain &equate_domain =
         fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION>(
             voiceN,
             range,
             g,
             sector,
             printme);
 
     std::vector<Chord> chords;
     chords.reserve(equate_domain.chords.size());
 
     std::map<std::vector<long long>, std::size_t> index_by_lattice_key;
 
     long long accepted = 0;
 
     for (const Chord &chord : equate_domain.chords)
     {
         const bool in_domain =
             predicate<EQUIVALENCE_RELATION>(
                 chord,
                 range,
                 g,
                 sector);
 
         if (in_domain)
         {
             if (fundamental_domain_detail::add_unique_chord(
                     chords,
                     index_by_lattice_key,
                     chord,
                     g))
             {
                 ++accepted;
             }
         }
         else
         {
             System::message(
                 "Warning: fundamentalDomainByPredicate_<%s>: equate-domain chord does not satisfy predicate "
                 "(sector: %d chord: %s).\n",
                 name,
                 sector,
                 print_chord(chord).c_str());
         }
     }
 
     fundamental_domain_detail::sort_unique_domain(chords, g);
 
     System::message(
         "fundamentalDomainByPredicate_<%s>: voices: %d range: %f g: %f sector: %d generated: %lld accepted: %lld size: %d\n",
         name,
         voiceN,
         range,
         g,
         sector,
         static_cast<long long>(equate_domain.chords.size()),
         accepted,
         static_cast<int>(chords.size()));
 
     return chords;
 }
 
 template<>
 inline std::vector<Chord> fundamentalDomainByPredicate_<EQUIVALENCE_RELATION_RPTIg>(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     (void)printme;
 
     /*
      * Do not call predicate<RPTIg> on every representative here. The domain
      * is exactly the minor-half filter of RPTg, computed by the RPTIg equate
      * specialization above.
      */
     return fundamentalDomainByEquate_<EQUIVALENCE_RELATION_RPTIg>(
         voiceN,
         range,
         g,
         sector,
         false);
 }
 
 template<int EQUIVALENCE_RELATION>
 const fundamental_domain &fundamentalDomainByEquateDomain(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     const char *name =
         namesForEquivalenceRelations[EQUIVALENCE_RELATION];
 
     if (!(range > 0.0))
     {
         range = OCTAVE();
     }
 
     if (!(g > 0.0))
     {
         g = 1.0;
     }
 
     if (voiceN <= 0)
     {
         static fundamental_domain empty_domain;
         return empty_domain;
     }
 
     if (sector < 0 || sector >= voiceN)
     {
         sector = 0;
     }
 
     const fundamental_domain_cache_key key =
         fundamental_domain_detail::make_cache_key(
             EQUIVALENCE_RELATION,
             voiceN,
             range,
             g,
             sector,
             fundamental_domain_generation_mode::by_equate);
 
     {
         std::lock_guard<std::mutex> lock(
             fundamental_domain_detail::domain_cache_mutex());
 
         auto it =
             fundamental_domain_detail::domain_cache().find(key);
 
         if (it != fundamental_domain_detail::domain_cache().end())
         {
             return it->second;
         }
     }
 
     std::vector<Chord> chords =
         fundamentalDomainByEquate_<EQUIVALENCE_RELATION>(
             voiceN,
             range,
             g,
             sector,
             printme);
 
     const int steps =
         fundamental_domain_detail::temperament_steps(range, g);
 
     const int expected_size =
         fundamental_domain_detail::expected_domain_size<EQUIVALENCE_RELATION>(
             voiceN,
             steps);
 
     const long long generated =
         fundamental_domain_detail::binomial_ll(
             steps + voiceN - 1,
             voiceN);
 
     const long long accepted =
         static_cast<long long>(chords.size());
 
     fundamental_domain domain =
         fundamental_domain_detail::make_domain_from_chords(
             std::move(chords),
             generated,
             accepted,
             expected_size,
             g);
 
     std::lock_guard<std::mutex> lock(
         fundamental_domain_detail::domain_cache_mutex());
 
     auto existing =
         fundamental_domain_detail::domain_cache().find(key);
 
     if (existing != fundamental_domain_detail::domain_cache().end())
     {
         return existing->second;
     }
 
     auto result =
         fundamental_domain_detail::domain_cache().emplace(
             key,
             std::move(domain));
 
     fundamental_domain_detail::log_domain_summary(
         "fundamentalDomainByEquate",
         name,
         result.first->second,
         voiceN,
         range,
         g,
         sector,
         false);
 
     return result.first->second;
 }
 
 template<int EQUIVALENCE_RELATION>
 const fundamental_domain &fundamentalDomainByPredicateDomain(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     const char *name =
         namesForEquivalenceRelations[EQUIVALENCE_RELATION];
 
     if (!(range > 0.0))
     {
         range = OCTAVE();
     }
 
     if (!(g > 0.0))
     {
         g = 1.0;
     }
 
     if (voiceN <= 0)
     {
         static fundamental_domain empty_domain;
         return empty_domain;
     }
 
     if (sector < 0 || sector >= voiceN)
     {
         sector = 0;
     }
 
     const fundamental_domain_cache_key key =
         fundamental_domain_detail::make_cache_key(
             EQUIVALENCE_RELATION,
             voiceN,
             range,
             g,
             sector,
             fundamental_domain_generation_mode::by_predicate);
 
     {
         std::lock_guard<std::mutex> lock(
             fundamental_domain_detail::domain_cache_mutex());
 
         auto it =
             fundamental_domain_detail::domain_cache().find(key);
 
         if (it != fundamental_domain_detail::domain_cache().end())
         {
             return it->second;
         }
     }
 
     std::vector<Chord> chords =
         fundamentalDomainByPredicate_<EQUIVALENCE_RELATION>(
             voiceN,
             range,
             g,
             sector,
             printme);
 
     const int steps =
         fundamental_domain_detail::temperament_steps(range, g);
 
     const int expected_size =
         fundamental_domain_detail::expected_domain_size<EQUIVALENCE_RELATION>(
             voiceN,
             steps);
 
     const long long generated =
         static_cast<long long>(chords.size());
 
     const long long accepted =
         static_cast<long long>(chords.size());
 
     fundamental_domain domain =
         fundamental_domain_detail::make_domain_from_chords(
             std::move(chords),
             generated,
             accepted,
             expected_size,
             g);
 
     std::lock_guard<std::mutex> lock(
         fundamental_domain_detail::domain_cache_mutex());
 
     auto existing =
         fundamental_domain_detail::domain_cache().find(key);
 
     if (existing != fundamental_domain_detail::domain_cache().end())
     {
         return existing->second;
     }
 
     auto result =
         fundamental_domain_detail::domain_cache().emplace(
             key,
             std::move(domain));
 
     fundamental_domain_detail::log_domain_summary(
         "fundamentalDomainByPredicate",
         name,
         result.first->second,
         voiceN,
         range,
         g,
         sector,
         false);
 
     return result.first->second;
 }
 
 template<int EQUIVALENCE_RELATION>
 const std::vector<Chord> &fundamentalDomainByEquate(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     const fundamental_domain &domain =
         fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION>(
             voiceN,
             range,
             g,
             sector,
             printme);
 
     return domain.chords;
 }
 
 template<int EQUIVALENCE_RELATION>
 const std::vector<Chord> &fundamentalDomainByPredicate(
     int voiceN,
     double range,
     double g,
     int sector,
     bool printme)
 {
     const fundamental_domain &domain =
         fundamentalDomainByPredicateDomain<EQUIVALENCE_RELATION>(
             voiceN,
             range,
             g,
             sector,
             printme);
 
     return domain.chords;
 }
 
 template<int EQUIVALENCE_RELATION>
 const Chord *fundamentalDomainChordForIndex(
     int voiceN,
     double range,
     double g,
     int sector,
     bool by_predicate,
     std::size_t index)
 {
     const fundamental_domain &domain =
         by_predicate ?
             fundamentalDomainByPredicateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false) :
             fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false);
 
     return domain.chord_for_index(index);
 }
 
 template<int EQUIVALENCE_RELATION>
 const Chord *fundamentalDomainChordForName(
     int voiceN,
     double range,
     double g,
     int sector,
     bool by_predicate,
     const std::string &name)
 {
     const fundamental_domain &domain =
         by_predicate ?
             fundamentalDomainByPredicateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false) :
             fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false);
 
     return domain.chord_for_name(name);
 }
 
 template<int EQUIVALENCE_RELATION>
 const Chord *fundamentalDomainChordForToString(
     int voiceN,
     double range,
     double g,
     int sector,
     bool by_predicate,
     const std::string &text)
 {
     const fundamental_domain &domain =
         by_predicate ?
             fundamentalDomainByPredicateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false) :
             fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false);
 
     return domain.chord_for_to_string(text);
 }
 
 template<int EQUIVALENCE_RELATION>
 int fundamentalDomainIndexForChord(
     int voiceN,
     double range,
     double g,
     int sector,
     bool by_predicate,
     const Chord &chord)
 {
     const fundamental_domain &domain =
         by_predicate ?
             fundamentalDomainByPredicateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false) :
             fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false);
 
     return domain.index_for_chord(chord);
 }
 
 template<int EQUIVALENCE_RELATION>
 int fundamentalDomainIndexForName(
     int voiceN,
     double range,
     double g,
     int sector,
     bool by_predicate,
     const std::string &name)
 {
     const fundamental_domain &domain =
         by_predicate ?
             fundamentalDomainByPredicateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false) :
             fundamentalDomainByEquateDomain<EQUIVALENCE_RELATION>(
                 voiceN,
                 range,
                 g,
                 sector,
                 false);
 
     return domain.index_for_name(name);
 }
 
 /*
  * ChordSpaceTests.cpp timing gate:
  *
  * Put this near the other test-printing constants, then wrap expensive domain
  * output, printPass, printPitv, and detailed printSet calls in
  * `if constexpr (print_domains)`.
  */
 constexpr bool print_domains = false;
 
 /*
  * Typical replacements in ChordSpaceTests.cpp:
  *
  *     constexpr bool printPass = print_domains;
  *     constexpr bool printPitv = print_domains;
  *
  *     if constexpr (print_domains)
  *     {
  *         printSet(...);
  *     }
  *
  * If printSet itself has detailed per-chord output, guard that detail with
  * `if constexpr (print_domains)` as well.
  */
 
#pragma endregion


class SILENCE_PUBLIC ChordScore;

/**
 * How notes governed by a harmony timeline entry are conformed at \ref
 * ChordScore::conformToChords.
 */
enum class HarmonyConformMode {
    /** Legacy insertChord: use the global \c octave_equivalence flag in conformToChords. */
    Default = 0,
    /** Quantize to pitch-classes of the reference chord (\c Hc). */
    Hc = 1,
    /** Quantize to the actual pitches of the reference chord (\c Hcv). */
    Hcv = 2,
    /** Voice-lead from sounding score notes to pitch-classes of the reference (\c Hcs). */
    Hcs = 3,
    /** Quantize to pitch-classes at a scale degree (\c Hd). */
    Hd = 4,
    /** Voice-lead from sounding score notes to functional harmony at a scale degree (\c Hds). */
    Hds = 5,
};

/**
 * One harmony constraint on the timeline: a reference chord or scale context,
 * plus the conform mode that applies until the next entry.
 */
struct SILENCE_PUBLIC HarmonyEntry {
    Chord chord;
    HarmonyConformMode mode;
    Scale scale;
    int scale_degree;
    int voices;
    double voice_leading_range;
    HarmonyEntry();
};

SILENCE_PUBLIC const std::vector<Chord> &allOfEquivalenceClass(int voice_count, std::string equivalence_class, double range, double g, int sector, bool printme);

SILENCE_PUBLIC void apply(Score &score, const Chord &chord, double startTime, double endTime, bool octaveEquivalence = true);

/**
 * Score equipped with chords. The notes in the score may be conformed to the
 * chord that obtains at the time of the notes. The times and durations of
 * notes and chords are rescaled together. This is done by finding minimum and
 * maximum times by counting both note times and chord times.
 */
class SILENCE_PUBLIC ChordScore : public Score {
public:
    std::map<double, HarmonyEntry> harmonies_for_times;
    /**
     * Conforms each note using the harmony entry from getHarmony at that note's
     * onset time. The \c octave_equivalence flag applies only to entries
     * inserted with insertChord(time, chord) and mode Default.
     */
    virtual void conformToChords(bool tie_overlaps, bool octave_equivalence = true);
    /**
     * Returns the most recent timeline chord at or before \c time_, or if
     * none exists, the soonest chord after \c time_. Null if the timeline is
     * empty.
     */
    virtual Chord *getChord(double time_);
    /**
     * Returns the most recent harmony entry at or before \c time_, or if none
     * exists, the soonest entry after \c time_. Null if the timeline is empty.
     */
    virtual HarmonyEntry *getHarmony(double time_);
    virtual double getDuration();
    void getScale(std::vector<Event> &score, int dimension, size_t beginAt, size_t endAt, double &minimum, double &range);
    /** Records a reference chord on the harmony timeline with mode Default. */
    virtual void insertChord(double tyme, const Chord chord);
    /**
     * Records a reference chord on the harmony timeline with an explicit
     * conform mode (\c Hc, \c Hcv, or \c Hcs).
     */
    virtual void insertChord(double tyme,
                             const Chord &chord,
                             HarmonyConformMode mode,
                             int voices = -1,
                             double voice_leading_range = OCTAVE());
    /**
     * Records functional harmony at \c scale_degree of \c scale with mode
     * \c Hd or \c Hds.
     */
    virtual void insertFunctionalHarmony(double tyme,
                                         const Scale &scale,
                                         int scale_degree,
                                         int voices,
                                         HarmonyConformMode mode,
                                         double voice_leading_range = OCTAVE());
    /**
     * Returns pitches for voice-leading from the score segment between
     * \c prior_harmony_time and \c time. Notes still sounding at \c time are
     * returned first; if \c voices is positive and there are fewer such notes,
     * the remainder are the most recently ended notes from that segment.
     */
    static Chord gatherSoundingChord(const Score &score,
                                   double prior_harmony_time,
                                   double time);
    static Chord gatherSoundingChord(const Score &score,
                                   double prior_harmony_time,
                                   double time,
                                   int voices);
    virtual void setDuration(double targetDuration);
    void setScale(std::vector<Event> &score,
                   int dimension,
                   bool rescaleMinimum,
                   bool rescaleRange,
                   size_t beginAt,
                   size_t endAt,
                   double targetMinimum,
                   double targetRange);
};

/**
 * If the Event is a note, moves its pitch to the closest pitch of the chord.
 * If octaveEquivalence is true (the default), the pitch-class of the note is 
 * moved to the closest pitch-class of the chord, i.e. keeping the note more 
 * or less in its original register; otherwise, the pitch of the note is moved 
 * to the closest absolute pitch of the chord.
 */
SILENCE_PUBLIC void conformToChord_equivalence(Event &event, const Chord &chord, bool octaveEquivalence);

SILENCE_PUBLIC void conformToChord(Event &event, const Chord &chord);

/**
 * Returns a chord containing all the pitches of the score beginning at or 
 * later than the start time, and up to but not including the end time.
 */
SILENCE_PUBLIC Chord gather(Score &score, double startTime, double endTime);

/**
 * Inserts the notes of the chord into the score at the specified time.
 */
SILENCE_PUBLIC void insert(Score &score,
                                  const Chord &chord,
                                  double time_,
                                  bool voice_is_instrument);
                                  
SILENCE_PUBLIC void insert(Score &score,
                                  const Chord &chord,
                                  double time_);
                                  
/**
 * Creates a complete "note on" Event for the indicated voice of the 
 * chord. If the optional duration, channel, velocity, and pan parameters
 * are not passed, then the Chord's own values for these are used.
 */
SILENCE_PUBLIC Event note(const Chord &chord, 
    int voice,
    double time_,
    double duration_ = DBL_MAX,
    double channel_ = DBL_MAX,
    double velocity_ = DBL_MAX,
    double pan_ = DBL_MAX);

/**
 * Returns an individual note for each voice of the chord. If the optional
 * duration, channel, velocity, and pan parameters are not passed, then 
 * the Chord's own values for these are used.
 */
SILENCE_PUBLIC Score notes(const Chord &chord,
    double time_,
    double duration_ = DBL_MAX,
    double channel_ = DBL_MAX,
    double velocity_ = DBL_MAX,
    double pan_ = DBL_MAX);
    
SILENCE_PUBLIC void numerics_information(double a, double b, int epsilons, int ulps);

/**
 * Returns a slice of the Score starting at the start time and extending up
 * to but not including the end time. The slice contains pointers to the Events
 * in the Score.
 */
SILENCE_PUBLIC std::vector<Event *> slice(Score &score, double startTime, double endTime);

SILENCE_PUBLIC void toScore(const Chord &chord, 
    Score &score,
    double time_, bool voiceIsInstrument);
    

} // End of namespace csound.

#pragma GCC diagnostic push
  
#endif
