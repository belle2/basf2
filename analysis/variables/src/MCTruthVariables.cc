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
#include <analysis/variables/ParameterVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/TauPairDecay.h>
#include <analysis/utility/MCMatching.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/core/Environment.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/BeamParameters.h>

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

    double isSignalAcceptMissingMassive(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissingResonance);
      status &= (~MCMatching::c_MissMassiveParticle);
      status &= (~MCMatching::c_MissKlong);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isSignalAcceptMissingGamma(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissGamma);
      status &= (~MCMatching::c_MissingResonance);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double isSignalAcceptMissing(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissGamma);
      status &= (~MCMatching::c_MissingResonance);
      status &= (~MCMatching::c_MissMassiveParticle);
      status &= (~MCMatching::c_MissKlong);
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

    double particleMCMatchPT(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Pt();
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
      for (auto daughter : daughters) {
        const MCParticle* mcD = daughter->getRelatedTo<MCParticle>();
        if (mcD == nullptr)
          return -999;

        pDaughters += mcD->get4Vector();
      }

      return (pInitial - pDaughters).M();
    }

    double mcParticleSecondaryPhysicsProcess(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        return mcp->getSecondaryPhysicsProcess();
      } else {
        return -1;
      }
    }

    double mcParticleStatus(const Particle* p)
    {
      const MCParticle* mcp = p->getMCParticle();
      if (mcp) {
        return mcp->getStatus();
      } else {
        return -1;
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
        return -1;
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
        return -1;
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
        return -1;
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
        return -1;
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
        return -1;
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
        return -1;
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

    int genNStepsToDaughter(const Particle* p, const std::vector<double>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for genNStepsToDaughter");

      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp) {
        B2WARNING("No MCParticle is associated to the particle");
        return -1;
      }

      int nChildren = p->getNDaughters();
      if (arguments[0] >= nChildren) {
        return -999;
      }

      const Particle*   daugP   = p->getDaughter(arguments[0]);
      const MCParticle* daugMCP = daugP->getRelated<MCParticle>();
      if (!daugMCP) {
        // This is a strange case.
        // The particle, p, has the related MC particle, but i-th daughter does not have the related MC Particle.
        B2WARNING("No MCParticle is associated to the i-th daughter");
        return -1;
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

      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp) {
        B2WARNING("No MCParticle is associated to the particle");
        return -1;
      }

      return MCMatching::countMissingParticle(p, mcp, PDGcodes);
    }

    double particleClusterMatchWeight(const Particle* particle)
    {
      /* Get the weight of the *cluster* mc match for the mcparticle matched to
       * this particle.
       *
       * Note that for track-based particles this is different from the mc match
       * of the partcle (which it inherits from the mc match of the track)
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

      return -1.0;
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

    double isMC(const Particle*)
    {
      return Environment::Instance().isMC();
    }

    // Beam Kinematics
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

    VARIABLE_GROUP("MC matching and MC truth");
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
                      "return 1 if the partice is misidentified: one or more of the final state particles have the wrong PDG code assignment (including wrong charge), 0 in all other cases.");
    REGISTER_VARIABLE("isWrongCharge", isWrongCharge,
                      "return 1 if the charge of the particle is wrongly assigned, 0 in all other cases");
    REGISTER_VARIABLE("isCloneTrack", isCloneTrack,
                      "Return 1 if the charged final state particle comes from a cloned track, 0 if not a clone. Returns NAN if neutral, composite, or MCParticle not found (like for data or if not MCMatched)");
    REGISTER_VARIABLE("isOrHasCloneTrack", isOrHasCloneTrack,
                      "Return 1 if the particle is a clone track or has a clone track as a daughter, 0 otherwise.");
    REGISTER_VARIABLE("mcPDG", particleMCMatchPDGCode,
                      "The PDG code of matched MCParticle, 0 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcErrors", particleMCErrors,
                      "The bit pattern indicating the quality of MC match (see MCMatching::MCErrorFlags)");
    REGISTER_VARIABLE("mcMatchWeight", particleMCMatchWeight,
                      "The weight of the Particle -> MCParticle relation (only for the first Relation = largest weight).");
    REGISTER_VARIABLE("nMCMatches", particleNumberOfMCMatch,
                      "The number of relations of this Particle to MCParticle.");
    REGISTER_VARIABLE("mcDecayTime", particleMCMatchDecayTime,
                      "The decay time of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcLifeTime", particleMCMatchLifeTime,
                      "The life time of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPX", particleMCMatchPX,
                      "The px of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPY", particleMCMatchPY,
                      "The py of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPZ", particleMCMatchPZ,
                      "The pz of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPT", particleMCMatchPT,
                      "The pt of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcE", particleMCMatchE,
                      "The energy of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcP", particleMCMatchP,
                      "The total momentum of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcPhi", particleMCMatchPhi,
                      "The phi of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcTheta", particleMCMatchTheta,
                      "The theta of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the reconstructed particles, or a particle list filled with generator particles (MCParticle objects).");
    REGISTER_VARIABLE("mcRecoilMass", particleMCRecoilMass,
                      "The mass recoiling against the particles attached as particle's daughters calculated using MC truth values.");


    REGISTER_VARIABLE("mcSecPhysProc", mcParticleSecondaryPhysicsProcess,
                      "Returns the secondary physics process flag.");
    REGISTER_VARIABLE("mcParticleStatus", mcParticleStatus,
                      "Returns status bits of related MCParticle or - 1 if MCParticle relation is not set.");
    REGISTER_VARIABLE("mcPrimary", particleMCPrimaryParticle,
                      "Returns 1 if Particle is related to primary MCParticle, 0 if Particle is related to non - primary MCParticle,"
                      "-1 if Particle is not related to MCParticle.");
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
    REGISTER_VARIABLE("generatorEventWeight", generatorEventWeight,
                      "[Eventbased] Returns the event weight produced by the event generator")

    REGISTER_VARIABLE("genNStepsToDaughter(i)", genNStepsToDaughter,
                      "Returns number of steps to i-th daughter from the particle at generator level."
                      "-1 if the no MCParticle is associated to the particle or i-th daughter."
                      "-999 if i-th daughter does not exist.");
    REGISTER_VARIABLE("genNMissingDaughter(PDG)", genNMissingDaughter,
                      "Returns the number of missing daughters having assigned PDG codes."
                      "-1 if the no MCParticle is associated to the particle.")


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

    VARIABLE_GROUP("MC Matching for ECLClusters");
    REGISTER_VARIABLE("clusterMCMatchWeight", particleClusterMatchWeight,
                      "Returns the weight of the ECLCluster -> MCParticle relation for the MCParticle matched to the particle. "
                      "Returns NaN if: no cluster is related to the particle, the particle is not MC matched, or if there are no mcmatches for the cluster. "
                      "Returns -1 if the cluster *was* matched to particles, but not the match of the particle provided.");
    REGISTER_VARIABLE("clusterBestMCMatchWeight", particleClusterBestMCMatchWeight,
                      "returns the weight of the ECLCluster -> MCParticle relation for the relation with the largest weight.");
    REGISTER_VARIABLE("clusterBestMCPDG", particleClusterBestMCPDGCode,
                      "returns the PDG code of the MCParticle for the ECLCluster -> MCParticle relation with the largest weight.");
    REGISTER_VARIABLE("isMC", isMC,
                      "Returns 1 if run on MC and 0 for data.");
    VARIABLE_GROUP("Nominal beam kinematics")
    REGISTER_VARIABLE("Eher", getHEREnergy, "[Eventbased] Nominal HER energy");
    REGISTER_VARIABLE("Eler", getLEREnergy, "[Eventbased] Nominal LER energy");
    REGISTER_VARIABLE("XAngle", getCrossingAngle, "[Eventbased] Nominal beam crossing angle");

  }
}
