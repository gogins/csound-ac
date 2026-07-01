#include "ChordSpace.hpp"
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#endif

#pragma GCC diagnostic ignored "-Wformat"

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Matrix;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> Vector;

static bool printPass = true;
static bool printPitv = false;
static bool failureExits = false ;
static int passCount = 0;
static int failureCount = 0;
static int testCount = 0;
static int exitAfterFailureCount = 5;
static int testSector = 0;
static double g = 1.0;

static bool pass(std::string message) {
    passCount = passCount + 1;
    testCount = passCount + failureCount;
    if (printPass) {
        csound::System::message("\nPASSED (passed: %-9d failed: %-9d of %9d): %s\n", passCount, failureCount, testCount, message.c_str());
    }
    return true;
}

static bool fail(std::string message) {
    failureCount = failureCount + 1;
    testCount = passCount + failureCount;
    csound::System::message("================================================================================================================\n");
    csound::System::message("FAILED (passed: %-9d failed: %-9d of %d): %s\n", passCount, failureCount, testCount, message.c_str());
    csound::System::message("================================================================================================================\n");
    if (failureExits && (failureCount >= exitAfterFailureCount)) {
        std::exit(-1);
    }
    return false;
}

static void summary() {
    testCount = passCount + failureCount;
    std::time_t time_ = std::time(nullptr);
    auto datetime = std::asctime(std::localtime(&time_));
    csound::System::message("\n================================================================================================================\n");
    csound::System::message("SUMMARY  Passed: %-9d  Failed: %-9d  Total: %d  Completed: %s", passCount, failureCount, testCount, datetime); 
    csound::System::message("================================================================================================================\n");
}

static bool test(bool passes, std::string message) {
    if (passes) {
        pass(message);
    } else {
        fail(message);
    }
    return passes;
}

static bool print_sets = false;

static void printSet(std::string name, const std::vector<csound::Chord> &chords) {
    csound::System::message("%s\n", name.c_str());
    std::multimap<csound::Chord, csound::Chord, csound::ChordTickLess> sorted;
    for (auto &e : chords) {
        sorted.insert({e.normal_form(), e}); 
    }
    int i = 1;
    for (auto &value : sorted) {
        auto &c = value.second;
        csound::System::message("normal: %s  chord[%04d]: %s  OPTT: %s  OPTTI: %s\n", c.normal_form().toString().c_str(), i, c.toString().c_str(), c.eOPTg().toString().c_str(), c.eOPTIg().toString().c_str()); 
        auto s = print_opti_sectors(c);
        csound::System::message("%s", s.c_str());
        csound::System::message("\n");
        i = i + 1;
    }
}

static void test_pitv(const csound::PITV &pitv_, std::string chordName) {
    csound::System::message("BEGAN test PITV for %s...\n", chordName.c_str());
    csound::Chord originalChord = csound::chordForName(chordName);
    csound::Chord optti = originalChord.eOPTIg();
    csound::System::message("Original chord:\n%s\n", originalChord.information().c_str());
    Eigen::VectorXi pitv = pitv_.fromChord(originalChord, printPitv);
    csound::Chord reconstitutedChord = pitv_.toChord(pitv[0], pitv[1], pitv[2], pitv[3], printPitv)[0];
    csound::System::message("Reconstituted chord:\n%s\n", reconstitutedChord.information().c_str());
    test(originalChord == reconstitutedChord, "Reconstituted chord must be the same as the original chord.");
    csound::Chord revoicedOriginalChord = originalChord;
    revoicedOriginalChord.setPitch(1,  revoicedOriginalChord.getPitch(1) + 12.);
    revoicedOriginalChord.setPitch(2,  revoicedOriginalChord.getPitch(2) + 24.);
    csound::System::message("Revoiced original chord:\n%s\n", revoicedOriginalChord.information().c_str());
    pitv = pitv_.fromChord(revoicedOriginalChord, printPitv);
    csound::Chord reconstitutedRevoicedChord = pitv_.toChord_vector(pitv, printPitv)[0];
    csound::System::message("Reconstituted revoiced chord:\n%s\n", reconstitutedRevoicedChord.information().c_str());
    test(revoicedOriginalChord == reconstitutedRevoicedChord, "Reconstituted revoiced chord must be the same as the original revoiced chord.");
    csound::Chord invertedChord = originalChord.I().eOP();
    csound::System::message("Inverted original chord:\n%s\n", invertedChord.information().c_str());
    pitv = pitv_.fromChord(invertedChord, printPitv);
    csound::Chord reconstitutedInvertedChord = pitv_.toChord_vector(pitv, true)[0];
    csound::System::message("Reconstituted inverted chord:\n%s\n", reconstitutedInvertedChord.information().c_str());
    test(invertedChord == reconstitutedInvertedChord,"Reconstituted inverted chord must be the same as the original inverted chord.");
    csound::System::message("ENDED test PITV for %s.\n", chordName.c_str());
    csound::System::message("\n");
}

static void test_pitv(int initialVoiceCount, int finalVoiceCount) {
    double range = 60.0;
    for (int voiceCount = initialVoiceCount; voiceCount <= finalVoiceCount; ++voiceCount) {
        csound::PITV pitv;
        csound::System::message("Testing all of PITV: voices: %d  range: %f\n", voiceCount, range);
        csound::System::message("Testing PITV to chord and back...\n");
        pitv.initialize(voiceCount, range, 1., true);
        pitv.list(true, true, true);
        for (int P = 0; P < pitv.countP; ++P) {
            for (int T = 0; T < pitv.countT; ++T) {
                for (int V = 0; V < pitv.countV; ++V) {
                    for (int I = 0; I < pitv.countI; ++I) {
                        if (printPass) csound::System::message("PITV => chord from PITV\n");
                        csound::Chord chord_from_pitv = pitv.toChord(P, I, T, V, printPitv)[0];
                        if (printPass) csound::System::message("chord from PITV => PITV from chord\n");
                        Eigen::VectorXi pitv_from_chord = pitv.fromChord(chord_from_pitv, printPitv);
                        if (printPass) csound::System::message("PITV from chord => chord from PITV from chord\n");
                        csound::Chord chord_from_pitv_from_chord = pitv.toChord(pitv_from_chord(0), pitv_from_chord(1), pitv_from_chord(2), pitv_from_chord(3), printPitv)[0];
                        bool equals_ = (chord_from_pitv.equals(chord_from_pitv_from_chord));
                        if (!equals_) {
                            csound::System::message("chord_from_pitv (toChord):\n%s\n", chord_from_pitv.information().c_str());
                            csound::System::message("chord_from_pitv_from_chord (fromChord):\n%s\n", chord_from_pitv_from_chord.information().c_str());
                        }
                        test(equals_, "chord_from_pitv must match chord_from_pitv_from_chord.");
                        if (printPass) csound::System::message("\n\n");
                    }
                }
            }
        }
    }
}

static void test_nrR() {
    auto C = csound::chordForName("CM");
    auto a = csound::chordForName("Am");
    for (double t = 0.; t < 12.; ++t) {
        auto I = C.T(t);
        csound::System::message("test_nrR: I:       %s I.eOP():      %s normal form: %s\n", I.toString().c_str(), I.eOP().toString().c_str(), I.normal_form().toString().c_str());
        auto vi = a.T(t);
        csound::System::message("test_nrR: vi:      %s vi.eOP():     %s normal form: %s\n", vi.toString().c_str(), vi.eOP().toString().c_str(), vi.normal_form().toString().c_str());
        auto I_nrR = I.nrR();
        csound::System::message("test_nrR: I_nrR:   %s I_nrR.eOP():  %s normal form: %s\n", I_nrR.toString().c_str(), I_nrR.eOP().toString().c_str(), I_nrR.normal_form().toString().c_str());
        test(I_nrR.eOP().equals(vi.eOP()), "I.nrR() should be vi.\n");
        auto vi_nrR =vi.nrR();
        csound::System::message("test_nrR: vi_nrR:  %s vi_nrR.eOP(): %s normal form: %s\n", vi_nrR.toString().c_str(), vi_nrR.eOP().toString().c_str(), vi_nrR.normal_form().toString().c_str());
        test(vi_nrR.eOP().equals(I.eOP()), "vi.nrR() should be I.\n\n");
    }
}

static void test_nrP() {
    auto C = csound::chordForName("CM");
    auto c = csound::chordForName("Cm");
    for (double t = 0.; t < 12.; ++t) {
        auto I = C.T(t);
        csound::System::message("test_nrP: I:       %s I.eOP():     %s normal form: %s\n", I.toString().c_str(), I.eOP().toString().c_str(), I.normal_form().toString().c_str());
        auto i = c.T(t);   
        csound::System::message("test_nrP: i:       %s i.eOP():     %s normal form: %s\n", i.toString().c_str(), i.eOP().toString().c_str(), i.normal_form().toString().c_str());
        auto I_nrP = I.nrP();
        csound::System::message("test_nrP: I_nrP:   %s I_nrP.eOP(): %s normal form: %s\n", I_nrP.toString().c_str(), I_nrP.eOP().toString().c_str(), I_nrP.normal_form().toString().c_str());
        test(I_nrP.eOP().equals(i.eOP()), "I.nrP() should be i.\n");
        auto i_nrP = i.nrP();
        csound::System::message("test_nrP: i_nrP:   %s i_nrP.eOP(): %s normal form: %s\n", i_nrP.toString().c_str(), i_nrP.eOP().toString().c_str(), i_nrP.normal_form().toString().c_str());
        test(i_nrP.eOP().equals(I.eOP()), "i.nrP() should be I.\n\n");
    }
}

static void test_nrL() {
    auto C = csound::chordForName("CM");
    auto e = csound::chordForName("Em");
    for (double t = 0.; t < 12.; ++t) {
        auto I = C.T(t);
        csound::System::message("test_nrL: I:       %s I.eOP():       %s normal form: %s\n", I.toString().c_str(), I.eOP().toString().c_str(), I.normal_form().toString().c_str());
        auto iii = e.T(t);
        csound::System::message("test_nrL: iii:     %s iii.eOP():     %s normal form: %s\n", iii.toString().c_str(), iii.eOP().toString().c_str(), iii.normal_form().toString().c_str());
        auto I_nrL = I.nrL();
        csound::System::message("test_nrL: I_nrL:   %s I_nrL.eOP():   %s normal form: %s\n", I_nrL.toString().c_str(), I_nrL.eOP().toString().c_str(), I_nrL.normal_form().toString().c_str());
        test(I_nrL.eOP().equals(iii.eOP()), "I.nrL() should be iii.\n");
        auto iii_nrL = iii.nrL();
        csound::System::message("test_nrL: iii_nrL: %s iii_nrL.eOP(): %s normal form: %s\n", iii_nrL.toString().c_str(), iii_nrL.eOP().toString().c_str(), iii_nrL.normal_form().toString().c_str());
        char buffer[0x500];
        std::snprintf(buffer,sizeof(buffer), "iii.nrL() %s should be I %s.\n\n", iii_nrL.toString().c_str(), I.toString().c_str());
        test(iii_nrL.eOP().equals(I.eOP()), buffer);
    }
}

std::vector<std::string> equivalenceRelationsToTest = {"RP", "RPT", "RPTg", "RPTI", "RPTIg"};
typedef csound::Chord(*equate_t)(const csound::Chord &, double, double, int);
typedef bool (*predicate_t)(const csound::Chord &, double, double, int);
typedef const std::vector<csound::Chord> &(*fundamentalDomainByEquate_t)(int, double, double, int);
typedef const std::vector<csound::Chord> &(*fundamentalDomainByPredicate_t)(int, double, double, int, bool);
std::map<std::string, equate_t> equatesForEquivalenceRelations;
std::map<std::string, predicate_t> predicatesForEquivalenceRelations;
std::map<std::string, std::set<std::string> > equivalenceRelationsForCompoundEquivalenceRelations;
std::map<std::string, fundamentalDomainByEquate_t> fundamentalDomainByEquateForEquivalenceRelations;
std::map<std::string, fundamentalDomainByPredicate_t> fundamentalDomainByPredicateForEquivalenceRelations;

static bool testNormalsAndEquivalents(std::string equivalence,
                                      std::vector<csound::Chord> &found_equivalents,
                                      double range,
                                      double g) {
    char buffer[0x200];
    auto is_equivalent = predicatesForEquivalenceRelations[equivalence];
    csound::System::message("\nequivalence: %s  is_normal: %ld  range: %f  g: %f\n", equivalence.c_str(), found_equivalents.size(), range, g);
    int count = 1;
    for (auto found_equivalent = found_equivalents.begin(); found_equivalent != found_equivalents.end(); ++found_equivalent) {
        std::snprintf(buffer, sizeof(buffer), "FOUND EQUIVALENT %d\n", count);
        if (!test(found_equivalent->test(), std::string(buffer))) {
            return false;
        }
        count = count + 1;
    }
    return true;
 }

static bool testEquivalenceRelation(std::string equivalenceRelation, int voiceCount, double range, double g) {
    bool passes = true;
    char buffer[0x200];

    auto equivalentsForEquivalenceRelation = fundamentalDomainByPredicateForEquivalenceRelations[equivalenceRelation](voiceCount, range, g, testSector, false);
    if (!testNormalsAndEquivalents(equivalenceRelation,
                                   equivalentsForEquivalenceRelation,
                                   range,
                                   g)) {
        passes = false;
    }

    if (equivalenceRelation == "RPTIg") {
        if (voiceCount == 3) {
            if (equivalentsForEquivalenceRelation.size() != 19) {
                csound::System::message("%-8s 'found_equivalents' size should be 19 but is %ld.\n", equivalenceRelation.c_str(), equivalentsForEquivalenceRelation.size());
                passes = false;
                test(passes, "Size of found equivalents not correct for 3 voices.");
            } else {
                test(true, "Size of found equivalents is correct for 3 voices.");
            }
         }
        if (voiceCount == 4) {
            if (equivalentsForEquivalenceRelation.size() != 72) {
                csound::System::message("%-8s 'found_equivalents' size should be 72 but is %ld.\n", equivalenceRelation.c_str(), equivalentsForEquivalenceRelation.size());
                passes = false;    
                test(passes, "Size of found equivalents not correct for 4 voices.");
            } else {
                test(true, "Size of found equivalents is correct for 4 voices.");
            }
        }
    }
    return passes;
}

static bool test_eIg_idempotent_on_lattice(double g, int opt_sector) {
    bool passes = true;
    csound::System::message("\nTesting eIg idempotency on Tg-lattice chords (sector %d, g %f)...\n", opt_sector, g);

    auto check = [&](const csound::Chord &chord, const char *label) {
        const csound::Chord on_lattice = chord.eTg(g);
        const csound::Chord once = on_lattice.eIg(g, opt_sector);
        const csound::Chord twice = once.eIg(g, opt_sector);
        const bool idempotent = (once == twice);
        csound::System::message(
            "  %s: lattice %s  eIg %s  eIg(eIg) %s  idempotent %d\n",
            label,
            on_lattice.toString().c_str(),
            once.toString().c_str(),
            twice.toString().c_str(),
            idempotent ? 1 : 0);
        if (!idempotent) {
            passes = false;
        }
    };

    check(csound::Chord({0, 4, 7}), "major triad");
    check(csound::Chord({0, 3, 7}), "minor triad");
    check(csound::Chord({0, 3, 6}), "diminished");
    check(csound::chordForName("C7"), "C7");
    check(csound::chordForName("CM7"), "CM7");

    test(passes, "eIg is idempotent on Tg-lattice chords.");
    return passes;
}

static bool testEquivalenceRelations(int voiceCount, double range, double g) {
    bool passes = true;
    csound::System::message("\nTesting equivalence relations for %d voices over range %f with g %f...\n\n", voiceCount, range, g);
    for (auto equivalenceRelationI = equivalenceRelationsToTest.begin();
            equivalenceRelationI != equivalenceRelationsToTest.end();
            ++equivalenceRelationI) {
        if (!testEquivalenceRelation(*equivalenceRelationI, voiceCount, range, g)) {
            passes = false;
        }
    }
    return passes;
}

static void test_eq_tolerance() {
    double mp_double_small = .00000000000000000001;
    double mp_double_large = 1e40;
    double test_a = 1.;
    double test_b = 0.;
    for (int i = 1; i <= 100; ++i) 
    {
        std::fprintf(stderr, "step %d:\n", i);
        csound::numerics_information(test_a, test_b, 100, 10000);
        test_a /= 2.;
       //test_b *= 2.;
    }
    std::cerr << "ulp(mp_double_small): " << boost::math::ulp(mp_double_small) << std::endl;
    std::cerr << "ulp(mp_double_large): " << boost::math::ulp(mp_double_large) << std::endl;
    std::cerr << "csound::eq_tolerance(0.15, 0.15): " << csound::eq_tolerance(0.15, 0.15) << std::endl;
    std::cerr << "csound::eq_tolerance(0.1500000000000000001, 0.15): " << csound::eq_tolerance(0.1500000000000000001, 0.15) << std::endl;
    std::cerr << "csound::eq_tolerance(0.1500000000001, 0.15): " << csound::eq_tolerance(0.1500000000001, 0.15) << std::endl;
    std::cerr << "csound::gt_tolerance(14.0, 12.0): " << csound::gt_tolerance(14.0, 12.0) << std::endl;
    std::cerr << "csound::ge_tolerance(14.0, 12.0): " << csound::ge_tolerance(14.0, 12.0) << std::endl;
}

static void test_gather_sounding_chord() {
    csound::ChordScore score;
    score.append(0.0, 1.0, 144.0, 1.0, 60.0, 80.0, 0.0);
    score.append(0.0, 3.0, 144.0, 1.0, 64.0, 80.0, 0.0);
    score.append(1.5, 1.0, 144.0, 1.0, 67.0, 80.0, 0.0);
    csound::Chord sounding_only = csound::ChordScore::gatherSoundingChord(score, 0.0, 2.0, 2);
    if (sounding_only.voices() != 2
        || !csound::eq_tolerance(sounding_only.getPitch(0), 67.0)
        || !csound::eq_tolerance(sounding_only.getPitch(1), 64.0)) {
        fail("gatherSoundingChord sounding at segment end");
    } else {
        pass("gatherSoundingChord sounding at segment end");
    }

    csound::Chord with_recent = csound::ChordScore::gatherSoundingChord(score, 0.0, 2.0, 3);
    if (with_recent.voices() != 3
        || !csound::eq_tolerance(with_recent.getPitch(2), 60.0)) {
        fail("gatherSoundingChord recent ended fill");
    } else {
        pass("gatherSoundingChord recent ended fill");
    }

    csound::ChordScore recent_score;
    recent_score.append(0.0, 0.5, 144.0, 1.0, 50.0, 80.0, 0.0);
    recent_score.append(0.5, 1.0, 144.0, 1.0, 55.0, 80.0, 0.0);
    recent_score.append(1.0, 2.0, 144.0, 1.0, 58.0, 80.0, 0.0);
    csound::Chord recent_order = csound::ChordScore::gatherSoundingChord(recent_score, 0.0, 2.0, 3);
    if (recent_order.voices() != 3
        || !csound::eq_tolerance(recent_order.getPitch(0), 58.0)
        || !csound::eq_tolerance(recent_order.getPitch(1), 55.0)
        || !csound::eq_tolerance(recent_order.getPitch(2), 50.0)) {
        fail("gatherSoundingChord recent ended ordering");
    } else {
        pass("gatherSoundingChord recent ended ordering");
    }
}

static void test_chord_score_conform_modes() {
    csound::ChordScore score;
    score.append(0.0, 1.0, 144.0, 1.0, 62.0, 80.0, 0.0);
    csound::Chord c_major = csound::chordForName("CM");
    score.insertChord(0.0, c_major, csound::HarmonyConformMode::Hc);
    score.conformToChords(false, true);
    const double hc_pitch = score[0].getKey();
    if (!csound::eq_tolerance(hc_pitch, 60.0) && !csound::eq_tolerance(hc_pitch, 64.0) && !csound::eq_tolerance(hc_pitch, 67.0)) {
        fail("ChordScore Hc conform");
    } else {
        pass("ChordScore Hc conform");
    }

    csound::ChordScore score_hcv;
    score_hcv.append(0.0, 1.0, 144.0, 1.0, 50.0, 80.0, 0.0);
    csound::Chord voiced = csound::chordForName("CM");
    voiced.setPitch(0, 72.0);
    voiced.setPitch(1, 76.0);
    voiced.setPitch(2, 79.0);
    score_hcv.insertChord(0.0, voiced, csound::HarmonyConformMode::Hcv);
    score_hcv.conformToChords(false, true);
    const double hcv_pitch = score_hcv[0].getKey();
    if (!csound::eq_tolerance(hcv_pitch, 72.0)) {
        fail("ChordScore Hcv conform");
    } else {
        pass("ChordScore Hcv conform");
    }

    csound::ChordScore score_hcs;
    score_hcs.append(0.0, 2.0, 144.0, 1.0, 60.0, 80.0, 0.0);
    score_hcs.append(0.0, 2.0, 144.0, 2.0, 64.0, 80.0, 0.0);
    score_hcs.append(1.0, 1.0, 144.0, 1.0, 74.0, 80.0, 0.0);
    csound::Chord target = csound::chordForName("Em");
    score_hcs.insertChord(1.0, target, csound::HarmonyConformMode::Hcs);
    score_hcs.conformToChords(false, true);
    const double hcs_pitch = score_hcs[2].getKey();
    const csound::Chord em_pcs = target.epcs();
    bool in_em = false;
    for (int voice = 0; voice < static_cast<int>(em_pcs.voices()); ++voice) {
        if (csound::eq_tolerance(hcs_pitch, em_pcs.getPitch(voice))) {
            in_em = true;
            break;
        }
    }
    if (!in_em) {
        fail("ChordScore Hcs conform");
    } else {
        pass("ChordScore Hcs conform");
    }

    csound::ChordScore score_hd;
    score_hd.append(0.0, 1.0, 144.0, 1.0, 62.0, 80.0, 0.0);
    const csound::Scale &c_major_scale = csound::scaleForName("C major");
    score_hd.insertFunctionalHarmony(0.0, c_major_scale, 1, 3, csound::HarmonyConformMode::Hd);
    score_hd.conformToChords(false, true);
    const double hd_pitch = score_hd[0].getKey();
    const csound::Chord tonic = c_major_scale.chord(1, 3);
    bool in_tonic = false;
    for (int voice = 0; voice < static_cast<int>(tonic.voices()); ++voice) {
        if (csound::eq_tolerance(hd_pitch, tonic.getPitch(voice))) {
            in_tonic = true;
            break;
        }
    }
    if (!in_tonic) {
        fail("ChordScore Hd conform");
    } else {
        pass("ChordScore Hd conform");
    }
}

int main(int argc, char **argv) {
    csound::System::message("C H O R D S P A C E   U N I T   T E S T S\n\n");
    setvbuf(stderr, nullptr, _IONBF, 0);
    const char *wait_for_debugger = std::getenv("CSOUND_AC_WAIT_FOR_DEBUGGER");
    if (wait_for_debugger != nullptr && wait_for_debugger[0] != '\0' && wait_for_debugger[0] != '0')
    {
        auto pid = getpid();
        std::fprintf(stderr, "Raising SIGSTOP for pid %d to allow attaching a debugger;\n", pid);
        std::fprintf(stderr, "execute 'fg' in terminal to resume, or attach debugger with 'lldb -p %d'\n", pid);
#ifdef _WIN32
        while (!IsDebuggerPresent())
        {
            Sleep(100);
        }
        __debugbreak();
#else
        raise(SIGSTOP);
#endif
    }
    std::cerr << csound::chord_space_version() << std::endl;
    csound::Chord CM = csound::chordForName("C+");
    CM = CM.T(-4.);
    std::cerr << CM.information() << std::endl;
    test_eq_tolerance();
    test_nrR();
    test_nrP();
    test_nrL();
    auto ops = csound::allOfEquivalenceClass(3, "RP", 12., 1., 0, false);
    if (print_sets) printSet("OPs", ops);
    auto optts = csound::allOfEquivalenceClass(3, "RPTg", 12., 1., 0, false);
    if (print_sets) printSet("OPTTs", optts);
    auto opttis = csound::allOfEquivalenceClass(3, "RPTIg", 12., 1., 0, false);
    if (print_sets) printSet("OPTTIs", opttis);

    auto chordx = csound::chordForName("CM7");
    auto dominantx = csound::chordForName("G7");
    csound::System::message("CM7:\n%s\n", chordx.information().c_str());
    csound::System::message("G7:\n%s\n", dominantx.information().c_str());

    equatesForEquivalenceRelations["R"] =        csound::equate<csound::EQUIVALENCE_RELATION_R>;
    equatesForEquivalenceRelations["P"] =        csound::equate<csound::EQUIVALENCE_RELATION_P>;
    equatesForEquivalenceRelations["T"] =        csound::equate<csound::EQUIVALENCE_RELATION_T>;
    equatesForEquivalenceRelations["Tg"] =       csound::equate<csound::EQUIVALENCE_RELATION_Tg>;
    equatesForEquivalenceRelations["I"] =        csound::equate<csound::EQUIVALENCE_RELATION_I>;
    equatesForEquivalenceRelations["RP"] =       csound::equate<csound::EQUIVALENCE_RELATION_RP>;
    equatesForEquivalenceRelations["RPT"] =      csound::equate<csound::EQUIVALENCE_RELATION_RPT>;
    equatesForEquivalenceRelations["RPTg"] =     csound::equate<csound::EQUIVALENCE_RELATION_RPTg>;
    equatesForEquivalenceRelations["RPI"] =      csound::equate<csound::EQUIVALENCE_RELATION_RPI>;
    equatesForEquivalenceRelations["RPTI"] =     csound::equate<csound::EQUIVALENCE_RELATION_RPTI>;
    equatesForEquivalenceRelations["RPTIg"] =    csound::equate<csound::EQUIVALENCE_RELATION_RPTIg>;
    predicatesForEquivalenceRelations["R"] =         csound::predicate<csound::EQUIVALENCE_RELATION_R>;
    predicatesForEquivalenceRelations["P"] =         csound::predicate<csound::EQUIVALENCE_RELATION_P>;
    predicatesForEquivalenceRelations["T"] =         csound::predicate<csound::EQUIVALENCE_RELATION_T>;
    predicatesForEquivalenceRelations["Tg"] =        csound::predicate<csound::EQUIVALENCE_RELATION_Tg>;
    predicatesForEquivalenceRelations["I"] =         csound::predicate<csound::EQUIVALENCE_RELATION_I>;
    predicatesForEquivalenceRelations["RP"] =        csound::predicate<csound::EQUIVALENCE_RELATION_RP>;
    predicatesForEquivalenceRelations["RPT"] =       csound::predicate<csound::EQUIVALENCE_RELATION_RPT>;
    predicatesForEquivalenceRelations["RPTg"] =      csound::predicate<csound::EQUIVALENCE_RELATION_RPTg>;
    predicatesForEquivalenceRelations["RPI"] =       csound::predicate<csound::EQUIVALENCE_RELATION_RPI>;
    predicatesForEquivalenceRelations["RPTI"] =      csound::predicate<csound::EQUIVALENCE_RELATION_RPTI>;
    predicatesForEquivalenceRelations["RPTIg"] =     csound::predicate<csound::EQUIVALENCE_RELATION_RPTIg>;
    equivalenceRelationsForCompoundEquivalenceRelations["RP"] =      {"R", "P"};
    equivalenceRelationsForCompoundEquivalenceRelations["RPT"] =     {"R", "P", "T"};
    equivalenceRelationsForCompoundEquivalenceRelations["RPTg"] =    {"R", "P", "Tg"};
    equivalenceRelationsForCompoundEquivalenceRelations["RPI"] =     {"R", "P"};
    equivalenceRelationsForCompoundEquivalenceRelations["RPTIg"] =   {"RPTIg", "RP", "R", "P", "Tg"};
    fundamentalDomainByPredicateForEquivalenceRelations["R"] =           csound::fundamentalDomainByPredicate<csound::EQUIVALENCE_RELATION_R>;
    fundamentalDomainByPredicateForEquivalenceRelations["P"] =           csound::fundamentalDomainByPredicate<csound::EQUIVALENCE_RELATION_P>;
    fundamentalDomainByPredicateForEquivalenceRelations["T"] =           csound::fundamentalDomainByPredicate<csound::EQUIVALENCE_RELATION_T>;
    fundamentalDomainByPredicateForEquivalenceRelations["Tg"] =          csound::fundamentalDomainByPredicate<csound::EQUIVALENCE_RELATION_Tg>;
    fundamentalDomainByPredicateForEquivalenceRelations["I"] =           csound::fundamentalDomainByPredicate<csound::EQUIVALENCE_RELATION_I>;
    fundamentalDomainByPredicateForEquivalenceRelations["RP"] =          csound::fundamentalDomainByEquate<csound::EQUIVALENCE_RELATION_RP>;
    fundamentalDomainByPredicateForEquivalenceRelations["RPT"] =          csound::fundamentalDomainByEquate<csound::EQUIVALENCE_RELATION_RPT>;
    fundamentalDomainByPredicateForEquivalenceRelations["RPTg"] =          csound::fundamentalDomainByEquate<csound::EQUIVALENCE_RELATION_RPTg>;
    fundamentalDomainByPredicateForEquivalenceRelations["RPI"] =          csound::fundamentalDomainByEquate<csound::EQUIVALENCE_RELATION_RPI>;
    fundamentalDomainByPredicateForEquivalenceRelations["RPTI"] =          csound::fundamentalDomainByEquate<csound::EQUIVALENCE_RELATION_RPTI>;
    fundamentalDomainByPredicateForEquivalenceRelations["RPTIg"] =          csound::fundamentalDomainByEquate<csound::EQUIVALENCE_RELATION_RPTIg>;

    test_eIg_idempotent_on_lattice(g, testSector);

    csound::System::message("\nTesting equivalence relations...\n\n");
    for (int voiceCount = 3; voiceCount <= 6; ++voiceCount) {
        testEquivalenceRelations(voiceCount, csound::OCTAVE(), 1.0);
    }
    
    csound::PITV pitv_3;
    pitv_3.initialize(3, 60., 1., true);
    pitv_3.list(true, true);
    
    csound::PITV pitv_4;
    pitv_4.initialize(4, 60., 1., true);
    pitv_4.list(true, true);
    
    test_pitv(pitv_4, "D#7b5");
    test_pitv(3, 4);

    test_gather_sounding_chord();
    test_chord_score_conform_modes();

    summary();
    return 0;
}
 
