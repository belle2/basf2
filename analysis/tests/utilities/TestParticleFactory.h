#pragma once
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <analysis/VariableManager/Utility.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <TMatrixFSym.h>
#include <TLorentzVector.h>


#include <string>
using namespace std;
using namespace Belle2;
namespace TestUtilities {
  /**
   * This is a class, which generates DataStore particles, according to
   * the provided decay string e.g. "^K_S0 -> ^pi+ ^pi-".
   * All particles are added to StoreArray<Particle>
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
    const Particle* produceParticle(const string& decayString, const TLorentzVector& momentum, const TVector3& vertex)
    {
      StoreArray<Particle> myParticles;
      DecayDescriptor* decaydescriptor = new DecayDescriptor();
      bool isString = decaydescriptor->init(decayString);
      if (!isString) {
        B2INFO("Decay string is not defined: " << decayString);
        return nullptr;
      }
      vector<string> strNames = decaydescriptor->getSelectionNames();
      for (auto& name : strNames) {
        B2INFO("Creation of particle: " << name);
      }
      // Recursive function
      auto* result = createParticle(decaydescriptor, momentum, vertex);
      delete decaydescriptor;
      return result;
    };

    /**
     * Helper method to get EParticleType from PDG code
     */
    Particle::EParticleType getType(const DecayDescriptorParticle* particleDescription)
    {
      int pdg = abs(particleDescription->getPDGCode());
      // Emmm, I still do not know how to make it in an elegant way
      if (pdg == 211 || pdg == 11 || pdg == 321 || pdg == 13 || pdg == 2212) {
        return Particle::EParticleType::c_Track;
      }
      if (pdg == 22) {
        return Particle::EParticleType::c_ECLCluster;
      }
      return Particle::EParticleType::c_Composite;
    }

    /**
     * This method is used for recursion.
     */
    const Particle* createParticle(const DecayDescriptor* particleDescriptor, const TLorentzVector& momentum, const TVector3& vertex)
    {
      Particle::EParticleType type = getType(particleDescriptor->getMother());
      if (type == Particle::EParticleType::c_Track) {
        return createCharged(particleDescriptor, momentum, vertex);
      }
      if (type == Particle::EParticleType::c_ECLCluster) {
        return createPhoton(momentum);
      }
      StoreArray<Particle> myParticles;
      //Create composite particle:
      auto* motherDescriptor = particleDescriptor->getMother();
      B2INFO("Mother PDG: " << motherDescriptor->getPDGCode() << " selected: " << motherDescriptor->isSelected() << " name: " <<
             motherDescriptor->getNameSimple());
      unsigned int nDaughters = particleDescriptor->getNDaughters();
      Particle mother(momentum, motherDescriptor->getPDGCode());
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
    const Particle* createPhoton(const TLorentzVector& momentum)
    {
      StoreArray<ECLCluster> myECLClusters;
      StoreArray<Particle> myParticles;
      ECLCluster myECL;
      myECL.setIsTrack(false);
      //TRandom3 generator;
      float eclREC = momentum[3];
      myECL.setConnectedRegionId(m_photonIndex++);
      myECL.setEnergy(eclREC);
      myECL.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      //This is necessary to avoid isCopyOf == true for ECLClusters:
      myECL.setClusterId(m_photonIndex++);
      ECLCluster* savedECL = myECLClusters.appendNew(myECL);

      Particle p(savedECL);
      Particle* part = myParticles.appendNew(p);
      B2INFO("\tParticle PDG: " << part->getPDGCode() << " charge: " << part->getCharge()  << " momentum: " <<
             part->getMomentumMagnitude() << " index: " << part->getArrayIndex() << " eclindex: " << part->getECLCluster()->getArrayIndex()
             << " theta: " << part->getECLCluster()->getTheta());
      return part;
    }


    /**
     * Creates different charged particles for tests
     * */
    const Particle* createCharged(const DecayDescriptor* particleDescriptor,  const TLorentzVector& momentum, const TVector3& vertex)
    {
      auto* particleDescription = particleDescriptor->getMother();
      TVector3 tmomentum(momentum[0], momentum[1], momentum[2]);
      const float pValue = 0.5;
      const float bField = 1.5;
      TMatrixDSym cov6(6);
      int chargefactor = (particleDescription->getPDGCode() == 11 || particleDescription->getPDGCode() == 13) ? -1 : 1;
      const int charge = (particleDescription->getPDGCode()) / abs(particleDescription->getPDGCode()) * chargefactor;
      unsigned long long int CDCValue = static_cast<unsigned long long int>(0x300000000000000);
      StoreArray<TrackFitResult> myTrackFits;
      StoreArray<Track> myTracks;
      myTrackFits.appendNew(vertex, tmomentum, cov6, charge, Const::ChargedStable(abs(particleDescription->getPDGCode())), pValue, bField,
                            CDCValue, 16777215);
      Track mytrack;
      StoreArray<Particle> myParticles;
      mytrack.setTrackFitResultIndex(Const::ChargedStable(abs(particleDescription->getPDGCode())), myTrackFits.getEntries() - 1);
      Track* savedTrack = myTracks.appendNew(mytrack);
      Particle* part = myParticles.appendNew(savedTrack, Const::ChargedStable(abs(particleDescription->getPDGCode())));
      B2INFO("\tParticle PDG: " << part->getPDGCode() << " charge: " << part->getCharge()  << " charge: " << charge << " momentum: " <<
             part->getMomentumMagnitude() << " index: " << part->getArrayIndex());
      return part;
    }

  private:
    // Used to differentiate photons for one another
    int m_photonIndex;
  };
};

