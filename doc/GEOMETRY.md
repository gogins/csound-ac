# GEOMETRY OF CHORD SPACE

Here I clarify and define the relationship in chord space between the equivalence relations OP (octave equivalence and permutational equivalence), OPT (OP plus transpositional equivalence), and OPTI (OPT plus inversional equivalence). I also consider OPTg (OPT quantized by g) and OPTgI (OPTI quantized by g)

First, pitch is MIDI key numbers but can be fractional. The octave thus is 12. The generator of transposition is g, which in 12-tone equal temperament is 1, but can be any integral or real fraction of 12.

We are concerned with defining the vertices of polytopes that can be used to test membership in representative regions of chord space, and thus to determine representatives of equivalence classes.

Let N be the dimensionality of a chord space. Under OP equivalence, chord space is an equilateral hyperprism with N side facets and height 12 / N, and its axis is the unison diagonal. Under the action of transposition, a chord rises (or falls) parallel to the axis, and when it passes the top or bottom base of the prism, it folds anticlockwise (or clockwise) back into bottom or top base of the next (or previous) sector. This defines a cyclical region that divides the OP prism into N sectors. Each of these sectors is an isoceles hyperprism. Each OP sector is divided in half by a hyperplane defined by the unison diagonal and the midpoint of that sector's base face of the prism. This is the inversion flat. Under the action of inversion, a chord reflects from one half of the OP sector to the other half.

Chords falling on shared boundaries of these polytopes belong to each of the adjacent polytopes. In testing these chords for belonging to a polytope, there must be a deterministic tie-breaking rule: select the chord with the lowest sector index in that polytope.

So, we have the following set of polytopes:

OP, the hyperprism that is the fundamental domain of O/P.

OPT, the base of the OP domain. Each layer of OP is a fundamental domain of OP/T. We choose the base layer of the OP hyperprism as the representative fundamental domain of OPT equivalence. OPT’s fundamental domain is the union of all OPT sectors; sectors are tiles, not fundamental domains by themselves.

Each OPT sector has two OPTI sectors, divided by an inversion flat. We choose the union of the N minor OPTI sectors as one fundamental domain of OPTI equivalence, and the union of the N major OPTI sectors as a second fundamental domain of OPTI; we choose the minor domain with the lowest sector index as the representative fundamental domain of OPTI equivalence.

The OPT sectors, the OPTI sectors, and the inversion flats can be extruded from the base to the top of the OP hyperprism to define polytopes representing the OPI fundamental domain and the full inversion flats.

Predicates are implemented strictly geometrically: is the chord in the specified polytope? Chords on the boundaries of that polytope count, even if they also belong to another polytope.

Equates are implemented strictly as iterating the relevant group action until the result is within the specified polytope.

The unit tests must satisfy:

 1. Idempotence: `equate<R>(equate<R>(chord, sector), sector) == equate<R>(chord, sector)`.

 2. Consistency: `predicate<R>(equate<R>(chord, sector), sector) == true`.
  
 3. Each predicate for a compound equivalence relations must return the same truth value as the predicate for each of the elementary relations in that compound.

 4. The count of chords in OPTg and OPTgI must agree with Tymoczko, and the visualization must show the actual cyclical region of N isoceles hyperprisms for OPTg and the N right hyperism minor halves of that geometry for OPTgI.

 5. Consequently there must be two predicates, one for idempotence and one for counting the unique representatives in a fundamental domain; the first is fine with boundary duplicates, the second selects that boundary duplicate with the lowest sector index.
