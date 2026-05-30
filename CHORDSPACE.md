# CHORD SPACE

Michael Gogins  
Irreducible Productions

Reference for the chord-space library in CsoundAC (`ChordSpace.hpp`). The design
follows Callender, Quinn, and Tymoczko, ["Generalized Voice-Leading
Spaces,"](https://www.science.org/doi/10.1126/science.1153021) *Science* 320,
2008. Continuous geometry is implemented first; a discrete layer (suffix `g`)
expresses how musicians usually think in equal temperament.

CsoundAC exposes this code in **C++**, **Python**, and **JavaScript** (including
the trichord-space visualization in csound-wasm).

---

## Purpose

The library supports algorithmic composition by providing:

- **Equivalence classes and normal forms** — decide whether a chord lies in a
  representative fundamental domain, and map arbitrary chords to canonical
  representatives (`eOP`, `eOPT`, `eOPTIg`, and related functions).

- **Catalogues of classes** — enumerate equivalence classes (e.g. all trichord
  set-classes in 12-TET via `fundamentalDomainByEquate` for `RPTIg`).

- **Orbifold navigation** — move progressions within a quotient space.

- **Neo-Riemannian harmony** — `P`, `L`, `R`, `D`, and contextual `K`, `Q`.

- **Voice-leading** — relate abstract equivalence classes via closest
  voice-leading in a less abstract space.

- **Scales and function** — scale degrees, conforming events to scales, and
  Roman-numeral-style operations.

---

## Pitches, voices, and chords

**Pitch** is a logarithmic quantity: octaves are doublings of frequency. Pitches
and intervals are **real numbers**. Middle C is `60`; one octave is `12`,
matching MIDI and 12-tone equal temperament (12-TET). Whole numbers are semitone
steps; fractions allow microtonal or continuous values.

A **voice** is one sounding line. A **chord** is an ordered tuple of pitches, a
point in **R^N** with one dimension per voice.

A **scale** is a chord whose lowest voice is a tonic pitch-class and whose other
voices are pitch-classes in ascending order.

For composition, a **score** can be read as a sequence of fleeting chords.

### Octave equivalence period

The period of pitch equivalence is **12 semitones**. This is foundational, not
merely MIDI convenience:

- octave equivalence is modulo `12`;
- OP, OPT, and OPTI geometry are built in octave-equivalence space;
- voice-leading, sectors, and inversion flats assume octave equivalence.

---

## Continuous vs discrete (`g`)

Two related geometries share the same `Chord` type.

### Continuous chord space

Pitches may be any reals. Equivalence relations **O**, **P**, **T**, **I** and
their compounds (**OP**, **OPT**, **OPTI**, **RPT**, **RPTI**, …) use exact
transposition (pitch sum `0` after `eT`) and hyperplane reflection where
appropriate. After `eT`, pitches need **not** be integers.

### Lattice-induced discrete space

Discrete space is **not** a separate storage type. It is the subset of
continuous space whose coordinates lie on an equally tempered lattice.

Let `g > 0` be the lattice step. A chord lies on the **g-lattice** if each voice
is (within tolerance) an integer multiple of `g`. For musical use, `g` should
divide the octave evenly:

```text
12 / g ∈ ℕ
```

Examples:

| `g`   | Temperament / lattice        |
|-------|------------------------------|
| `1`   | 12-TET (semitone grid)       |
| `0.5` | 24-TET (quarter-tone grid)   |
| `2`   | whole-tone lattice (6 steps) |
| `12/19` | 19-EDO                   |

Relations with suffix **`g`** (`Tg`, `Ig`, `OPTg`, `OPTIg`, `RPTg`, `RPTIg`, …)
first compute the continuous representative, then snap to the lattice (`eET`,
`eTg`).

### Recommendation for 12-TET composition

Prefer the **`g` APIs with `g = 1`**:

- `eOPTIg`, `eRPTIg`, `iseRPTIg`
- `fundamentalDomainByEquate` for `RPTIg`

These match **chord type** and **set class** as musicians use those terms in
12-TET.

---

## Plain chord space and cardinality

Plain chord space has no equivalence relation. Ordered chords are written
`(p₁, …, p_N)`; unordered (sorted) chords `{p₁, …, p_N}` illustrate
permutational equivalence.

**Cardinality equivalence is not assumed.** Spaces have fixed dimension `N`. A
single note may appear as `{60}`, `{60, 60}`, or `{60, 60, 60}` in trichord
space. This preserves a proto-metric that descends to quotient spaces.

---

## Equivalence relations

An equivalence relation groups chords considered “the same” under a symmetry.
Quotienting yields **orbifolds**: facets of a fundamental domain are identified
(octave wrap-around, inversion identification, etc.).

Many relations admit several geometrically identical copies of a fundamental
domain; this API chooses **representative** (normal) domains.

### Elementary relations

| Symbol | Meaning |
|--------|---------|
| **O** | Octave equivalence: pitches differing by whole octaves (`12`) are equivalent. |
| **P** | Permutational equivalence: reordering voices does not change the chord. Domain: voices sorted ascending. |
| **T** | Continuous transpositional equivalence: translate all voices by the same amount. Domain: hyperplane with pitch sum `0` (mean zero). |
| **Tg** | Transpositional equivalence on the **g-lattice** (`eTg`, `eET`). |
| **I** | Continuous inversion: reflection in a hyperplane (**inversion flat**), not registral revoicing. “Invert” here is the mathematician’s sense; shifting individual voices by octaves is **revoicing**. |
| **Ig** | Inversion on the **g-lattice** (`eIg`). |

### Compounds (continuous unless suffixed with `g`)

| Symbol | Meaning |
|--------|---------|
| **OP** | Octave + permutational (Tymoczko’s chord space). An **N**-voice space is an equilateral hyperprism, one octave high, with **N** cyclical **sectors** from octavewise revoicing. In 12-TET trichords: augmented triads along the center, major/minor columns around them. |
| **OPT** | **Chord type**: OP modulo **T**. Representative layer = base of the OP prism (normal form). **N** sector copies related by revoicing. C major and C minor are **different** OPT types. |
| **OPTI** | **Set class**: OPT modulo **I**. Major and minor triads share a class. Canonical inversion uses **direct musical inversion** (negate and reverse T-normal pitches, then reduce)—required for correctness when **N > 4**, not hyperplane reflection alone. |
| **OPTg**, **OPTIg** | As OPT / OPTI with representatives on the **g-lattice**. |
| **R**, **RP**, **RPT**, **RPTI** | Range and compounds: octavewise revoicing within a range combined with the above. |
| **RPTg**, **RPTIg** | Discrete (lattice) versions. **RPTIg** inversion pairs in the fundamental domain are resolved by **lexicographic tie-break** (keep the smaller of `{chord, inversion}`), which is stable after cyclic revoicing canonicalization. |

### Lattice-induced relations in general

For any continuous relation **EC**, the lattice-induced relation **ECg** is:

1. compute the continuous representative for **EC**;
2. project onto the **g-lattice**;
3. require idempotency and predicate consistency.

Examples: `OPTg`, `Ig`, `OPTIg`, `RPTg`, `RPTIg`.

---

## Geometry of OP, OPT, and OPTI

Let **N** be the number of voices.

### OP

Under OP, chord space is an equilateral hyperprism with **N** side facets and
height **12/N** along the unison direction.

Translating parallel to the unison diagonal moves a chord through the prism.
Octave equivalence folds the top and bottom bases, producing **N cyclical
sectors**. Each sector is an isosceles hyperprism.

### OPT

OPT is the **base layer** of the OP hyperprism. The representative OPT domain is
the union of all OPT sectors in that layer.

**Sector assignment** (for a T-normal chord):

1. project to OPT;
2. score each sector’s base centroid (dot-product / Voronoi rule in
   `is_in_rpt_sector_base`);
3. accept any sector with maximal score.

Boundary chords may belong to **more than one** sector.

### OPTI

Each OPT sector is cut in half by an **inversion flat** (hyperplane through the
sector center and the base centroid). Continuous inversion reflects across this
flat.

The representative OPTI domain is the union of chosen half-sectors. For
**OPTIg / RPTIg**, which representative of each inversion pair is kept is
decided by **lex order**, not solely by which side of the flat a chord lies on
(after `RPTg` canonicalization, hyperplane classification can misclassify pairs).

### Extrusions

OPT sectors, OPTI half-sectors, and inversion flats extend along the unison
diagonal into full OP space (OPI geometry, `Rg` reflection cache, etc.).

---

## Boundary behavior

Chords on shared **facets, edges, or vertices** of sectors belong to all
adjacent regions that share that boundary.

Consequences:

- predicates are **closed-set** tests;
- a boundary chord may satisfy predicates for **multiple** sectors;
- sector indices are **selectors** (which flat or region to use), not unique
  ownership labels.

When reflecting in the trichord-space model, the **same sector** must be used
for both halves of an invert/invert pair; otherwise facet chords appear
non-involutive because different sectors have different inversion flats.

Tie-breaking (lex order for RPTIg, lowest sector index in `opt_domain_sectors`)
is needed for **canonical catalogues**, not for bare membership.

---

## Predicates and projections

- **`ise<EC>(...)`** — true when the chord lies in the representative domain
  for **EC** (boundary points included).

- **`e<EC>(...)`** — canonical representative; should be **idempotent**:
  `e(e(x)) == e(x)`.

Template forms `equate<EQUIVALENCE_RELATION_…>`, `predicate<…>`, and
`fundamentalDomainByEquate` / `fundamentalDomainByPredicate` mirror the member
functions.

### Continuous vs lattice

| Layer | Rule |
|-------|------|
| Continuous | Geometry in the real-valued orbifold. |
| Lattice (`…g`) | Continuous representative, then snap to **g**-grid; must remain idempotent and satisfy `iseLattice(g)`. |

Conceptually: `eECg(g, sector) ≈ eET(g) ∘ eEC(sector)` (with sector and tolerance
details as implemented).

---

## Neo-Riemannian and contextual operations

Beyond equivalence maps:

| Operation | Role |
|-----------|------|
| `T(p, x)` | Translate chord by `x` semitones. |
| `I(p [, x])` | Reflect in point `x` (default origin). |
| `P`, `L`, `R` | Parallel, Leittonwechsel, Relative. |
| `D` | Dominant (down a perfect fifth). |
| `K(c)`, `Q(c, n, m)` | Contextual inversion and transposition (Fiore & Satyendra, *MTO* 11, 2008). |
| `R(sector)`, `Rg(sector, g)` | Reflection in OPT sector `sector`’s inversion flat; `Rg` uses a cached involution on **g**-lattice RPg representatives. |

---

## API overview

Member-function pattern on `Chord`:

```cpp
bool Chord::iseO() const;
Chord Chord::eO() const;

bool Chord::iseP() const;
Chord Chord::eP() const;

bool Chord::iseT() const;
Chord Chord::eT() const;

bool Chord::iseTg(double g = 1.0) const;
Chord Chord::eTg(double g = 1.0) const;

bool Chord::iseI(int sector = 0) const;
Chord Chord::eI(int sector = 0) const;

bool Chord::iseIg(double g = 1.0, int sector = 0) const;
Chord Chord::eIg(double g = 1.0, int sector = 0) const;

bool Chord::iseOP() const;
Chord Chord::eOP() const;

bool Chord::iseOPT(int sector = 0) const;
Chord Chord::eOPT(int sector = 0) const;

bool Chord::iseOPTg(double g = 1.0, int sector = 0) const;
Chord Chord::eOPTg(double g = 1.0, int sector = 0) const;

bool Chord::iseOPTI(int sector = 0) const;
Chord Chord::eOPTI(int sector = 0) const;

bool Chord::iseOPTIg(double g = 1.0, int sector = 0) const;
Chord Chord::eOPTIg(double g = 1.0, int sector = 0) const;

bool Chord::iseRPTIg(double range, double g = 1.0, int sector = 0) const;
Chord Chord::eRPTIg(double range, double g = 1.0, int sector = 0) const;
```

Lattice helpers:

```cpp
bool Chord::iseLattice(double g = 1.0) const;
Chord Chord::eLattice(double g = 1.0) const;  // snap each voice to nearest g multiple
Chord Chord::eET(double g = 1.0) const;     // exact tempered projection
```

Domain enumeration:

```cpp
const std::vector<Chord> &fundamentalDomainByEquate<RPTIg>(
    int voices, double range, double g, int sector, bool printme);
```

`opt_domain_sectors()` returns all OPT sectors a chord belongs to (sorted);
use the lowest index for a canonical sector choice.

---

## 12-TET set-class counts (`g = 1`, `range = 12`)

Verified by `ChordSpaceTests` for **RPTg** and **RPTIg** at **`g = 1`**:

| Voices | RPTg (OPT types) | RPTIg (set classes) |
|--------|------------------|---------------------|
| 3      | 31               | 19                  |
| 4      | 116              | 72                  |
| 5      | 364              | 196                 |
| 6      | 1038             | 561                 |

Formula for RPTIg size: `(|RPTg| + self_inverse_count) / 2` over the catalogue.

These counts may **differ** from manual tallies of illustrations in the CQT
paper (different conventions: inclusive octave span in pitch-class listings,
continuous vs lattice, etc.). Example: CQT-style tetrachord lists often cite
**84** classes; this library’s **RPTIg** domain at `g = 1` has **72**.

---

## Invariants and testing

`ChordSpaceTests` and `Chord::test()` check, for each relation **R** and sector:

1. **Idempotence** — `eR(eR(x)) == eR(x)`.
2. **Consistency** — `iseR(eR(x))` is true.
3. **Lattice consistency** (for `…g`) — representatives lie on the **g**-grid.
4. **Decomposability** — compound predicates agree with constituents where
   geometry decomposes.

At **`g = 1`**, **`range = 12`**, voices **3–6**, the suite reports on the order
of **two million** passing checks for discrete relations and most continuous
paths.

---

## Known limitations and test coverage

### Continuous `eOPT` / `eRPT` (six voices)

One **known failure** remains in `ChordSpaceTests`: for **N = 6**, some
representatives in the RPTI catalogue have pitch sums **not divisible by 6**.
Continuous `eT` then yields **non-integer** pitches, and **`eOPT` is not
idempotent** (`equate<RPT>` may fail to find a sector).

This does **not** affect the discrete path **`eOPTg` / `eOPTIg` / `RPTIg` at
`g = 1`**, which is the recommended API for 12-TET composition. The continuous
issue is **intentionally left unfixed** for now.

### Other values of `g`

The implementation is parameterized by **`g`**, but **`ChordSpaceTests` currently
exercises only `g = 1`** with `range = 12`. Hardcoded domain-size checks for
RPTg and RPTIg apply only to **12-TET**. Using **`g ≠ 1`** (e.g. 24-TET) should
be treated as **untested** until dedicated tests are added.

### CQT catalogues vs this library

CQT’s mathematics is **continuous**; their figures use **integer** pitch-class
illustrations. This library’s **RPTIg** domains at **`g = 1`** use exact lattice
representatives and **lex** inversion tie-breaks. Do not expect one-to-one
agreement with every table or hand count in the paper.

---

## References

- Callender, Quinn, Tymoczko, "Generalized Voice-Leading Spaces," *Science*
  320, 2008.
- Fiore and Satyendra, "Generalized Contextual Groups," *Music Theory Online*
  11, 2008.
- Header overview: `CsoundAC/ChordSpace.hpp` (file comment at top of file).
