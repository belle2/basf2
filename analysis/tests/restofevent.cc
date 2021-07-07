/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>
#include "utilities/TestParticleFactory.h"
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <framework/gearbox/Gearbox.h>
#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;
namespace {
  class ROETest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
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

      TestUtilities::TestParticleFactory factory;
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
      roe.excludeParticlesFromMask("excludeMask", excludeParticles, Particle::EParticleSourceObject::c_Track, true);
      roe.initializeMask("keepMask", "TestModule");
      roe.excludeParticlesFromMask("keepMask", excludeParticles, Particle::EParticleSourceObject::c_Track, false);
      roe.initializeMask("V0Mask", "TestModule");
      roe.updateMaskWithCuts("V0Mask"); // No selection
      //Add V0 to ROE mask:
      roe.updateMaskWithV0("V0Mask", ksParticle);
      myROEs.appendNew(roe);
      roe.print();
    }

    /** clear datastore */
    void TearDown() override
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
    StoreArray<RestOfEvent> myROEs{};
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

  TEST_F(ROETest, useROERecoilFrame)
  {
    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);

    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    StoreObjPtr<RestOfEvent> myROEObject;
    DataStore::Instance().setInitializeActive(true);
    myROEObject.registerInDataStore(DataStore::c_DontWriteOut);
    DataStore::Instance().setInitializeActive(false);
    myParticles[14]->addRelationTo(myROEs[0]);                 // Add relation to B0
    myROEObject.assign(myROEs[0]);

    PCmsLabTransform T;
    // Recoil vector against all ROE particles
    TLorentzVector pRecoil = T.getBeamFourMomentum() - myROEs[0]->get4Vector();
    Particle tmp(pRecoil, 0);
    RestFrame frame(&tmp);
    //std::cout << "HER: " << T.getBeamParams().getHER()[0] << " LER: " << T.getBeamParams().getLER()[0] << std::endl;

    const Manager::Var* var = Manager::Instance().getVariable("useROERecoilFrame(p)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(myParticles[5]), frame.getMomentum(myParticles[5]->get4Vector()).P()); // test on D0 in ROE
    EXPECT_FLOAT_EQ(var->function(myParticles[14]), frame.getMomentum(myParticles[14]->get4Vector()).P()); // test on B0 on signal side
    var = Manager::Instance().getVariable("useROERecoilFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(myParticles[5]), frame.getMomentum(myParticles[5]->get4Vector()).E()); // test on D0 in ROE
    EXPECT_FLOAT_EQ(var->function(myParticles[14]), frame.getMomentum(myParticles[14]->get4Vector()).E()); // test on B0 on signal side

    DataStore::Instance().setInitializeActive(true);
    DataStore::Instance().getEntry(myROEObject)->object = nullptr;
    DataStore::Instance().setInitializeActive(false);
  }

  TEST_F(ROETest, getParticles)
  {
    StoreArray<RestOfEvent> myROEs{};
    const RestOfEvent* roe = myROEs[0];

    EXPECT_TRUE(roe->getParticles().size() == 6);
    EXPECT_TRUE(roe->getPhotons().size() == 2);
    EXPECT_TRUE(roe->getHadrons().size() == 0);
    EXPECT_TRUE(roe->getChargedParticles().size() == 4);
    EXPECT_TRUE(roe->getChargedParticles("", 321).size() == 1);
    EXPECT_TRUE(roe->getChargedParticles("", 211).size() == 3);
  }

  TEST_F(ROETest, updateMaskWithCuts)
  {
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs{};
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
    StoreArray<RestOfEvent> myROEs{};
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
    StoreArray<RestOfEvent> myROEs{};
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
