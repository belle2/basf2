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
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ParticleList.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/logging/Logger.h>

// utility
#include <analysis/utility/MCMatching.h>

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

      // Count leptons
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
        // At the moment (16.7.2015) only TrackFitResult with pion mass hypothesis available
        const TrackFitResult* tfr = roeTracks[iTrack]->getTrackFitResult(Const::ChargedStable(211));
        roeCharge += tfr->getChargeSign();
      }

      return roeCharge;
    }

    double ROEDeltaE(const Particle* particle)
    {
      PCmsLabTransform T;

      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -999;
      }

      TLorentzVector vec = T.rotateLabToCms() * roe->getROE4Vector();

      return T.getCMSEnergy() / 2 - vec.E();
    }

    double ROEMbc(const Particle* particle)
    {
      PCmsLabTransform T;

      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -999;
      }

      TLorentzVector vec = T.rotateLabToCms() * roe->getROE4Vector();

      double E = T.getCMSEnergy() / 2;
      double m2 = E * E - vec.Vect().Mag2();
      double mbc = m2 > 0 ? sqrt(m2) : 0;

      return mbc;
    }

    double correctedDeltaE(const Particle* particle)
    {
      PCmsLabTransform T;
      TLorentzVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
      TLorentzVector neutrino4vec = T.rotateLabToCms() * neutrino4Vector(particle);
      double totalSigEnergy = (sig4vec + neutrino4vec).Energy();
      double E = T.getCMSEnergy() / 2;

      double deltaE = E - totalSigEnergy;

      return deltaE;
    }

    double correctedMbc(const Particle* particle)
    {
      PCmsLabTransform T;
      TLorentzVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
      TLorentzVector neutrino4vec = T.rotateLabToCms() * neutrino4Vector(particle);
      TVector3 totalSigMomentum = (sig4vec + neutrino4vec).Vect();
      double E = T.getCMSEnergy() / 2;

      double m2 = E * E - totalSigMomentum.Mag2();
      double mbc = m2 > 0 ? sqrt(m2) : 0;

      return mbc;
    }

    double ECMissingMass(const Particle* particle, const std::vector<double>& opt)
    {
      double missM2 = missing4VectorCMS(particle, opt).Mag2();

      return missM2;
    }

    double ROEMCErrors(const Particle* particle)
    {
      StoreArray<Particle> particles;

      //Get MC Particle of the other B meson
      const MCParticle* mcParticle = particle->getRelatedTo<MCParticle>();

      if (!mcParticle)
        return -999;

      const MCParticle* mcMother = mcParticle->getMother();

      if (!mcMother)
        return -999;

      const std::vector<MCParticle*> mcDaughters = mcMother->getDaughters();

      if (mcDaughters.size() != 2)
        return -999;

      const MCParticle* mcROE;
      if (mcDaughters[0]->getArrayIndex() == mcParticle->getArrayIndex())
        mcROE = mcDaughters[1];
      else
        mcROE = mcDaughters[0];

      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      // Load all Tracks and ECLClusters
      const std::vector<Track*> roeTracks = roe->getTracks();
      const std::vector<ECLCluster*> roeECL = roe->getECLClusters();

      // Create artificial particle
      Particle p(mcROE->get4Vector(), mcROE->getPDG());
      Particle* newPart = particles.appendNew(p);

      // Loop through ROE objects, create particles on ROE side
      for (unsigned iTrack = 0; iTrack < roeTracks.size(); iTrack++) {
        const PIDLikelihood* pid = roeTracks[iTrack]->getRelatedTo<PIDLikelihood>();

        if (!pid) {
          B2ERROR("No PID information for this track!");
          return -999;
        }

        Const::ChargedStable type(abs(pid->getMostLikely().getPDGCode()));
        Particle p(roeTracks[iTrack], type);
        Particle* tempPart = particles.appendNew(p);
        newPart->appendDaughter(tempPart);
      }

      for (unsigned iECL = 0; iECL < roeECL.size(); iECL++) {
        if (roeECL[iECL]->isNeutral()) {
          Particle p(roeECL[iECL]);
          Particle* tempPart = particles.appendNew(p);
          newPart->appendDaughter(tempPart);
        }
      }

      return MCMatching::getMCErrors(newPart, mcROE);
    }

    // ------------------------------------------------------------------------------
    // Below are some functions for ease of usage, they are not a part of variables
    // ------------------------------------------------------------------------------

    TLorentzVector missing4VectorCMS(const Particle* particle, const std::vector<double>& opt)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        TLorentzVector empty;
        return empty;
      }

      if (opt.size() != 1) {
        B2ERROR("Number of arguments should be 1!");
        TLorentzVector empty;
        return empty;
      }

      double definition = opt[0];

      PCmsLabTransform T;
      TLorentzVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
      TLorentzVector roe4vec = T.rotateLabToCms() * roe->getROE4Vector();

      TLorentzVector miss4vec;
      double E_beam_cms = T.getCMSEnergy() / 2.0;

      // Definition 1:
      if (definition == 0) {
        miss4vec.SetVect(- (sig4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(2 * E_beam_cms - (sig4vec.Energy() + roe4vec.Energy()));
      }

      // Definition 2:
      else if (definition == 1) {
        miss4vec.SetVect(- sig4vec.Vect());
        miss4vec.SetE(E_beam_cms - sig4vec.Energy());
      }

      return miss4vec;
    }

    TLorentzVector neutrino4Vector(const Particle* particle)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        TLorentzVector empty;
        return empty;
      }

      PCmsLabTransform T;
      TLorentzVector sig4vec = particle->get4Vector();
      TLorentzVector roe4vec = roe->getROE4Vector();

      TLorentzVector neutrino4vec;

      neutrino4vec.SetVect((-1) * (sig4vec.Vect() + roe4vec.Vect()));
      neutrino4vec.SetE(neutrino4vec.Vect().Mag());

      return neutrino4vec;
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

    REGISTER_VARIABLE("ROE_deltaE", ROEDeltaE,
                      "Returns energy difference of the related RestOfEvent object with respect to E_cms/2.");

    REGISTER_VARIABLE("ROE_Mbc", ROEMbc,
                      "Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2.");

    REGISTER_VARIABLE("ROE_MCErrors", ROEMCErrors,
                      "Returns MC Errors for an artificial particle, which corresponds to the ROE object.");

    REGISTER_VARIABLE("correctedDeltaE", correctedDeltaE,
                      "Returns energy difference of the signal side (reconstructed side + neutrino) with respect to E_cms/2.");

    REGISTER_VARIABLE("correctedMbc", correctedMbc,
                      "Returns energy difference of the signal side (reconstructed side + neutrino) with respect to E_cms/2");

    REGISTER_VARIABLE("ECMissingMass(opt)", ECMissingMass,
                      "Returns the missing mass squared. Two definitions exist:"
                      "Option 0: (E)vent based missing mass: calculates the missing 4-momentum based"
                      "					 on all the momentum and energy in the EVENT"
                      "Option 1: (C)andidate based missing mass: calculates the missing 4-momentum based"
                      "          on all the momentum and energy on the RECONSTRUCTED SIDE (signal candidate, p_B_cms is set to 0)");

  }
}
