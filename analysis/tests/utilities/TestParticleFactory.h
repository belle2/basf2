/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <TMatrixFSym.h>
#include <TLorentzVector.h>


#include <string>
namespace TestUtilities {
  /**
   * This is a class, which generates DataStore particles, according to
   * the provided decay string e.g. "^K_S0 -> ^pi+ ^pi-".
   * All particles are added to Belle2::StoreArray<Belle2::Particle>
   * It is primarily used in the ROE tests.
   */
  class TestParticleFactory {
  public:
    TestParticleFactory(): m_photonIndex(0) {};
    ~TestParticleFactory() {};
    /**
     * Main method to produce particles.
     * For simplification, all particles are created with the same momentum and
     * decay vertex. If for example "^K_S0 -> ^pi+ ^pi-" decay string
     * is provided as an argument, the factory will produce two daughter pions
     * with opposite charges first, and then these pions will be used to
     * form mother K_S0 particle which will be returned.
     * The decay string can have any complexity and all PDG codes of allowed
     * and charges  will be respected.
     */
    const Belle2::Particle* produceParticle(const std::string& decayString, const TLorentzVector& momentum, const TVector3& vertex)
    {
      Belle2::DecayDescriptor* decaydescriptor = new Belle2::DecayDescriptor();
      bool isString = decaydescriptor->init(decayString);
      if (!isString) {
        B2INFO("Decay string is not defined: " << decayString);
        delete decaydescriptor;
        return nullptr;
      }
      std::vector<std::string> strNames = decaydescriptor->getSelectionNames();
      for (auto& name : strNames) {
        B2INFO("Creation of particle: " << name);
      }
      // Recursive function
      auto* result = createParticle(decaydescriptor, momentum, vertex);
      delete decaydescriptor;
      return result;
    };

    /**
     * Helper method to get EParticleSourceObject from PDG code
     */
    Belle2::Particle::EParticleSourceObject getType(const Belle2::DecayDescriptorParticle* particleDescription)
    {
      int pdg = abs(particleDescription->getPDGCode());
      if (pdg == Belle2::Const::pion.getPDGCode() || pdg == Belle2::Const::electron.getPDGCode()
          || pdg == Belle2::Const::kaon.getPDGCode() || pdg == Belle2::Const::muon.getPDGCode()
          || pdg == Belle2::Const::proton.getPDGCode()) {
        return Belle2::Particle::EParticleSourceObject::c_Track;
      }
      if (pdg == Belle2::Const::photon.getPDGCode()) {
        return Belle2::Particle::EParticleSourceObject::c_ECLCluster;
      }
      return Belle2::Particle::EParticleSourceObject::c_Composite;
    }

    /**
     * This method is used for recursion.
     */
    const Belle2::Particle* createParticle(const Belle2::DecayDescriptor* particleDescriptor, const TLorentzVector& momentum,
                                           const TVector3& vertex)
    {
      Belle2::Particle::EParticleSourceObject type = getType(particleDescriptor->getMother());
      if (type == Belle2::Particle::EParticleSourceObject::c_Track) {
        return createCharged(particleDescriptor, momentum, vertex);
      }
      if (type == Belle2::Particle::EParticleSourceObject::c_ECLCluster) {
        return createPhoton(momentum);
      }
      Belle2::StoreArray<Belle2::Particle> myParticles;
      //Create composite particle:
      auto* motherDescriptor = particleDescriptor->getMother();
      B2INFO("Mother PDG: " << motherDescriptor->getPDGCode() << " selected: " << motherDescriptor->isSelected() << " name: " <<
             motherDescriptor->getNameSimple());
      unsigned int nDaughters = particleDescriptor->getNDaughters();
      Belle2::Particle mother(momentum, motherDescriptor->getPDGCode());
      for (unsigned int i = 0; i < nDaughters; i++) {
        auto* daughter = particleDescriptor->getDaughter(i)->getMother();
        B2INFO("\tDaughter PDG: " << daughter->getPDGCode() << " selected: " << daughter->isSelected() << " name: " <<
               daughter->getNameSimple());
        auto* daughterParticle = createParticle(particleDescriptor->getDaughter(i), momentum, vertex);
        mother.appendDaughter(daughterParticle);
      }
      auto* result = myParticles.appendNew(mother);
      return result;
    };

    /**
     * Creates different photons for tests
     * */
    const Belle2::Particle* createPhoton(const TLorentzVector& momentum)
    {
      Belle2::StoreArray<Belle2::ECLCluster> myECLClusters;
      Belle2::StoreArray<Belle2::Particle> myParticles;
      Belle2::ECLCluster myECL;
      myECL.setIsTrack(false);
      //TRandom3 generator;
      float eclREC = momentum[3];
      myECL.setConnectedRegionId(m_photonIndex++);
      myECL.setEnergy(eclREC);
      myECL.setHypothesis(Belle2::ECLCluster::EHypothesisBit::c_nPhotons);
      //This is necessary to avoid isCopyOf == true for Belle2::ECLClusters:
      myECL.setClusterId(m_photonIndex++);
      Belle2::ECLCluster* savedECL = myECLClusters.appendNew(myECL);

      Belle2::Particle p(savedECL);
      Belle2::Particle* part = myParticles.appendNew(p);
      B2INFO("\tParticle PDG: " << part->getPDGCode() << " charge: " << part->getCharge()  << " momentum: " <<
             part->getMomentumMagnitude() << " index: " << part->getArrayIndex() << " eclindex: " << part->getECLCluster()->getArrayIndex()
             << " theta: " << part->getECLCluster()->getTheta());
      return part;
    }


    /**
     * Creates different charged particles for tests
     * */
    const Belle2::Particle* createCharged(const Belle2::DecayDescriptor* particleDescriptor,  const TLorentzVector& momentum,
                                          const TVector3& vertex)
    {
      auto* particleDescription = particleDescriptor->getMother();
      TVector3 tmomentum(momentum[0], momentum[1], momentum[2]);
      const float pValue = 0.5;
      const float bField = 1.5;
      TMatrixDSym cov6(6);
      int chargefactor = (abs(particleDescription->getPDGCode()) == Belle2::Const::electron.getPDGCode()
                          || abs(particleDescription->getPDGCode()) == Belle2::Const::muon.getPDGCode()) ? -1 : 1;
      const int charge = (particleDescription->getPDGCode()) / abs(particleDescription->getPDGCode()) * chargefactor;
      unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);
      Belle2::StoreArray<Belle2::TrackFitResult> myTrackFits;
      Belle2::StoreArray<Belle2::Track> myTracks;
      myTrackFits.appendNew(vertex, tmomentum, cov6, charge, Belle2::Const::ChargedStable(abs(particleDescription->getPDGCode())), pValue,
                            bField,
                            CDCValue, 16777215, 0);
      Belle2::Track mytrack;
      Belle2::StoreArray<Belle2::Particle> myParticles;
      mytrack.setTrackFitResultIndex(Belle2::Const::ChargedStable(abs(particleDescription->getPDGCode())), myTrackFits.getEntries() - 1);
      Belle2::Track* savedTrack = myTracks.appendNew(mytrack);
      Belle2::Particle* part = myParticles.appendNew(savedTrack, Belle2::Const::ChargedStable(abs(particleDescription->getPDGCode())));
      B2INFO("\tParticle PDG: " << part->getPDGCode() << " charge: " << part->getCharge()  << " charge: " << charge << " momentum: " <<
             part->getMomentumMagnitude() << " index: " << part->getArrayIndex());
      return part;
    }

  private:
    int m_photonIndex; /**< Used to differentiate photons from one another */
  };
};

