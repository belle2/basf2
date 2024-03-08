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
      if (!Const::chargedStableSet.contains(Const::ParticleType(abs(particle->getPDGCode()))))
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
      for (auto& double_daughter : daughter_indices) {
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
      for (auto daughter : daughters) {
        const MCParticle* mcD = daughter->getMCParticle();
        if (!mcD) return Const::doubleNaN;

        pDaughters += mcD->get4Vector();
      }
      return (pInitial - pDaughters).M();
    }

    ROOT::Math::PxPyPzEVector MCInvisibleP4(const MCParticle* mcparticle)
    {
      ROOT::Math::PxPyPzEVector ResultP4;
      int pdg = abs(mcparticle->getPDG());
      bool isNeutrino = (pdg == 12 or pdg == 14 or pdg == 16);

      if (mcparticle->getNDaughters() > 0) {
        const std::vector<MCParticle*> daughters = mcparticle->getDaughters();
        for (auto daughter : daughters)
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

      int mcParticlePDG = abs(mcB->getPDG());
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

    double isReconstructible(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return Const::doubleNaN;
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) return Const::doubleNaN;

      // If charged: make sure it was seen in the SVD.
      // If neutral: make sure it was seen in the ECL.
      return (abs(mcp->getCharge()) > 0) ? seenInSVD(p) : seenInECL(p);
    }

    double isTrackFound(const Particle* p)
    {
      if (p->getParticleSource() != Particle::EParticleSourceObject::c_MCParticle)
        return Const::doubleNaN;
      const MCParticle* tmp_mcP = p->getMCParticle();
      if (!Const::chargedStableSet.contains(Const::ParticleType(abs(tmp_mcP->getPDG()))))
        return Const::doubleNaN;
      Track* tmp_track = tmp_mcP->getRelated<Track>();
      if (tmp_track) {
        const TrackFitResult* tmp_tfr = tmp_track->getTrackFitResultWithClosestMass(Const::ChargedStable(abs(tmp_mcP->getPDG())));
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
      return (beamParamsDB->getHER()).E();
    }

    double getLEREnergy(const Particle*)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      return (beamParamsDB->getLER()).E();
    }

    double getCrossingAngleX(const Particle*)
    {
      // get the beam momenta from the DB
      static DBObjPtr<BeamParameters> beamParamsDB;
      B2Vector3D herVec = beamParamsDB->getHER().Vect();
      B2Vector3D lerVec = beamParamsDB->getLER().Vect();

      // only looking at the horizontal (XZ plane) -> set y-coordinates to zero
      herVec.SetY(0);
      lerVec.SetY(0);

      //calculate the crossing angle
      return herVec.Angle(-lerVec);
    }

    double getCrossingAngleY(const Particle*)
    {
      // get the beam momenta from the DB
      static DBObjPtr<BeamParameters> beamParamsDB;
      B2Vector3D herVec = beamParamsDB->getHER().Vect();
      B2Vector3D lerVec = beamParamsDB->getLER().Vect();

      // only looking at the vertical (YZ plane) -> set x-coordinates to zero
      herVec.SetX(0);
      lerVec.SetX(0);

      //calculate the crossing angle
      return herVec.Angle(-lerVec);
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
      // cppcheck-suppress containerOutOfBounds
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
      if (abs(pdg) != 511 && abs(pdg) != 521 && abs(pdg) != 531)
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
          if (abs(pdg) == 511 || abs(pdg) == 521 || abs(pdg) == 531) {
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
                      "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 if not, and NaN if no related MCParticle could be found. \n"
                      "It behaves according to DecayStringGrammar.");
    REGISTER_VARIABLE("isSignalAcceptWrongFSPs", isSignalAcceptWrongFSPs,
                      "1.0 if Particle is almost correctly reconstructed (SIGNAL), 0.0 if not, and NaN if no related MCParticle could be found.\n"
                      "Misidentification of charged FSP is allowed.");
    REGISTER_VARIABLE("isPrimarySignal", isPrimarySignal,
                      "1.0 if Particle is correctly reconstructed (SIGNAL) and primary, 0.0 if not, and NaN if no related MCParticle could be found");
    REGISTER_VARIABLE("isSignalAcceptBremsPhotons", isSignalAcceptBremsPhotons,
                      "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 if not, and NaN if no related MCParticle could be found.\n"
                      "Particles with gamma daughters attached through the bremsstrahlung recovery modules are allowed.");
    REGISTER_VARIABLE("genMotherPDG", genMotherPDG,
                      "Check the PDG code of a particles MC mother particle");
    REGISTER_VARIABLE("genMotherPDG(i)", genNthMotherPDG,
                      "Check the PDG code of a particles n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc.  :noindex:");

    REGISTER_VARIABLE("genQ2PmPd(i,j,...)", genQ2PmPd, R"DOC(
                       Returns the generated four momentum transfer squared :math:`q^2` calculated as :math:`q^2 = (p_m - p_{d_i} - p_{d_j} - ...)^2`.

                       Here :math:`p_m` is the four momentum of the given (mother) particle,
                       and :math:`p_{d_{i,j,...}}` are the daughter particles with indices given as arguments .
                       The ordering of daughters is as defined in the DECAY.DEC file used in the generation, with the numbering starting at :math:`N=0`.

                       Returns NaN if no related MCParticle could be found.
                       Returns NaN if any of the given indices is larger than the number of daughters of the given particle.

                       )DOC", ":math:`[\\text{GeV}/\\text{c}]^2`");

    REGISTER_VARIABLE("genMotherID", genMotherIndex,
                      "Check the array index of a particles generated mother");
    REGISTER_VARIABLE("genMotherID(i)", genNthMotherIndex,
                      "Check the array index of a particle n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc. :noindex:");
    // genMotherPDG and genMotherID are overloaded (each are two C++ functions
    // sharing one variable name) so one of the two needs to be made the indexed
    // variable in sphinx
    REGISTER_VARIABLE("isBBCrossfeed", isBBCrossfeed,
                      "Returns 1 for crossfeed in reconstruction of given B meson, 0 for no crossfeed and NaN for no true B meson or failed truthmatching.");
    REGISTER_VARIABLE("ancestorBIndex", ancestorBIndex,
                      "Returns array index of B ancestor, or -1 if no B or no MC-matching is found.");
    REGISTER_VARIABLE("genMotherP", genMotherP,
                      "Generated momentum of a particles MC mother particle\n\n", "GeV/c");
    REGISTER_VARIABLE("genParticleID", genParticleIndex,
                      "Check the array index of a particle's related MCParticle");
    REGISTER_VARIABLE("isSignalAcceptMissingNeutrino",
                      isSignalAcceptMissingNeutrino,
                      "Same as isSignal, but also accept missing neutrino");
    REGISTER_VARIABLE("isSignalAcceptMissingMassive",
                      isSignalAcceptMissingMassive,
                      "Same as isSignal, but also accept missing massive particle");
    REGISTER_VARIABLE("isSignalAcceptMissingGamma",
                      isSignalAcceptMissingGamma,
                      "Same as isSignal, but also accept missing gamma, such as B -> K* gamma, pi0 -> gamma gamma");
    REGISTER_VARIABLE("isSignalAcceptMissing",
                      isSignalAcceptMissing,
                      "Same as isSignal, but also accept missing particle");
    REGISTER_VARIABLE("isMisidentified", isMisidentified,
                      "Return 1 if the particle is misidentified: at least one of the final state particles has the wrong PDG code assignment (including wrong charge), 0 if PDG code is fine, and NaN if no related MCParticle could be found.");
    REGISTER_VARIABLE("isWrongCharge", isWrongCharge,
                      "Return 1 if the charge of the particle is wrongly assigned, 0 if it's the correct charge, and NaN if no related MCParticle could be found.");
    REGISTER_VARIABLE("isCloneTrack", isCloneTrack,
                      "Return 1 if the charged final state particle comes from a cloned track, 0 if not a clone. Returns NAN if neutral, composite, or MCParticle not found (like for data or if not MCMatched)");
    REGISTER_VARIABLE("isOrHasCloneTrack", isOrHasCloneTrack,
                      "Return 1 if the particle is a clone track or has a clone track as a daughter, 0 otherwise.");
    REGISTER_VARIABLE("mcPDG", particleMCMatchPDGCode,
                      "The PDG code of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcErrors", particleMCErrors,
                      "The bit pattern indicating the quality of MC match (see MCMatching::MCErrorFlags)");
    REGISTER_VARIABLE("mcMatchWeight", particleMCMatchWeight,
                      "The weight of the Particle -> MCParticle relation (only for the first Relation = largest weight).");
    REGISTER_VARIABLE("nMCMatches", particleNumberOfMCMatch,
                      "The number of relations of this Particle to MCParticle.");
    REGISTER_VARIABLE("mcDecayTime", particleMCMatchDecayTime,
                      "The decay time of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "ns");
    REGISTER_VARIABLE("mcLifeTime", particleMCMatchLifeTime,
                      "The life time of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "ns");
    REGISTER_VARIABLE("mcPX", particleMCMatchPX,
                      "The px of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "GeV/c");
    REGISTER_VARIABLE("mcPY", particleMCMatchPY,
                      "The py of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "GeV/c");
    REGISTER_VARIABLE("mcPZ", particleMCMatchPZ,
                      "The pz of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "GeV/c");
    REGISTER_VARIABLE("mcPT", particleMCMatchPT,
                      "The pt of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "GeV/c");
    REGISTER_VARIABLE("mcE", particleMCMatchE,
                      "The energy of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "GeV");
    REGISTER_VARIABLE("mcP", particleMCMatchP,
                      "The total momentum of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "GeV/c");
    REGISTER_VARIABLE("mcPhi", particleMCMatchPhi,
                      "The phi of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "rad");
    REGISTER_VARIABLE("mcTheta", particleMCMatchTheta,
                      "The theta of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).\n\n",
                      "rad");
    REGISTER_VARIABLE("nMCDaughters", mcParticleNDaughters,
                      "The number of daughters of the matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcRecoilMass", particleMCRecoilMass,
                      "The mass recoiling against the particles attached as particle's daughters calculated using MC truth values.\n\n",
                      "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("mcCosThetaBetweenParticleAndNominalB",
                      particleMCCosThetaBetweenParticleAndNominalB,
                      "Cosine of the angle in CMS between momentum the particle and a nominal B particle. In this calculation, the momenta of all descendant neutrinos are subtracted from the B momentum.");


    REGISTER_VARIABLE("mcSecPhysProc", mcParticleSecondaryPhysicsProcess,
                      R"DOC(
Returns the secondary physics process flag, which is set by Geant4 on secondary particles. It indicates the type of process that produced the particle.

Returns NaN if the particle is not matched to a MCParticle.

Returns -1 in case of unknown process.

Returns 0 if the particle is primary, i.e. produced by the event generator and not Geant4. Particles produced by Geant4 (i.e. secondary particles) include those produced in interaction with detector material, Bremsstrahlung, and the decay products of long-lived particles (e.g. muons, pions, K_S0, K_L0, Lambdas, ...).

List of possible values (taken from the Geant4 source of
`G4DecayProcessType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/decay/include/G4DecayProcessType.hh>`_,
`G4HadronicProcessType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/hadronic/management/include/G4HadronicProcessType.hh>`_,
`G4TransportationProcessType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/transportation/include/G4TransportationProcessType.hh>`_ and
`G4EmProcessSubType <https://github.com/Geant4/geant4/blob/v10.6.3/source/processes/electromagnetic/utils/include/G4EmProcessSubType.hh>`_)

* 1 Coulomb scattering
* 2 Ionisation
* 3 Bremsstrahlung
* 4 Pair production by charged
* 5 Annihilation
* 6 Annihilation to mu mu
* 7 Annihilation to hadrons
* 8 Nuclear stopping
* 9 Electron general process
* 10 Multiple scattering
* 11 Rayleigh
* 12 Photo-electric effect
* 13 Compton scattering
* 14 Gamma conversion
* 15 Gamma conversion to mu mu
* 16 Gamma general process
* 21 Cerenkov
* 22 Scintillation
* 23 Synchrotron radiation
* 24 Transition radiation
* 91 Transportation
* 92 Coupled transportation
* 111 Hadron elastic
* 121 Hadron inelastic
* 131 Capture
* 132 Mu atomic capture
* 141 Fission
* 151 Hadron at rest
* 152 Lepton at rest
* 161 Charge exchange
* 201 Decay
* 202 Decay with spin
* 203 Decay (pion make spin)
* 210 Radioactive decay
* 211 Unknown decay
* 221 Mu atom decay
* 231 External decay

.. note:: This is what `modularAnalysis.printMCParticles` shows as ``creation process`` when ``showStatus`` is set to ``True``.
)DOC");
    REGISTER_VARIABLE("mcParticleStatus", mcParticleStatus,
                      "Returns status bits of related MCParticle or NaN if MCParticle relation is not set.");
    REGISTER_VARIABLE("mcPrimary", particleMCPrimaryParticle,
                      "Returns 1 if Particle is related to primary MCParticle, 0 if Particle is related to non - primary MCParticle, "
                      "NaN if Particle is not related to MCParticle.");
    REGISTER_VARIABLE("mcVirtual", particleMCVirtualParticle,
                      "Returns 1 if Particle is related to virtual MCParticle, 0 if Particle is related to non - virtual MCParticle, "
                      "NaN if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcInitial", particleMCInitialParticle,
                      "Returns 1 if Particle is related to initial MCParticle, 0 if Particle is related to non - initial MCParticle, "
                      "NaN if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcISR", particleMCISRParticle,
                      "Returns 1 if Particle is related to ISR MCParticle, 0 if Particle is related to non - ISR MCParticle, "
                      "NaN if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcFSR", particleMCFSRParticle,
                      "Returns 1 if Particle is related to FSR MCParticle, 0 if Particle is related to non - FSR MCParticle ,"
                      "NaN if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcPhotos", particleMCPhotosParticle,
                      "Returns 1 if Particle is related to Photos MCParticle, 0 if Particle is related to non - Photos MCParticle, "
                      "NaN if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("generatorEventWeight", generatorEventWeight,
                      "[Eventbased] Returns the event weight produced by the event generator")

    REGISTER_VARIABLE("genNStepsToDaughter(i)", genNStepsToDaughter,
                      "Returns number of steps to i-th daughter from the particle at generator level. "
                      "NaN if no MCParticle is associated to the particle or i-th daughter. "
                      "NaN if i-th daughter does not exist.");
    REGISTER_VARIABLE("genNMissingDaughter(PDG)", genNMissingDaughter,
                      "Returns the number of missing daughters having assigned PDG codes. "
                      "NaN if no MCParticle is associated to the particle.");
    REGISTER_VARIABLE("Eher", getHEREnergy, R"DOC(
[Eventbased] The nominal HER energy used by the generator.

.. warning:: This variable does not make sense for data.

)DOC","GeV");
    REGISTER_VARIABLE("Eler", getLEREnergy, R"DOC(
[Eventbased] The nominal LER energy used by the generator.

.. warning:: This variable does not make sense for data.

)DOC","GeV");
    REGISTER_VARIABLE("XAngle", getCrossingAngleX, R"DOC(
[Eventbased] The nominal beam crossing angle in the x-z plane from generator level beam kinematics.

.. warning:: This variable does not make sense for data.

)DOC","rad");
    REGISTER_VARIABLE("YAngle", getCrossingAngleY, R"DOC(
[Eventbased] The nominal beam crossing angle in the y-z plane from generator level beam kinematics.

.. warning:: This variable does not make sense for data.

)DOC","rad");

    VARIABLE_GROUP("Generated tau decay information");
    REGISTER_VARIABLE("tauPlusMCMode", tauPlusMcMode,
                      "[Eventbased] Decay ID for the positive tau lepton in a tau pair generated event.");
    REGISTER_VARIABLE("tauMinusMCMode", tauMinusMcMode,
                      "[Eventbased] Decay ID for the negative tau lepton in a tau pair generated event.");
    REGISTER_VARIABLE("tauPlusMCProng", tauPlusMcProng,
                      "[Eventbased] Prong for the positive tau lepton in a tau pair generated event.");
    REGISTER_VARIABLE("tauMinusMCProng", tauMinusMcProng,
                      "[Eventbased] Prong for the negative tau lepton in a tau pair generated event.");

    VARIABLE_GROUP("MC particle seen in subdetectors");
    REGISTER_VARIABLE("isReconstructible", isReconstructible,
                      "Checks charged particles were seen in the SVD and neutrals in the ECL, returns 1.0 if so, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInPXD", seenInPXD,
                      "Returns 1.0 if the MC particle was seen in the PXD, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("isTrackFound", isTrackFound,
                      "works on charged stable particle list created from MCParticles, returns NaN if not ; returns 1.0 if there is a reconstructed track related to the charged stable MCParticle with the correct charge, return -1.0 if the reconstructed track has the wrong charge, return 0.0 when no reconstructed track is found.");
    REGISTER_VARIABLE("seenInSVD", seenInSVD,
                      "Returns 1.0 if the MC particle was seen in the SVD, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInCDC", seenInCDC,
                      "Returns 1.0 if the MC particle was seen in the CDC, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInTOP", seenInTOP,
                      "Returns 1.0 if the MC particle was seen in the TOP, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInECL", seenInECL,
                      "Returns 1.0 if the MC particle was seen in the ECL, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInARICH", seenInARICH,
                      "Returns 1.0 if the MC particle was seen in the ARICH, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInKLM", seenInKLM,
                      "Returns 1.0 if the MC particle was seen in the KLM, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");

    VARIABLE_GROUP("MC Matching for ECLClusters");
    REGISTER_VARIABLE("clusterMCMatchWeight", particleClusterMatchWeight,
                      "Returns the weight of the ECLCluster -> MCParticle relation for the MCParticle matched to the particle. "
                      "Returns NaN if: no cluster is related to the particle, the particle is not MC matched, or if there are no mcmatches for the cluster. "
                      "Returns -1 if the cluster *was* matched to particles, but not the match of the particle provided.");
    REGISTER_VARIABLE("clusterBestMCMatchWeight", particleClusterBestMCMatchWeight,
                      "Returns the weight of the ECLCluster -> MCParticle relation for the relation with the largest weight.");
    REGISTER_VARIABLE("clusterBestMCPDG", particleClusterBestMCPDGCode,
                      "Returns the PDG code of the MCParticle for the ECLCluster -> MCParticle relation with the largest weight.");
    REGISTER_VARIABLE("clusterTotalMCMatchWeight", particleClusterTotalMCMatchWeight,
                      "Returns the sum of all weights of the ECLCluster -> MCParticles relations.");

    REGISTER_VARIABLE("clusterTotalMCMatchWeightForKlong", particleClusterTotalMCMatchWeightForKlong,
                      "Returns the sum of all weights of the ECLCluster -> MCParticles relations when MCParticle is a Klong or daughter of a Klong");
    REGISTER_VARIABLE("clusterTotalMCMatchWeightForBestKlong", particleClusterTotalMCMatchWeightForBestKlong,
                      "Returns the sum of all weights of the ECLCluster -> MCParticles relations when MCParticle is the same Klong or daughter of the Klong. If multiple MC Klongs are related to the ECLCluster, returns the sum of weights for the best matched Klong.");


  }
}
