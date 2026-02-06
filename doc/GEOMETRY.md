# GEOMETRY OF CHORD SPACE EQUIVALENCE RELATIONS

Michael Gogins
Irreducible Productions


Here I clarify and define the relationship in chord space between the equivalence relations OP (octave equivalence plus permutational equivalence), OPT (OP plus transpositional equivalence), and OPTI (OPT plus inversional equivalence). I also consider OPTg (OPT quantized by equal temperament) and OPTgI (OPTI quantized by equal temperament).

Pitch is represented as MIDI key numbers and may be fractional. The octave is therefore 12. The generator of transposition is g, which in 12-tone equal temperament is 1, but may be any integral or real fraction of 12.

We are concerned with defining the vertices of convex polytopes that can be used to test membership in representative regions of chord space, and thus to determine representatives of equivalence classes.

Let N be the dimensionality of a chord space. Under OP equivalence, chord space is an equilateral hyperprism with N side facets and height 12 / N, whose axis is the unison diagonal. Under the action of transposition, a chord rises (or falls) parallel to this axis, and when it passes the top (or bottom) base of the prism, it folds anti-sectorwise (or sectorwise) back into the bottom (or top) base of the adjacent sector. This defines a cyclical region that divides the OP prism into N sectors. Each sector is an isosceles hyperprism.

Each OP sector is divided in half by a hyperplane defined by the unison diagonal and the midpoint of that sector’s base face. This hyperplane is the inversion flat. Under the action of inversion, a chord reflects from one half of the OP sector to the other.

Chords falling on shared boundaries of these polytopes belong to all adjacent polytopes. Thus chords on boundaries may satisfy predicates for multiple equivalence domains. Since this API supports sector-indexed operations, `equate<R>(chord, sector)` uses sector as a target selector and must return a representative in the target domain. Tie-breaking is not required for counting under global scans; it is only needed if an ownership notion is introduced.

We therefore define the following polytopes:

_OP_: the full hyperprism, which is the fundamental domain of octave and permutation equivalence.

_OPT_: the base of the OP hyperprism. Each layer of OP is a fundamental domain of OP/T; we choose the base layer as the representative of the OPT fundamental domain. The OPT fundamental domain is the union of all OPT sectors; individual sectors are tiles, not fundamental domains by themselves.

_OPTI_: each OPT sector is divided by an inversion flat into two OPTI sectors. There are two possible OPTI fundamental domains (the union of all “minor” halves or the union of all “major” halves); we choose the minor domain with the lowest sector index as the representative OPTI fundamental domain.

The OPT sectors, OPTI sectors, and inversion flats may be extruded along the unison diagonal to define the corresponding OPI domains and full inversion flats in OP space.

Predicates are implemented strictly geometrically: a predicate tests whether a chord lies in a specified polytope. Chords on polytope boundaries satisfy the predicate even if they also satisfy the predicate in other sectors.

Equates are implemented by applying the relevant group action (iteratively or analytically) until the result lies within the chosen representative polytope.

The unit tests must satisfy:

 1. _Idempotence_: `equate<R>(equate<R>(chord, sector), sector) == equate<R>(chord, sector)`.

 2. _Consistency_: `predicate<R>(equate<R>(chord, sector), sector) == true`.

 3. _Decomposability_: The predicate for a compound equivalence relation must return the same truth value as the conjunction of predicates for each elementary relation.
 
 4. _Membership (representatives)_: All representative chords (one per equivalence class) can be assembled into a vector by globally iterating over a space enclosing the _representative_ fundamental domain, and inserting each chord that satisfies the __union membership predicate_ for that domain.  The union predicate is satisfied if the sector predicate is satisfied for one or more sectors in the domain.

    -  _OPT_: The chord lies in any OPT sector.

    -  _OPTg_: OPT membership, and the chord is on the g-lattice.

    -  _OPTI_: The chord lies in the minor half of any OPT sector.
  
    -  _OPTgI_: OPTI membership, and the chord is on the g-lattice.

 5. _Correct cardinality_: The counts of OPTg and OPTgI representatives must agree with Tymoczko, and visualizations must show the expected cyclical regions and their inversional subdivisions.
