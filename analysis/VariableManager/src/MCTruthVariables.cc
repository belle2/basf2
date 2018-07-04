/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/MCTruthVariables.h>
#include <analysis/VariableManager/ParameterVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TauPairDecay.h>
#include <analysis/utility/MCMatching.h>

#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <queue>

namespace Belle2 {
  namespace Variable {

    double isSignal(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissingResonance);
      //status &= (~MCMatching::c_DecayInFlight);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isExtendedSignal(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissingResonance);
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
      const MCParticle* mcp = part->getRelatedTo<MCParticle>();
      if (!mcp) return std::numeric_limits<double>::quiet_NaN();
      int st = MCMatching::getMCErrors(part, mcp);
      return double((st & MCMatching::c_MisID) != 0);
    }

    double isWrongCharge(const Particle* part)
    {
      const MCParticle* mcp = part->getRelatedTo<MCParticle>();
      if (!mcp) return std::numeric_limits<double>::quiet_NaN();
      int pch = part->getCharge(),
          mch = mcp->getCharge();
      return double((pch != mch));
    }

    double isCloneTrack(const Particle* particle)
    {
      // neutrals and composites don't make sense
      if (!Const::chargedStableSet.contains(Const::ParticleType(particle->getPDGCode())))
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
        qq.pop();            // remove the daugher from the queue
        if (isCloneTrack(d)) return 1.0;
        size_t nDau = d->getNDaughters(); // number of daughers of daughters
        for (size_t iDau = 0; iDau < nDau; iDau++)
          qq.push(d->getDaughter(iDau));
      }
      return 0.0;
    }

    double genMotherPDG(const Particle* part)
    {
      const std::vector<double> args = {};
      return genNthMotherPDG(part, args);
    }

    double genMotherP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      const MCParticle* mcmother = mcparticle->getMother();
      if (mcmother == nullptr)
        return 0.0;

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
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (!mcparticle)
        return -1.0;

      double m_ID = mcparticle->getArrayIndex();
      return m_ID;
    }

    double isSignalAcceptMissingNeutrino(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissingResonance);
      //status &= (~MCMatching::c_DecayInFlight);
      status &= (~MCMatching::c_MissNeutrino);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double particleMCMatchPDGCode(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

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
        return 0.0;
      }
    }

    double particleMCMatchDecayTime(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayTime();
    }

    double particleMCMatchLifeTime(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getLifetime();
    }

    double particleMCMatchPX(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Px();
    }

    double particleMCMatchPY(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Py();
    }

    double particleMCMatchPZ(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Pz();
    }

    double particleMCMatchDX(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayVertex().Px();
    }

    double particleMCMatchDY(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayVertex().Py();
    }

    double particleMCMatchDZ(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayVertex().Pz();
    }

    double particleMCMatchE(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getEnergy();
    }

    double particleMCMatchP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Mag();
    }

    double particleMCMatchTheta(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Theta();
    }

    double particleMCMatchPhi(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Phi();
    }

    double particleMCRecoilMass(const Particle* part)
    {
      StoreArray<MCParticle> mcparticles;
      if (mcparticles.getEntries() < 1)
        return -999;

      TLorentzVector pInitial = mcparticles[0]->get4Vector();
      TLorentzVector pDaughters;
      const std::vector<Particle*> daughters = part->getDaughters();
      for (unsigned i = 0; i < daughters.size(); i++) {
        const MCParticle* mcD = daughters[i]->getRelatedTo<MCParticle>();
        if (mcD == nullptr)
          return -999;

        pDaughters += mcD->get4Vector();
      }

      return (pInitial - pDaughters).M();
    }

    double particleMCVirtualParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsVirtual;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCInitialParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_Initial;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCISRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsISRPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCFSRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsFSRPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCPhotosParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsPHOTOSPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
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


    double isReconstructible(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
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
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::PXD);
    }

    double seenInSVD(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::SVD);
    }

    double seenInCDC(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::CDC);
    }

    double seenInTOP(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::TOP);
    }

    double seenInECL(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::ECL);
    }

    double seenInARICH(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::ARICH);
    }

    double seenInKLM(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::KLM);
    }

    VARIABLE_GROUP("MC Matching");
    REGISTER_VARIABLE("isSignal", isSignal,
                      "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 otherwise");
    REGISTER_VARIABLE("isExtendedSignal", isExtendedSignal,
                      "1.0 if Particle is almost correctly reconstructed (SIGNAL), 0.0 otherwise.\n"
                      "Misidentification of charged FSP is allowed.");
    REGISTER_VARIABLE("isPrimarySignal", isPrimarySignal,
                      "1.0 if Particle is correctly reconstructed (SIGNAL) and primary, 0.0 otherwise");
    REGISTER_VARIABLE("genMotherPDG", genMotherPDG,
                      "Check the PDG code of a particles MC mother particle");
    REGISTER_VARIABLE("genMotherID", genMotherIndex,
                      "Check the array index of a particles generated mother");
    REGISTER_VARIABLE("genMotherP", genMotherP,
                      "Generated momentum of a particles MC mother particle");
    REGISTER_VARIABLE("genParticleID", genParticleIndex,
                      "Check the array index of a particle's related MCParticle");
    REGISTER_VARIABLE("isSignalAcceptMissingNeutrino",
                      isSignalAcceptMissingNeutrino,
                      "same as isSignal, but also accept missing neutrino");
    REGISTER_VARIABLE("isMisidentified", isMisidentified,
                      "return 1 if the partice is misidentified: one or more of the final state particles have the wrong PDG code assignment (including wrong charge), 0 in all other cases.");
    REGISTER_VARIABLE("isWrongCharge", isWrongCharge,
                      "return 1 if the charge of the particle is wrongly assigned, 0 in all other cases");
    REGISTER_VARIABLE("isCloneTrack", isCloneTrack,
                      "Return 1 if the charged final state particle comes from a cloned track, 0 if not a clone. Returns NAN if neutral, composite, or MCParticle not found (like for data or if not MCMatched)");
    REGISTER_VARIABLE("isOrHasCloneTrack", isOrHasCloneTrack,
                      "Return 1 if the particle is a clone track or has a clone track as a daughter, 0 otherwise.");
    REGISTER_VARIABLE("mcPDG", particleMCMatchPDGCode,
                      "The PDG code of matched MCParticle, 0 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcErrors", particleMCErrors,
                      "The bit pattern indicating the quality of MC match (see MCMatching::MCErrorFlags)");
    REGISTER_VARIABLE("mcMatchWeight", particleMCMatchWeight,
                      "The weight of the Particle -> MCParticle relation (only for the first Relation = largest weight).");
    REGISTER_VARIABLE("nMCMatches", particleNumberOfMCMatch,
                      "The number of relations of this Particle to MCParticle.");
    REGISTER_VARIABLE("mcDecayTime", particleMCMatchDecayTime,
                      "The decay time of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcLifeTime", particleMCMatchLifeTime,
                      "The life time of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcPX", particleMCMatchPX,
                      "The px of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcPY", particleMCMatchPY,
                      "The py of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcPZ", particleMCMatchPZ,
                      "The pz of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcDX", particleMCMatchDX,
                      "The decay x-Vertex of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcDY", particleMCMatchDY,
                      "The decay y-Vertex of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcDZ", particleMCMatchDZ,
                      "The decay z-Vertex of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcE", particleMCMatchE,
                      "The energy of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcP", particleMCMatchP,
                      "The total momentum of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcPhi", particleMCMatchPhi,
                      "The phi of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcTheta", particleMCMatchTheta,
                      "The theta of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcRecoilMass", particleMCRecoilMass,
                      "The mass recoiling against the particles attached as particle's daughters calculated using MC truth values.");
    REGISTER_VARIABLE("mcVirtual", particleMCVirtualParticle,
                      "Returns 1 if Particle is related to virtual MCParticle, 0 if Particle is related to non - virtual MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcInitial", particleMCInitialParticle,
                      "Returns 1 if Particle is related to initial MCParticle, 0 if Particle is related to non - initial MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcISR", particleMCISRParticle,
                      "Returns 1 if Particle is related to ISR MCParticle, 0 if Particle is related to non - ISR MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcFSR", particleMCFSRParticle,
                      "Returns 1 if Particle is related to FSR MCParticle, 0 if Particle is related to non - FSR MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcPhotos", particleMCPhotosParticle,
                      "Returns 1 if Particle is related to Photos MCParticle, 0 if Particle is related to non - Photos MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("tauPlusMCMode", tauPlusMcMode,
                      "Decay ID for the positive tau lepton in a tau pair generated event.")
    REGISTER_VARIABLE("tauMinusMCMode", tauMinusMcMode,
                      "Decay ID for the negative tau lepton in a tau pair generated event.")

    VARIABLE_GROUP("MC particle seen in subdetectors");
    REGISTER_VARIABLE("isReconstructible", isReconstructible,
                      "checks charged particles were seen in the SVD and neutrals in the ECL, returns 1.0 if so, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInPXD", seenInPXD,
                      "returns 1.0 if the MC particle was seen in the PXD, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInSVD", seenInSVD,
                      "returns 1.0 if the MC particle was seen in the SVD, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInCDC", seenInCDC,
                      "returns 1.0 if the MC particle was seen in the CDC, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInTOP", seenInTOP,
                      "returns 1.0 if the MC particle was seen in the TOP, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInECL", seenInECL,
                      "returns 1.0 if the MC particle was seen in the ECL, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInARICH", seenInARICH,
                      "returns 1.0 if the MC particle was seen in the ARICH, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");
    REGISTER_VARIABLE("seenInKLM", seenInKLM,
                      "returns 1.0 if the MC particle was seen in the KLM, 0.0 if not, -1.0 for composite particles. Useful for generator studies, not for reconstructed particles.");

  }
}


