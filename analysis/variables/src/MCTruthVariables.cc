/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/MCTruthVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TauPairDecay.h>
#include <analysis/utility/MCMatching.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/utility/ValueIndexPairSorting.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>


#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/BeamParameters.h>

#include <Math/VectorUtil.h>

#include <cmath>
#include <queue>

namespace Belle2 {
  namespace Variable {

    double isSignal(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      int status = MCMatching::getMCErrors(part, mcparticle);
      return (status == MCMatching::c_Correct);
    }

    double isSignalAcceptWrongFSPs(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MisID);
      status &= (~MCMatching::c_AddedWrongParticle);

      return (status == MCMatching::c_Correct);
    }

    double isPrimarySignal(const Particle* part)
    {
      return (isSignal(part) > 0.5 and particleMCPrimaryParticle(part) > 0.5);
    }

    double isMisidentified(const Particle* part)
    {
      const MCParticle* mcp = part->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      int st = MCMatching::getMCErrors(part, mcp);
      return ((st & MCMatching::c_MisID) != 0);
    }

    double isWrongCharge(const Particle* part)
    {
      const MCParticle* mcp = part->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return (part->getCharge() != mcp->getCharge());
    }

    double isCloneTrack(const Particle* particle)
    {
      // neutrals and composites don't make sense
      if (!Const::chargedStableSet.contains(Const::ParticleType(std::abs(particle->getPDGCode()))))
        return Const::doubleNaN;
      // get mcparticle weight (mcmatch weight)
      const auto mcpww = particle->getRelatedToWithWeight<MCParticle>();
      if (!mcpww.first) return Const::doubleNaN;
      return (mcpww.second < 0);
    }

    double isOrHasCloneTrack(const Particle* particle)
    {
      // use std::queue to check daughters-- granddaughters etc recursively
      std::queue<const Particle*> qq;
      qq.push(particle);
      while (!qq.empty()) {
        const auto d = qq.front(); // get daughter
        qq.pop();            // remove the daughter from the queue
        if (isCloneTrack(d) == 1.0) return 1.0;
        size_t nDau = d->getNDaughters(); // number of daughters of daughters
        for (size_t iDau = 0; iDau < nDau; ++iDau)
          qq.push(d->getDaughter(iDau));
      }
      return 0.0;
    }

    double genNthMotherPDG(const Particle* part, const std::vector<double>& args)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return 0.0;

      unsigned int nLevels = args.empty() ? 0 : args[0];

      const MCParticle* curMCParticle = mcparticle;
      for (unsigned int i = 0; i <= nLevels; ++i) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (!curMCMother) return 0.0;
        curMCParticle = curMCMother;
      }
      return curMCParticle->getPDG();
    }

    double genNthMotherIndex(const Particle* part, const std::vector<double>& args)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return 0.0;

      unsigned int nLevels = args.empty() ? 0 : args[0];

      const MCParticle* curMCParticle = mcparticle;
      for (unsigned int i = 0; i <= nLevels; ++i) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (!curMCMother) return 0.0;
        curMCParticle = curMCMother;
      }
      return curMCParticle->getArrayIndex();
    }

    double genQ2PmPd(const Particle* part, const std::vector<double>& daughter_indices)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      auto daughters = mcparticle->getDaughters();

      ROOT::Math::PxPyPzEVector  p4Daughters;
      for (const auto& double_daughter : daughter_indices) {
        unsigned long daughter = std::lround(double_daughter);
        if (daughter >= daughters.size()) return Const::doubleNaN;

        p4Daughters += daughters[daughter]->get4Vector();
      }
      auto p4Mother = mcparticle->get4Vector();
      return (p4Mother - p4Daughters).mag2();
    }

    double genMotherPDG(const Particle* part)
    {
      return genNthMotherPDG(part, {});
    }

    double genMotherP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const MCParticle* mcmother = mcparticle->getMother();
      if (!mcmother) return Const::doubleNaN;

      return mcmother->getMomentum().R();
    }

    double genMotherIndex(const Particle* part)
    {
      return genNthMotherIndex(part, {});
    }

    double genParticleIndex(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;
      return mcparticle->getArrayIndex();
    }

    double isSignalAcceptMissingNeutrino(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissNeutrino);

      return (status == MCMatching::c_Correct);
    }

    double isSignalAcceptMissingMassive(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissMassiveParticle);
      status &= (~MCMatching::c_MissKlong);

      return (status == MCMatching::c_Correct);
    }

    double isSignalAcceptMissingGamma(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissGamma);

      return (status == MCMatching::c_Correct);
    }

    double isSignalAcceptMissing(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissGamma);
      status &= (~MCMatching::c_MissMassiveParticle);
      status &= (~MCMatching::c_MissKlong);
      status &= (~MCMatching::c_MissNeutrino);

      return (status == MCMatching::c_Correct);
    }

    double isSignalAcceptBremsPhotons(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_AddedRecoBremsPhoton);

      return (status == MCMatching::c_Correct);
    }

    double particleMCMatchPDGCode(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;
      return mcparticle->getPDG();
    }

    double particleMCErrors(const Particle* part)
    {
      return MCMatching::getMCErrors(part);
    }

    double particleNumberOfMCMatch(const Particle* particle)
    {
      RelationVector<MCParticle> mcRelations = particle->getRelationsTo<MCParticle>();
      return (mcRelations.size());
    }

    double particleMCMatchWeight(const Particle* particle)
    {
      auto relWithWeight = particle->getRelatedToWithWeight<MCParticle>();
      if (!relWithWeight.first) return Const::doubleNaN;
      return relWithWeight.second;
    }

    double particleMCMatchDecayTime(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;
      return mcparticle->getDecayTime();
    }

    double particleMCMatchLifeTime(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;
      return mcparticle->getLifetime();
    }

    double particleMCMatchPX(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Px();
    }

    double particleMCMatchPY(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Py();
    }

    double particleMCMatchPZ(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Pz();
    }

    double particleMCMatchPT(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Pt();
    }

    double particleMCMatchE(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).E();
    }

    double particleMCMatchP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).P();
    }

    double particleMCMatchTheta(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Theta();
    }

    double particleMCMatchPhi(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle) return Const::doubleNaN;

      const auto& frame = ReferenceFrame::GetCurrent();
      ROOT::Math::PxPyPzEVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Phi();
    }

    double mcParticleNDaughters(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();

      if (!mcparticle) return Const::doubleNaN;
      return mcparticle->getNDaughters();
    }

    double particleMCRecoilMass(const Particle* part)
    {
      StoreArray<MCParticle> mcparticles;
      if (mcparticles.getEntries() < 1) return Const::doubleNaN;

      ROOT::Math::PxPyPzEVector pInitial = mcparticles[0]->get4Vector();
      ROOT::Math::PxPyPzEVector pDaughters;
      const std::vector<Particle*> daughters = part->getDaughters();
      for (const auto* daughter : daughters) {
        const MCParticle* mcD = daughter->getMCParticle();
        if (!mcD) return Const::doubleNaN;

        pDaughters += mcD->get4Vector();
      }
      return (pInitial - pDaughters).M();
    }

    ROOT::Math::PxPyPzEVector MCInvisibleP4(const MCParticle* mcparticle)
    {
      ROOT::Math::PxPyPzEVector ResultP4;
      int pdg = std::abs(mcparticle->getPDG());
      bool isNeutrino = (pdg == 12 or pdg == 14 or pdg == 16);

      if (mcparticle->getNDaughters() > 0) {
        const std::vector<MCParticle*> daughters = mcparticle->getDaughters();
        for (const auto* daughter : daughters)
          ResultP4 += MCInvisibleP4(daughter);
      } else if (isNeutrino)
        ResultP4 += mcparticle->get4Vector();

      return ResultP4;
    }

    double particleMCCosThetaBetweenParticleAndNominalB(const Particle* part)
    {
      int particlePDG = abs(part->getPDGCode());
      if (particlePDG != 511 and particlePDG != 521)
        B2FATAL("The variable mcCosThetaBetweenParticleAndNominalB is only meant to be used on B mesons!");

      PCmsLabTransform T;
      double e_Beam = T.getCMSEnergy() / 2.0; // GeV
      double m_B = part->getPDGMass();

      // Y(4S) mass according PDG (https://pdg.lbl.gov/2020/listings/rpp2020-list-upsilon-4S.pdf)
      const double  mY4S = 10.5794; // GeV

      // if this is a continuum run, use an approximate Y(4S) CMS energy
      if (e_Beam * e_Beam - m_B * m_B < 0) {
        e_Beam = mY4S / 2.0;
      }
      double p_B = std::sqrt(e_Beam * e_Beam - m_B * m_B);

      // Calculate cosThetaBY with daughter neutrino momenta subtracted
      const MCParticle* mcB = part->getMCParticle();
      if (!mcB) return Const::doubleNaN;

      int mcParticlePDG = std::abs(mcB->getPDG());
      if (mcParticlePDG != 511 and mcParticlePDG != 521)
        return Const::doubleNaN;

      ROOT::Math::PxPyPzEVector p = T.rotateLabToCms() * (mcB->get4Vector() - MCInvisibleP4(mcB));
      double e_d = p.E();
      double m_d = p.M();
      double p_d = p.P();

      double theta_BY = (2 * e_Beam * e_d - m_B * m_B - m_d * m_d)
                        / (2 * p_B * p_d);
      return theta_BY;
    }

    double mcParticleSecondaryPhysicsProcess(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->getSecondaryPhysicsProcess();
    }

    double mcParticleStatus(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->getStatus();
    }

    double particleMCPrimaryParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      unsigned int bitmask = MCParticle::c_PrimaryParticle;
      return mcp->hasStatus(bitmask);
    }

    double particleMCVirtualParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      unsigned int bitmask = MCParticle::c_IsVirtual;
      return mcp->hasStatus(bitmask);
    }

    double particleMCInitialParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      unsigned int bitmask = MCParticle::c_Initial;
      return mcp->hasStatus(bitmask);
    }

    double particleMCISRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      unsigned int bitmask = MCParticle::c_IsISRPhoton;
      return mcp->hasStatus(bitmask);
    }

    double particleMCFSRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      unsigned int bitmask = MCParticle::c_IsFSRPhoton;
      return mcp->hasStatus(bitmask);
    }

    double particleMCPhotosParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      unsigned int bitmask = MCParticle::c_IsPHOTOSPhoton;
      return mcp->hasStatus(bitmask);
    }

    double generatorEventWeight(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      if (!evtMetaData) return Const::doubleNaN;
      return evtMetaData->getGeneratedWeight();
    }

    int tauPlusMcMode(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau decay ID, did you forget to run TauDecayMarkerModule?");
        return 0;
      }
      return tauDecay->getTauPlusIdMode();
    }

    int tauMinusMcMode(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau decay ID, did you forget to run TauDecayMarkerModule?");
        return 0;
      }
      return tauDecay->getTauMinusIdMode();
    }

    int tauPlusMcProng(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau prong, did you forget to run TauDecayMarkerModule?");
        return 0;
      }
      return tauDecay->getTauPlusMcProng();
    }

    int tauMinusMcProng(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau prong, did you forget to run TauDecayMarkerModule?");
        return 0;
      }
      return tauDecay->getTauMinusMcProng();
    }

    double tauPlusEgstar(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau prong, did you forget to run TauDecayMarkerModule?");
        return 0;
      }
      return tauDecay->getTauPlusEgstar();
    }

    double tauMinusEgstar(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau prong, did you forget to run TauDecayMarkerModule?");
        return 0;
      }
      return tauDecay->getTauMinusEgstar();
    }

    double isReconstructible(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      // If charged: make sure it was seen in the SVD.
      // If neutral: make sure it was seen in the ECL.
      return (std::abs(mcp->getCharge()) > 0) ? seenInSVD(p) : seenInECL(p);
    }

    double isTrackFound(const Particle* p)
    {
      if (p->getParticleSource() != Particle::EParticleSourceObject::c_MCParticle)
        return Const::doubleNaN;
      const MCParticle* tmp_mcP = p->getMCParticle();
      if (!Const::chargedStableSet.contains(Const::ParticleType(std::abs(tmp_mcP->getPDG()))))
        return Const::doubleNaN;
      const Track* tmp_track = tmp_mcP->getRelated<Track>();
      if (tmp_track) {
        const TrackFitResult* tmp_tfr = tmp_track->getTrackFitResultWithClosestMass(Const::ChargedStable(std::abs(tmp_mcP->getPDG())));
        if (!tmp_tfr) {
          // p value of TrackFitResult is NaN so cannot check charge
          return 0;
        }
        if (tmp_tfr->getChargeSign()*tmp_mcP->getCharge() > 0)
          return 1;
        else
          return -1;
      }
      return 0;
    }

    double seenInPXD(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->hasSeenInDetector(Const::PXD);
    }

    double seenInSVD(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->hasSeenInDetector(Const::SVD);
    }

    double seenInCDC(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->hasSeenInDetector(Const::CDC);
    }

    double seenInTOP(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->hasSeenInDetector(Const::TOP);
    }

    double seenInECL(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->hasSeenInDetector(Const::ECL);
    }

    double seenInARICH(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->hasSeenInDetector(Const::ARICH);
    }

    double seenInKLM(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;
      return mcp->hasSeenInDetector(Const::KLM);
    }

    int genNStepsToDaughter(const Particle* p, const std::vector<double>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for genNStepsToDaughter");

      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) {
        B2WARNING("No MCParticle is associated to the particle");
        return 0;
      }

      int nChildren = p->getNDaughters();
      if (arguments[0] >= nChildren) {
        return 0;
      }

      const Particle*   daugP   = p->getDaughter(arguments[0]);
      const MCParticle* daugMCP = daugP->getMCParticle();
      if (!daugMCP) {
        // This is a strange case.
        // The particle, p, has the related MC particle, but i-th daughter does not have the related MC Particle.
        B2WARNING("No MCParticle is associated to the i-th daughter");
        return 0;
      }

      if (nChildren == 1) return 1;

      std::vector<int> genMothers;
      MCMatching::fillGenMothers(daugMCP, genMothers);
      auto match = std::find(genMothers.begin(), genMothers.end(), mcp->getIndex());
      return match - genMothers.begin();
    }

    int genNMissingDaughter(const Particle* p, const std::vector<double>& arguments)
    {
      if (arguments.size() < 1)
        B2FATAL("Wrong number of arguments for genNMissingDaughter");

      const std::vector<int> PDGcodes(arguments.begin(), arguments.end());

      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) {
        B2WARNING("No MCParticle is associated to the particle");
        return 0;
      }

      return MCMatching::countMissingParticle(p, mcp, PDGcodes);
    }

    double getHEREnergy(const Particle*)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      if (!beamParamsDB.isValid())
        return Const::doubleNaN;
      return (beamParamsDB->getHER()).E();
    }

    double getLEREnergy(const Particle*)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      if (!beamParamsDB.isValid())
        return Const::doubleNaN;
      return (beamParamsDB->getLER()).E();
    }

    double getCrossingAngleX(const Particle*)
    {
      // get the beam momenta from the DB
      static DBObjPtr<BeamParameters> beamParamsDB;
      if (!beamParamsDB.isValid())
        return Const::doubleNaN;
      ROOT::Math::PxPyPzEVector herVec = beamParamsDB->getHER();
      ROOT::Math::PxPyPzEVector lerVec = beamParamsDB->getLER();
      // only looking at the horizontal (XZ plane) -> set y-coordinates to zero
      herVec.SetPy(0);
      lerVec.SetPy(0);
      // calculate the crossing angle
      return ROOT::Math::VectorUtil::Angle(herVec, -lerVec);
    }

    double getCrossingAngleY(const Particle*)
    {
      // get the beam momenta from the DB
      static DBObjPtr<BeamParameters> beamParamsDB;
      if (!beamParamsDB.isValid())
        return Const::doubleNaN;
      ROOT::Math::PxPyPzEVector herVec = beamParamsDB->getHER();
      ROOT::Math::PxPyPzEVector lerVec = beamParamsDB->getLER();
      // only looking at the vertical (YZ plane) -> set x-coordinates to zero
      herVec.SetPx(0);
      lerVec.SetPx(0);
      // calculate the crossing angle
      return ROOT::Math::VectorUtil::Angle(herVec, -lerVec);
    }


    double particleClusterMatchWeight(const Particle* particle)
    {
      /* Get the weight of the *cluster* mc match for the mcparticle matched to
       * this particle.
       *
       * Note that for track-based particles this is different from the mc match
       * of the particle (which it inherits from the mc match of the track)
       */
      const MCParticle* matchedToParticle = particle->getMCParticle();
      if (!matchedToParticle) return Const::doubleNaN;
      int matchedToIndex = matchedToParticle->getArrayIndex();

      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return Const::doubleNaN;

      const auto mcps = cluster->getRelationsTo<MCParticle>();
      for (unsigned int i = 0; i < mcps.size(); ++i)
        if (mcps[i]->getArrayIndex() == matchedToIndex)
          return mcps.weight(i);

      return Const::doubleNaN;
    }

    double particleClusterBestMCMatchWeight(const Particle* particle)
    {
      /* Get the weight of the best mc match of the cluster associated to
       * this particle.
       *
       * Note for electrons (or any track-based particle) this may not be
       * the same thing as the mc match of the particle (which is taken
       * from the track).
       *
       * For photons (or any ECL-based particle) this will be the same as the
       * mcMatchWeight
       */
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return Const::doubleNaN;

      /* loop over all mcparticles related to this cluster, find the largest
       * weight by std::sort-ing the doubles
       */
      auto mcps = cluster->getRelationsTo<MCParticle>();
      if (mcps.size() == 0) return Const::doubleNaN;

      std::vector<double> weights;
      for (unsigned int i = 0; i < mcps.size(); ++i)
        weights.emplace_back(mcps.weight(i));

      // sort descending by weight
      std::sort(weights.begin(), weights.end());
      std::reverse(weights.begin(), weights.end());
      return weights[0];
    }

    double particleClusterBestMCPDGCode(const Particle* particle)
    {
      /* Get the PDG code of the best mc match of the cluster associated to this
       * particle.
       *
       * Note for electrons (or any track-based particle) this may not be the
       * same thing as the mc match of the particle (which is taken from the track).
       *
       * For photons (or any ECL-based particle) this will be the same as the mcPDG
       */
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return Const::doubleNaN;

      auto mcps = cluster->getRelationsTo<MCParticle>();
      if (mcps.size() == 0) return Const::doubleNaN;

      std::vector<std::pair<double, int>> weightsAndIndices;
      for (unsigned int i = 0; i < mcps.size(); ++i)
        weightsAndIndices.emplace_back(mcps.weight(i), i);

      // sort descending by weight
      std::sort(weightsAndIndices.begin(), weightsAndIndices.end(),
                ValueIndexPairSorting::higherPair<decltype(weightsAndIndices)::value_type>);
      return mcps.object(weightsAndIndices[0].second)->getPDG();
    }

    double particleClusterTotalMCMatchWeight(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return Const::doubleNaN;

      auto mcps = cluster->getRelationsTo<MCParticle>();

      // if there are no relations to any MCParticles, we return 0!
      double weightsum = 0;
      for (unsigned int i = 0; i < mcps.size(); ++i)
        weightsum += mcps.weight(i);

      return weightsum;
    }

    // Helper function for particleClusterTotalMCMatchWeightForKlong
    void getKlongWeightMap(const Particle* particle, std::map<int, double>& mapMCParticleIndxAndWeight)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      auto mcps = cluster->getRelationsTo<MCParticle>();

      for (unsigned int i = 0; i < mcps.size(); ++i) {
        double weight = mcps.weight(i);
        const MCParticle* mcp = mcps[i];

        while (mcp) {
          if (mcp->getPDG() == 130) {
            int index = mcp->getArrayIndex();
            if (mapMCParticleIndxAndWeight.find(index) != mapMCParticleIndxAndWeight.end()) {
              mapMCParticleIndxAndWeight.at(index) = mapMCParticleIndxAndWeight.at(index) + weight;
            } else {
              mapMCParticleIndxAndWeight.insert({index, weight});
            }
            break;
          } else {
            mcp = mcp->getMother();
          }
        }
      }
    }

    double particleClusterTotalMCMatchWeightForKlong(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return Const::doubleNaN;

      auto mcps = cluster->getRelationsTo<MCParticle>();
      if (mcps.size() == 0) return Const::doubleNaN;

      std::map<int, double> mapMCParticleIndxAndWeight;
      getKlongWeightMap(particle, mapMCParticleIndxAndWeight);

      double totalWeight = 0;
      for (const auto& map : mapMCParticleIndxAndWeight) {
        totalWeight += map.second;
      }

      return totalWeight;
    }

    double particleClusterTotalMCMatchWeightForBestKlong(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return Const::doubleNaN;

      auto mcps = cluster->getRelationsTo<MCParticle>();
      if (mcps.size() == 0) return Const::doubleNaN;

      std::map<int, double> mapMCParticleIndxAndWeight;
      getKlongWeightMap(particle, mapMCParticleIndxAndWeight);

      if (mapMCParticleIndxAndWeight.size() == 0)
        return 0.0;

      auto maxMap = std::max_element(mapMCParticleIndxAndWeight.begin(), mapMCParticleIndxAndWeight.end(),
      [](const auto & x, const auto & y) { return x.second < y.second; }
                                    );

      return maxMap->second;
    }

    double isBBCrossfeed(const Particle* particle)
    {
      if (particle == nullptr)
        return Const::doubleNaN;

      int pdg = particle->getPDGCode();
      if (std::abs(pdg) != 511 && std::abs(pdg) != 521 && std::abs(pdg) != 531)
        return Const::doubleNaN;

      std::vector<const Particle*> daughters = particle->getFinalStateDaughters();
      int nDaughters = daughters.size();
      if (nDaughters <= 1)
        return 0;
      std::vector<int> mother_ids;

      for (int j = 0; j < nDaughters; ++j) {
        const MCParticle* curMCParticle = daughters[j]->getMCParticle();
        while (curMCParticle != nullptr) {
          pdg = curMCParticle->getPDG();
          if (std::abs(pdg) == 511 || std::abs(pdg) == 521 || std::abs(pdg) == 531) {
            mother_ids.emplace_back(curMCParticle->getArrayIndex());
            break;
          }
          const MCParticle* curMCMother = curMCParticle->getMother();
          curMCParticle = curMCMother;
        }
        if (curMCParticle == nullptr) {
          return Const::doubleNaN;
        }
      }

      std::set<int> distinctIDs = std::set(mother_ids.begin(), mother_ids.end());
      if (distinctIDs.size() == 1)
        return 0;
      else
        return 1;
    }

    int ancestorBIndex(const Particle* particle)
    {
      const MCParticle* mcpart = particle->getMCParticle();

      while (mcpart) {
        int pdg = std::abs(mcpart->getPDG());

        if ((pdg == 521) || (pdg == 511))
          return mcpart->getArrayIndex();

        mcpart = mcpart->getMother();
      }

      return -1;
    }


    VARIABLE_GROUP("MC matching and MC truth");
    REGISTER_VARIABLE("isSignal", isSignal,
                      "Returns 1.0 if the particle is correctly reconstructed, 0.0 if not, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isSignalAcceptWrongFSPs", isSignalAcceptWrongFSPs,
                      "Returns 1.0 if the particle is almost correctly reconstructed (**mis-identified final state particles are allowed**), 0.0 if not, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isPrimarySignal", isPrimarySignal,
                      "Returns 1.0 if the particle is correctly reconstructed and primary, 0.0 if not, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isSignalAcceptBremsPhotons", isSignalAcceptBremsPhotons,
                      "Returns 1.0 if the particle is correctly reconstructed, 0.0 if not, and ``NaN`` if no related MC particle could be found.\n"
                      "Reconstruction involving any recovered Bremsstrahlung photons attached to the particle are still considered correct.");
    REGISTER_VARIABLE("genMotherPDG", genMotherPDG,
                      "Returns the PDG code of generated mother of the particle.");
    REGISTER_VARIABLE("genMotherPDG(i)", genNthMotherPDG,
                      "Returns the PDG code the :math:`n`-th generated mother of the particle. The argument is the generation: 0 is first mother, 1 is grandmother etc.:noindex:");
    REGISTER_VARIABLE("genQ2PmPd(i,j,...)", genQ2PmPd, R"DOC(
Returns the generated 4-momentum transfer squared :math:`q^2` calculated as

.. math:: q^2 = (p_m - p_{d_i} - p_{d_j} - ...)^2

where :math:`p_m` is the 4-momentum of the given (mother) particle,
and :math:`p_{d_{i,j,...}}` are the daughter particles with indices :math:`i, j, ...` given as arguments .
The ordering of daughters is as defined in the ``DECAY_BELLE2.DEC``
file used in the generation, with the numbering starting at :math:`n=0`. 

Returns ``NaN`` if no related MC particle could be found ot if any of the given indices are larger than the number of daughters of
the given particle. 

.. admonition:: Remember

      The ``DECAY_BELLE2.DEC`` can change between MC campaigns so make sure you look at the correct decay file corresponding to your MC samples.

)DOC", ":math:`[\\text{GeV}/\\text{c}]^2`");
    REGISTER_VARIABLE("genMotherID", genMotherIndex,
                      "Returns the generated particle array index of a particle's generated mother");
    REGISTER_VARIABLE("genMotherID(i)", genNthMotherIndex,
                      "Returns the generated particle array index of the particle's :math:`i`-th generated mother. 0 is first mother, 1 is grandmother etc. :noindex:");
    // genMotherPDG and genMotherID are overloaded (each are two C++ functions
    // sharing one variable name) so one of the two needs to be made the indexed
    // variable in sphinx
    REGISTER_VARIABLE("isBBCrossfeed", isBBCrossfeed, R"DOC(
Returns 1 if there is cross-feed between the reconstructed :math:`B` mesons, 0 for no cross-feed and ``NaN`` for
no :math:`B` meson reconstructed or there is a failed truth-matching.
                      )DOC");
    REGISTER_VARIABLE("ancestorBIndex", ancestorBIndex,
                      "Returns the generated particle array index of the particle's :math:`B` meson ancestor, or -1 if no :math:`B` meson or MC particle is found.");
    REGISTER_VARIABLE("genMotherP", genMotherP, R"DOC(
Returns the equivalent of ``genParticle(genMotherID, p)`` and can be extended to any other (kinematic) variable by replacing the second argument.

.. tip::
      Check out the documentation for ``genParticle(index, variable)`` to better understand this. 

)DOC", "GeV/c");
    REGISTER_VARIABLE("genParticleID", genParticleIndex,
                      "Returns the generated particle array index of the particle's matched MC particle.");
    REGISTER_VARIABLE("isSignalAcceptMissingNeutrino",
                      isSignalAcceptMissingNeutrino,
                      "Returns 1.0 if the particle is almost correctly reconstructed (**missing neutrinos are allowed**), 0.0 if not, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isSignalAcceptMissingMassive",
                      isSignalAcceptMissingMassive,
                      "Returns 1.0 if the particle is almost correctly reconstructed (**missing massive particles are allowed**), 0.0 if not, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isSignalAcceptMissingGamma",
                      isSignalAcceptMissingGamma,
                      "Returns 1.0 if the particle is almost correctly reconstructed (**missing photons are allowed**), 0.0 if not, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isSignalAcceptMissing",
                      isSignalAcceptMissing,
                      "Returns 1.0 if the particle is almost correctly reconstructed (**missing particles are allowed**), 0.0 if not, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isMisidentified", isMisidentified,
                      "Returns 1 if the particle is mis-identified (the wrong PDG code is assigned), 0 if PDG code is correct, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isWrongCharge", isWrongCharge,
                      "Returns 1 if the charge of the particle is wrongly assigned, 0 if it's the correct charge, and ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("isCloneTrack", isCloneTrack,
                      "Returns 1 if the charged final state particle comes from a cloned track, 0 if it does not come from a clone, and ``NaN`` if the particle is neutral, composite, or no MC particle could be found.");
    REGISTER_VARIABLE("isOrHasCloneTrack", isOrHasCloneTrack,
                      "Returns 1 if the particle is a clone track or has a clone track as a daughter, 0 otherwise.");
    REGISTER_VARIABLE("mcPDG", particleMCMatchPDGCode, R"DOC(
Returns the PDG code of matched MC particle or ``NaN`` if no match could be found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.

)DOC");
    REGISTER_VARIABLE("mcErrors", particleMCErrors,
                      "Returns the bit pattern indicating the quality of MC matching. The bit pattern is explained in :ref:`Error_flags`.");
    REGISTER_VARIABLE("mcMatchWeight", particleMCMatchWeight, R"DOC(
Returns the weight of the first (and largest) ``Particle -> MCParticle`` relation.")
)DOC");
    REGISTER_VARIABLE("nMCMatches", particleNumberOfMCMatch, 
                      "Returns the number of ``Particle -> MCParticle`` relations.");
    REGISTER_VARIABLE("mcDecayTime", particleMCMatchDecayTime, R"DOC(
"Returns the decay time of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.

)DOC", "ns");
    REGISTER_VARIABLE("mcLifeTime", particleMCMatchLifeTime,R"DOC(
"Returns the lifetime of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "ns");
    REGISTER_VARIABLE("mcPX", particleMCMatchPX, R"DOC(
"Returns the momentum component :math:`p_x` of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "GeV/c");
    REGISTER_VARIABLE("mcPY", particleMCMatchPY,R"DOC(
"Returns the momentum component :math:`p_y` of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "GeV/c");
    REGISTER_VARIABLE("mcPZ", particleMCMatchPZ,R"DOC(
"Returns the momentum component :math:`p_z` of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "GeV/c");
    REGISTER_VARIABLE("mcPT", particleMCMatchPT,R"DOC(
"Returns the transverse momentum component :math:`p_T` of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "GeV/c");
    REGISTER_VARIABLE("mcE", particleMCMatchE,R"DOC(
"Returns the energy of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "GeV");
    REGISTER_VARIABLE("mcP", particleMCMatchP,R"DOC(
"Returns the total momentum :math:`p` of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
  
)DOC", "GeV/c");
    REGISTER_VARIABLE("mcPhi", particleMCMatchPhi,R"DOC(
"Returns the azimuthal angle :math:`\phi` of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "rad");
    REGISTER_VARIABLE("mcTheta", particleMCMatchTheta,R"DOC(
"Returns the polar angle :math:`\theta` of matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC", "rad");
    REGISTER_VARIABLE("nMCDaughters", mcParticleNDaughters,R"DOC(
"Returns the number of daughters of the matched MC particle, or ``NaN`` if no match is found. 

.. attention::
    This requires running `matchMCTruth()` either on the reconstructed particle, or one of its ancestors, or a particle list filled with MC particle objects.
    
)DOC");    
    REGISTER_VARIABLE("mcRecoilMass", particleMCRecoilMass,
                      "Returns the mass recoiling against the given particle's daughters, calculated using MC truth values.",
                      "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("mcCosThetaBetweenParticleAndNominalB",
                      particleMCCosThetaBetweenParticleAndNominalB,
                      "Returns the cosine of the angle between the CM momentum :math:`p_\mathrm{CM}` of the selected :math:`B` meson and its daughters. It is calculated using MC truth values, with all neutrinos descending from the :math:`B` removed.");
    REGISTER_VARIABLE("mcSecPhysProc", mcParticleSecondaryPhysicsProcess, R"DOC(
Returns the Geant4 process flag for the matched (secondary) MC particle, ``NaN`` if no MC particle is found, 0 if the matched MC particle is primary
or -1 in the case of an unknown process.  

The process flags are:
* 1 - Coulomb scattering
* 2 - Ionisation
* 3 - Bremsstrahlung
* 4 - Pair production by charged
* 5 - Annihilation
* 6 - Annihilation to mu mu
* 7 - Annihilation to hadrons
* 8 - Nuclear stopping
* 9 - Electron general process
* 10 - Multiple scattering
* 11 - Rayleigh
* 12 - Photo-electric effect
* 13 - Compton scattering
* 14 - Gamma conversion
* 15 - Gamma conversion to mu mu
* 16 - Gamma general process
* 21 - Cerenkov
* 22 - Scintillation
* 23 - Synchrotron radiation
* 24 - Transition radiation
* 91 - Transportation
* 92 - Coupled transportation
* 111 - Hadron elastic
* 121 - Hadron inelastic
* 131 - Capture
* 132 - Mu atomic capture
* 141 - Fission
* 151 - Hadron at rest
* 152 - Lepton at rest
* 161 - Charge exchange
* 201 - Decay
* 202 - Decay with spin
* 203 - Decay (pion make spin)
* 210 - Radioactive decay
* 211 - Unknown decay
* 221 - Mu atom decay
* 231 - External decay

.. note::
      The list of Geant4 processes was taken from the following sources:
      - `G4DecayProcessType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/decay/include/G4DecayProcessType.hh>`_
      - `G4HadronicProcessType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/hadronic/management/include/G4HadronicProcessType.hh>`_
      - `G4TransportationProcessType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/transportation/include/G4TransportationProcessType.hh>`_
      - `G4EmProcessSubType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/electromagnetic/utils/include/G4EmProcessSubType.hh>`_


.. attention::
      This code is shown by `modularAnalysis.printMCParticles` under the name of ``creation process`` when ``showStatus`` is set.

)DOC");
    REGISTER_VARIABLE("mcParticleStatus", mcParticleStatus,
                      "Returns the status bit of the matched MC particle, or ``NaN`` if the MC particle relation was not set. The particle status is explained in :ref:`Particle_status`");
    REGISTER_VARIABLE("mcPrimary", particleMCPrimaryParticle,
                      "Returns 1 if the particle is matched to a primary MC particle, 0 if Particle is matched to secondary MC particle, "
                      "or ``NaN`` if no MC particle is found.");
    REGISTER_VARIABLE("mcVirtual", particleMCVirtualParticle,
                      "Returns 1 if the particle is matched to a virtual MC particle, 0 if the particle is matched to a non-virtual MC particle, "
                      "or ``NaN`` if no MC particle is found.");
    REGISTER_VARIABLE("mcInitial", particleMCInitialParticle,
                      "Returns 1 if the particle is matched to an initial MC particle, 0 if the particle is matched to a non-initial MC particle, "
                      "or ``NaN`` if no MC particle is found.");
    REGISTER_VARIABLE("mcISR", particleMCISRParticle,
                      "Returns 1 if the particle is related to an ISR MC particle, 0 if it is related to a non-ISR MC particle, or "
                      "or ``NaN`` if no MC particle is found.");
    REGISTER_VARIABLE("mcFSR", particleMCFSRParticle,
                      "Returns 1 if the particle is related to an FSR MC particle, 0 if it is related to a non-FSR MC particle, or "
                      "or ``NaN`` if no MC particle is found.");
    REGISTER_VARIABLE("mcPhotos", particleMCPhotosParticle,
                      "Returns 1 if the particle is related to Photos particle, 0 if it is related to a non-Photos MC particle, or "
                      "or ``NaN`` if no MC particle is found.");
    REGISTER_VARIABLE("generatorEventWeight", generatorEventWeight,
                      "**[Eventbased]** Returns the event weight produced by the event generator.");
    REGISTER_VARIABLE("genNStepsToDaughter(i)", genNStepsToDaughter,
                      "Returns the number of steps to :math:`i`-th daughter of the particle at generator level, or "
                      "``NaN`` if no MC particle is associated to the particle or :math:`i`-th daughter, or if the "
                      ":math:`i`-th daughter does not exist.");
    REGISTER_VARIABLE("genNMissingDaughter(PDG)", genNMissingDaughter,
                      "Returns the number of missing daughters with the specified PDG code, or ``NaN`` if no related MC particle could be found.");
    REGISTER_VARIABLE("Eher", getHEREnergy, R"DOC(
**[Eventbased]** Returns the nominal HER energy used by the generator.

.. warning:: This variable does not make sense for data and should not be used.

)DOC","GeV");
    REGISTER_VARIABLE("Eler", getLEREnergy, R"DOC(
**[Eventbased]** Returns the nominal LER energy used by the generator.

.. warning:: This variable does not make sense for data and should not be used.

)DOC","GeV");
    REGISTER_VARIABLE("XAngle", getCrossingAngleX, R"DOC(
**[Eventbased]** Returns the nominal beam crossing angle in the :math:`x-z` plane from generator-level beam kinematics.

.. warning:: This variable does not make sense for data and should not be used.

)DOC","rad");
    REGISTER_VARIABLE("YAngle", getCrossingAngleY, R"DOC(
**[Eventbased]** Returns the nominal beam crossing angle in the :math:`y-z` plane from generator-level beam kinematics.

.. warning:: This variable does not make sense for data and should not be used.

)DOC","rad");

    VARIABLE_GROUP("Generated tau decay information");
    REGISTER_VARIABLE("tauPlusMCMode", tauPlusMcMode,
                      "**[Eventbased]** Returns the decay ID for the positive :math:`\tau` lepton in a :math:`\tau\tau` generated event.");
    REGISTER_VARIABLE("tauMinusMCMode", tauMinusMcMode,
                      "**[Eventbased]** Returns the decay ID for the negative :math:`\tau` lepton in a :math:`\tau\tau` generated event.");
    REGISTER_VARIABLE("tauPlusMCProng", tauPlusMcProng,
                      "**[Eventbased]** Returns the prong for the positive :math:`\tau` lepton in a :math:`\tau\tau` generated event.");
    REGISTER_VARIABLE("tauMinusMCProng", tauMinusMcProng,
                      "**[Eventbased]** Returns the prong for the negative :math:`\tau` lepton in a :math:`\tau\tau` generated event.");
    REGISTER_VARIABLE("tauPlusEgstar", tauPlusEgstar,
                      "**[Eventbased]** Returns the energy of radiated photon from the positive :math:`\tau` lepton in a :math:`\tau\tau` generated event.");
    REGISTER_VARIABLE("tauMinusEgstar", tauMinusEgstar,
                      "**[Eventbased]** Returns the energy of radiated photon from the negative :math:`\tau` lepton in a :math:`\tau\tau` generated event.");

    VARIABLE_GROUP("MC particle seen in subdetectors");
    REGISTER_VARIABLE("isReconstructible", isReconstructible, R"DOC(
Returns 1.0 if a charged particle was seen in the SVD or a neutral particle was seen in the ECL, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");
    REGISTER_VARIABLE("seenInPXD", seenInPXD, R"DOC(
Returns 1.0 if the particle was seen in the PXD, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");                
    REGISTER_VARIABLE("isTrackFound", isTrackFound, R"DOC(
Returns 1.0 if there is a reconstructed track related to a charged stable MC particle with the correct charge, -1.0 if the reconstructed track has the wrong charge, or
0.0 if no reconstructed track is found. Returns ``NaN`` if there is no charged stable particle list created from the MC particles. 
)DOC");                
    REGISTER_VARIABLE("seenInSVD", seenInSVD, R"DOC(
Returns 1.0 if the particle was seen in the SVD, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");                
    REGISTER_VARIABLE("seenInCDC", seenInCDC, R"DOC(
Returns 1.0 if the particle was seen in the CDC, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");                
    REGISTER_VARIABLE("seenInTOP", seenInTOP, R"DOC(
Returns 1.0 if the particle was seen in the TOP, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");                
    REGISTER_VARIABLE("seenInECL", seenInECL, R"DOC(
Returns 1.0 if the particle was seen in the ECL, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");                
    REGISTER_VARIABLE("seenInARICH", seenInARICH, R"DOC(
Returns 1.0 if the particle was seen in the ARICH, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");                
    REGISTER_VARIABLE("seenInKLM", seenInKLM, R"DOC(
Returns 1.0 if the particle was seen in the KLM, 0.0 if not, or ``NaN`` for composite particles or if no related MC particle could be found. 

.. tip:: This is useful for generator studies not for particle reconstruction. 

)DOC");                
    REGISTER_VARIABLE("clusterMCMatchWeight", particleClusterMatchWeight, R"DOC(
Returns the weight of the ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation for the matched MC particle. It returns ``NaN``
if no ECL cluster is related to the reconstructed particle or if there are no MC matches for the cluster. 

.. seealso:: The ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation is described in detail in :ref:`ecl-mcmatching`

)DOC"); 
    REGISTER_VARIABLE("clusterBestMCMatchWeight", particleClusterBestMCMatchWeight, R"DOC(
Returns the weight of the ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation for the relation with the largest weight. It returns ``NaN``
if no ECL cluster is related to the reconstructed particle or if there are no MC matches for the cluster.

.. seealso:: The ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation is described in detail in :ref:`ecl-mcmatching`

)DOC"); 
    REGISTER_VARIABLE("clusterBestMCPDG", particleClusterBestMCPDGCode, R"DOC(
Returns the PDG code of the MC particle for the ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation with the largest weight. It returns ``NaN``
if no ECL cluster is related to the reconstructed particle or if there are no MC matches for the cluster.

.. seealso:: The ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation is described in detail in :ref:`ecl-mcmatching`

)DOC"); 
    REGISTER_VARIABLE("clusterTotalMCMatchWeight", particleClusterTotalMCMatchWeight, R"DOC(
Returns the sum of all weights for the ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation. It returns ``NaN``
if no ECL cluster is related to the particle.

.. seealso:: The ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation is described in detail in :ref:`ecl-mcmatching`

)DOC"); 
    REGISTER_VARIABLE("clusterTotalMCMatchWeightForKlong", particleClusterTotalMCMatchWeightForKlong, R"DOC(
Returns the sum of all weights for the ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation when the MC particle is either a :math:`K_L^0` or
a daughter of a :math:`K_L^0`. It returns ``NaN`` if no ECL cluster is related to the reconstructed particle or if there are no MC matches for the cluster, and
it returns 0 if there are no weights between the ECL cluster and :math:`K_L^0` particles. 

.. seealso:: The ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation is described in detail in :ref:`ecl-mcmatching`

)DOC"); 
    REGISTER_VARIABLE("clusterTotalMCMatchWeightForBestKlong", particleClusterTotalMCMatchWeightForBestKlong, R"DOC(
Returns the sum of all weights for the ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation when the MC particle is either a :math:`K_L^0` or
a daughter of a :math:`K_L^0`. If multiple :math:`K_L^0` are related to the ECL cluster, the sum of weights for the best-matched (highest weighted) :math:`K_L^0` are returned.
It returns ``NaN`` if no ECL cluster is related to the reconstructed particle or if there are no MC matches for the cluster, and it returns 0 if there are
no weights between the ECL cluster and :math:`K_L^0` particles. 

.. seealso:: The ``ECLCluster`` :math:`\rightarrow` ``MCParticle`` relation is described in detail in :ref:`ecl-mcmatching`

)DOC"); 
  }
}
