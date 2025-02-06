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
#include "VoiceleadingNode.hpp"
#include "Conversions.hpp"
#include "Voicelead.hpp"
#include "System.hpp"

#include <cmath>
#include <cfloat>
#include <sstream>

namespace csound
{
extern void SILENCE_PUBLIC printChord(std::ostream &stream, std::string label, const std::vector<double> &chord);

extern void SILENCE_PUBLIC printChord(std::string label, const std::vector<double> &chord);

/**
 * Utility class for storing voice-leading operations.
 */
VoiceleadingOperation::VoiceleadingOperation() :
    beginTime(0.0),
    rescaledBeginTime(0.0),
    endTime(0.0),
    rescaledEndTime(0.0),
    P_(DBL_MAX),
    T_(DBL_MAX),
    C_(DBL_MAX),
    K_(DBL_MAX),
    Q_(DBL_MAX),
    V_(DBL_MAX),
    N_(false),
    L_(false),
    begin(0),
    end(0),
    avoidParallels(false)
{
}

VoiceleadingOperation::~VoiceleadingOperation()
{
}

VoiceleadingNode::VoiceleadingNode() :
    base(36.0),
    range(60.0),
    avoidParallels(true),
    divisionsPerOctave(12),
    rescaleTimes(false)
{
}

VoiceleadingNode::~VoiceleadingNode()
{
}

std::ostream &operator << (std::ostream &stream, const VoiceleadingOperation &operation)
{
    stream << "  beginTime:         " << operation.beginTime << std::endl;
    stream << "  rescaledBeginTime: " << operation.rescaledBeginTime << std::endl;
    stream << "  rescaledEndTime:   " << operation.rescaledEndTime << std::endl;
    stream << "  begin:             " << operation.begin << std::endl;
    stream << "  end:               " << operation.end << std::endl;
    if (!(operation.P_ == DBL_MAX)) {
        stream << "  P:                 " << operation.P_ << std::endl;
    }
    if (!(operation.T_ == DBL_MAX)) {
        stream << "  T:                 " << operation.T_ << std::endl;
    }
    if (!(operation.C_ == DBL_MAX)) {
        stream << "  C:                 " << operation.C_ << std::endl;
    }
    if (!(operation.K_ == DBL_MAX)) {
        stream << "  K:                 " << operation.K_ << std::endl;
    }
    if (!(operation.Q_ == DBL_MAX)) {
        stream << "  Q:                 " << operation.Q_ << std::endl;
    }
    if (!(operation.V_ == DBL_MAX)) {
        stream << "  V:                 " << operation.V_ << std::endl;
    }
    if (operation.N_) {
        stream << "  NOP:               " << operation.N_ << std::endl;
    }
    if (operation.L_) {
        stream << "  L:                 " << int(operation.L_) << std::endl;
    }
    if (operation.chord.size() > 0) {
        stream << "Chd:                 " << operation.chord.information() << std::endl;
    }
    return stream;
}

void VoiceleadingNode::apply(Score &score, const VoiceleadingOperation &priorOperation, const VoiceleadingOperation &operation)
{
    if ( (System::getMessageLevel() & System::INFORMATION_LEVEL) == System::INFORMATION_LEVEL) {
        std::stringstream stream;
        stream << "VoiceleadingNode::apply:..." << std::endl;
        stream << "  Events in score:     " << score.size() << std::endl;
        stream << "  Score duration:      " << score.getDuration() << std::endl;
        stream << "  Events in operation: " << (operation.end - operation.begin) << std::endl;
        stream << "  priorOperation:      " << std::endl << priorOperation;
        stream << "  currrentOperation:   " << std::endl << operation;
        stream << "  modality:            ";
        printChord(stream, "", modality);
        stream << std::endl;
        System::inform(stream.str().c_str());
    }
    if (operation.begin == operation.end) {
        return;
    }
    // For actual Chord objects...
    if ((operation.P_ == DBL_MAX) && 
        (operation.T_ == DBL_MAX) && 
        (operation.C_ == DBL_MAX) && 
        (operation.K_ == DBL_MAX) && 
        (operation.Q_ == DBL_MAX) && 
        (operation.V_ == DBL_MAX) &&
        (operation.N_ == false)) {
        auto epcs_ = operation.chord.epcs();
        std::vector<double> pcs;
        for (int voice = 0, voices = epcs_.voices(); voice < voices; ++voice) {
            auto pc = epcs_.getPitch(voice);
            if (std::find(pcs.begin(), pcs.end(), pc) == pcs.end()) {
                pcs.push_back(pc);
            }
        }
        // Without voice-leading from the prior segment.
        if (operation.L_ != DBL_MAX) {
            System::inform("  Operation: chord\n");
            score.setPitchClassSet(operation.begin, operation.end, pcs);
        // With voice-leading from the prior segment.
        } else {
            System::inform("  Operation: chordVoiceLeading\n");
            score.voicelead(priorOperation.begin,
                            priorOperation.end,
                            operation.begin,
                            operation.end,
                            pcs,
                            base,
                            range,
                            avoidParallels,
                            divisionsPerOctave);
            return;
        }
    }
    if (!(operation.K_ == DBL_MAX)) {
        if ((operation.V_ == DBL_MAX) && (!operation.L_)) {
            System::inform("  Operation: K\n");
            score.setK(priorOperation.begin,
                       operation.begin,
                       operation.end,
                       base,
                       range);
        } else if ((operation.V_ != DBL_MAX) && (!operation.L_)) {
            System::inform("  Operation: KV\n");
            score.setKV(priorOperation.begin,
                        operation.begin,
                        operation.end,
                        operation.V_,
                        base,
                        range);
        } else if ((operation.V_ == DBL_MAX) && (operation.L_)) {
            System::inform("  Operation: KL\n");
            score.setKL(priorOperation.begin,
                        operation.begin,
                        operation.end,
                        base,
                        range,
                        operation.avoidParallels);
        }
    } else if (!(operation.Q_ == DBL_MAX)) {
        if ((operation.V_ == DBL_MAX) && (!operation.L_)) {
            System::inform("  Operation: Q\n");
            score.setQ(priorOperation.begin,
                       operation.begin,
                       operation.end,
                       operation.Q_,
                       modality,
                       base,
                       range);
        } else if ((operation.V_ != DBL_MAX) && (!operation.L_)) {
            System::inform("  Operation: QV\n");
            score.setQV(priorOperation.begin,
                        operation.begin,
                        operation.end,
                        operation.Q_,
                        modality,
                        operation.V_,
                        base,
                        range);
        } else if ((operation.V_ == DBL_MAX) && (operation.L_)) {
            System::inform("  Operation: QL\n");
            score.setQL(priorOperation.begin,
                        operation.begin,
                        operation.end,
                        operation.Q_,
                        modality,
                        base,
                        range,
                        operation.avoidParallels);
        }
    } else if (!(operation.P_ == DBL_MAX) && !(operation.T_ == DBL_MAX)) {
        if (!(operation.V_ == DBL_MAX)) {
            System::inform("  Operation: PTV\n");
            score.setPTV(operation.begin,
                         operation.end,
                         operation.P_,
                         operation.T_,
                         operation.V_,
                         base,
                         range);
        } else if (operation.L_) {
            System::inform("  Operation: PTL\n");
            score.setPT(operation.begin,
                        operation.end,
                        operation.P_,
                        operation.T_,
                        base,
                        range,
                        divisionsPerOctave);
            score.voicelead(priorOperation.begin,
                            priorOperation.end,
                            operation.begin,
                            operation.end,
                            base,
                            range,
                            avoidParallels,
                            divisionsPerOctave);
        } else {
            System::inform("  Operation: PT\n");
            score.setPT(operation.begin,
                        operation.end,
                        operation.P_,
                        operation.T_,
                        base,
                        range,
                        divisionsPerOctave);
        }
    } else if (!(operation.C_ == DBL_MAX)) {
        if (!(operation.V_ == DBL_MAX)) {
            System::inform("  Operation: CV\n");
            std::vector<double> pcs = Voicelead::mToPitchClassSet(Voicelead::cToM(operation.C_, divisionsPerOctave), divisionsPerOctave);
            printChord("  CV", pcs);
            std::vector<double> pt = Voicelead::pitchClassSetToPandT(pcs, divisionsPerOctave);
            double prime = pt[0];
            double transposition = pt[1];
            System::inform("  prime: %f transposition %f: divisionsPerOctave %d\n", prime, transposition, divisionsPerOctave);
            score.setPTV(operation.begin,
                         operation.end,
                         prime,
                         transposition,
                         operation.V_,
                         base,
                         range);
        } else if (operation.L_) {
            System::inform("  Operation: CL\n");
            std::vector<double> pcs = Voicelead::mToPitchClassSet(Voicelead::cToM(operation.C_, divisionsPerOctave), divisionsPerOctave);
            printChord("  CL", pcs);
            score.voicelead(priorOperation.begin,
                            priorOperation.end,
                            operation.begin,
                            operation.end,
                            pcs,
                            base,
                            range,
                            avoidParallels,
                            divisionsPerOctave);
        } else {
            System::inform("  Operation: C\n");
            std::vector<double> pcs = Voicelead::mToPitchClassSet(Voicelead::cToM(operation.C_, divisionsPerOctave), divisionsPerOctave);
            score.setPitchClassSet(operation.begin,
                                   operation.end,
                                   pcs,
                                   divisionsPerOctave);
        }
    } else {
        if (!(operation.V_ == DBL_MAX)) {
            System::inform("  Operation: V\n");
            std::vector<double> ptv = score.getPTV(operation.begin,
                                                   operation.end,
                                                   base,
                                                   range,
                                                   divisionsPerOctave);
            score.setPTV(operation.begin,
                         operation.end,
                         ptv[0],
                         ptv[1],
                         operation.V_,
                         base,
                         range,
                         divisionsPerOctave);
        } else if (operation.L_) {
            System::inform("  Operation: L\n");
            score.voicelead(priorOperation.begin,
                            priorOperation.end,
                            operation.begin,
                            operation.end,
                            base,
                            range,
                            avoidParallels,
                            divisionsPerOctave);
        }
    }
    System::inform("VoiceleadingNode::apply.\n");
}

void VoiceleadingNode::PT(double time, double P_, double T_)
{
    operations[time].beginTime = time;
    operations[time].P_ = P_;
    operations[time].T_ = T_;
}

void VoiceleadingNode::PTV(double time, double P_, double T_, double V_)
{
    operations[time].beginTime = time;
    operations[time].P_ = P_;
    operations[time].T_ = T_;
    operations[time].V_ = V_;
}

void VoiceleadingNode::PTL(double time, double P_, double T_, bool avoidParallels)
{
    operations[time].beginTime = time;
    operations[time].P_ = P_;
    operations[time].T_ = T_;
    operations[time].L_ = true;
    operations[time].avoidParallels = avoidParallels;
}

void VoiceleadingNode::C(double time, double C_)
{
    operations[time].beginTime = time;
    operations[time].C_ = C_;
}

void VoiceleadingNode::C_name(double time, std::string C_)
{
    C(time, Voicelead::nameToC(C_, divisionsPerOctave));
}

void VoiceleadingNode::CV(double time, double C_, double V_)
{
    operations[time].beginTime = time;
    operations[time].C_ = C_;
    operations[time].V_ = V_;
}

void VoiceleadingNode::CV_name(double time, std::string C_, double V_)
{
    CV(time, Voicelead::nameToC(C_, divisionsPerOctave), V_);
}

void VoiceleadingNode::CL(double time, double C_, bool avoidParallels)
{
    operations[time].beginTime = time;
    operations[time].C_ = C_;
    operations[time].L_ = true;
    operations[time].avoidParallels = avoidParallels;
}

void VoiceleadingNode::CL_name(double time, std::string C_, bool avoidParallels)
{
    CL(time, Voicelead::nameToC(C_, divisionsPerOctave), avoidParallels);
}

void VoiceleadingNode::K(double time)
{
    operations[time].beginTime = time;
    operations[time].K_ = 1.0;
}

void VoiceleadingNode::KV(double time, double V_)
{
    operations[time].beginTime = time;
    operations[time].K_ = 1.0;
    operations[time].V_ = V_;
}

void VoiceleadingNode::KL(double time, bool avoidParallels)
{
    operations[time].beginTime = time;
    operations[time].K_ = 1.0;
    operations[time].L_ = true;
    operations[time].avoidParallels = avoidParallels;
}

void VoiceleadingNode::Q(double time, double Q_)
{
    operations[time].beginTime = time;
    operations[time].Q_ = Q_;
}

void VoiceleadingNode::QV(double time, double Q_, double V_)
{
    operations[time].beginTime = time;
    operations[time].Q_ = Q_;
    operations[time].V_ = V_;
}

void VoiceleadingNode::QL(double time, double Q_, bool avoidParallels)
{
    operations[time].beginTime = time;
    operations[time].Q_ = Q_;
    operations[time].L_ = true;
    operations[time].avoidParallels = avoidParallels;
}

void VoiceleadingNode::V(double time, double V_)
{
    operations[time].beginTime = time;
    operations[time].V_ = V_;
}

void VoiceleadingNode::NOP(double time)
{
    operations[time].beginTime = time;
    operations[time].N_ = true;
}

void VoiceleadingNode::L(double time, bool avoidParallels)
{
    operations[time].beginTime = time;
    operations[time].L_ = true;
    operations[time].avoidParallels = avoidParallels;
}

void VoiceleadingNode::transform(Score &score)
{
    if (operations.empty()) {
        return;
    }
    // Find the time rescale factor.
    score.sort();
    score.findScale();
    double scoreMinTime = score.scaleActualMinima.getTime();
    double scoreDuration = score.getDuration();
    double scoreMaxTime = scoreMinTime + scoreDuration;
    double operationMaxTime = 0.0;
    double timeScale = 1.0;
    System::inform("VoiceleadingNode::transform...\n");
    System::inform("  operations:    %d\n", operations.size());
    System::inform("  scoreMinTime:  %f\n", scoreMinTime);
    System::inform("  scoreDuration: %f\n", scoreDuration);
    System::inform("  scoreMaxTime:  %f\n", scoreMaxTime);
    System::inform("  rescaleTimes:  %b\n", rescaleTimes);
    std::vector<VoiceleadingOperation *> ops;
    for (std::map<double, VoiceleadingOperation>::iterator it = operations.begin(); it != operations.end(); ++it) {
        if (it->second.beginTime > operationMaxTime) {
            operationMaxTime = it->second.beginTime;
        }
        ops.push_back(&it->second);
    }
    if (rescaleTimes) {
        if (operationMaxTime > 0.0) {
            timeScale = scoreMaxTime / operationMaxTime;
        }
    }
    VoiceleadingOperation *priorOperation = 0;
    VoiceleadingOperation *currentOperation = 0;
    VoiceleadingOperation *nextOperation = 0;
    int priorIndex = 0;
    int currentIndex = 0;
    int nextIndex = 0;
    int backIndex = ops.size() - 1;
    int endIndex = ops.size();
    for (currentIndex = 0; currentIndex < endIndex; ++currentIndex) {
        if (currentIndex == 0) {
            priorIndex = currentIndex;
        } else {
            priorIndex = currentIndex - 1;
        }
        if (currentIndex == backIndex) {
            nextIndex = currentIndex;
        } else {
            nextIndex = currentIndex + 1;
        }
        priorOperation = ops[priorIndex];
        currentOperation = ops[currentIndex];
        nextOperation = ops[nextIndex];
        currentOperation->rescaledBeginTime = currentOperation->beginTime * timeScale;
        currentOperation->begin = score.indexAtTime(currentOperation->rescaledBeginTime);
        currentOperation->rescaledEndTime = nextOperation->beginTime * timeScale;
        currentOperation->end = score.indexAfterTime(currentOperation->rescaledEndTime);
        apply(score, *priorOperation, *currentOperation);
    }
    System::inform("VoiceleadingNode::transform.\n");
}

void VoiceleadingNode::setModality(const std::vector<double> &pcs)
{
    modality = pcs;
}

std::vector<double> VoiceleadingNode::getModality() const
{
    return modality;
}

void VoiceleadingNode::chord(const csound::Chord &chord, double time)
{
    operations[time].beginTime = time;
    operations[time].chord = chord;
}

void VoiceleadingNode::chordVoiceleading(const csound::Chord &chord, double time, bool avoid_parallels = true)
{
    operations[time].beginTime = time;
    operations[time].chord = chord;
    operations[time].L_ = true;
    operations[time].avoidParallels = avoid_parallels;
}

}
