/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Matic Lubej                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/ROEVariables.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ParticleList.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/logging/Logger.h>

#include <iostream>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double isInRestOfEvent(const Particle* particle)
    {

      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 1.0;

      // Check for Tracks
      const auto& tracks = roe->getTracks();
      if (std::find(tracks.begin(), tracks.end(), particle->getTrack()) != tracks.end()) {
        return 1.0;
      }

      // Check for KLMClusters
      const auto& klm = roe->getKLMClusters();
      if (std::find(klm.begin(), klm.end(), particle->getKLMCluster()) != klm.end()) {
        return 1.0;
      }

      // Check for ECLClusters
      const auto& ecl = roe->getECLClusters();
      if (std::find(ecl.begin(), ecl.end(), particle->getECLCluster()) != ecl.end()) {
        return 1.0;
      }
      return 0;
    }

    double nROETracks(const Particle* particle)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -1;
      }

      return roe->getNTracks();
    }

    double nRemainingTracksInRestOfEvent(const Particle* particle)
    {

      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 0.0;


      int roe_tracks = roe->getNTracks();
      int par_tracks = 0;
      const auto& daughters = particle->getFinalStateDaughters();
      for (const auto& daughter : daughters) {
        int pdg = abs(daughter->getPDGCode());
        if (pdg == 11 or pdg == 13 or pdg == 211 or pdg == 321 or pdg == 2212)
          par_tracks++;
      }
      return roe_tracks - par_tracks;
    }

    double nROEECLClusters(const Particle* particle)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -1;
      }

      return roe->getNECLClusters();
    }

    double nROEKLMClusters(const Particle* particle)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -1;
      }

      return roe->getNKLMClusters();
    }

    double pionVeto(const Particle* particle)
    {
      double pion0Mass = 0.135;           // neutral pion mass from PDG
      double deltaE = 0.03;               // mass range around pion0Mass that will be accepted

      StoreObjPtr<ParticleList> PhotonList("gamma:veto");

      const Particle* sig_Photon = particle->getDaughter(1)->getDaughter(0);
      TLorentzVector vec = sig_Photon->get4Vector();

      for (unsigned int i = 0; i < PhotonList->getListSize(); i++) {
        Particle* p_Photon = PhotonList->getParticle(i);
        if ((p_Photon->getEnergy() >= 0.1) && (p_Photon->getMdstArrayIndex() != sig_Photon->getMdstArrayIndex())) {
          double tempCombination = (p_Photon->get4Vector() + vec).M();
          if (abs(tempCombination - pion0Mass) <= deltaE) {
            return 1;
          }
        }
      }

      return 0;
    }

    double nROENeutralECLClusters(const Particle* particle)
    {
      int nNeutrals = 0;

      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -999;
      }

      // Get all ECLClusters in ROE
      const std::vector<ECLCluster*> roeClusters = roe->getECLClusters();

      // Select ECLClusters with no associated tracks
      for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++) {
        if (roeClusters[iEcl]->isNeutral())
          nNeutrals++;
      }

      return nNeutrals;
    }

    double nROELeptons(const Particle* particle)
    {
      int nLeptons = 0;

      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -999;
      }

      // Get all Tracks in ROE
      const std::vector<Track*> roeTracks = roe->getTracks();

      // Count tracks from leptons
      for (unsigned int iTrack = 0; iTrack < roeTracks.size(); iTrack++) {
        const PIDLikelihood* pid = roeTracks[iTrack]->getRelatedTo<PIDLikelihood>();

        if (!pid) {
          B2ERROR("No PID information for this track!");
          return -1;
        }

        int absPDGCode = abs(pid->getMostLikely().getPDGCode());

        if (absPDGCode == 11 or absPDGCode == 13)
          nLeptons++;
      }

      return nLeptons;
    }

    double ROECharge(const Particle* particle)
    {
      int roeCharge = 0;

      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -999;
      }

      // Get all tracks in ROE
      const std::vector<Track*> roeTracks = roe->getTracks();

      for (unsigned int iTrack = 0; iTrack < roeTracks.size(); iTrack++) {
        const TrackFitResult* tfr = roeTracks[iTrack]->getTrackFitResult(Const::ChargedStable(211));
        roeCharge += tfr->getChargeSign();
      }

      return roeCharge;
    }

    double ROEDeltaEnergyTag(const Particle* particle)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * ROE4Vector(particle);

      return T.getCMSEnergy() / 2 - vec.E();
    }

    double ROEMassTag(const Particle* particle)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * ROE4Vector(particle);
      double E = T.getCMSEnergy() / 2;
      double m2 = E * E - vec.Vect().Mag2();
      double mbc = m2 > 0 ? sqrt(m2) : 0;
      return mbc;
    }

    TLorentzVector ROE4Vector(const Particle* particle)
    {
      TLorentzVector ROE4Vector;

      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        TLorentzVector empty;
        return empty;
      }

      // Get all ECLClusters in ROE
      const std::vector<ECLCluster*> roeClusters = roe->getECLClusters();

      // Get all tracks in ROE
      const std::vector<Track*> roeTracks = roe->getTracks();

      // Add all momentum from tracks
      for (unsigned int iTrack = 0; iTrack < roeTracks.size(); iTrack++) {
        const PIDLikelihood* pid = roeTracks[iTrack]->getRelatedTo<PIDLikelihood>();

        if (!pid) {
          B2ERROR("No PID information for this track!");
          TLorentzVector empty;
          return empty;
        }

        // At the moment (16.7.2015) only Pion TrackFitResult available
        const TrackFitResult* tfr = roeTracks[iTrack]->getTrackFitResult(pid->getMostLikely());
        ROE4Vector += tfr->get4Momentum();
      }

      // Add all momentum from neutral ECLClusters
      for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++) {
        if (roeClusters[iEcl]->isNeutral())
          ROE4Vector += roeClusters[iEcl]->get4Vector();
      }

      return ROE4Vector;
    }


    VARIABLE_GROUP("Rest Of Event");

    REGISTER_VARIABLE("isInRestOfEvent", isInRestOfEvent,
                      "Returns 1 if a track, ecl or klmCluster associated to particle is in the current RestOfEvent object, 0 otherwise."
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("nROETracks",  nROETracks,
                      "Returns number of tracks in the related RestOfEvent object.");

    REGISTER_VARIABLE("nRemainingTracksInRestOfEvent", nRemainingTracksInRestOfEvent,
                      "Returns number of tracks in ROE - number of tracks of given particle"
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("nROEECLClusters", nROEECLClusters,
                      "Returns number of ECL clusters in the related RestOfEvent object.");

    REGISTER_VARIABLE("nROENeutralECLClusters", nROENeutralECLClusters,
                      "Returns number of remaining neutral ECL clusters in the related RestOfEvent object.");

    REGISTER_VARIABLE("nROEKLMClusters", nROEKLMClusters,
                      "Returns number of remaining KLM clusters in the related RestOfEvent object.");

    REGISTER_VARIABLE("pionVeto", pionVeto,
                      "Returns the Flag 1 if a combination of photons has the invariant mass of a neutral pion");

    REGISTER_VARIABLE("nROELeptons", nROELeptons,
                      "Returns number of lepton particles in the related RestOfEvent object.");

    REGISTER_VARIABLE("ROE_Charge", ROECharge,
                      "Returns total charge of the related RestOfEvent object.");

    REGISTER_VARIABLE("ROE_dEtag", ROEDeltaEnergyTag,
                      "Returns energy difference of the related RestOfEvent object with respect to E_cms/2.");

    REGISTER_VARIABLE("ROE_Mtag", ROEMassTag,
                      "Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2.");

  }
}
