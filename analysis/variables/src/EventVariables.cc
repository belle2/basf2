/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/EventVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventKinematics.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <framework/dataobjects/EventT0.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

// database
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>

#include <analysis/utility/PCmsLabTransform.h>

#include <framework/core/Environment.h>
#include <framework/logging/Logger.h>


namespace Belle2 {
  namespace Variable {

    // Event ------------------------------------------------
    bool isMC(const Particle*)
    {
      return Environment::Instance().isMC();
    }

    bool eventType(const Particle*)
    {
      StoreArray<MCParticle> mcparticles;
      return (mcparticles.getEntries()) > 0 ? 0 : 1;
    }

    bool isContinuumEvent(const Particle*)
    {
      return (isNotContinuumEvent(nullptr) == 1 ? 0 : 1);
    }

    bool isChargedBEvent(const Particle*)
    {
      StoreArray<MCParticle> mcParticles;
      for (const auto& mcp : mcParticles) {
        int pdg_no = mcp.getPDG();
        if (abs(pdg_no) == 521) return 1.0;
      }
      return 0.0;
    }

    double isUnmixedBEvent(const Particle*)
    {
      StoreArray<MCParticle> mcParticles;
      std::vector<int> bPDGs;
      for (const auto& mcp : mcParticles) {
        int pdg_no = mcp.getPDG();
        if (abs(pdg_no) == 511) bPDGs.push_back(pdg_no);
      }
      if (bPDGs.size() == 2) {
        return bPDGs[0] * bPDGs[1] < 0;
      }
      return Const::doubleNaN;
    }

    bool isNotContinuumEvent(const Particle*)
    {
      StoreArray<MCParticle> mcParticles;
      for (const MCParticle& mcp : mcParticles) {
        int pdg_no = mcp.getPDG();
        if (mcp.getMother() == nullptr &&
            ((pdg_no == 553) ||
             (pdg_no == 100553) ||
             (pdg_no == 200553) ||
             (pdg_no == 300553) ||
             (pdg_no == 9000553) ||
             (pdg_no == 9010553)))
          return 1;
      }
      return 0;
    }

    int nMCParticles(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      return mcps.getEntries();
    }

    int nPrimaryMCParticles(const Particle*)
    {
      int n = 0;
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (mcp.isPrimaryParticle())
          n++;
      return n;
    }

    int nInitialPrimaryMCParticles(const Particle*)
    {
      int n = 0;
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (mcp.isInitial() and mcp.isPrimaryParticle())
          n++;
      return n;
    }

    int nVirtualPrimaryMCParticles(const Particle*)
    {
      int n = 0;
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (mcp.isVirtual() and mcp.isPrimaryParticle())
          n++;
      return n;
    }

    int nTracks(const Particle*)
    {
      StoreArray<Track> tracks;
      return tracks.getEntries();
    }

    int nChargeZeroTrackFits(const Particle*)
    {
      StoreArray<TrackFitResult> tfrs;
      int out = 0;
      for (const auto& t : tfrs)
        if (t.getChargeSign() == 0) out++;
      return out;
    }

    double belleECLEnergy(const Particle*)
    {
      StoreArray<ECLCluster> eclClusters;
      double result = 0;
      for (int i = 0; i < eclClusters.getEntries(); ++i) {
        // sum only ECLClusters which have the N1 (n photons) hypothesis
        if (!eclClusters[i]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
          continue;

        result += eclClusters[i]->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
      }
      return result;
    }

    int nKLMClusters(const Particle*)
    {
      StoreArray<KLMCluster> klmClusters;
      return klmClusters.getEntries();
    }

    int expNum(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      int exp_no = evtMetaData->getExperiment();
      return exp_no;
    }

    int productionIdentifier(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      int eventProduction = evtMetaData->getProduction();
      return eventProduction;
    }

    int evtNum(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      int evt_no = evtMetaData->getEvent();
      return evt_no;
    }

    int runNum(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      int run_no = evtMetaData->getRun();
      return run_no;
    }

    // Beam Energies
    double getCMSEnergy(const Particle*)
    {
      PCmsLabTransform T;
      return T.getCMSEnergy();
    }

    double getBeamPx(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamFourMomentum()).Px();
    }

    double getBeamPy(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamFourMomentum()).Py();
    }

    double getBeamPz(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamFourMomentum()).Pz();
    }

    double getBeamE(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamFourMomentum()).E();
    }

    // get total 4-momentum of all final-state particles in MC
    static ROOT::Math::PxPyPzEVector getTotalMcFinalStateMomentum()
    {
      StoreArray<MCParticle> mcps;
      ROOT::Math::PxPyPzEVector sum;
      for (const auto& mcp : mcps) {
        // only consider primary final-state particle generated by generator
        if (mcp.isPrimaryParticle() and not(mcp.isInitial() or mcp.isVirtual())) {
          const MCParticle* mother = mcp.getMother();
          // only consider particles with no mother or particles whose mother is not initial or virtual
          if (not mother or not(mother->isPrimaryParticle() and not(mother->isInitial() or mother->isVirtual())))
            sum += mcp.get4Vector();
        }
      }
      return sum;
    }


    // get 4-momentum of the incoming electron/positron in MC event
    static ROOT::Math::PxPyPzEVector getMcBeamMomentum(int charge)
    {
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps) {
        if (mcp.isInitial() && mcp.getPDG() == -charge * 11) {
          return mcp.get4Vector();
        }
      }

      // if no initial electron/positron found
      return  ROOT::Math::PxPyPzEVector(Const::doubleNaN, Const::doubleNaN, Const::doubleNaN, Const::doubleNaN);
    }

    // get HER/LER 4-momentum based on the calibration payloads
    static ROOT::Math::PxPyPzEVector getBeamMomentum(int charge)
    {
      PCmsLabTransform T;
      double EbeamCM = T.getCMSEnergy() / 2;
      double pBeamCM = sqrt(pow(EbeamCM, 2) - pow(Const::electronMass, 2));

      ROOT::Math::PxPyPzEVector pCM(0, 0, -charge * pBeamCM, EbeamCM);

      return T.cmsToLab(pCM);
    }


    double getMcPxHER(const Particle*) {return getMcBeamMomentum(-1).Px();}
    double getMcPyHER(const Particle*) {return getMcBeamMomentum(-1).Py();}
    double getMcPzHER(const Particle*) {return getMcBeamMomentum(-1).Pz();}

    double getMcPxLER(const Particle*) {return getMcBeamMomentum(+1).Px();}
    double getMcPyLER(const Particle*) {return getMcBeamMomentum(+1).Py();}
    double getMcPzLER(const Particle*) {return getMcBeamMomentum(+1).Pz();}


    double getPxHER(const Particle*) {return getBeamMomentum(-1).Px();}
    double getPyHER(const Particle*) {return getBeamMomentum(-1).Py();}
    double getPzHER(const Particle*) {return getBeamMomentum(-1).Pz();}

    double getPxLER(const Particle*) {return getBeamMomentum(+1).Px();}
    double getPyLER(const Particle*) {return getBeamMomentum(+1).Py();}
    double getPzLER(const Particle*) {return getBeamMomentum(+1).Pz();}


    double getCMSEnergyMC(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      if (!mcps)  {
        return Const::doubleNaN;
      } else return getTotalMcFinalStateMomentum().M();
    }

    double getTotalEnergyMC(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      if (!mcps)  {
        return Const::doubleNaN;
      } else return getTotalMcFinalStateMomentum().E();
    }

    double getGenIPX(const Particle*)
    {
      // generated IP corresponds to the generated vertex of the
      // first not-initial and not-virtual MCParticle
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (not mcp.isInitial() and not mcp.isVirtual() and mcp.isPrimaryParticle())
          return mcp.getVertex().X();
      return Const::doubleNaN;
    }

    double getGenIPY(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (not mcp.isInitial() and not mcp.isVirtual() and mcp.isPrimaryParticle())
          return mcp.getVertex().Y();
      return Const::doubleNaN;
    }

    double getGenIPZ(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (not mcp.isInitial() and not mcp.isVirtual() and mcp.isPrimaryParticle())
          return mcp.getVertex().Z();
      return Const::doubleNaN;
    }

    double getIPX(const Particle*)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      return (beamSpotDB->getIPPosition()).X();
    }

    double getIPY(const Particle*)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      return (beamSpotDB->getIPPosition()).Y();
    }

    double getIPZ(const Particle*)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      return (beamSpotDB->getIPPosition()).Z();
    }

    double ipCovMatrixElement(const Particle*, const std::vector<double>& element)
    {
      int elementI = int(std::lround(element[0]));
      int elementJ = int(std::lround(element[1]));

      if (elementI < 0 || elementI > 3) {
        B2WARNING("Requested IP covariance matrix element is out of boundaries [0 - 3]:" << LogVar("i", elementI));
        return Const::doubleNaN;
      }
      if (elementJ < 0 || elementJ > 3) {
        B2WARNING("Requested particle's momentumVertex covariance matrix element is out of boundaries [0 - 3]:" << LogVar("j", elementJ));
        return Const::doubleNaN;
      }

      static DBObjPtr<BeamSpot> beamSpotDB;
      return beamSpotDB->getCovVertex()(elementI, elementJ);
    }

    // Event kinematics -> missing momentum in lab and CMS, missing energy and mass2, visible energy
    double missingMomentumOfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentum().R();
      return missing;
    }

    double missingMomentumOfEvent_Px(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentum().X();
      return missing;
    }

    double missingMomentumOfEvent_Py(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentum().Y();
      return missing;
    }

    double missingMomentumOfEvent_Pz(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentum().Z();
      return missing;
    }

    double missingMomentumOfEvent_theta(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentum().Theta();
      return missing;
    }

    double missingMomentumOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentumCMS().R();
      return missing;
    }

    double genMissingMomentumOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentumCMS().R();
      return missing;
    }

    double missingMomentumOfEventCMS_Px(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentumCMS().X();
      return missing;
    }

    double missingMomentumOfEventCMS_Py(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentumCMS().Y();
      return missing;
    }

    double missingMomentumOfEventCMS_Pz(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMomentumCMS().Z();
      return missing;
    }

    double missingMomentumOfEventCMS_theta(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double theta = evtShape->getMissingMomentumCMS().Theta();
      return theta;
    }

    double missingEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingEnergyCMS();
      return missing;
    }

    double genMissingEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingEnergyCMS();
      return missing;
    }


    double missingMass2OfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMass2();
      return missing;
    }

    double genMissingMass2OfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return Const::doubleNaN;
      }
      double missing = evtShape->getMissingMass2();
      return missing;
    }

    double visibleEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double visible = evtShape->getVisibleEnergyCMS();
      return visible;
    }

    double genVisibleEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return Const::doubleNaN;
      }
      double visible = evtShape->getVisibleEnergyCMS();
      return visible;
    }


    double totalPhotonsEnergyOfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return Const::doubleNaN;
      }
      double energyOfPhotons = evtShape->getTotalPhotonsEnergy();
      return energyOfPhotons;
    }

    double genTotalPhotonsEnergyOfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return Const::doubleNaN;
      }
      double energyOfPhotons = evtShape->getTotalPhotonsEnergy();
      return energyOfPhotons;
    }

    double eventYearMonthDay(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      if (!evtMetaData) {
        return Const::doubleNaN;
      }
      std::time_t rawtime = trunc(evtMetaData->getTime() / 1e9);
      auto tt = std::gmtime(&rawtime);  // GMT
      int y = tt->tm_year + 1900; // years since 1900
      int m = tt->tm_mon + 1;     // months since January
      int d = tt->tm_mday;        // day of the month
      return (y * 1e4) + (m * 1e2) + d;
    }

    double eventYear(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      if (!evtMetaData) {
        return Const::doubleNaN;
      }
      std::time_t rawtime = trunc(evtMetaData->getTime() / 1e9);
      auto tt = std::gmtime(&rawtime);
      return tt->tm_year + 1900;
    }

    double eventTimeSeconds(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;

      if (!evtMetaData) {
        return Const::doubleNaN;
      }
      double evtTime = trunc(evtMetaData->getTime() / 1e9);

      return evtTime;
    }

    double eventTimeSecondsFractionRemainder(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;

      if (!evtMetaData) {
        return Const::doubleNaN;
      }
      double evtTime = trunc(evtMetaData->getTime() / 1e9);

      double evtTimeFrac = (evtMetaData->getTime() - evtTime * 1e9) / 1e9;

      return evtTimeFrac;
    }

    double eventT0(const Particle*)
    {
      StoreObjPtr<EventT0> evtT0;

      if (!evtT0) {
        B2WARNING("StoreObjPtr<EventT0> does not exist, are you running over cDST data?");
        return Const::doubleNaN;
      }

      if (evtT0->hasEventT0()) {
        return evtT0->getEventT0();
      } else {
        return Const::doubleNaN;
      }
    }

    double timeSincePrevTriggerClockTicks(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->getTimeSincePrevTrigger();
      } else {
        return Const::doubleNaN;
      }
    }

    double timeSincePrevTriggerMicroSeconds(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->getTimeSincePrevTriggerInMicroSeconds();
      } else {
        return Const::doubleNaN;
      }
    }

    double triggeredBunchNumberTTD(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->getBunchNumber();
      } else {
        return Const::doubleNaN;
      }
    }

    double triggeredBunchNumber(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->getTriggeredBunchNumberGlobal();
      } else {
        return Const::doubleNaN;
      }
    }

    double hasRecentInjection(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->hasInjection();
      } else {
        return Const::doubleNaN;
      }
    }

    double timeSinceLastInjectionSignalClockTicks(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->getTimeSinceLastInjection();
      } else {
        return Const::doubleNaN;
      }
    }

    double timeSinceLastInjectionSignalMicroSeconds(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->getTimeSinceLastInjectionInMicroSeconds();
      } else {
        return Const::doubleNaN;
      }
    }

    double timeSinceLastInjectionClockTicks(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->getTimeSinceInjectedBunch();
      } else {
        return Const::doubleNaN;
      }
    }

    double timeSinceLastInjectionMicroSeconds(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->getTimeSinceInjectedBunchInMicroSeconds();
      } else {
        return Const::doubleNaN;
      }
    }

    double injectionInHER(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->isHER();
      } else {
        return Const::doubleNaN;
      }
    }

    double revolutionCounter2(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return Const::doubleNaN;
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->isRevo2();
      } else {
        return Const::doubleNaN;
      }
    }


    VARIABLE_GROUP("Event");

    REGISTER_VARIABLE("isMC", isMC,
                      "[Eventbased] Returns 1 if current basf2 process is running over simulated (Monte-Carlo) dataset and 0 in case of real experimental data.");
    REGISTER_VARIABLE("isContinuumEvent", isContinuumEvent,
                      "[Eventbased] Returns 1.0 if event doesn't contain a :math:`\\Upsilon(4S)` particle on generator level, 0.0 otherwise.");
    REGISTER_VARIABLE("isNotContinuumEvent", isNotContinuumEvent,
                      "[Eventbased] Returns 1.0 if event does contain an :math:`\\Upsilon(4S)` particle on generator level and therefore is not a continuum event, 0.0 otherwise.");

    REGISTER_VARIABLE("isChargedBEvent", isChargedBEvent,
                      "[Eventbased] Returns 1.0 if event contains a charged B-meson on generator level.");
    REGISTER_VARIABLE("isUnmixedBEvent", isUnmixedBEvent,
                      R"DOC([Eventbased] Returns 1.0 if the event contains opposite flavor neutral B-mesons on generator level,
0.0 in case of same flavor B-mesons and NaN if the event has no generated neutral B.)DOC");

    REGISTER_VARIABLE("nTracks", nTracks, R"DOC(
[Eventbased] Returns the total number of tracks (unfiltered) in the event.

.. warning:: This variable is exceedingly background-dependent and should not really be used in any selections (other than perhaps for monitoring purposes).
.. seealso:: :b2:var:`nCleanedTracks` for a more useful variable for use in selections.
)DOC");
    REGISTER_VARIABLE("nChargeZeroTrackFits", nChargeZeroTrackFits, R"DOC(
[Eventbased] Returns number of track fits with zero charge.

.. note::
  Sometimes, track fits can have zero charge, if background or non IP originating tracks, for example, are fit from the IP.
  These tracks are excluded from particle lists, but a large amount of charge zero
  fits may indicate problems with whole event constraints
  or abnominally high beam backgrounds and/or noisy events.
)DOC");

    REGISTER_VARIABLE("belleECLEnergy", belleECLEnergy, R"DOC(
[Eventbased][Legacy] Returns total energy in ECL in the event as used in Belle 1 analyses.

.. warning::
  For Belle II use cases use either ``totalEnergyOfParticlesInList(gamma:all)``,
  or (probably better) fill a photon list with some minimal cleanup cuts and use that instead:

  .. code-block:: python

    from variables import variables as vm
    fillParticleList("gamma:cleaned", "E > 0.05 and isFromECL==1", path=path)
    fillParticleList("e+:cleaned", "clusterE > 0.05", path=path)
    vm.addAlias("myNeutralECLEnergy", "totalEnergyOfParticlesInList(gamma:cleaned)")
    vm.addAlias("myChargedECLEnergy", "totalEnergyOfParticlesInList(e+:cleaned)")
    vm.addAlias("myECLEnergy", "formula(myNeutralECLEnergy+myChargedECLEnergy)")

)DOC","GeV");
    REGISTER_VARIABLE("nKLMClusters", nKLMClusters,
                      "[Eventbased] Returns number of KLM clusters in the event.");
    REGISTER_VARIABLE("nMCParticles", nMCParticles,
                      "[Eventbased] Returns number of MCParticles in the event.");
    REGISTER_VARIABLE("nPrimaryMCParticles", nPrimaryMCParticles,
                      "[Eventbased] Returns number of primary MCParticles in the event.");
    REGISTER_VARIABLE("nInitialPrimaryMCParticles", nInitialPrimaryMCParticles,
                      "[Eventbased] Returns number of initial primary MCParticles in the event.");
    REGISTER_VARIABLE("nVirtualPrimaryMCParticles", nVirtualPrimaryMCParticles,
                      "[Eventbased] Returns number of virtual primary MCParticles in the event.");

    REGISTER_VARIABLE("expNum", expNum, "[Eventbased] Returns the experiment number.");
    REGISTER_VARIABLE("evtNum", evtNum, "[Eventbased] Returns the event number.");
    REGISTER_VARIABLE("runNum", runNum, "[Eventbased] Returns the run number.");
    REGISTER_VARIABLE("productionIdentifier", productionIdentifier, R"DOC(
[Eventbased] Production identifier.
Uniquely identifies an MC sample by the (grid-jargon) production ID.
This is useful when analysing large MC samples split between more than one production or combining different MC samples (e.g. combining all continuum samples).
In such cases the event numbers are sequential *only within a production*, so experiment/run/event will restart with every new sample analysed.

.. tip:: Experiment/run/event/production is unique for all MC samples. Experiment/run/event is unique for data.

.. seealso:: `Where can I rely on uniqueness of the ['__experiment__', '__run__', '__event__', '__candidate__'] combination? <https://questions.belle2.org/question/9704>`__
)DOC");

    REGISTER_VARIABLE("Ecms", getCMSEnergy, "[Eventbased] Returns center-of-mass energy.\n\n", "GeV");
    REGISTER_VARIABLE("beamE", getBeamE, "[Eventbased] Returns total beam energy in the laboratory frame.\n\n","GeV");
    REGISTER_VARIABLE("beamPx", getBeamPx, "[Eventbased] Returns x component of total beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("beamPy", getBeamPy, "[Eventbased] Returns y component of total beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("beamPz", getBeamPz, "[Eventbased] Returns z component of total beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("EcmsMC", getCMSEnergyMC, "[Eventbased] Truth value of sqrt(s)\n\n", "GeV");
    REGISTER_VARIABLE("totalEnergyMC", getTotalEnergyMC, "[Eventbased] Truth value of sum of energies of all the generated particles\n\n", "GeV");


    REGISTER_VARIABLE("PxHER", getPxHER, "[Eventbased] Returns truth value of the x component of the incoming electron momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("PyHER", getPyHER, "[Eventbased] Returns truth value of the y component of the incoming electron momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("PzHER", getPzHER, "[Eventbased] Returns truth value of the z component of the incoming electron momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("PxLER", getPxLER, "[Eventbased] Returns truth value of the x component of the incoming positron momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("PyLER", getPyLER, "[Eventbased] Returns truth value of the y component of the incoming positron momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("PzLER", getPzLER, "[Eventbased] Returns truth value of the z component of the incoming positron momentum in the laboratory frame.\n\n","GeV/c");

    REGISTER_VARIABLE("mcPxHER", getMcPxHER, "[Eventbased] Returns x component of the electron beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("mcPyHER", getMcPyHER, "[Eventbased] Returns y component of the electron beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("mcPzHER", getMcPzHER, "[Eventbased] Returns z component of the electron beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("mcPxLER", getMcPxLER, "[Eventbased] Returns x component of the positron beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("mcPyLER", getMcPyLER, "[Eventbased] Returns y component of the positron beam momentum in the laboratory frame.\n\n","GeV/c");
    REGISTER_VARIABLE("mcPzLER", getMcPzLER, "[Eventbased] Returns z component of the positron beam momentum in the laboratory frame.\n\n","GeV/c");



    REGISTER_VARIABLE("IPX", getIPX, R"DOC(
[Eventbased] Returns x coordinate of the measured interaction point.

.. note:: For old data and uncalibrated MC files this will return 0.0.

.. note:: You might hear tracking and calibration people refer to this as the ``BeamSpot``.

)DOC","cm");
    REGISTER_VARIABLE("IPY", getIPY, "[Eventbased] Returns y coordinate of the measured interaction point.\n\n","cm");
    REGISTER_VARIABLE("IPZ", getIPZ, "[Eventbased] Returns z coordinate of the measured interaction point.\n\n","cm");
    REGISTER_VARIABLE("IPCov(i,j)", ipCovMatrixElement, "[Eventbased] Returns (i,j)-th element of the covariance matrix of the measured interaction point.\n\n",":math:`\\text{cm}^2`");

    REGISTER_VARIABLE("genIPX", getGenIPX, R"DOC(
[Eventbased] Returns x coordinate of the interaction point used for the underlying **MC generation**.
Returns NaN for data.

.. note:: This is normally smeared from 0.0

)DOC","cm");
    REGISTER_VARIABLE("genIPY", getGenIPY, "[Eventbased] Returns y coordinate of the interaction point used for the underlying **MC generation**. Returns NaN for data.\n\n","cm");
    REGISTER_VARIABLE("genIPZ", getGenIPZ, "[Eventbased] Returns z coordinate of the interaction point used for the underlying **MC generation**. Returns NaN for data.\n\n","cm");

    REGISTER_VARIABLE("date", eventYearMonthDay, R"DOC(
[Eventbased] Returns the date when the event was recorded, a number of the form YYYYMMDD (in UTC).

.. seealso:: :b2:var:`year`, :b2:var:`eventTimeSeconds`, :b2:var:`eventTimeSecondsFractionRemainder`, provided for convenience.
)DOC");
    REGISTER_VARIABLE("year", eventYear, R"DOC(
[Eventbased] Returns the year when the event was recorded (in UTC).

.. tip::
  For more precise event time, see :b2:var:`eventTimeSeconds` and :b2:var:`eventTimeSecondsFractionRemainder`.
)DOC");
    REGISTER_VARIABLE("eventTimeSeconds", eventTimeSeconds,
                      "[Eventbased] Time of the event (truncated down) since 1970/1/1 (Unix epoch).\n\n","s");
    REGISTER_VARIABLE("eventTimeSecondsFractionRemainder", eventTimeSecondsFractionRemainder, R"DOC(
[Eventbased] Remainder of the event time.

.. tip:: Use eventTimeSeconds + eventTimeSecondsFractionRemainder to get the total event time in seconds.

)DOC","s");

    REGISTER_VARIABLE("timeSincePrevTriggerClockTicks", timeSincePrevTriggerClockTicks,
                      "[Eventbased] Time since the previous trigger (127MHz=RF/4 clock).\n\n","clock ticks");

    REGISTER_VARIABLE("timeSincePrevTriggerMicroSeconds", timeSincePrevTriggerMicroSeconds,
                      "[Eventbased] Time since the previous trigger.\n\n",":math:`\\mathrm{\\mu s}`");

    REGISTER_VARIABLE("triggeredBunchNumberTTD", triggeredBunchNumberTTD, R"DOC(
[Eventbased] Number of triggered bunch ranging from 0-1279.

.. warning:: This is the bunch number as provided by the TTD, which does not necessarily correspond to the 'global' SKB bunch number.
.. note:: There are a maximum of 5120 buckets, which could each carry one bunch of e+/e-, but we only have 1280 clock ticks (=5120/4) to identify the bunches.
)DOC");

    REGISTER_VARIABLE("triggeredBunchNumber", triggeredBunchNumber, R"DOC(
[Eventbased] Number of triggered bunch ranging from 0-1279.

.. note:: There are a maximum of 5120 buckets, which could each carry one bunch of e+/e-, but we only have 1280 clock ticks (=5120/4) to identify the bunches
)DOC");

    REGISTER_VARIABLE("hasRecentInjection", hasRecentInjection,
                      "[Eventbased] Returns 1 if an injection happened recently, 0 otherwise.");

    REGISTER_VARIABLE("timeSinceLastInjectionSignalClockTicks", timeSinceLastInjectionSignalClockTicks, R"DOC(
[Eventbased] Time since the last injection pre-kick signal (127MHz=RF/4 clock)

.. warning:: this returns the time without the delay until the injected bunch reaches the detector (which differs for HER/LER)

)DOC","clock ticks");

    REGISTER_VARIABLE("timeSinceLastInjectionSignalMicroSeconds", timeSinceLastInjectionSignalMicroSeconds, R"DOC(
[Eventbased] Time since the last injection pre-kick signal

.. warning:: this returns the time without the delay until the injected bunch reaches the detector (which differs for HER/LER)

)DOC",":math:`\\mathrm{\\mu s}`");

    REGISTER_VARIABLE("timeSinceLastInjectionClockTicks", timeSinceLastInjectionClockTicks,
      "[Eventbased] Time since the last injected bunch passed by the detector.\n\n","clock ticks")

    REGISTER_VARIABLE("timeSinceLastInjectionMicroSeconds", timeSinceLastInjectionMicroSeconds,
      "[Eventbased] Time since the last injected bunch passed by the detector.\n\n",":math:`\\mathrm{\\mu s}`")

    REGISTER_VARIABLE("injectionInHER", injectionInHER,
                  "[Eventbased] Returns 1 if injection was in HER, 0 otherwise.");

    REGISTER_VARIABLE("revolutionCounter2", revolutionCounter2, R"DOC(
[Eventbased] The lowest bit of revolution counter, i.e. return 0 or 1

.. note:: related to PXD data acquisition; PXD needs ~2 revolutions to read out one frame
)DOC");

    VARIABLE_GROUP("EventKinematics");

    REGISTER_VARIABLE("missingMomentumOfEvent", missingMomentumOfEvent, R"DOC(
[Eventbased] The magnitude of the missing momentum in laboratory frame.

.. warning:: You have to run the Event Kinematics builder module for this variable to be meaningful.
.. seealso:: `modularAnalysis.buildEventKinematics`.

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEvent_Px", missingMomentumOfEvent_Px, R"DOC(
[Eventbased] The x component of the missing momentum in laboratory frame.

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEvent_Py", missingMomentumOfEvent_Py, R"DOC(
[Eventbased] The y component of the missing momentum in laboratory frame.

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEvent_Pz", missingMomentumOfEvent_Pz, R"DOC(
[Eventbased] The z component of the missing momentum in laboratory frame.

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEvent_theta", missingMomentumOfEvent_theta, R"DOC(
[Eventbased] The theta angle of the missing momentum of the event in laboratory frame.

)DOC","rad");
    REGISTER_VARIABLE("missingMomentumOfEventCMS", missingMomentumOfEventCMS, R"DOC(
[Eventbased] The magnitude of the missing momentum in center-of-mass frame.

)DOC","GeV/c");
    REGISTER_VARIABLE("genMissingMomentumOfEventCMS", genMissingMomentumOfEventCMS, R"DOC(
[Eventbased] The magnitude of the missing momentum in center-of-mass frame from generator

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Px", missingMomentumOfEventCMS_Px, R"DOC(
[Eventbased] The x component of the missing momentum in center-of-mass frame.

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Py", missingMomentumOfEventCMS_Py, R"DOC(
[Eventbased] The y component of the missing momentum in center-of-mass frame.

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Pz", missingMomentumOfEventCMS_Pz, R"DOC(
[Eventbased] The z component of the missing momentum in center-of-mass frame.

)DOC","GeV/c");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_theta", missingMomentumOfEventCMS_theta, R"DOC(
[Eventbased] The theta angle of the missing momentum in center-of-mass frame.

)DOC","rad");
    REGISTER_VARIABLE("missingEnergyOfEventCMS", missingEnergyOfEventCMS, R"DOC(
[Eventbased] The missing energy in center-of-mass frame.

)DOC","GeV");
    REGISTER_VARIABLE("genMissingEnergyOfEventCMS", genMissingEnergyOfEventCMS, R"DOC(
[Eventbased] The missing energy in center-of-mass frame from generator.

)DOC","GeV");
    REGISTER_VARIABLE("missingMass2OfEvent", missingMass2OfEvent, R"DOC(
[Eventbased] The missing mass squared.

)DOC",":math:`[\\text{GeV}/\\text{c}^2]^2`");
    REGISTER_VARIABLE("genMissingMass2OfEvent", genMissingMass2OfEvent, R"DOC(
[Eventbased] The missing mass squared from generator

)DOC",":math:`[\\text{GeV}/\\text{c}^2]^2`");
    REGISTER_VARIABLE("visibleEnergyOfEventCMS", visibleEnergyOfEventCMS, R"DOC(
[Eventbased] The visible energy in center-of-mass frame.

)DOC","GeV");
    REGISTER_VARIABLE("genVisibleEnergyOfEventCMS", genVisibleEnergyOfEventCMS, R"DOC(
[Eventbased] The visible energy in center-of-mass frame from generator.

)DOC","GeV");
    REGISTER_VARIABLE("totalPhotonsEnergyOfEvent", totalPhotonsEnergyOfEvent, R"DOC(
[Eventbased] The energy in laboratory frame of all the photons.

)DOC","GeV");
    REGISTER_VARIABLE("genTotalPhotonsEnergyOfEvent", genTotalPhotonsEnergyOfEvent, R"DOC(
[Eventbased] The energy in laboratory frame of all the photons. from generator.

)DOC","GeV");

    VARIABLE_GROUP("Event (cDST only)");
    REGISTER_VARIABLE("eventT0", eventT0, R"DOC(
[Eventbased][Calibration] The Event t0, is the time of the event relative to the trigger time.

.. note::
    The event time can be measured by several sub-detectors including the SVD, CDC, ECL, and TOP.
    This eventT0 variable is the final combined value of all the event time measurements.
    Currently, only the SVD and ECL are used in this combination.

)DOC","ns");
  }
}
