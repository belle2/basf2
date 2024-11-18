/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/KinkVariables.h>

// variables
#include <analysis/VariableManager/Manager.h>

// dataobjects from the analysis
#include <analysis/dataobjects/Particle.h>

// dataobjects from the framework
#include <framework/dataobjects/Helix.h>

// dataobjects from the MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/Kink.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

// framework aux
#include <framework/logging/Logger.h>

// extra
#include <algorithm>
#include <TVector2.h>
#include <cmath>
#include <Math/Boost.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    //-----------------------------------------------
    // Helper Functions

    Helix kinkDaughterHelixAtKinkVertex(const Kink* kink)
    {
      Helix daughterHelixAtKinkVertex = kink->getDaughterTrackFitResult()->getHelix();
      daughterHelixAtKinkVertex.passiveMoveBy(kink->getFittedVertexX(), kink->getFittedVertexY(), 0);
      return daughterHelixAtKinkVertex;
    }

    ROOT::Math::XYZVector kinkDaughterMomentumAtKinkVertex(const Kink* kink)
    {
      const double BzAtKinkVertex = BFieldManager::getFieldInTesla(
      {kink->getFittedVertexX(), kink->getFittedVertexY(), kink->getFittedVertexZ()}).Z();
      Helix daughterHelixAtKinkVertex = kink->getDaughterTrackFitResult()->getHelix();
      daughterHelixAtKinkVertex.passiveMoveBy(kink->getFittedVertexX(), kink->getFittedVertexY(), 0);
      return daughterHelixAtKinkVertex.getMomentum(BzAtKinkVertex);
    }

    Helix kinkMotherHelixAtKinkVertex(const Kink* kink)
    {
      Helix motherHelixAtKinkVertex = kink->getMotherTrackFitResultEnd()->getHelix();
      motherHelixAtKinkVertex.passiveMoveBy(kink->getFittedVertexX(), kink->getFittedVertexY(), 0);
      return motherHelixAtKinkVertex;
    }

    ROOT::Math::XYZVector kinkMotherMomentumAtKinkVertex(const Kink* kink)
    {
      const double BzAtKinkVertex = BFieldManager::getFieldInTesla(
      {kink->getFittedVertexX(), kink->getFittedVertexY(), kink->getFittedVertexZ()}).Z();
      Helix motherHelixAtKinkVertex = kink->getMotherTrackFitResultEnd()->getHelix();
      motherHelixAtKinkVertex.passiveMoveBy(kink->getFittedVertexX(), kink->getFittedVertexY(), 0);
      return motherHelixAtKinkVertex.getMomentum(BzAtKinkVertex);
    }

    double kinkDaughterMomentumAndCosThetaInMotherRF(const Particle* part, Const::ChargedStable motherType,
                                                     Const::ChargedStable daughterType, bool returnCosTheta)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      ROOT::Math::XYZVector motherMomentumAtKinkVertex = kinkMotherMomentumAtKinkVertex(kink);
      ROOT::Math::XYZVector daughterMomentumAtKinkVertex = kinkDaughterMomentumAtKinkVertex(kink);

      double motherEnergy = sqrt(motherMomentumAtKinkVertex.Mag2() + motherType.getMass() * motherType.getMass());
      double daughterEnergy = sqrt(daughterMomentumAtKinkVertex.Mag2() + daughterType.getMass() * daughterType.getMass());

      ROOT::Math::PxPyPzEVector mother4MomentumAtKinkVertex(motherMomentumAtKinkVertex.X(),
                                                            motherMomentumAtKinkVertex.Y(),
                                                            motherMomentumAtKinkVertex.Z(),
                                                            motherEnergy);
      int signDiff = kink->getMotherTrackFitResultEnd()->getChargeSign() * kink->getDaughterTrackFitResult()->getChargeSign();
      ROOT::Math::PxPyPzEVector daughter4MomentumAtKinkVertex(signDiff * daughterMomentumAtKinkVertex.X(),
                                                              signDiff * daughterMomentumAtKinkVertex.Y(),
                                                              signDiff * daughterMomentumAtKinkVertex.Z(),
                                                              daughterEnergy);

      B2Vector3D motherBoostAtKinkVertex = mother4MomentumAtKinkVertex.BoostToCM();
      daughter4MomentumAtKinkVertex = ROOT::Math::Boost(motherBoostAtKinkVertex) * daughter4MomentumAtKinkVertex;

      if (returnCosTheta)
        return daughter4MomentumAtKinkVertex.Vect().Unit().Dot(mother4MomentumAtKinkVertex.Vect().Unit());
      else
        return sqrt(daughter4MomentumAtKinkVertex.Vect().Mag2());
    }

    ROOT::Math::PxPyPzEVector kinkMotherMCP4AtDecayVertex(const MCParticle* p)
    {
      ROOT::Math::PxPyPzEVector P4(0, 0, 0, 0);
      ROOT::Math::XYZVector mcDecayVertex = p->getDecayVertex();
      std::vector<MCParticle*> mcDaughters = p->getDaughters();
      for (std::vector<MCParticle*>::iterator daughterIter = mcDaughters.begin();
           daughterIter != mcDaughters.end(); ++daughterIter) {
        ROOT::Math::XYZVector mcDaughterVertex = (*daughterIter)->getVertex();
        if ((mcDaughterVertex - mcDecayVertex).Mag2() < 1)
          P4 += (*daughterIter)->get4Vector();
      }
      return P4;
    }

    double kinkDaughterMCMomentumAndCosThetaInMotherRF(const Particle* part, bool returnCosTheta)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();
      if (motherTrack == daughterTrack) return Const::doubleNaN;

      const MCParticle* motherMCParticle = motherTrack->getRelated<MCParticle>();
      const MCParticle* daughterMCParticle = daughterTrack->getRelated<MCParticle>();

      if (!motherMCParticle || !daughterMCParticle) return Const::doubleNaN;

      ROOT::Math::PxPyPzEVector mother4MomentumAtDecayVertex = kinkMotherMCP4AtDecayVertex(motherMCParticle);
      ROOT::Math::PxPyPzEVector daughter4MomentumAtProductionVertex = daughterMCParticle->get4Vector();

      B2Vector3D motherBoostAtDecayVertex = mother4MomentumAtDecayVertex.BoostToCM();
      ROOT::Math::PxPyPzEVector daughter4MomentumAtDecayVertex = ROOT::Math::Boost(motherBoostAtDecayVertex) *
                                                                 daughter4MomentumAtProductionVertex;

      if (returnCosTheta)
        return daughter4MomentumAtDecayVertex.Vect().Unit().Dot(mother4MomentumAtDecayVertex.Vect().Unit());
      else
        return sqrt(daughter4MomentumAtDecayVertex.Vect().Mag2());
    }

    //-----------------------------------------------
    // MEASURED VARIABLES
    // Kink General Variables

    double kinkVertexX(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getFittedVertexX();
    }

    double kinkVertexY(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getFittedVertexY();
    }

    double kinkVertexZ(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getFittedVertexZ();
    }

    double kinkFilterID(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getPrefilterFlag();
    }

    double kinkCombinedFitResultFlag(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      if (kinkFilterID(part) < 3)
        return kink->getCombinedFitResultFlag();
      else
        return Const::doubleNaN;
    }

    double kinkCombinedFitResultFlagBit1(const Particle* part)
    {
      double flag = kinkCombinedFitResultFlag(part);
      if (flag < 16)
        return static_cast<int>(flag) & 0b0001;
      else
        return Const::doubleNaN;
    }

    double kinkCombinedFitResultFlagBit2(const Particle* part)
    {
      double flag = kinkCombinedFitResultFlag(part);
      if (flag < 16)
        return  static_cast<int>(flag) & 0b0010;
      else
        return Const::doubleNaN;
    }

    double kinkCombinedFitResultFlagBit3(const Particle* part)
    {
      double flag = kinkCombinedFitResultFlag(part);
      if (flag < 16)
        return  static_cast<int>(flag) & 0b0100;
      else
        return Const::doubleNaN;
    }

    double kinkCombinedFitResultFlagBit4(const Particle* part)
    {
      double flag = kinkCombinedFitResultFlag(part);
      if (flag < 16)
        return  static_cast<int>(flag) & 0b1000;
      else
        return Const::doubleNaN;
    }

    double kinkSplitTrackDistanceAtVertexFlag(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      if (kinkFilterID(part) < 3)
        return Const::doubleNaN;
      else
        return kink->getSplitTrackDistanceAtVertexFlag();
    }

    double kinkNumberOfReassignedHits(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getNumberOfReassignedHits();
    }

    double kinkIsSameCharge(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      return kink->getMotherTrackFitResultEnd()->getChargeSign() *
             kink->getDaughterTrackFitResult()->getChargeSign() > 0 ? true : false;
    }

    // Kink Decay Kinematics

    double kinkDaughterMomentumInMotherRF(const Particle* part)
    {
      if (!(part->hasExtraInfo("kinkDaughterPDGCode")))
        return Const::doubleNaN;
      Const::ChargedStable daughterType(abs(part->getExtraInfo("kinkDaughterPDGCode")));
      Const::ChargedStable motherType(abs(part->getPDGCode()));
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, motherType, daughterType, false);
    }

    double kinkDaughterCosThetaInMotherRF(const Particle* part)
    {
      if (!(part->hasExtraInfo("kinkDaughterPDGCode")))
        return Const::doubleNaN;
      Const::ChargedStable daughterType(abs(part->getExtraInfo("kinkDaughterPDGCode")));
      Const::ChargedStable motherType(abs(part->getPDGCode()));
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, motherType, daughterType, true);
    }

    double kinkDaughterMomentumInMotherRFKPi(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::kaon, Const::pion, false);
    }

    double kinkDaughterCosThetaInMotherRFKPi(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::kaon, Const::pion, true);
    }

    double kinkDaughterMomentumInMotherRFKMu(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::kaon, Const::muon, false);
    }

    double kinkDaughterCosThetaInMotherRFKMu(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::kaon, Const::muon, true);
    }

    double kinkDaughterMomentumInMotherRFPiMu(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::pion, Const::muon, false);
    }

    double kinkDaughterCosThetaInMotherRFPiMu(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::pion, Const::muon, true);
    }

    double kinkDaughterMomentumInMotherRFMuE(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::muon, Const::electron, false);
    }

    double kinkDaughterCosThetaInMotherRFMuE(const Particle* part)
    {
      return kinkDaughterMomentumAndCosThetaInMotherRF(part, Const::muon, Const::electron, true);
    }

    // Kink Daughter Measured Track Parameters

    double kinkDaughterTrackNCDCHits(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getHitPatternCDC().getNHits();
    }

    double kinkDaughterTrackNSVDHits(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getHitPatternVXD().getNSVDHits();
    }

    double kinkDaughterTrackNPXDHits(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getHitPatternVXD().getNPXDHits();
    }

    double kinkDaughterTrackNVXDHits(const Particle* part)
    {
      return kinkDaughterTrackNSVDHits(part) + kinkDaughterTrackNPXDHits(part);
    }

    double kinkDaughterTrackFirstSVDLayer(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getHitPatternVXD().getFirstSVDLayer();
    }

    double kinkDaughterTrackFirstPXDLayer(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getHitPatternVXD().getFirstPXDLayer(HitPatternVXD::PXDMode::normal);
    }

    double kinkDaughterTrackFirstCDCLayer(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getHitPatternCDC().getFirstLayer();
    }

    double kinkDaughterTrackLastCDCLayer(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getHitPatternCDC().getLastLayer();
    }

    double kinkDaughterTrackPValue(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getPValue();
    }

    double kinkDaughterTrackNDF(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getNDF();
    }

    double kinkDaughterTrackChi2(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getChi2();
    }

    double kinkDaughterInitTrackPValue(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      const Track* daughterTrack = kink->getDaughterTrack();
      const TrackFitResult* daughterTrackFitResult = daughterTrack->getTrackFitResultWithClosestMass(Const::pion);
      if (!daughterTrackFitResult) return Const::doubleNaN;
      return daughterTrackFitResult->getPValue();
    }

    double kinkDaughterInitTrackNDF(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      const Track* daughterTrack = kink->getDaughterTrack();
      const TrackFitResult* daughterTrackFitResult = daughterTrack->getTrackFitResultWithClosestMass(Const::pion);
      if (!daughterTrackFitResult) return Const::doubleNaN;
      return daughterTrackFitResult->getNDF();
    }

    double kinkDaughterInitTrackChi2(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      const Track* daughterTrack = kink->getDaughterTrack();
      const TrackFitResult* daughterTrackFitResult = daughterTrack->getTrackFitResultWithClosestMass(Const::pion);
      if (!daughterTrackFitResult) return Const::doubleNaN;
      return daughterTrackFitResult->getChi2();
    }

    double kinkDaughterTrackD0(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getD0();
    }

    double kinkDaughterTrackPhi0(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getPhi0();
    }

    double kinkDaughterTrackOmega(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getOmega();
    }

    double kinkDaughterTrackZ0(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getZ0();
    }

    double kinkDaughterTrackTanLambda(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;
      return kink->getDaughterTrackFitResult()->getTanLambda();
    }

    double kinkDaughterTrackD0Error(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      double errorSquared = kink->getDaughterTrackFitResult()->getCovariance5()[0][0];
      if (errorSquared <= 0) return Const::doubleNaN;
      return sqrt(errorSquared);
    }

    double kinkDaughterTrackPhi0Error(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      double errorSquared = kink->getDaughterTrackFitResult()->getCovariance5()[1][1];
      if (errorSquared <= 0) return Const::doubleNaN;
      return sqrt(errorSquared);
    }

    double kinkDaughterTrackOmegaError(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      double errorSquared = kink->getDaughterTrackFitResult()->getCovariance5()[2][2];
      if (errorSquared <= 0) return Const::doubleNaN;
      return sqrt(errorSquared);
    }

    double kinkDaughterTrackZ0Error(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      double errorSquared = kink->getDaughterTrackFitResult()->getCovariance5()[3][3];
      if (errorSquared <= 0) return Const::doubleNaN;
      return sqrt(errorSquared);
    }

    double kinkDaughterTrackTanLambdaError(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      double errorSquared = kink->getDaughterTrackFitResult()->getCovariance5()[4][4];
      if (errorSquared <= 0) return Const::doubleNaN;
      return sqrt(errorSquared);
    }

    double kinkDaughterTrackD0AtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      Helix daughterHelixAtKinkVertex = kinkDaughterHelixAtKinkVertex(kink);
      return daughterHelixAtKinkVertex.getD0();
    }

    double kinkDaughterTrackZ0AtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      Helix daughterHelixAtKinkVertex = kinkDaughterHelixAtKinkVertex(kink);
      return daughterHelixAtKinkVertex.getZ0();
    }

    double kinkDaughterPtAtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      ROOT::Math::XYZVector daughterMomentumAtKinkVertex = kinkDaughterMomentumAtKinkVertex(kink);
      return sqrt(daughterMomentumAtKinkVertex.Perp2());
    }

    double kinkDaughterPzAtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      ROOT::Math::XYZVector daughterMomentumAtKinkVertex = kinkDaughterMomentumAtKinkVertex(kink);
      return daughterMomentumAtKinkVertex.Z();
    }

    double kinkDaughterPAtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      ROOT::Math::XYZVector daughterMomentumAtKinkVertex = kinkDaughterMomentumAtKinkVertex(kink);
      return sqrt(daughterMomentumAtKinkVertex.Mag2());
    }

    // Kink Mother Measured Track Parameters

    double kinkMotherInitTrackPValue(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const TrackFitResult* motherTrackFitResult = motherTrack->getTrackFitResultWithClosestMass(Const::pion);
      if (!motherTrackFitResult) return Const::doubleNaN;

      return motherTrackFitResult->getPValue();
    }

    double kinkMotherInitTrackNDF(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const TrackFitResult* motherTrackFitResult = motherTrack->getTrackFitResultWithClosestMass(Const::pion);
      if (!motherTrackFitResult) return Const::doubleNaN;

      return motherTrackFitResult->getNDF();
    }

    double kinkMotherInitTrackChi2(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const TrackFitResult* motherTrackFitResult = motherTrack->getTrackFitResultWithClosestMass(Const::pion);
      if (!motherTrackFitResult) return Const::doubleNaN;

      return motherTrackFitResult->getChi2();
    }

    double kinkMotherTrackD0AtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      Helix motherHelixAtKinkVertex = kinkMotherHelixAtKinkVertex(kink);
      return motherHelixAtKinkVertex.getD0();
    }

    double kinkMotherTrackZ0AtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      Helix motherHelixAtKinkVertex = kinkMotherHelixAtKinkVertex(kink);
      return motherHelixAtKinkVertex.getZ0();
    }

    double kinkMotherPtAtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      ROOT::Math::XYZVector motherMomentumAtKinkVertex = kinkMotherMomentumAtKinkVertex(kink);
      return sqrt(motherMomentumAtKinkVertex.Perp2());
    }

    double kinkMotherPzAtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      ROOT::Math::XYZVector motherMomentumAtKinkVertex = kinkMotherMomentumAtKinkVertex(kink);
      return motherMomentumAtKinkVertex.Z();
    }

    double kinkMotherPAtKinkVertex(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      ROOT::Math::XYZVector motherMomentumAtKinkVertex = kinkMotherMomentumAtKinkVertex(kink);
      return sqrt(motherMomentumAtKinkVertex.Mag2());
    }

    //-----------------------------------------------
    // Check if the particle is a part of any Kink

    double particleIsInKink(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return Const::doubleNaN;

      const short trackIndex = track->getArrayIndex();
      StoreArray<Kink> kinks;

      for (int i = 0; i < kinks.getEntries(); i++) {
        const Kink* kink = kinks[i];
        const short motherTrackIndex = kink->getMotherTrackIndex();
        const short daughterTrackIndex = kink->getDaughterTrackIndex();
        if (trackIndex == motherTrackIndex || trackIndex == daughterTrackIndex)
          return true;
      }
      return false;
    }


    double particleIsMotherInKink(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return Const::doubleNaN;

      const short trackIndex = track->getArrayIndex();
      StoreArray<Kink> kinks;

      for (int i = 0; i < kinks.getEntries(); i++) {
        const Kink* kink = kinks[i];
        const short motherTrackIndex = kink->getMotherTrackIndex();
        if (trackIndex == motherTrackIndex)
          return true;
      }
      return false;
    }


    double particleIsDaughterInKink(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return Const::doubleNaN;

      const short trackIndex = track->getArrayIndex();
      StoreArray<Kink> kinks;

      for (int i = 0; i < kinks.getEntries(); i++) {
        const Kink* kink = kinks[i];
        const short daughterTrackIndex = kink->getDaughterTrackIndex();
        if (trackIndex == daughterTrackIndex)
          return true;
      }
      return false;
    }

    double particleIsSplitKink(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return Const::doubleNaN;

      const short trackIndex = track->getArrayIndex();
      StoreArray<Kink> kinks;

      for (int i = 0; i < kinks.getEntries(); i++) {
        const Kink* kink = kinks[i];
        const short motherTrackIndex = kink->getMotherTrackIndex();
        const short daughterTrackIndex = kink->getDaughterTrackIndex();
        if (trackIndex == motherTrackIndex && motherTrackIndex == daughterTrackIndex)
          return true;
      }
      return false;
    }

    //-----------------------------------------------
    // MC VARIABLES
    // Kink from track pair MC variables

    double kinkPairIsMCRelated(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();

      if (motherTrack == daughterTrack) return Const::doubleNaN;

      const MCParticle* motherMCParticle = motherTrack->getRelated<MCParticle>();
      const MCParticle* daughterMCParticle = daughterTrack->getRelated<MCParticle>();

      if (!motherMCParticle || !daughterMCParticle) return false;
      if (motherMCParticle == daughterMCParticle) return true;
      if (daughterMCParticle->getMother() && daughterMCParticle->getMother() == motherMCParticle) return true;
      return false;
    }

    double kinkPairIsClone(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();

      if (motherTrack == daughterTrack) return Const::doubleNaN;

      const MCParticle* motherMCParticle = motherTrack->getRelated<MCParticle>();
      const MCParticle* daughterMCParticle = daughterTrack->getRelated<MCParticle>();

      if (!motherMCParticle || !daughterMCParticle) return false;
      if (motherMCParticle == daughterMCParticle) return true;
      return false;
    }

    double kinkPairIsReal(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();

      if (motherTrack == daughterTrack) return Const::doubleNaN;

      const MCParticle* motherMCParticle = motherTrack->getRelated<MCParticle>();
      const MCParticle* daughterMCParticle = daughterTrack->getRelated<MCParticle>();

      if (!motherMCParticle || !daughterMCParticle) return false;
      if (daughterMCParticle->getMother() && daughterMCParticle->getMother() == motherMCParticle) return true;
      return false;
    }

    double kinkPairIsDecayInFlight(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();

      if (motherTrack == daughterTrack) return Const::doubleNaN;

      const MCParticle* motherMCParticle = motherTrack->getRelated<MCParticle>();
      const MCParticle* daughterMCParticle = daughterTrack->getRelated<MCParticle>();

      if (!motherMCParticle || !daughterMCParticle) return false;
      if (daughterMCParticle->getMother() &&
          daughterMCParticle->getMother() == motherMCParticle &&
          daughterMCParticle->getSecondaryPhysicsProcess() == 201) return true;
      return false;
    }

    double kinkPairIsHadronScattering(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return Const::doubleNaN;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();

      if (motherTrack == daughterTrack) return Const::doubleNaN;

      const MCParticle* motherMCParticle = motherTrack->getRelated<MCParticle>();
      const MCParticle* daughterMCParticle = daughterTrack->getRelated<MCParticle>();

      if (!motherMCParticle || !daughterMCParticle) return false;
      if (daughterMCParticle->getMother() &&
          daughterMCParticle->getMother() == motherMCParticle &&
          daughterMCParticle->getSecondaryPhysicsProcess() == 121) return true;
      return false;
    }

    // Kink from track pair MC kinematics

    double kinkDaughterMomentumInMotherRFMC(const Particle* part)
    {
      return kinkDaughterMCMomentumAndCosThetaInMotherRF(part, false);
    }

    double kinkDaughterCosThetaInMotherRFMC(const Particle* part)
    {
      return kinkDaughterMCMomentumAndCosThetaInMotherRF(part, true);
    }

    // Kink from track pair daughter MC variables

    const MCParticle* kinkPairDaughterMCParticle(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return nullptr;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();

      if (motherTrack == daughterTrack) return nullptr;
      return daughterTrack->getRelated<MCParticle>();
    }

    double kinkPairDaughterMCPDG(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->getPDG();
    }

    double kinkPairDaughterMCMotherPDG(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      if (daughterMCParticle->getMother())
        return daughterMCParticle->getMother()->getPDG();
      else
        return Const::doubleNaN;
    }

    double kinkPairDaughterMCSecProc(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->getSecondaryPhysicsProcess();
    }

    double kinkPairDaughterMCVertexX(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->getVertex().X();
    }

    double kinkPairDaughterMCVertexY(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->getVertex().Y();
    }

    double kinkPairDaughterMCVertexZ(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->getVertex().Z();
    }

    double kinkPairDaughterMCPX(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->get4Vector().X();
    }

    double kinkPairDaughterMCPY(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->get4Vector().Y();
    }

    double kinkPairDaughterMCPZ(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->get4Vector().Z();
    }

    double kinkPairDaughterMCPT(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return sqrt(daughterMCParticle->get4Vector().Vect().Perp2());
    }

    double kinkPairDaughterMCP(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return sqrt(daughterMCParticle->get4Vector().Vect().Mag2());
    }

    double kinkPairDaughterMCE(const Particle* part)
    {
      const MCParticle* daughterMCParticle = kinkPairDaughterMCParticle(part);
      if (!daughterMCParticle) return Const::doubleNaN;
      return daughterMCParticle->get4Vector().E();
    }

    // Kink from track pair mother MC variables

    const MCParticle* kinkPairMotherMCParticle(const Particle* part)
    {
      const Kink* kink = part->getKink();
      if (!kink) return nullptr;

      const Track* motherTrack = kink->getMotherTrack();
      const Track* daughterTrack = kink->getDaughterTrack();

      if (motherTrack == daughterTrack) return nullptr;

      return motherTrack->getRelated<MCParticle>();
    }

    double kinkPairMotherMCPXAtDecayVertex(const Particle* part)
    {
      const MCParticle* motherMCParticle = kinkPairMotherMCParticle(part);
      if (!motherMCParticle) return Const::doubleNaN;
      return kinkMotherMCP4AtDecayVertex(motherMCParticle).X();
    }

    double kinkPairMotherMCPYAtDecayVertex(const Particle* part)
    {
      const MCParticle* motherMCParticle = kinkPairMotherMCParticle(part);
      if (!motherMCParticle) return Const::doubleNaN;
      return kinkMotherMCP4AtDecayVertex(motherMCParticle).Y();
    }

    double kinkPairMotherMCPZAtDecayVertex(const Particle* part)
    {
      const MCParticle* motherMCParticle = kinkPairMotherMCParticle(part);
      if (!motherMCParticle) return Const::doubleNaN;
      return kinkMotherMCP4AtDecayVertex(motherMCParticle).Z();
    }

    double kinkPairMotherMCPTAtDecayVertex(const Particle* part)
    {
      const MCParticle* motherMCParticle = kinkPairMotherMCParticle(part);
      if (!motherMCParticle) return Const::doubleNaN;
      return sqrt(kinkMotherMCP4AtDecayVertex(motherMCParticle).Vect().Perp2());
    }

    double kinkPairMotherMCPAtDecayVertex(const Particle* part)
    {
      const MCParticle* motherMCParticle = kinkPairMotherMCParticle(part);
      if (!motherMCParticle) return Const::doubleNaN;
      return sqrt(kinkMotherMCP4AtDecayVertex(motherMCParticle).Vect().Mag2());
    }

    double kinkPairMotherMCEAtDecayVertex(const Particle* part)
    {
      const MCParticle* motherMCParticle = kinkPairMotherMCParticle(part);
      if (!motherMCParticle) return Const::doubleNaN;
      return kinkMotherMCP4AtDecayVertex(motherMCParticle).E();
    }

    //-----------------------------------------------
    // REGISTER VARIABLES

    VARIABLE_GROUP("Kink");

    //-----------------------------------------------
    // MEASURED VARIABLES
    // Kink General Variables
    REGISTER_VARIABLE("kinkVertexX", kinkVertexX, "x coordinate of kink vertex");
    REGISTER_VARIABLE("kinkVertexY", kinkVertexY, "y coordinate of kink vertex");
    REGISTER_VARIABLE("kinkVertexZ", kinkVertexZ, "z coordinate of kink vertex");
    REGISTER_VARIABLE("kinkFilterID", kinkFilterID, "Filter ID with which kink was preselected");
    REGISTER_VARIABLE("kinkCombinedFitResultFlag", kinkCombinedFitResultFlag, "Flag of the combined kink fit result");
    REGISTER_VARIABLE("kinkCombinedFitResultFlagB1", kinkCombinedFitResultFlagBit1,
                      "The first bit of the flag of the combined kink fit result");
    REGISTER_VARIABLE("kinkCombinedFitResultFlagB2", kinkCombinedFitResultFlagBit2,
                      "The first bit of the flag of the combined kink fit result");
    REGISTER_VARIABLE("kinkCombinedFitResultFlagB3", kinkCombinedFitResultFlagBit3,
                      "The first bit of the flag of the combined kink fit result");
    REGISTER_VARIABLE("kinkCombinedFitResultFlagB4", kinkCombinedFitResultFlagBit4,
                      "The first bit of the flag of the combined kink fit result");
    REGISTER_VARIABLE("kinkSplitTrackDistanceAtVertexFlag", kinkSplitTrackDistanceAtVertexFlag,
                      "Flag showing if the split kink failed the distance criteria at the kink vertex");
    REGISTER_VARIABLE("kinkNumberOfReassignedHits", kinkNumberOfReassignedHits,
                      "Number of reassigned hits between kink mother and daughter tracks");
    REGISTER_VARIABLE("kinkIsSameCharge", kinkIsSameCharge,
                      "Check if charges of mother and daughter tracks are the same");

    // Kink Decay Kinematics
    REGISTER_VARIABLE("pMD", kinkDaughterMomentumInMotherRF,
                      "Kink daughter momentum in mother rest frame with default pair of mass hypotheses,"
                      " set by a user in the decay string");
    REGISTER_VARIABLE("cosMD", kinkDaughterCosThetaInMotherRF,
                      "Kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame"
                      " with default pair of mass hypotheses, set by a user in the decay string");
    REGISTER_VARIABLE("pKpi", kinkDaughterMomentumInMotherRFKPi,
                      "Kink daughter momentum in mother rest frame with pion and kaon mass hypotheses");
    REGISTER_VARIABLE("cosKpi", kinkDaughterCosThetaInMotherRFKPi,
                      "Kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame"
                      " with pion and kaon mass hypotheses");
    REGISTER_VARIABLE("pKmu", kinkDaughterMomentumInMotherRFKMu,
                      "Kink daughter momentum in mother rest frame with muon and kaon mass hypotheses");
    REGISTER_VARIABLE("cosKmu", kinkDaughterCosThetaInMotherRFKMu,
                      "Kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame"
                      " with muon and kaon mass hypotheses");
    REGISTER_VARIABLE("pPimu", kinkDaughterMomentumInMotherRFPiMu,
                      "Kink daughter momentum in mother rest frame with muon and pion mass hypotheses");
    REGISTER_VARIABLE("cosPimu", kinkDaughterCosThetaInMotherRFPiMu,
                      "Kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame"
                      " with muon and pion mass hypotheses");
    REGISTER_VARIABLE("pMue", kinkDaughterMomentumInMotherRFMuE,
                      "Kink daughter momentum in mother rest frame with electron and muon mass hypotheses");
    REGISTER_VARIABLE("cosMue", kinkDaughterCosThetaInMotherRFMuE,
                      "Kink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame"
                      " with electron and muon mass hypotheses");

    // Kink Daughter Measured Track Parameters
    REGISTER_VARIABLE("kinkDaughterTrackNCDCHits", kinkDaughterTrackNCDCHits,
                      "Number of CDC hits of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackNSVDHits", kinkDaughterTrackNSVDHits,
                      "Number of SVD hits of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackNPXDHits", kinkDaughterTrackNPXDHits,
                      "Number of PXD hits of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackNVXDHits", kinkDaughterTrackNVXDHits,
                      "Number of VXD hits of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackFirstSVDLayer", kinkDaughterTrackFirstSVDLayer,
                      "First activated SVD layer of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackFirstPXDLayer", kinkDaughterTrackFirstPXDLayer,
                      "First activated PXD layer of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackFirstCDCLayer", kinkDaughterTrackFirstCDCLayer,
                      "First activated CDC layer of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackLastCDCLayer", kinkDaughterTrackLastCDCLayer,
                      "Last activated CDC layer of kink daughter track");
    REGISTER_VARIABLE("kinkDaughterTrackPValue", kinkDaughterTrackPValue,
                      "P-value of kink daughter track fit");
    REGISTER_VARIABLE("kinkDaughterTrackNDF", kinkDaughterTrackNDF,
                      "NDF of kink daughter track fit");
    REGISTER_VARIABLE("kinkDaughterTrackChi2", kinkDaughterTrackChi2,
                      "Chi2 of kink daughter track fit");
    REGISTER_VARIABLE("kinkDaughterInitTrackPValue", kinkDaughterInitTrackPValue,
                      "P-value of initial kink daughter track fit");
    REGISTER_VARIABLE("kinkDaughterInitTrackNDF", kinkDaughterInitTrackNDF,
                      "NDF of initial kink daughter track fit");
    REGISTER_VARIABLE("kinkDaughterInitTrackChi2", kinkDaughterInitTrackChi2,
                      "Chi2 of initial kink daughter track fit");
    REGISTER_VARIABLE("kinkDaughterTrackD0", kinkDaughterTrackD0,
                      "D0 impact parameter of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackPhi0", kinkDaughterTrackPhi0,
                      "Transverse momentum angle of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackOmega", kinkDaughterTrackOmega,
                      "Curvature of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackZ0", kinkDaughterTrackZ0,
                      "Z0 impact parameter of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackTanLambda", kinkDaughterTrackTanLambda,
                      "Slope of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackD0Error", kinkDaughterTrackD0Error,
                      "DO error of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackPhi0Error", kinkDaughterTrackPhi0Error,
                      "Phi0 error of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackOmegaError", kinkDaughterTrackOmegaError,
                      "Omega error of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackZ0Error", kinkDaughterTrackZ0Error,
                      "Z0 error of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackTanLambdaError", kinkDaughterTrackTanLambdaError,
                      "Tan(lambda) error of kink daughter track at IP");
    REGISTER_VARIABLE("kinkDaughterTrackD0AtKinkVertex", kinkDaughterTrackD0AtKinkVertex,
                      "D0 impact parameter of kink daughter track at kink vertex");
    REGISTER_VARIABLE("kinkDaughterTrackZ0AtKinkVertex", kinkDaughterTrackZ0AtKinkVertex,
                      "Z0 impact parameter of kink daughter track at kink vertex");
    REGISTER_VARIABLE("kinkDaughterPtAtKinkVertex", kinkDaughterPtAtKinkVertex,
                      "Pt of kink daughter track at kink vertex");
    REGISTER_VARIABLE("kinkDaughterPzAtKinkVertex", kinkDaughterPzAtKinkVertex,
                      "Pz of kink daughter track at kink vertex");
    REGISTER_VARIABLE("kinkDaughterPAtKinkVertex", kinkDaughterPAtKinkVertex,
                      "P of kink daughter track at kink vertex");

    // Kink Mother Measured Track Parameters
    REGISTER_VARIABLE("kinkMotherInitTrackPValue", kinkMotherInitTrackPValue,
                      "P-value of initial kink mother track fit");
    REGISTER_VARIABLE("kinkMotherInitTrackNDF", kinkMotherInitTrackNDF,
                      "NDF of initial kink mother track fit");
    REGISTER_VARIABLE("kinkMotherInitTrackChi2", kinkMotherInitTrackChi2,
                      "Chi2 of initial kink mother track fit");
    REGISTER_VARIABLE("kinkMotherTrackD0AtKinkVertex", kinkMotherTrackD0AtKinkVertex,
                      "D0 impact parameter of kink mother track at kink vertex");
    REGISTER_VARIABLE("kinkMotherTrackZ0AtKinkVertex", kinkMotherTrackZ0AtKinkVertex,
                      "Z0 impact parameter of kink mother track at kink vertex");
    REGISTER_VARIABLE("kinkMotherPtAtKinkVertex", kinkMotherPtAtKinkVertex,
                      "Pt of kink mother track at kink vertex");
    REGISTER_VARIABLE("kinkMotherPzAtKinkVertex", kinkMotherPzAtKinkVertex,
                      "Pz of kink mother track at kink vertex");
    REGISTER_VARIABLE("kinkMotherPAtKinkVertex", kinkMotherPAtKinkVertex,
                      "P of kink mother track at kink vertex");

    //-----------------------------------------------
    // Check if the particle is a part of Kink
    REGISTER_VARIABLE("particleIsInKink", particleIsInKink,
                      "Particle is used in a Kink object");
    REGISTER_VARIABLE("particleIsMotherInKink", particleIsMotherInKink,
                      "Particle is a mother in a Kink object");
    REGISTER_VARIABLE("particleIsDaughterInKink", particleIsDaughterInKink,
                      "Particle is a daughter in a Kink object");
    REGISTER_VARIABLE("particleIsSplitKink", particleIsSplitKink,
                      "Particle is a split track in a Kink object");

    //-----------------------------------------------
    // MC VARIABLES

    // Kink from track pair MC variables
    REGISTER_VARIABLE("kinkIsMCRelated", kinkPairIsMCRelated,
                      "Mother and daughter tracks have MC relations and two of them are related");
    REGISTER_VARIABLE("kinkIsClone", kinkPairIsClone,
                      "Mother and daughter tracks have the same MCParticle relation");
    REGISTER_VARIABLE("kinkIsReal", kinkPairIsReal,
                      "Mother and daughter tracks are mother/daughter related");
    REGISTER_VARIABLE("kinkIsDecayInFlight", kinkPairIsDecayInFlight,
                      "Kink is a decay-in-flight");
    REGISTER_VARIABLE("kinkIsHadronScattering", kinkPairIsHadronScattering,
                      "Kink is a hadron scattering");

    // Kink from track pair MC kinematics
    REGISTER_VARIABLE("mcPMd", kinkDaughterMomentumInMotherRFMC,
                      "Kink daughter momentum in mother rest frame MC (works only for kink created from a track pair)\n"
                      "Makes sense only for real decays-in-flight; however, it is not checked here");
    REGISTER_VARIABLE("mcCosMd", kinkDaughterCosThetaInMotherRFMC,
                      "ink daughter direction in mother rest frame with respect to mother momentum direction in the lab frame"
                      " MC (works only for kink created from a track pair)\n"
                      "Makes sense only for real decays-in-flight; however, it is not checked here");

    // Kink from track pair daughter MC variables
    REGISTER_VARIABLE("kinkDaughterMCPDG", kinkPairDaughterMCPDG,
                      "PDG code of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCMotherPDG", kinkPairDaughterMCMotherPDG,
                      "PDG code of the kink daughter's mother for kinks created from two separate tracks"
                      " (might be different from the mother track MCParticle)");
    REGISTER_VARIABLE("kinkDaughterMCSecProc", kinkPairDaughterMCSecProc,
                      "Secondary Physics Process code of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCVertexX", kinkPairDaughterMCVertexX,
                      "Production vertex X coordinate of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCVertexY", kinkPairDaughterMCVertexY,
                      "Production vertex Y coordinate of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCVertexZ", kinkPairDaughterMCVertexZ,
                      "Production vertex Z coordinate of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCPX", kinkPairDaughterMCPX,
                      "Generated PX of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCPY", kinkPairDaughterMCPY,
                      "Generated PY of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCPZ", kinkPairDaughterMCPZ,
                      "Generated PZ of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCPT", kinkPairDaughterMCPT,
                      "Generated PT of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCP", kinkPairDaughterMCP,
                      "Generated P of the kink daughter for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkDaughterMCE", kinkPairDaughterMCE,
                      "Generated E of the kink daughter for kinks created from two separate tracks");

    // Kink from track pair mother MC variables
    REGISTER_VARIABLE("kinkMotherMCPXAtDV", kinkPairMotherMCPXAtDecayVertex,
                      "Generated PX of the kink mother at the decay vertex for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkMotherMCPYAtDV", kinkPairMotherMCPYAtDecayVertex,
                      "Generated PY of the kink mother at the decay vertex for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkMotherMCPZAtDV", kinkPairMotherMCPZAtDecayVertex,
                      "Generated PZ of the kink mother at the decay vertex for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkMotherMCPTAtDV", kinkPairMotherMCPTAtDecayVertex,
                      "Generated PT of the kink mother at the decay vertex for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkMotherMCPAtDV", kinkPairMotherMCPAtDecayVertex,
                      "Generated P of the kink mother at the decay vertex for kinks created from two separate tracks");
    REGISTER_VARIABLE("kinkMotherMCEAtDV", kinkPairMotherMCEAtDecayVertex,
                      "Generated E of the kink mother at the decay vertex for kinks created from two separate tracks");

  }
}
