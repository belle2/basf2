#include <gtest/gtest.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/VariableManager/Utility.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <framework/utilities/TestHelpers.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/Const.h>
#include <TMatrixFSym.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;


namespace Belle2 {
  class TestParticleFactory {
  public:
    TestParticleFactory(): m_photonIndex(0) {};
    ~TestParticleFactory() {};

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

    Particle::EParticleType getType(const DecayDescriptorParticle* particleDescription)
    {
      int pdg = abs(particleDescription->getPDGCode());
      // Emmm, I still do not know how to make it in an elegant way
      if (pdg == 211 || pdg == 11 || pdg == 321) {
        return Particle::EParticleType::c_Track;
      }
      if (pdg == 22) {
        return Particle::EParticleType::c_ECLCluster;
      }
      return Particle::EParticleType::c_Composite;
    }

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

    const Particle* createPhoton(const TLorentzVector& momentum)
    {
      StoreArray<ECLCluster> myECLClusters;
      StoreArray<Particle> myParticles;
      ECLCluster myECL;
      myECL.setIsTrack(false);
      TRandom3 generator;
      float eclREC = momentum[3];
      myECL.setConnectedRegionId(m_photonIndex++);
      myECL.setEnergy(eclREC);
      myECL.setHypothesisId(5);
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

    const Particle* createCharged(const DecayDescriptor* particleDescriptor,  const TLorentzVector& momentum, const TVector3& vertex)
    {
      auto* particleDescription = particleDescriptor->getMother();
      TVector3 tmomentum(momentum[0], momentum[1], momentum[2]);
      const float pValue = 0.5;
      const float bField = 1.5;
      TMatrixDSym cov6(6);
      const int charge = (particleDescription->getPDGCode()) / abs(particleDescription->getPDGCode());
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
    int m_photonIndex;
  };
};
namespace {
  class ROETest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    virtual void SetUp()
    {

      DataStore::Instance().setInitializeActive(true);
      StoreArray<ECLCluster> myECLClusters;
      StoreArray<KLMCluster> myKLMClusters;
      StoreArray<TrackFitResult> myTFRs;
      StoreArray<Track> myTracks;
      StoreArray<Particle> myParticles;
      StoreArray<RestOfEvent> myROEs;
      StoreArray<PIDLikelihood> myPIDLikelihoods;
      myECLClusters.registerInDataStore();
      myKLMClusters.registerInDataStore();
      myTFRs.registerInDataStore();
      myTracks.registerInDataStore();
      myParticles.registerInDataStore();
      myROEs.registerInDataStore();
      myPIDLikelihoods.registerInDataStore();
      myParticles.registerRelationTo(myROEs);
      myTracks.registerRelationTo(myPIDLikelihoods);
      DataStore::Instance().setInitializeActive(false);

      TestParticleFactory factory;
      TVector3 ipposition(0, 0, 0);
      TLorentzVector ksmomentum(1, 0, 0, 3);
      TVector3 ksposition(1.0, 0, 0);
      //Creation of test particles:
      //All daughters and mother particles have the same momenta within a decay
      //In principle, this concept can be better developed if needed
      auto* ksParticle = factory.produceParticle(string("^K_S0 -> ^pi+ ^pi-"), ksmomentum, ksposition);
      TLorentzVector d0momentum(-2, 0, 0, 4);
      auto* d0Particle = factory.produceParticle(string("^D0 -> ^K+ ^pi-"), d0momentum, ipposition);
      TLorentzVector pi0momentum(-0.2, 0, 0, 1);
      auto* pi0Particle = factory.produceParticle(string("^pi0 -> ^gamma ^gamma"), pi0momentum, ipposition);
      TLorentzVector b0momentum(3, 0, 0, 5);
      factory.produceParticle(string("^B0 -> [^K_S0 -> ^pi+ ^pi-] [^pi0 -> ^gamma ^gamma] ^gamma"), b0momentum, ipposition);

      RestOfEvent roe;
      vector<const Particle*> roeParticles;
      roeParticles.push_back(ksParticle->getDaughter(0));
      roeParticles.push_back(ksParticle->getDaughter(1));
      roeParticles.push_back(d0Particle->getDaughter(0));
      roeParticles.push_back(d0Particle->getDaughter(1));
      roeParticles.push_back(pi0Particle->getDaughter(0));
      roeParticles.push_back(pi0Particle->getDaughter(1));
      roe.addParticles(roeParticles);
      roe.initializeMask("cutMask", "TestModule");
      //Exclude K_S0 pions
      std::shared_ptr<Variable::Cut> trackSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 1.5"));
      //Exclude pi0 gammas
      std::shared_ptr<Variable::Cut> eclSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 1"));
      roe.updateMaskWithCuts("cutMask", trackSelection, eclSelection);
      roe.initializeMask("excludeMask", "TestModule");
      vector<const Particle*> excludeParticles = {ksParticle->getDaughter(1), d0Particle->getDaughter(0)};
      roe.excludeParticlesFromMask("excludeMask", excludeParticles, Particle::EParticleType::c_Track, true);
      roe.initializeMask("keepMask", "TestModule");
      roe.excludeParticlesFromMask("keepMask", excludeParticles, Particle::EParticleType::c_Track, false);
      roe.initializeMask("V0Mask", "TestModule");
      roe.updateMaskWithCuts("V0Mask"); // No selection
      //Add V0 to ROE mask:
      roe.updateMaskWithV0("V0Mask", ksParticle);
      myROEs.appendNew(roe);
      roe.print();
    }

    /** clear datastore */
    virtual void TearDown()
    {
      DataStore::Instance().reset();
    }
  };

  TEST_F(ROETest, hasParticle)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    const RestOfEvent* roe = myROEs[0];
    EXPECT_TRUE(roe->hasParticle(myParticles[0]));  // K_S0_pi0
    EXPECT_TRUE(roe->hasParticle(myParticles[1]));  // K_S0_pi1
    EXPECT_FALSE(roe->hasParticle(myParticles[2])); // K_S0
    EXPECT_TRUE(roe->hasParticle(myParticles[3]));  // D0_K
    EXPECT_TRUE(roe->hasParticle(myParticles[4]));  // D0_pi
    EXPECT_FALSE(roe->hasParticle(myParticles[5])); // D0
    EXPECT_TRUE(roe->hasParticle(myParticles[6]));  // pi0_gamma0
    EXPECT_TRUE(roe->hasParticle(myParticles[7]));  // pi0_gamma1
    EXPECT_FALSE(roe->hasParticle(myParticles[8])); // pi0
    EXPECT_FALSE(roe->hasParticle(myParticles[9])); // B0_K_S0_pi0
    EXPECT_FALSE(roe->hasParticle(myParticles[10])); // B0_K_S0_pi1
    EXPECT_FALSE(roe->hasParticle(myParticles[11])); // B0_pi0_gamma0
  }

  TEST_F(ROETest, updateMaskWithCuts)
  {
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    const RestOfEvent* roe = myROEs[0];

    EXPECT_FALSE(roe->hasParticle(myParticles[0], "cutMask")); // K_S0_pi0
    EXPECT_FALSE(roe->hasParticle(myParticles[1], "cutMask")); // K_S0_pi0
    EXPECT_TRUE(roe->hasParticle(myParticles[3], "cutMask")); // D0_K
    EXPECT_TRUE(roe->hasParticle(myParticles[4], "cutMask")); // D0_pi
    EXPECT_FALSE(roe->hasParticle(myParticles[6], "cutMask")); // pi0_gamma0
    EXPECT_FALSE(roe->hasParticle(myParticles[7], "cutMask")); // pi0_gamma1
  }
  TEST_F(ROETest, excludeParticlesFromMask)
  {
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    const RestOfEvent* roe = myROEs[0];

    EXPECT_TRUE(roe->hasParticle(myParticles[0], "excludeMask")); // K_S0_pi0
    EXPECT_FALSE(roe->hasParticle(myParticles[1], "excludeMask")); // K_S0_pi0
    EXPECT_FALSE(roe->hasParticle(myParticles[3], "excludeMask")); // D0_K
    EXPECT_TRUE(roe->hasParticle(myParticles[4], "excludeMask")); // D0_pi
    EXPECT_TRUE(roe->hasParticle(myParticles[6], "excludeMask")); // pi0_gamma0
    EXPECT_TRUE(roe->hasParticle(myParticles[7], "excludeMask")); // pi0_gamma1
    // Inverted result with "!"
    EXPECT_TRUE(!roe->hasParticle(myParticles[0], "keepMask")); // K_S0_pi0
    EXPECT_FALSE(!roe->hasParticle(myParticles[1], "keepMask")); // K_S0_pi0
    EXPECT_FALSE(!roe->hasParticle(myParticles[3], "keepMask")); // D0_K
    EXPECT_TRUE(!roe->hasParticle(myParticles[4], "keepMask")); // D0_pi
    // Photons not touched:
    EXPECT_TRUE(roe->hasParticle(myParticles[6], "keepMask")); // pi0_gamma0
    EXPECT_TRUE(roe->hasParticle(myParticles[7], "keepMask")); // pi0_gamma1
  }
  TEST_F(ROETest, updateMaskWithV0)
  {
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    const RestOfEvent* roe = myROEs[0];
    // Has particle checks for daughters
    EXPECT_TRUE(roe->hasParticle(myParticles[0], "V0Mask")); // K_S0_pi0
    EXPECT_TRUE(roe->hasParticle(myParticles[1], "V0Mask")); // K_S0_pi0
    EXPECT_TRUE(roe->hasParticle(myParticles[3], "V0Mask")); // D0_K
    EXPECT_TRUE(roe->hasParticle(myParticles[4], "V0Mask")); // D0_pi
    EXPECT_TRUE(roe->hasParticle(myParticles[6], "V0Mask")); // pi0_gamma0
    EXPECT_TRUE(roe->hasParticle(myParticles[7], "V0Mask")); // pi0_gamma1
    //Get all particles, including the K_S0 in the mask:
    auto v0maskParticles = roe->getParticles("V0Mask", false);
    //Get all particles, but substitute K_S0 FS daughters:
    auto v0maskParticlesUnpacked = roe->getParticles("V0Mask", true);
    B2INFO("packed size is " << v0maskParticles.size());
    for (auto* particle : v0maskParticles) {
      B2INFO("My pdg: " << particle->getPDGCode());
    }
    B2INFO("unpacked size is " << v0maskParticlesUnpacked.size());
    for (auto* particle : v0maskParticlesUnpacked) {
      B2INFO("My pdg: " << particle->getPDGCode());
    }
    EXPECT_FLOAT_EQ(v0maskParticles.size() , 5);
    EXPECT_FLOAT_EQ(v0maskParticlesUnpacked.size() , 6);
  }
} //
