/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/EventVariables.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

//
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/dbobjects/BeamParameters.h>

#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TVector3.h>

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

    double nTracks(const Particle*)
    {
      StoreArray<Track> tracks;
      return tracks.getEntries();
    }

    double nECLClusters(const Particle*)
    {
      StoreArray<ECLCluster> eclClusters;
      return eclClusters.getEntries();
    }

    double nKLMClusters(const Particle*)
    {
      StoreArray<KLMCluster> klmClusters;
      return klmClusters.getEntries();
    }

    double ECLEnergy(const Particle*)
    {
      StoreArray<ECLCluster> eclClusters;
      double result = 0;
      for (int i = 0; i < eclClusters.getEntries(); ++i) {
        result += eclClusters[i]->getEnergy();
      }
      return result;
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


    VARIABLE_GROUP("Event");

    REGISTER_VARIABLE("EventType", eventType, "EventType (0 MC, 1 Data)");
    REGISTER_VARIABLE("isContinuumEvent", isContinuumEvent,
                      "[Eventbased] true if event doesn't contain an Y(4S)");
    REGISTER_VARIABLE("isNotContinuumEvent", isNotContinuumEvent,
                      "[Eventbased] 1.0 if event does contain an Y(4S) and therefore is not a continuum Event");

    REGISTER_VARIABLE("nTracks", nTracks,
                      "[Eventbased] number of tracks in the event");
    REGISTER_VARIABLE("nECLClusters", nECLClusters,
                      "[Eventbased] number of ECL in the event");
    REGISTER_VARIABLE("nKLMClusters", nKLMClusters,
                      "[Eventbased] number of KLM in the event");
    REGISTER_VARIABLE("ECLEnergy", ECLEnergy,
                      "[Eventbased] total energy in ECL in the event");
    REGISTER_VARIABLE("KLMEnergy", KLMEnergy,
                      "[Eventbased] total energy in KLM in the event");

    REGISTER_VARIABLE("expNum", expNum, "[Eventbased] experiment number");
    REGISTER_VARIABLE("evtNum", evtNum, "[Eventbased] event number");
    REGISTER_VARIABLE("runNum", runNum, "[Eventbased] run number");
    REGISTER_VARIABLE("productionIdentifier", productionIdentifier, "[Eventbased] production identifier");

    REGISTER_VARIABLE("Eher", getHEREnergy, "[Eventbased] HER energy");
    REGISTER_VARIABLE("Eler", getLEREnergy, "[Eventbased] LER energy");
    REGISTER_VARIABLE("Ecms", getCMSEnergy, "[Eventbased] CMS energy");
    REGISTER_VARIABLE("XAngle", getCrossingAngle, "[Eventbased] Crossing angle");

    REGISTER_VARIABLE("IPX", getIPX, "[Eventbased] x coordinate of the IP");
    REGISTER_VARIABLE("IPY", getIPY, "[Eventbased] y coordinate of the IP");
    REGISTER_VARIABLE("IPZ", getIPZ, "[Eventbased] z coordinate of the IP");

    REGISTER_VARIABLE("IPCov(i,j)", ipCovMatrixElement, "[Eventbased] (i,j)-th element of the IP covariance matrix")
  }
}
