/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/MCTruthVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TauPairDecay.h>
#include <analysis/utility/MCMatching.h>
#include <analysis/utility/ReferenceFrame.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>

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
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int status = MCMatching::getMCErrors(part, mcparticle);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isSignalAcceptWrongFSPs(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MisID);
      status &= (~MCMatching::c_AddedWrongParticle);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isPrimarySignal(const Particle* part)
    {
      if (isSignal(part) > 0.5 and particleMCPrimaryParticle(part) > 0.5)
        return 1.0;
      else
        return 0.0;
    }

    double isMisidentified(const Particle* part)
    {
      const MCParticle* mcp = part->getMCParticle();
      if (!mcp) return std::numeric_limits<double>::quiet_NaN();
      int st = MCMatching::getMCErrors(part, mcp);
      return double((st & MCMatching::c_MisID) != 0);
    }

    double isWrongCharge(const Particle* part)
    {
      const MCParticle* mcp = part->getMCParticle();
      if (!mcp) return std::numeric_limits<double>::quiet_NaN();
      int pch = part->getCharge(),
          mch = mcp->getCharge();
      return double((pch != mch));
    }

    double isCloneTrack(const Particle* particle)
    {
      // neutrals and composites don't make sense
      if (!Const::chargedStableSet.contains(Const::ParticleType(abs(particle->getPDGCode()))))
        return std::numeric_limits<double>::quiet_NaN();
      // get mcparticle weight (mcmatch weight)
      auto mcpww = particle->getRelatedToWithWeight<MCParticle>();
      if (!mcpww.first) return std::numeric_limits<double>::quiet_NaN();
      return double(mcpww.second < 0);
    }

    double isOrHasCloneTrack(const Particle* particle)
    {
      // use std::queue to check daughters-- granddaughters etc recursively
      std::queue<const Particle*> qq;
      qq.push(particle);
      while (!qq.empty()) {
        auto d = qq.front(); // get daughter
        qq.pop();            // remove the daughter from the queue
        if (isCloneTrack(d) == 1.0) return 1.0;
        size_t nDau = d->getNDaughters(); // number of daughters of daughters
        for (size_t iDau = 0; iDau < nDau; iDau++)
          qq.push(d->getDaughter(iDau));
      }
      return 0.0;
    }

    double genNthMotherPDG(const Particle* part, const std::vector<double>& args)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return 0.0;

      unsigned int nLevels;
      if (args.empty())
        nLevels = 0;
      else
        nLevels = args[0];

      const MCParticle* curMCParticle = mcparticle;
      for (unsigned int i = 0; i <= nLevels; i++) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (curMCMother == nullptr)
          return 0.0;
        curMCParticle = curMCMother;
      }
      int m_pdg = curMCParticle->getPDG();
      return m_pdg;
    }

    double genNthMotherIndex(const Particle* part, const std::vector<double>& args)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return 0.0;

      unsigned int nLevels;
      if (args.empty())
        nLevels = 0;
      else
        nLevels = args[0];

      const MCParticle* curMCParticle = mcparticle;
      for (unsigned int i = 0; i <= nLevels; i++) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (curMCMother == nullptr)
          return 0.0;
        curMCParticle = curMCMother;
      }
      int m_id = curMCParticle->getArrayIndex();
      return m_id;
    }

    double genMotherPDG(const Particle* part)
    {
      const std::vector<double> args = {};
      return genNthMotherPDG(part, args);
    }

    double genMotherP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const MCParticle* mcmother = mcparticle->getMother();
      if (mcmother == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      double p = mcmother->getMomentum().Mag();
      return p;
    }

    double genMotherIndex(const Particle* part)
    {
      const std::vector<double> args = {};
      return genNthMotherIndex(part, args);
    }

    double genParticleIndex(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (!mcparticle)
        return std::numeric_limits<float>::quiet_NaN();

      double m_ID = mcparticle->getArrayIndex();
      return m_ID;
    }

    double isSignalAcceptMissingNeutrino(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissNeutrino);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isSignalAcceptMissingMassive(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissMassiveParticle);
      status &= (~MCMatching::c_MissKlong);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isSignalAcceptMissingGamma(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissGamma);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isSignalAcceptMissing(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_MissGamma);
      status &= (~MCMatching::c_MissMassiveParticle);
      status &= (~MCMatching::c_MissKlong);
      status &= (~MCMatching::c_MissNeutrino);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isSignalAcceptBremsPhotons(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits
      status &= (~MCMatching::c_AddedRecoBremsPhoton);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double particleMCMatchPDGCode(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      return mcparticle->getPDG();
    }

    double particleMCErrors(const Particle* part)
    {
      return MCMatching::getMCErrors(part);
    }

    double particleNumberOfMCMatch(const Particle* particle)
    {
      RelationVector<MCParticle> mcRelations =
        particle->getRelationsTo<MCParticle>();
      return double(mcRelations.size());
    }

    double particleMCMatchWeight(const Particle* particle)
    {
      auto relWithWeight = particle->getRelatedToWithWeight<MCParticle>();

      if (relWithWeight.first) {
        return relWithWeight.second;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double particleMCMatchDecayTime(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      return mcparticle->getDecayTime();
    }

    double particleMCMatchLifeTime(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      return mcparticle->getLifetime();
    }

    double particleMCMatchPX(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Px();
    }

    double particleMCMatchPY(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Py();
    }

    double particleMCMatchPZ(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Pz();
    }

    double particleMCMatchPT(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Pt();
    }

    double particleMCMatchE(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).E();
    }

    double particleMCMatchP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).P();
    }

    double particleMCMatchTheta(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Theta();
    }

    double particleMCMatchPhi(const Particle* part)
    {
      const MCParticle* mcparticle = part->getMCParticle();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const auto& frame = ReferenceFrame::GetCurrent();
      TLorentzVector mcpP4 = mcparticle->get4Vector();
      return frame.getMomentum(mcpP4).Phi();
    }

    double particleMCRecoilMass(const Particle* part)
    {
      StoreArray<MCParticle> mcparticles;
      if (mcparticles.getEntries() < 1)
        return std::numeric_limits<double>::quiet_NaN();

      TLorentzVector pInitial = mcparticles[0]->get4Vector();
      TLorentzVector pDaughters;
      const std::vector<Particle*> daughters = part->getDaughters();
      for (auto daughter : daughters) {
        const MCParticle* mcD = daughter->getMCParticle();
        if (mcD == nullptr)
          return std::numeric_limits<double>::quiet_NaN();

        pDaughters += mcD->get4Vector();
      }

      return (pInitial - pDaughters).M();
    }

    TLorentzVector MCInvisibleP4(const MCParticle* mcparticle)
    {
      TLorentzVector ResultP4;
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
      // if this is a continuum run, use an approximate Y(4S) CMS energy
      if (e_Beam * e_Beam - m_B * m_B < 0) {
        e_Beam = 1.0579400E+1 / 2.0; // GeV
      }
      double p_B = std::sqrt(e_Beam * e_Beam - m_B * m_B);

      // Calculate cosThetaBY with daughter neutrino momenta subtracted
      const MCParticle* mcB = part->getMCParticle();
      if (mcB == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      int mcParticlePDG = abs(mcB->getPDG());
      if (mcParticlePDG != 511 and mcParticlePDG != 521)
        return std::numeric_limits<double>::quiet_NaN();

      TLorentzVector p = T.rotateLabToCms() * (mcB->get4Vector() - MCInvisibleP4(mcB));
      double e_d = p.E();
      double m_d = p.M();
      double p_d = p.Rho();

      double theta_BY = (2 * e_Beam * e_d - m_B * m_B - m_d * m_d)
                        / (2 * p_B * p_d);
      return theta_BY;
    }

    double mcParticleSecondaryPhysicsProcess(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        return mcp->getSecondaryPhysicsProcess();
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double mcParticleStatus(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        return mcp->getStatus();
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double particleMCPrimaryParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_PrimaryParticle;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double particleMCVirtualParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsVirtual;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double particleMCInitialParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_Initial;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double particleMCISRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsISRPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double particleMCFSRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsFSRPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double particleMCPhotosParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsPHOTOSPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double generatorEventWeight(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      if (!evtMetaData)
        return std::numeric_limits<double>::quiet_NaN();
      return evtMetaData->getGeneratedWeight();
    }

    int tauPlusMcMode(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau decay ID, did you forget to run TauDecayMarkerModule?");
        return std::numeric_limits<int>::quiet_NaN();
      }
      int tauPlusId = tauDecay->getTauPlusIdMode();
      return tauPlusId;
    }

    int tauMinusMcMode(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau decay ID, did you forget to run TauDecayMarkerModule?");
        return std::numeric_limits<int>::quiet_NaN();
      }
      int tauMinusId = tauDecay->getTauMinusIdMode();
      return tauMinusId;
    }

    int tauPlusMcProng(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau prong, did you forget to run TauDecayMarkerModule?");
        return std::numeric_limits<int>::quiet_NaN();
      }
      int tauPlusMcProng = tauDecay->getTauPlusMcProng();
      return tauPlusMcProng;
    }

    int tauMinusMcProng(const Particle*)
    {
      StoreObjPtr<TauPairDecay> tauDecay;
      if (!tauDecay) {
        B2WARNING("Cannot find tau prong, did you forget to run TauDecayMarkerModule?");
        return std::numeric_limits<int>::quiet_NaN();
      }
      int tauMinusMcProng = tauDecay->getTauMinusMcProng();
      return tauMinusMcProng;
    }



    double isReconstructible(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();

      // If charged: make sure it was seen in the SVD.
      // If neutral: make sure it was seen in the ECL.
      if (abs(mcp->getCharge()) > 0)
        return seenInSVD(p);
      else
        return seenInECL(p);
    }

    double seenInPXD(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::PXD);
    }

    double seenInSVD(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::SVD);
    }

    double seenInCDC(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::CDC);
    }

    double seenInTOP(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::TOP);
    }

    double seenInECL(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::ECL);
    }

    double seenInARICH(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::ARICH);
    }

    double seenInKLM(const Particle* p)
    {
      if (p->getParticleSource() == Particle::EParticleSourceObject::c_Composite)
        return std::numeric_limits<float>::quiet_NaN();
      const MCParticle* mcp = p->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::KLM);
    }

    int genNStepsToDaughter(const Particle* p, const std::vector<double>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for genNStepsToDaughter");

      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) {
        B2WARNING("No MCParticle is associated to the particle");
        return std::numeric_limits<int>::quiet_NaN();
      }

      int nChildren = p->getNDaughters();
      if (arguments[0] >= nChildren) {
        return std::numeric_limits<int>::quiet_NaN();
      }

      const Particle*   daugP   = p->getDaughter(arguments[0]);
      const MCParticle* daugMCP = daugP->getMCParticle();
      if (!daugMCP) {
        // This is a strange case.
        // The particle, p, has the related MC particle, but i-th daughter does not have the related MC Particle.
        B2WARNING("No MCParticle is associated to the i-th daughter");
        return std::numeric_limits<int>::quiet_NaN();
      }

      if (nChildren == 1) {
        return 1;
      } else {
        int motherIndex = mcp->getIndex();

        std::vector<int> genMothers;
        MCMatching::fillGenMothers(daugMCP, genMothers);
        auto match = std::find(genMothers.begin(), genMothers.end(), motherIndex);

        return match - genMothers.begin();
      }
    }

    int genNMissingDaughter(const Particle* p, const std::vector<double>& arguments)
    {
      if (arguments.size() < 1)
        B2FATAL("Wrong number of arguments for genNMissingDaughter");

      const std::vector<int> PDGcodes(arguments.begin(), arguments.end());

      const MCParticle* mcp = p->getMCParticle();
      if (!mcp) {
        B2WARNING("No MCParticle is associated to the particle");
        return std::numeric_limits<int>::quiet_NaN();
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

    double getCrossingAngle(const Particle*)
    {
      static DBObjPtr<BeamParameters> beamParamsDB;
      return (beamParamsDB->getHER()).Vect().Angle(-1.0 * (beamParamsDB->getLER()).Vect());
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
      if (!matchedToParticle) return std::numeric_limits<float>::quiet_NaN();
      int matchedToIndex = matchedToParticle->getArrayIndex();

      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return std::numeric_limits<float>::quiet_NaN();

      auto mcps = cluster->getRelationsTo<MCParticle>();
      if (mcps.size() == 0) return std::numeric_limits<float>::quiet_NaN();

      for (unsigned int i = 0; i < mcps.size(); ++i)
        if (mcps[i]->getArrayIndex() == matchedToIndex)
          return mcps.weight(i);

      return std::numeric_limits<float>::quiet_NaN();
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
      if (!cluster) return std::numeric_limits<float>::quiet_NaN();

      /* loop over all mcparticles related to this cluster, find the largest
       * weight by std::sort-ing the doubles
       */
      auto mcps = cluster->getRelationsTo<MCParticle>();
      if (mcps.size() == 0) return std::numeric_limits<float>::quiet_NaN();

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
      if (!cluster) return std::numeric_limits<float>::quiet_NaN();

      auto mcps = cluster->getRelationsTo<MCParticle>();
      if (mcps.size() == 0) return std::numeric_limits<float>::quiet_NaN();

      std::vector<std::pair<double, int>> weightsAndIndices;
      for (unsigned int i = 0; i < mcps.size(); ++i)
        weightsAndIndices.emplace_back(mcps.weight(i), i);

      // sort descending by weight
      std::sort(
        weightsAndIndices.begin(), weightsAndIndices.end(),
      [](const std::pair<double, int>& l, const std::pair<double, int>& r) {
        return l.first > r.first;
      });
      return mcps.object(weightsAndIndices[0].second)->getPDG();
    }

    double isBBCrossfeed(const Particle* particle)
    {
      if (particle == nullptr)
        return std::numeric_limits<float>::quiet_NaN();

      int pdg = particle->getPDGCode();
      if (abs(pdg) != 511 && abs(pdg) != 521 && abs(pdg) != 531)
        return std::numeric_limits<float>::quiet_NaN();

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
          return std::numeric_limits<float>::quiet_NaN();
        }
      }

      std::set<int> distinctIDs = std::set(mother_ids.begin(), mother_ids.end());
      if (distinctIDs.size() == 1)
        return 0;
      else
        return 1;
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

    REGISTER_VARIABLE("genMotherID", genMotherIndex,
                      "Check the array index of a particles generated mother");
    REGISTER_VARIABLE("genMotherID(i)", genNthMotherIndex,
                      "Check the array index of a particle n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc. :noindex:");
    // genMotherPDG and genMotherID are overloaded (each are two C++ functions
    // sharing one variable name) so one of the two needs to be made the indexed
    // variable in sphinx
    REGISTER_VARIABLE("isBBCrossfeed", isBBCrossfeed,
                      "Returns 1 for crossfeed in reconstruction of given B meson, 0 for no crossfeed and nan for no true B meson or failed truthmatching.");
    REGISTER_VARIABLE("genMotherP", genMotherP,
                      "Generated momentum of a particles MC mother particle");
    REGISTER_VARIABLE("genParticleID", genParticleIndex,
                      "Check the array index of a particle's related MCParticle");
    REGISTER_VARIABLE("isSignalAcceptMissingNeutrino",
                      isSignalAcceptMissingNeutrino,
                      "same as isSignal, but also accept missing neutrino");
    REGISTER_VARIABLE("isSignalAcceptMissingMassive",
                      isSignalAcceptMissingMassive,
                      "same as isSignal, but also accept missing massive particle");
    REGISTER_VARIABLE("isSignalAcceptMissingGamma",
                      isSignalAcceptMissingGamma,
                      "same as isSignal, but also accept missing gamma, such as B -> K* gamma, pi0 -> gamma gamma");
    REGISTER_VARIABLE("isSignalAcceptMissing",
                      isSignalAcceptMissing,
                      "same as isSignal, but also accept missing particle");
    REGISTER_VARIABLE("isMisidentified", isMisidentified,
                      "return 1 if the particle is misidentified: at least one of the final state particles has the wrong PDG code assignment (including wrong charge), 0 if PDG code is fine, and NaN if no related MCParticle could be found.");
    REGISTER_VARIABLE("isWrongCharge", isWrongCharge,
                      "return 1 if the charge of the particle is wrongly assigned, 0 if it's the correct charge, and NaN if no related MCParticle could be found.");
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
                      "The decay time of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcLifeTime", particleMCMatchLifeTime,
                      "The life time of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPX", particleMCMatchPX,
                      "The px of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPY", particleMCMatchPY,
                      "The py of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPZ", particleMCMatchPZ,
                      "The pz of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPT", particleMCMatchPT,
                      "The pt of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcE", particleMCMatchE,
                      "The energy of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcP", particleMCMatchP,
                      "The total momentum of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPhi", particleMCMatchPhi,
                      "The phi of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcTheta", particleMCMatchTheta,
                      "The theta of matched MCParticle, NaN if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcRecoilMass", particleMCRecoilMass,
                      "The mass recoiling against the particles attached as particle's daughters calculated using MC truth values.");
    REGISTER_VARIABLE("mcCosThetaBetweenParticleAndNominalB",
                      particleMCCosThetaBetweenParticleAndNominalB,
                      "Cosine of the angle in CMS between momentum the particle and a nominal B particle. In this calculation, the momenta of all descendant neutrinos are subtracted from the B momentum.");


    REGISTER_VARIABLE("mcSecPhysProc", mcParticleSecondaryPhysicsProcess,
                      "Returns the secondary physics process flag.");
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
                      "NaN if no MCParticle is associated to the particle.")
    REGISTER_VARIABLE("Eher", getHEREnergy, R"DOC(
[Eventbased] The nominal HER energy used by the generator.

.. warning:: This variable does not make sense for data.
)DOC");
    REGISTER_VARIABLE("Eler", getLEREnergy, R"DOC(
[Eventbased] The nominal LER energy used by the generator.

.. warning:: This variable does not make sense for data.
)DOC");
    REGISTER_VARIABLE("XAngle", getCrossingAngle, R"DOC(
[Eventbased] The nominal beam crossing angle from generator level beam kinematics.

.. warning:: This variable does not make sense for data.
)DOC");

    VARIABLE_GROUP("Generated tau decay information");
    REGISTER_VARIABLE("tauPlusMCMode", tauPlusMcMode,
                      "Decay ID for the positive tau lepton in a tau pair generated event.")
    REGISTER_VARIABLE("tauMinusMCMode", tauMinusMcMode,
                      "Decay ID for the negative tau lepton in a tau pair generated event.")
    REGISTER_VARIABLE("tauPlusMCProng", tauPlusMcProng,
                      "Prong for the positive tau lepton in a tau pair generated event.")
    REGISTER_VARIABLE("tauMinusMCProng", tauMinusMcProng,
                      "Prong for the negative tau lepton in a tau pair generated event.")

    VARIABLE_GROUP("MC particle seen in subdetectors");
    REGISTER_VARIABLE("isReconstructible", isReconstructible,
                      "checks charged particles were seen in the SVD and neutrals in the ECL, returns 1.0 if so, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInPXD", seenInPXD,
                      "returns 1.0 if the MC particle was seen in the PXD, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInSVD", seenInSVD,
                      "returns 1.0 if the MC particle was seen in the SVD, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInCDC", seenInCDC,
                      "returns 1.0 if the MC particle was seen in the CDC, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInTOP", seenInTOP,
                      "returns 1.0 if the MC particle was seen in the TOP, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInECL", seenInECL,
                      "returns 1.0 if the MC particle was seen in the ECL, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInARICH", seenInARICH,
                      "returns 1.0 if the MC particle was seen in the ARICH, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInKLM", seenInKLM,
                      "returns 1.0 if the MC particle was seen in the KLM, 0.0 if not, NaN for composite particles or if no related MCParticle could be found. Useful for generator studies, not for reconstructed particles.");

    VARIABLE_GROUP("MC Matching for ECLClusters");
    REGISTER_VARIABLE("clusterMCMatchWeight", particleClusterMatchWeight,
                      "Returns the weight of the ECLCluster -> MCParticle relation for the MCParticle matched to the particle. "
                      "Returns NaN if: no cluster is related to the particle, the particle is not MC matched, or if there are no mcmatches for the cluster. "
                      "Returns -1 if the cluster *was* matched to particles, but not the match of the particle provided.");
    REGISTER_VARIABLE("clusterBestMCMatchWeight", particleClusterBestMCMatchWeight,
                      "returns the weight of the ECLCluster -> MCParticle relation for the relation with the largest weight.");
    REGISTER_VARIABLE("clusterBestMCPDG", particleClusterBestMCPDGCode,
                      "returns the PDG code of the MCParticle for the ECLCluster -> MCParticle relation with the largest weight.");

  }
}
