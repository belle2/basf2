/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2020 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck, Sam Cunliffe     *
 *       for the EventKinematics variables: Ami Rostomyan,                *
 *                                          Michel Villanueva             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/EventVariables.h>

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

#include <TLorentzVector.h>
#include <TVector3.h>

namespace Belle2 {
  namespace Variable {

    // Event ------------------------------------------------
    double isMC(const Particle*)
    {
      return Environment::Instance().isMC();
    }

    double eventType(const Particle*)
    {
      StoreArray<MCParticle> mcparticles;
      return (mcparticles.getEntries()) > 0 ? 0 : 1;
    }

    double isContinuumEvent(const Particle*)
    {
      return (isNotContinuumEvent(nullptr) == 1.0 ? 0.0 : 1.0);
    }

    double isChargedBEvent(const Particle*)
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
      return std::numeric_limits<float>::quiet_NaN();
    }


    double isNotContinuumEvent(const Particle*)
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
          return 1.0;
      }
      return 0.0;
    }

    double nMCParticles(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      return mcps.getEntries();
    }

    double nTracks(const Particle*)
    {
      StoreArray<Track> tracks;
      return tracks.getEntries();
    }

    double nChargeZeroTrackFits(const Particle*)
    {
      StoreArray<TrackFitResult> tfrs;
      int out = 0;
      for (const auto& t : tfrs)
        if (t.getChargeSign() == 0) out++;
      return double(out);
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

    double nKLMClusters(const Particle*)
    {
      StoreArray<KLMCluster> klmClusters;
      return klmClusters.getEntries();
    }

    double expNum(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      int exp_no = evtMetaData->getExperiment();
      return exp_no;
    }

    double productionIdentifier(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      int eventProduction = evtMetaData->getProduction();
      return eventProduction;
    }

    double evtNum(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      int evt_no = evtMetaData->getEvent();
      return evt_no;
    }

    double runNum(const Particle*)
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

    double getGenIPX(const Particle*)
    {
      // generated IP corresponds to the generated vertex of the
      // first not-initial and not-virtual MCParticle
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (not mcp.isInitial() and not mcp.isVirtual() and mcp.isPrimaryParticle())
          return mcp.getVertex().X();
      return std::numeric_limits<double>::quiet_NaN();
    }

    double getGenIPY(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (not mcp.isInitial() and not mcp.isVirtual() and mcp.isPrimaryParticle())
          return mcp.getVertex().Y();
      return std::numeric_limits<double>::quiet_NaN();
    }

    double getGenIPZ(const Particle*)
    {
      StoreArray<MCParticle> mcps;
      for (const auto& mcp : mcps)
        if (not mcp.isInitial() and not mcp.isVirtual() and mcp.isPrimaryParticle())
          return mcp.getVertex().Z();
      return std::numeric_limits<double>::quiet_NaN();
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
        B2WARNING("Requested IP covariance matrix element is out of boundaries [0 - 3]: i = " << elementI);
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (elementJ < 0 || elementJ > 3) {
        B2WARNING("Requested particle's momentumVertex covariance matrix element is out of boundaries [0 - 3]: j = " << elementJ);
        return std::numeric_limits<float>::quiet_NaN();
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
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentum().Mag();
      return missing;
    }

    double missingMomentumOfEvent_Px(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentum().Px();
      return missing;
    }

    double missingMomentumOfEvent_Py(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentum().Py();
      return missing;
    }

    double missingMomentumOfEvent_Pz(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentum().Pz();
      return missing;
    }

    double missingMomentumOfEvent_theta(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentum().Theta();
      return missing;
    }

    double missingMomentumOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentumCMS().Mag();
      return missing;
    }

    double genMissingMomentumOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentumCMS().Mag();
      return missing;
    }

    double missingMomentumOfEventCMS_Px(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentumCMS().Px();
      return missing;
    }

    double missingMomentumOfEventCMS_Py(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentumCMS().Py();
      return missing;
    }

    double missingMomentumOfEventCMS_Pz(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMomentumCMS().Pz();
      return missing;
    }

    double missingMomentumOfEventCMS_theta(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double theta = evtShape->getMissingMomentumCMS().Theta();
      return theta;
    }

    double missingEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingEnergyCMS();
      return missing;
    }

    double genMissingEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingEnergyCMS();
      return missing;
    }


    double missingMass2OfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMass2();
      return missing;
    }

    double genMissingMass2OfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double missing = evtShape->getMissingMass2();
      return missing;
    }

    double visibleEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double visible = evtShape->getVisibleEnergyCMS();
      return visible;
    }

    double genVisibleEnergyOfEventCMS(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double visible = evtShape->getVisibleEnergyCMS();
      return visible;
    }


    double totalPhotonsEnergyOfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double energyOfPhotons = evtShape->getTotalPhotonsEnergy();
      return energyOfPhotons;
    }

    double genTotalPhotonsEnergyOfEvent(const Particle*)
    {
      StoreObjPtr<EventKinematics> evtShape("EventKinematicsFromMC");
      if (!evtShape) {
        B2WARNING("Cannot find missing momentum information, did you forget to run EventKinematicsModule with usingMC parameter set to true?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      double energyOfPhotons = evtShape->getTotalPhotonsEnergy();
      return energyOfPhotons;
    }


    double eventYearMonthDay(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      if (!evtMetaData) {
        return std::numeric_limits<float>::quiet_NaN();
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
        return std::numeric_limits<float>::quiet_NaN();
      }
      std::time_t rawtime = trunc(evtMetaData->getTime() / 1e9);
      auto tt = std::gmtime(&rawtime);
      return tt->tm_year + 1900;
    }

    double eventTimeSeconds(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;

      if (!evtMetaData) {
        return std::numeric_limits<float>::quiet_NaN();
      }
      double evtTime = trunc(evtMetaData->getTime() / 1e9);

      return evtTime;
    }

    double eventTimeSecondsFractionRemainder(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;

      if (!evtMetaData) {
        return std::numeric_limits<float>::quiet_NaN();
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
        return std::numeric_limits<float>::quiet_NaN();
      }

      if (evtT0->hasEventT0()) {
        return evtT0->getEventT0();
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double timeSincePrevTriggerClockTicks(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->getTimeSincePrevTrigger();
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double timeSincePrevTriggerMicroSeconds(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->getTimeSincePrevTriggerInMicroSeconds();
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double triggeredBunchNumber(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->getBunchNumber();
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double hasRecentInjection(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->hasInjection() > 0.5 ? 1 : 0;
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double timeSinceLastInjectionSignalClockTicks(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->getTimeSinceLastInjection();
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double timeSinceLastInjectionSignalMicroSeconds(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->getTimeSinceLastInjectionInMicroSeconds();
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double injectionInHER(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid and if an injection happened recently
      if (TTDInfo->isValid() && TTDInfo->hasInjection()) {
        return TTDInfo->isHER() > 0.5 ? 1 : 0;
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double revolutionCounter2(const Particle*)
    {
      StoreObjPtr<EventLevelTriggerTimeInfo> TTDInfo;

      // Check if the pointer is valid
      if (!TTDInfo.isValid()) {
        B2WARNING("StoreObjPtr<EventLevelTriggerTimeInfo> does not exist, are you running over data reconstructed with release-05 or earlier?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // And check if the stored data is valid
      if (TTDInfo->isValid()) {
        return TTDInfo->isRevo2() > 0.5 ? 1 : 0;
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
    }


    VARIABLE_GROUP("Event");

    REGISTER_VARIABLE("isMC", isMC,
                      "[Eventbased] Returns 1 if current basf2 process is running over simulated (Monte-Carlo) dataset and 0 in case of real experimental data.");
    REGISTER_VARIABLE("EventType", eventType, "[Eventbased] EventType (0 MC, 1 Data)");
    MAKE_DEPRECATED("EventType", true, "light-minos-2012", R"DOC(
                     Use `isMC` instead of this variable but keep in mind that the meaning of the outcome is reversed.)DOC");
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
  or (probably better) fill a photon list with some minimal cleanup cuts and use that instea
  
  .. code-block:: python

    from variables import variables as vm
    fillParticleList("gamma:cleaned", "E > 0.05 and isFromECL==1", path=path)
    fillParticleList("e+:cleaned", "clusterE > 0.05", path=path)
    vm.addAlias("myNeutralECLEnergy", "totalEnergyOfParticlesInList(gamma:cleaned)")
    vm.addAlias("myChargedECLEnergy", "totalEnergyOfParticlesInList(e+:cleaned)")
    vm.addAlias("myECLEnergy", "formula(myNeutralECLEnergy+myChargedECLEnergy)")
)DOC");
    REGISTER_VARIABLE("nKLMClusters", nKLMClusters,
                      "[Eventbased] Returns number of KLM clusters in the event.");
    REGISTER_VARIABLE("nMCParticles", nMCParticles,
                      "[Eventbased] Returns number of MCParticles in the event.");

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

    REGISTER_VARIABLE("Ecms", getCMSEnergy, "[Eventbased] Returns center-of-mass energy.");
    REGISTER_VARIABLE("beamE", getBeamE, "[Eventbased] Returns total beam energy in the laboratory frame.");
    REGISTER_VARIABLE("beamPx", getBeamPx, "[Eventbased] Returns x component of total beam momentum in the laboratory frame.");
    REGISTER_VARIABLE("beamPy", getBeamPy, "[Eventbased] Returns y component of total beam momentum in the laboratory frame.");
    REGISTER_VARIABLE("beamPz", getBeamPz, "[Eventbased] Returns z component of total beam momentum in the laboratory frame.");

    REGISTER_VARIABLE("IPX", getIPX, R"DOC(
[Eventbased] Returns x coordinate of the measured interaction point.

.. note:: For old data and uncalibrated MC files this will return 0.0.

.. note:: You might hear tracking and calibration people refer to this as the ``BeamSpot``.
)DOC");
    REGISTER_VARIABLE("IPY", getIPY, "[Eventbased] Returns y coordinate of the measured interaction point.");
    REGISTER_VARIABLE("IPZ", getIPZ, "[Eventbased] Returns z coordinate of the measured interaction point.");
    REGISTER_VARIABLE("IPCov(i,j)", ipCovMatrixElement, "[Eventbased] Returns (i,j)-th element of the covariance matrix of the measured interaction point.");

    REGISTER_VARIABLE("genIPX", getGenIPX, R"DOC(
[Eventbased] Returns x coordinate of the interaction point used for the underlying **MC generation**.
Returns NaN for data.

.. note:: This is normally smeared from 0.0
)DOC");
    REGISTER_VARIABLE("genIPY", getGenIPY, "[Eventbased] Returns y coordinate of the interaction point used for the underlying **MC generation**. Returns NaN for data.");
    REGISTER_VARIABLE("genIPZ", getGenIPZ, "[Eventbased] Returns z coordinate of the interaction point used for the underlying **MC generation**. Returns NaN for data.");

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
                      "[Eventbased] Time of the event in seconds (truncated down) since 1970/1/1 (Unix epoch).");
    REGISTER_VARIABLE("eventTimeSecondsFractionRemainder", eventTimeSecondsFractionRemainder, R"DOC(
[Eventbased] Remainder of the event time in fractions of a second.

.. tip::
  Use eventTimeSeconds + eventTimeSecondsFractionRemainder to get the total event time in seconds.
)DOC");

    REGISTER_VARIABLE("timeSincePrevTriggerClockTicks", timeSincePrevTriggerClockTicks,
                      "[Eventbased] Time since the previous trigger in clock ticks (127MHz=RF/4 clock).");

    REGISTER_VARIABLE("timeSincePrevTriggerMicroSeconds", timeSincePrevTriggerMicroSeconds,
                      "[Eventbased] Time since the previous trigger in micro seconds.");

    REGISTER_VARIABLE("triggeredBunchNumber", triggeredBunchNumber, R"DOC(
[Eventbased] Number of triggered bunch ranging from 0-1279.

.. note:: There are a maximum of 5120 buckets, which could each carry one bunch of e+/e-, but we only have 1280 clock ticks (=5120/4) to identify the bunches
)DOC");

    REGISTER_VARIABLE("hasRecentInjection", hasRecentInjection,
                      "[Eventbased] Returns 1 if an injection happened recently, 0 otherwise.");

    REGISTER_VARIABLE("timeSinceLastInjectionSignalClockTicks", timeSinceLastInjectionSignalClockTicks, R"DOC(
[Eventbased] Time since the last injection pre-kick signal in clock ticks (127MHz=RF/4 clock)

.. note:: this returns the time without the delay until the injected bunch reaches the detector (which differs for HER/LER)
)DOC");

    REGISTER_VARIABLE("timeSinceLastInjectionSignalMicroSeconds", timeSinceLastInjectionSignalMicroSeconds, R"DOC(
[Eventbased] Time since the last injection pre-kick signal in micro seconds

.. note:: this returns the time without the delay until the injected bunch reaches the detector (which differs for HER/LER)
)DOC");

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
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEvent_Px", missingMomentumOfEvent_Px, R"DOC(
[Eventbased] The x component of the missing momentum in laboratory frame.
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEvent_Py", missingMomentumOfEvent_Py, R"DOC(
[Eventbased] The y component of the missing momentum in laboratory frame.
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEvent_Pz", missingMomentumOfEvent_Pz, R"DOC(
[Eventbased] The z component of the missing momentum in laboratory frame.
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEvent_theta", missingMomentumOfEvent_theta, R"DOC(
[Eventbased] The theta angle of the missing momentum of the event in laboratory frame.
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEventCMS", missingMomentumOfEventCMS, R"DOC(
[Eventbased] The magnitude of the missing momentum in center-of-mass frame.
)DOC");
    REGISTER_VARIABLE("genMissingMomentumOfEventCMS", genMissingMomentumOfEventCMS, R"DOC(
[Eventbased] The magnitude of the missing momentum in center-of-mass frame from generator
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Px", missingMomentumOfEventCMS_Px, R"DOC(
[Eventbased] The x component of the missing momentum in center-of-mass frame.
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Py", missingMomentumOfEventCMS_Py, R"DOC(
[Eventbased] The y component of the missing momentum in center-of-mass frame.
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Pz", missingMomentumOfEventCMS_Pz, R"DOC(
[Eventbased] The z component of the missing momentum in center-of-mass frame.
)DOC");
    REGISTER_VARIABLE("missingMomentumOfEventCMS_theta", missingMomentumOfEventCMS_theta, R"DOC(
[Eventbased] The theta angle of the missing momentum in center-of-mass frame.
)DOC");
    REGISTER_VARIABLE("missingEnergyOfEventCMS", missingEnergyOfEventCMS, R"DOC(
[Eventbased] The missing energy in center-of-mass frame.
)DOC");
    REGISTER_VARIABLE("genMissingEnergyOfEventCMS", genMissingEnergyOfEventCMS, R"DOC(
[Eventbased] The missing energy in center-of-mass frame from generator.
)DOC");
    REGISTER_VARIABLE("missingMass2OfEvent", missingMass2OfEvent, R"DOC(
[Eventbased] The missing mass squared.
)DOC");
    REGISTER_VARIABLE("genMissingMass2OfEvent", genMissingMass2OfEvent, R"DOC(
[Eventbased] The missing mass squared from generator
)DOC");
    REGISTER_VARIABLE("visibleEnergyOfEventCMS", visibleEnergyOfEventCMS, R"DOC(
[Eventbased] The visible energy in center-of-mass frame.
)DOC");
    REGISTER_VARIABLE("genVisibleEnergyOfEventCMS", genVisibleEnergyOfEventCMS, R"DOC(
[Eventbased] The visible energy in center-of-mass frame from generator.
)DOC");
    REGISTER_VARIABLE("totalPhotonsEnergyOfEvent", totalPhotonsEnergyOfEvent, R"DOC(
[Eventbased] The energy in laboratory frame of all the photons.
)DOC");
    REGISTER_VARIABLE("genTotalPhotonsEnergyOfEvent", genTotalPhotonsEnergyOfEvent, R"DOC(
[Eventbased] The energy in laboratory frame of all the photons. from generator.
)DOC");

    VARIABLE_GROUP("Event (cDST only)");
    REGISTER_VARIABLE("eventT0", eventT0, R"DOC(
[Eventbased][Calibration] The Event t0, measured in ns, is the time of the event relative to the trigger time. 

.. note::
  The event time can be measured by several sub-detectors including the CDC, ECL, and TOP.
  This Event t0 variable is the final combined value of all the event time measurements.
  Currently only the CDC and ECL are used in this combination.
)DOC");
  }
}
