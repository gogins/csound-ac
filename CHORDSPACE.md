# CHORD SPACE

Michael Gogins  
Irreducible Productions

## Purpose

The chord space library in CsoundAC, declared in `ChordSpace.hpp`, provides mathematically based code for wworking with chords, scales, neo-Riemannian transformations of chords, and voice-leading in the context of algorithmic composition. The CsoundAC library, including the chord space code, has interfaces in C++, Python, and JavaScript.

The chord space code has recently been changed to cleanly separate:

- **continuous geometry**, defined in chord spaces whose coordinates are real-valued pitches; from
- **discrete geometry induced on a lattice**, defined by quantizing continuous pitches to an equally tempered lattice with step size `g` that evenly divides the period (usually the octave) of pitch equivalence.

---

## Representation

### Pitches

Pitches and intervals are represented as real numbers. Middle C is `60`. The perid of octave equivalence defined as `12` semitones. Thus pitches in chord space are consistent with the MIDI specification, with the exception that pitches in chord space may take fractional values.

Whole numbers represent equal temperament. Fractions represent microtonal or otherwise continuous pitch values.

### Voices and chords

A **voice** is a distinct pitched sound.

A **chord** is an ordered tuple of voices, represented as a point in an `N`-dimensional chord space, one dimension per voice.

### Pitch equivalence

The period of pitch equivalence is defined as `12.0` semitones, i.e., the octave.

This is not merely a convenience for MIDI compatibility. It is a foundational assumption of the current theory and implementation:

- octave equivalence is defined modulo `12`;
- OP, OPT, and OPTI geometry are constructed in octave-equivalence space;
- voice-leading, sector structure, and inversion flats all assume octave equivalence.

---

## Continuous and discrete chord spaces

### Continuous chord space

Continuous chord space is the real-valued space `R^N` modulo whichever equivalence relations are applied. In this space, pitches may take any real values, including fractional values.

### Lattice-induced discrete chord space

Discrete chord space is **not** a separate storage type. It is the subset of continuous chord space whose coordinates lie on an equally tempered lattice.

Let `g > 0` be the lattice step size. Then a chord lies on the `g`-lattice if each voice is an integer multiple of `g`, within tolerance. To be musically well-formed in this library, `g` must evenly divide the the octave (12 semitones):

```text
12 / g ∈ N
```

Examples:

- `g = 1` gives 12-tone equal temperament.
- `g = 1/2` gives 24-tone equal temperament.
- `g = 12/19` gives 19 equal divisions of the octave.
- `g = 2` gives a 6-step whole-tone lattice.

The voices of `Chord` therefore remain floating-point numbers in all cases. Discrete geometry is induced by projecting continuous chords onto exact multiples of `g`.

---

## Definitions

### Plain chord space

Plain chord space has no equivalence relation. Ordered chords are represented as vectors in parentheses `(p1, ..., pN)`. Unordered chords are represented as sorted vectors in braces `{p1, ..., pN}`.

### Cardinality equivalence

Cardinality equivalence is **not** assumed. Chord spaces are of fixed dimension. A one-note sonority may therefore be represented as `{60}`, `{60, 60}`, `{60, 60, 60}`, and so on, depending on the dimensionality of the space.

This preserves a proto-metric in plain chord space that descends to child chord spaces.

### Scale

A scale is a chord whose first and lowest voice is a tonic pitch-class and whose other voices are pitch-classes in ascending order.

### Score

For algorithmic composition, a score may be treated as a sequence of more or less fleeting chords.

---

## Equivalence relations

An equivalence relation identifies different elements of a set as belonging to the same class. Operations that send elements to their equivalents induce quotient spaces or orbifolds, where identified facets of the space are glued together.

In most cases, a chord space can be divided into many geometrically equivalent fundamental domains for the same equivalence relation. This API therefore uses the notion of a **representative fundamental domain**.

The following equivalence relations are defined.

### C

Cardinality equivalence. Not assumed in this library.

### O

Octave equivalence. Pitches differing by integer multiples of `12` are equivalent.

### P

Permutational equivalence. Chords that differ only by reordering of voices are equivalent. The representative fundamental domain is the wedge in which voices are sorted in ascending order.

### T

Continuous transpositional equivalence. Chords that differ by translation parallel to the unison diagonal are equivalent. The representative fundamental domain is the hyperplane orthogonal to the unison diagonal with sum of pitches equal to `0`.

### I

Continuous inversional equivalence. Here inversion is used in the mathematician’s sense: reflection in a point or hyperplane, not registral revoicing. In the chord-space orbifolds used here, inversion is represented geometrically by reflection in an inversion flat.

### OP

Octave equivalence plus permutational equivalence. This is Tymoczko’s orbifold for chords of fixed cardinality in harmonic context.

For `N` voices, the OP fundamental domain is an equilateral hyperprism with `N` side facets and height `12 / N` along the unison direction.

### OPT

OP plus continuous transpositional equivalence. The representative OPT fundamental domain is chosen as the base layer of the OP hyperprism. The full OPT domain is the union of all OPT sectors; any individual sector is a tile of that domain, not the whole fundamental domain.

### OPTI

OPT plus continuous inversional equivalence. Each OPT sector is divided into two halves by an inversion flat. Two possible global OPTI domains exist, corresponding informally to the “minor” and “major” halves; this API chooses the minor domain with the lowest sector index as the representative OPTI fundamental domain.

### Lattice-induced relations

For any continuous equivalence relation `EC`, the corresponding lattice-induced relation is written `ECg`. Its representative domain is the intersection of the representative continuous domain for `EC` with the `g`-lattice.

Examples:

- `OPTg`: OPT representatives that also lie on the `g`-lattice.
- `Ig`: inversional representatives constrained to the `g`-lattice.
- `OPTIg`: OPTI representatives constrained to the `g`-lattice.

These are **not** separate spaces with separate storage types. They are induced by predicates and projections defined on `Chord`.

---

## Geometry of OP, OPT, and OPTI

Let `N` be the dimensionality of the chord space.

### OP

Under OP equivalence, chord space is an equilateral hyperprism with `N` side facets and height `12 / N` along the unison axis.

A chord translated parallel to the unison diagonal rises or falls through the prism. When it passes through the top or bottom base, octave equivalence folds it back into the adjacent sector. This produces a cyclical region that divides OP into `N` sectors. Each OP sector is an isosceles hyperprism.

### OPT

OPT is the base layer of the OP hyperprism. The OPT representative fundamental domain is the union of all OPT sectors in that layer.

To determine the sector of a chord in OP:

1. project the chord onto OPT;
2. compute the Euclidean distance from the chord to the centroid of each sector’s base face;
3. assign the chord to any sector whose centroid is minimal in distance.

Boundary points may satisfy this criterion for more than one sector.

### OPTI

Each OPT sector is divided into two halves by a hyperplane through the center of the cyclical region and the centroid of that sector’s base face. This hyperplane is the inversion flat.

Under continuous inversion, a chord reflects across this inversion flat from one half-sector to the other.

The OPTI representative fundamental domain is the union of the chosen half-sectors over all sectors.

### Extrusions in OP

OPT sectors, OPTI sectors, and inversion flats can be extruded along the unison diagonal to define corresponding OPI domains and full inversion flats in OP space.

---

## Boundary behavior

Chords lying on shared boundaries of these polytopes belong to all adjacent polytopes.

Therefore:

- predicates are **closed-set** tests;
- a boundary chord may satisfy predicates for more than one sector;
- no unique sector ownership is required unless explicitly needed for counting or labeling.

Sector-indexed operations therefore use the sector argument as a **target selector**, not as a proof that the chord belongs exclusively to that sector.

Tie-breaking is not required merely to define membership. It is only required if an ownership notion is introduced.

---

## Predicates and projections

Predicates are implemented geometrically:

- `ise<EC>(...)` returns `true` exactly when the chord lies in the chosen representative polytope for `EC`.
- Boundary points satisfy the predicate.

Projections are implemented by applying the relevant group action, analytically or iteratively, until the chord lies in the chosen representative polytope.

### Continuous rule

For continuous equivalence classes, predicates and projections are defined entirely in the continuous orbifold geometry.

### Lattice rule

For lattice-induced equivalence classes, the implementation rule is:

1. compute the representative in the corresponding continuous geometry;
2. project that representative onto the `g`-lattice;
3. accept boundary points that lie in more than one sector;
4. require the result to be idempotent and to satisfy the corresponding predicate.

This rule applies in particular to `Ig`, `OPTg`, and `OPTIg`.

---

## API

The public API retains the existing member-function pattern:

- `Chord::ise<EC>(...)` is a predicate;
- `Chord::e<EC>(...)` is a projection.

The following conventions apply.

### Continuous API

```cpp
bool Chord::iseO() const;
Chord Chord::eO() const;

bool Chord::iseP() const;
Chord Chord::eP() const;

bool Chord::iseT(int sector = -1) const;
Chord Chord::eT(int sector = -1) const;

bool Chord::iseI(int sector = -1) const;
Chord Chord::eI(int sector = -1) const;

bool Chord::iseOP(int sector = -1) const;
Chord Chord::eOP(int sector = -1) const;

bool Chord::iseOPT(int sector = -1) const;
Chord Chord::eOPT(int sector = -1) const;

bool Chord::iseOPTI(int sector = -1) const;
Chord Chord::eOPTI(int sector = -1) const;
```

### Lattice helpers

The following lattice helper functions are added:

```cpp
bool Chord::iseLattice(double g = 1.0) const;
Chord Chord::eLattice(double g = 1.0) const;
```

`iseLattice(g)` returns `true` if and only if every voice lies on the `g`-lattice, within tolerance.

`eLattice(g)` snaps every voice independently to the nearest multiple of `g`.

### Lattice-induced inversion

The following lattice-induced inversion functions are added:

```cpp
bool Chord::iseIg(double g = 1.0, int sector = -1) const;
Chord Chord::eIg(double g = 1.0, int sector = -1) const;
```

Semantics:

- `iseIg(g, sector)` is true when the chord lies in the chosen representative inversion domain **and** lies on the `g`-lattice.
- `eIg(g, sector)` computes the continuous inversional representative and then projects the result onto the `g`-lattice.

### General lattice-induced API

For every continuous equivalence class `EC`, the lattice-induced version follows the same pattern:

```cpp
bool Chord::iseECg(double g = 1.0, int sector = -1) const;
Chord Chord::eECg(double g = 1.0, int sector = -1) const;
```

Semantics:

- `iseECg(g, sector)` is true if and only if the chord lies in the representative domain for `EC` and lies on the `g`-lattice.
- `eECg(g, sector)` computes the continuous representative for `EC` and then projects it to the `g`-lattice.

Thus:

```text
eECg(g, sector) = eLattice(g) ∘ eEC(sector)
```

provided the composition satisfies the invariants below.

---

## Invariants required by the unit tests

The unit tests define part of the specification and must be taken seriously.

For each equivalence relation `R` and sector selector `sector`, the following must hold.

### 1. Idempotence

```text
equate<R>(equate<R>(chord, sector), sector) == equate<R>(chord, sector)
```

For lattice-induced relations:

```text
equate<Rg>(equate<Rg>(chord, g, sector), g, sector) == equate<Rg>(chord, g, sector)
```

within tolerance.

### 2. Predicate/projection consistency

```text
predicate<R>(equate<R>(chord, sector), sector) == true
```

For lattice-induced relations:

```text
predicate<Rg>(equate<Rg>(chord, g, sector), g, sector) == true
```

### 3. Lattice consistency

For lattice-induced projections:

```text
equate<Rg>(chord, g, sector).iseLattice(g) == true
```

### 4. Decomposability

The predicate for a compound equivalence relation must agree with the conjunction of the predicates for its constituent elementary relations, insofar as the geometry actually decomposes in that way.

### 5. Membership of representative domains

Representative chords can be collected by globally iterating over a space enclosing the representative fundamental domain and inserting each chord that satisfies the **union membership predicate** for that domain.

The union predicate is satisfied if the sector predicate is satisfied for one or more sectors in the domain.

Examples:

- `OPT`: the chord lies in any OPT sector.
- `OPTg`: OPT membership and lattice membership on `g`.
- `OPTI`: the chord lies in the chosen inversion half of any OPT sector.
- `OPTIg`: OPTI membership and lattice membership on `g`.

