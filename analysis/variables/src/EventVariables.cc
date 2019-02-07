/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
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
#include <analysis/dataobjects/TauPairDecay.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/dataobjects/EventT0.h>

// cluster utils
#include <analysis/ClusterUtility/ClusterUtils.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <framework/dbobjects/BeamParameters.h>

#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TVector3.h>

#include <functional>
#include <string>

namespace Belle2 {
  namespace Variable {

    // Event ------------------------------------------------
    double eventType(const Particle*)
    {
      StoreArray<MCParticle> mcparticles;
      return (mcparticles.getEntries()) > 0 ? 0 : 1;
    }

    double isContinuumEvent(const Particle*)
    {
      return (isNotContinuumEvent(nullptr) == 1.0 ? 0.0 : 1.0);
    }

    double isNotContinuumEvent(const Particle*)
    {
      StoreArray<MCParticle> mcParticles;
      if (!mcParticles) {
        B2ERROR("Cannot find MCParticles array.");
        return 0.0;
      }
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
      if (!mcps)  {
        B2DEBUG(19, "Cannot find MCParticles array.");
        return 0.0;
      }
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

    double nECLClusters(const Particle*)
    {
      StoreArray<ECLCluster> eclClusters;
      return eclClusters.getEntries();
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

    double KLMEnergy(const Particle*)
    {
      StoreArray<KLMCluster> klmClusters;
      double result = 0;
      for (int i = 0; i < klmClusters.getEntries(); ++i) {
        result += klmClusters[i]->getMomentum().Energy();
      }
      return result;
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
    double getHEREnergy(const Particle*)
    {
      PCmsLabTransform T;
      return T.getBeamParams().getHER().E();
    }

    double getLEREnergy(const Particle*)
    {
      PCmsLabTransform T;
      return T.getBeamParams().getLER().E();
    }

    double getCrossingAngle(const Particle*)
    {
      PCmsLabTransform T;
      return T.getBeamParams().getHER().Vect().Angle(-1.0 * T.getBeamParams().getLER().Vect());
    }

    double getCMSEnergy(const Particle*)
    {
      PCmsLabTransform T;
      return T.getBeamParams().getMass();
    }

    double getBeamPx(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamParams().getHER() + T.getBeamParams().getLER()).Px();
    }

    double getBeamPy(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamParams().getHER() + T.getBeamParams().getLER()).Py();
    }

    double getBeamPz(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamParams().getHER() + T.getBeamParams().getLER()).Pz();
    }

    double getBeamE(const Particle*)
    {
      PCmsLabTransform T;
      return (T.getBeamParams().getHER() + T.getBeamParams().getLER()).E();
    }

    double getIPX(const Particle*)
    {
      PCmsLabTransform T;
      return T.getBeamParams().getVertex().X();
    }

    double getIPY(const Particle*)
    {
      PCmsLabTransform T;
      return T.getBeamParams().getVertex().Y();
    }

    double getIPZ(const Particle*)
    {
      PCmsLabTransform T;
      return T.getBeamParams().getVertex().Z();
    }

    double ipCovMatrixElement(const Particle*, const std::vector<double>& element)
    {
      int elementI = int(std::lround(element[0]));
      int elementJ = int(std::lround(element[1]));

      if (elementI < 0 || elementI > 3) {
        B2WARNING("Requested IP covariance matrix element is out of boundaries [0 - 3]: i = " << elementI);
        return 0;
      }
      if (elementJ < 0 || elementJ > 3) {
        B2WARNING("Requested particle's momentumVertex covariance matrix element is out of boundaries [0 - 3]: j = " << elementJ);
        return 0;
      }

      PCmsLabTransform T;
      return T.getBeamParams().getCovVertex()(elementI, elementJ);
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
      double evtTime = 0.;

      if (!evtMetaData) {
        return std::numeric_limits<float>::quiet_NaN();
      }
      evtTime = trunc(evtMetaData->getTime() / 1e9);

      return evtTime;
    }

    double eventTimeSecondsFractionRemainder(const Particle*)
    {
      StoreObjPtr<EventMetaData> evtMetaData;
      double evtTimeFrac = 0.;

      if (!evtMetaData) {
        return std::numeric_limits<float>::quiet_NaN();
      }
      double evtTime = trunc(evtMetaData->getTime() / 1e9);

      evtTimeFrac = (evtMetaData->getTime() - evtTime * 1e9) / 1e9;

      return evtTimeFrac;
    }

    double eventT0(const Particle*)
    {
      StoreObjPtr<EventT0> evtT0;
      double t0 = 0.;

      if (!evtT0) {
        B2WARNING("StoreObjPtr<EventT0> does not exist, are you running over cDST data?");
        return std::numeric_limits<float>::quiet_NaN();
      }

      if (evtT0->hasEventT0()) {
        t0 = evtT0->getEventT0();
      } else {
        return std::numeric_limits<float>::quiet_NaN();
      }
      return t0;
    }



    VARIABLE_GROUP("Event");

    REGISTER_VARIABLE("EventType", eventType, "EventType (0 MC, 1 Data)");
    REGISTER_VARIABLE("isContinuumEvent", isContinuumEvent,
                      "[Eventbased] true if event doesn't contain an Y(4S)");
    REGISTER_VARIABLE("isNotContinuumEvent", isNotContinuumEvent,
                      "[Eventbased] 1.0 if event does contain an Y(4S) and therefore is not a continuum Event");

    REGISTER_VARIABLE("nTracks", nTracks,
                      "[Eventbased] number of tracks in the event");
    REGISTER_VARIABLE("nChargeZeroTrackFits", nChargeZeroTrackFits,
                      "[Eventbased] number of track fits with a zero charge."
                      "Sometimes this can happen if background or non IP originating "
                      "tracks (for example) are fit from the IP. These tracks are "
                      "removed from particle lists but a large number charge zero "
                      "fits them may indicate problems with whole event constraints "
                      "or abnominally high beam backgrounds and/or noisy events.")
    REGISTER_VARIABLE("nECLClusters", nECLClusters,
                      "[Eventbased] number of ECL in the event");
    REGISTER_VARIABLE("belleECLEnergy", belleECLEnergy,
                      "[Eventbased] legacy total energy in ECL in the event as used in Belle 1 analyses. For Belle II "
                      "consider totalEnergyOfParticlesInList(gamma:all) instead");
    REGISTER_VARIABLE("nKLMClusters", nKLMClusters,
                      "[Eventbased] number of KLM in the event");
    REGISTER_VARIABLE("KLMEnergy", KLMEnergy,
                      "[Eventbased] total energy in KLM in the event");
    REGISTER_VARIABLE("nMCParticles", nMCParticles,
                      "[Eventbased] number of MCParticles in the event");

    REGISTER_VARIABLE("expNum", expNum, "[Eventbased] experiment number");
    REGISTER_VARIABLE("evtNum", evtNum, "[Eventbased] event number");
    REGISTER_VARIABLE("runNum", runNum, "[Eventbased] run number");
    REGISTER_VARIABLE("productionIdentifier", productionIdentifier, "[Eventbased] production identifier");

    REGISTER_VARIABLE("Eher", getHEREnergy, "[Eventbased] HER energy");
    REGISTER_VARIABLE("Eler", getLEREnergy, "[Eventbased] LER energy");
    REGISTER_VARIABLE("Ecms", getCMSEnergy, "[Eventbased] CMS energy");
    REGISTER_VARIABLE("XAngle", getCrossingAngle, "[Eventbased] Crossing angle");
    REGISTER_VARIABLE("beamE", getBeamE, "[Eventbased] Beam energy (lab)");
    REGISTER_VARIABLE("beamPx", getBeamPx, "[Eventbased] Beam momentum Px (lab)");
    REGISTER_VARIABLE("beamPy", getBeamPy, "[Eventbased] Beam momentum Py (lab)");
    REGISTER_VARIABLE("beamPz", getBeamPz, "[Eventbased] Beam momentum Pz (lab)");

    REGISTER_VARIABLE("IPX", getIPX, "[Eventbased] x coordinate of the IP");
    REGISTER_VARIABLE("IPY", getIPY, "[Eventbased] y coordinate of the IP");
    REGISTER_VARIABLE("IPZ", getIPZ, "[Eventbased] z coordinate of the IP");

    REGISTER_VARIABLE("IPCov(i,j)", ipCovMatrixElement, "[Eventbased] (i,j)-th element of the IP covariance matrix")

    REGISTER_VARIABLE("date", eventYearMonthDay,
                      "[Eventbased] Returns the date when the event was recorded, a number of the form YYYYMMDD (in UTC).\n"
                      " See also eventYear, provided for convenience."
                      " For more precise eventTime, see eventTimeSeconds and eventTimeSecondsFractionRemainder.");
    REGISTER_VARIABLE("year", eventYear,
                      "[Eventbased] Returns the year when the event was recorded (in UTC).\n"
                      "For more precise eventTime, see eventTimeSeconds and eventTimeSecondsFractionRemainder.");
    REGISTER_VARIABLE("eventTimeSeconds", eventTimeSeconds,
                      "[Eventbased] Time of the event in seconds (truncated down) since 1970/1/1 (Unix epoch).");
    REGISTER_VARIABLE("eventTimeSecondsFractionRemainder", eventTimeSecondsFractionRemainder,
                      "[Eventbased] Remainder of the event time in fractions of a second.\n"
                      "Use eventTimeSeconds + eventTimeSecondsFractionRemainder to get the total event time in seconds.");

    VARIABLE_GROUP("EventKinematics");

    REGISTER_VARIABLE("missingMomentumOfEvent", missingMomentumOfEvent,
                      "[Eventbased] The magnitude of the missing momentum in lab obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEvent_Px", missingMomentumOfEvent_Px,
                      "[Eventbased] The x component of the missing momentum in lab obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEvent_Py", missingMomentumOfEvent_Py,
                      "[Eventbased] The y component of the missing momentum in lab obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEvent_Pz", missingMomentumOfEvent_Pz,
                      "[Eventbased] The z component of the missing momentum in lab obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEvent_theta", missingMomentumOfEvent_theta,
                      "[Eventbased] The theta angle of the missing momentum of the event in lab obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEventCMS", missingMomentumOfEventCMS,
                      "[Eventbased] The magnitude of the missing momentum in CMS obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Px", missingMomentumOfEventCMS_Px,
                      "[Eventbased] The x component of the missing momentum in CMS obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Py", missingMomentumOfEventCMS_Py,
                      "[Eventbased] The y component of the missing momentum in CMS obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEventCMS_Pz", missingMomentumOfEventCMS_Pz,
                      "[Eventbased] The z component of the missing momentum in CMS obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMomentumOfEventCMS_theta", missingMomentumOfEventCMS_theta,
                      "[Eventbased] The theta angle of the missing momentum in CMS obtained with EventKinematics module")
    REGISTER_VARIABLE("missingEnergyOfEventCMS", missingEnergyOfEventCMS,
                      "[Eventbased] The missing energy in CMS obtained with EventKinematics module")
    REGISTER_VARIABLE("missingMass2OfEvent", missingMass2OfEvent,
                      "[Eventbased] The missing mass squared obtained with EventKinematics module")
    REGISTER_VARIABLE("visibleEnergyOfEventCMS", visibleEnergyOfEventCMS,
                      "[Eventbased] The visible energy in CMS obtained with EventKinematics module")
    REGISTER_VARIABLE("totalPhotonsEnergyOfEvent", totalPhotonsEnergyOfEvent,
                      "[Eventbased] The energy in lab of all the photons obtained with EventKinematics module");

    VARIABLE_GROUP("Event (cDST only)");
    REGISTER_VARIABLE("eventT0", eventT0,
                      "[Eventbased][Calibration] Event T0 relative to trigger time in ns");
  }
}
