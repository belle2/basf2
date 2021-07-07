/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>
#include "utilities/TestParticleFactory.h"

#include <analysis/VariableManager/Manager.h>
#include <analysis/variables/SpecificKinematicVariables.h>
#include <analysis/variables/Variables.h>
#include <analysis/variables/ROEVariables.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/utilities/TestHelpers.h>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {
  class ROEVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {

      StoreObjPtr<ParticleList> pi0ParticleList("pi0:vartest");
      StoreObjPtr<ParticleList> b0ParticleList("B0:vartest");
      DataStore::Instance().setInitializeActive(true);
      pi0ParticleList.registerInDataStore(DataStore::c_DontWriteOut);
      b0ParticleList.registerInDataStore(DataStore::c_DontWriteOut);
      StoreArray<ECLCluster> myECLClusters;
      StoreArray<KLMCluster> myKLMClusters;
      StoreArray<TrackFitResult> myTFRs;
      StoreArray<Track> myTracks;
      StoreArray<Particle> myParticles;
      StoreArray<RestOfEvent> myROEs;
      StoreObjPtr<RestOfEvent> roeobjptr;
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
      roeobjptr.registerInDataStore("RestOfEvent", DataStore::c_DontWriteOut);
      DataStore::Instance().setInitializeActive(false);

      pi0ParticleList.create();
      pi0ParticleList->initialize(111, "pi0:vartest");
      b0ParticleList.create();
      b0ParticleList->initialize(521, "B0:vartest");

      PCmsLabTransform T;

      TestUtilities::TestParticleFactory factory;
      TVector3 ipposition(0, 0, 0);
      double halfEcms = T.getCMSEnergy() / 2;

      TLorentzVector e_momentum(0., 0,  halfEcms / 2,  halfEcms / 2);
      e_momentum = T.rotateCmsToLab() * e_momentum;
      TLorentzVector p_momentum(0., 0, -halfEcms / 2,  halfEcms / 2);
      p_momentum = T.rotateCmsToLab() * p_momentum;

      TLorentzVector b0_momentum(0, 0, 0, halfEcms);
      b0_momentum = T.rotateCmsToLab() * b0_momentum;
      factory.produceParticle(string("^B0 -> e- e+"), b0_momentum, ipposition);

      myParticles[0]->set4Vector(e_momentum);
      myParticles[1]->set4Vector(p_momentum);

      myParticles[0]->print(); // e-
      TLorentzVector fsp1_momentum(0., 0, halfEcms / 4, halfEcms / 4);
      fsp1_momentum = T.rotateCmsToLab() * fsp1_momentum;
      TLorentzVector fsp2_momentum(0., 0, -halfEcms / 4, halfEcms / 4);
      fsp2_momentum = T.rotateCmsToLab() * fsp2_momentum;
      TLorentzVector kl_momentum(0., 0, 0.1, 0.5);
      kl_momentum = T.rotateCmsToLab() * fsp2_momentum;
      factory.produceParticle(string("^B0 -> [pi0 -> gamma gamma] [K_S0 -> pi+ pi-]"), b0_momentum, ipposition);
      KLMCluster myROEKLM;
      KLMCluster* savedROEKLM = myKLMClusters.appendNew(myROEKLM);
      Particle* roeKLMParticle = myParticles.appendNew(savedROEKLM);

      // Set momentum of daughters
      myParticles[3]->set4Vector(fsp1_momentum); // gamma
      myParticles[4]->set4Vector(fsp2_momentum); // gamma
      myECLClusters[0]->setEnergy(fsp1_momentum.E()); // gamma
      myECLClusters[1]->setEnergy(fsp2_momentum.E()); // gamma
      myParticles[6]->set4Vector(fsp1_momentum); // pi+
      myParticles[7]->set4Vector(fsp2_momentum); // pi-
      myParticles[3]->print(); // gamma
      myParticles[4]->print(); // gamma
      myParticles[7]->set4Vector(kl_momentum); // K_L0
      myParticles[7]->print(); // K_L0
      // Create ROE
      RestOfEvent roe;
      vector<const Particle*> roeParticlesToAdd;
      // Add particles to the ROE
      roeParticlesToAdd.push_back(myParticles[3]);
      roeParticlesToAdd.push_back(myParticles[4]);
      roeParticlesToAdd.push_back(myParticles[6]);
      roeParticlesToAdd.push_back(myParticles[7]);
      roeParticlesToAdd.push_back(roeKLMParticle);
      roe.addParticles(roeParticlesToAdd);
      RestOfEvent* savedROE = myROEs.appendNew(roe);
      // Add relation ROE - particle (first B)
      myParticles[2]->addRelationTo(savedROE); // B0
      savedROE->print();
      savedROE->initializeMask("my_mask", "test");
      std::shared_ptr<Variable::Cut> chargedSelection = std::shared_ptr<Variable::Cut>
                                                        (Variable::Cut::compile("charge > 0")); // - exclude pi
      std::shared_ptr<Variable::Cut> photonSelection = std::shared_ptr<Variable::Cut>
                                                       (Variable::Cut::compile("p > 1.5")); // - exclude gamma
      std::shared_ptr<Variable::Cut> klSelection = std::shared_ptr<Variable::Cut>
                                                   (Variable::Cut::compile("E < 0")); // - exclude Klong
      // Add mask, which should have 1 gamma, 1 pi+, 0 K_L0
      savedROE->updateMaskWithCuts("my_mask",  chargedSelection,  photonSelection, klSelection);
      savedROE->print("my_mask");
      // Add pi0 from ROE to particle list
      pi0ParticleList->addParticle(5, 111, Particle::c_Unflavored);
      b0ParticleList->addParticle(2, 521, Particle::c_Unflavored);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  /*
   * Test ROE particle composition variables
   */
  TEST_F(ROEVariablesTest, ROEParticleCompositionVariables)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[2];  // B0
    auto* var = Manager::Instance().getVariable("nROE_Charged()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 2.0);

    var = Manager::Instance().getVariable("nROE_Charged(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Charged(my_mask, 13)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_Charged(my_mask, 211)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Photons()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 2.0);

    var = Manager::Instance().getVariable("nROE_Photons(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_NeutralHadrons()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_NeutralHadrons(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

  }
  /*
   * Test ROE track/cluster composition variables
   */
  TEST_F(ROEVariablesTest, ROETrackClusterCompositionVariables)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[2];  // B0

    auto* var = Manager::Instance().getVariable("nROE_Tracks()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 2.0);

    var = Manager::Instance().getVariable("nROE_Tracks(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 2.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 2.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_KLMClusters");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);
  }
  /*
   * Test ROE recoil frame variable
   */
  TEST_F(ROEVariablesTest, ROERecoilFrameVariable)
  {
    StoreArray<RestOfEvent> myROEs{};
    StoreArray<Particle> myParticles{};
    auto part = myParticles[2];  // B0
    auto partNotROE = myParticles[0];  // electron has no ROE

    auto* var = Manager::Instance().getVariable("useROERecoilFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 5.2959199);

    DataStore::StoreEntry& roeobjptr = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at("RestOfEvent");
    roeobjptr.object = myROEs[0];
    roeobjptr.ptr = myROEs[0];

    var = Manager::Instance().getVariable("useROERecoilFrame(E)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partNotROE), 2.8002837);

    // Clear ptr at the end
    roeobjptr.object = nullptr;
    roeobjptr.ptr = nullptr;
  }
  /*
   * Test ROE kinematics variables
   */
  TEST_F(ROEVariablesTest, ROEKinematicsVariables)
  {
    StoreArray<Particle> myParticles;
    auto part = myParticles[2];  // B0

    // Tag side 4 vector
    TLorentzVector roe4Vec(0, 0, 0, 0);
    roe4Vec += myParticles[3]->get4Vector();
    roe4Vec += myParticles[4]->get4Vector();
    roe4Vec += myParticles[6]->get4Vector();
    roe4Vec += myParticles[7]->get4Vector();
    // Tag side 4 vector in mask
    TLorentzVector mask4Vec(0, 0, 0, 0);
    mask4Vec += myParticles[3]->get4Vector();
    mask4Vec += myParticles[6]->get4Vector();
    // Signal side 4 vector
    TLorentzVector  sig4Vec = part->get4Vector();

    PCmsLabTransform T;
    double E0 = T.getCMSEnergy() / 2;
    auto roe4VecCMS = T.rotateLabToCms() * roe4Vec;
    auto mask4VecCMS = T.rotateLabToCms() * mask4Vec;
    auto sig4VecCMS = T.rotateLabToCms() * sig4Vec;

    auto* var = Manager::Instance().getVariable("roeCharge()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("roeCharge(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("roeEextra()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), myParticles[3]->getEnergy() + myParticles[4]->getEnergy());

    var = Manager::Instance().getVariable("roeE()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4Vec.E());

    var = Manager::Instance().getVariable("useCMSFrame(roeE())");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4VecCMS.E());

    var = Manager::Instance().getVariable("roeM()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4Vec.Mag());

    var = Manager::Instance().getVariable("roeP()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4Vec.P());

    var = Manager::Instance().getVariable("useCMSFrame(roeP())");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4VecCMS.P());

    var = Manager::Instance().getVariable("roePTheta()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4Vec.Theta());

    var = Manager::Instance().getVariable("useCMSFrame(roePTheta())");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4VecCMS.Theta());

    var = Manager::Instance().getVariable("roeDeltae()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4VecCMS.E() - E0);

    var = Manager::Instance().getVariable("roeDeltae(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), mask4VecCMS.E() - E0);

    var = Manager::Instance().getVariable("roeMbc()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - roe4VecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("roeMbc(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - mask4VecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("weDeltae(my_mask,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), mask4VecCMS.E() + sig4VecCMS.E() - E0);

    var = Manager::Instance().getVariable("weMbc(my_mask,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - mask4VecCMS.Vect().Mag2()));

    TLorentzVector miss4VecCMS(0, 0, 0, 0);
    miss4VecCMS.SetVect(- (sig4VecCMS.Vect() + mask4VecCMS.Vect()));
    miss4VecCMS.SetE(2 * E0 - (sig4VecCMS.E() + mask4VecCMS.E()));

    var = Manager::Instance().getVariable("weMissM2(my_mask,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), miss4VecCMS.Mag2());

    var = Manager::Instance().getVariable("weMissP(my_mask,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), miss4VecCMS.P());

    var = Manager::Instance().getVariable("weMissE(my_mask,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), miss4VecCMS.E());

  }
  /*
   * Test specific kinematic variables
   */
  TEST_F(ROEVariablesTest, ROESpecificKinematicVariables)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[2];  // B0
    // Signal side 4 vector
    TLorentzVector  sig4Vec = part->get4Vector();

    auto* var = Manager::Instance().getVariable("bssMassDifference()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), sig4Vec.M());

    var = Manager::Instance().getVariable("weCosThetaEll()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), -0.99858648);

    var = Manager::Instance().getVariable("weXiZ()");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.31121328);

    var = Manager::Instance().getVariable("weQ2lnuSimple(my_mask,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), -2.1852231e-06);

    // FIXME: This value is the same as for weQ2lnuSimple
    // More complicated test setup is required to pass abs(cos_angle_nu) < 1
    var = Manager::Instance().getVariable("weQ2lnu(my_mask,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), -2.1852231e-06);
  }
  /*
   * Test isInROE variables
   */
  TEST_F(ROEVariablesTest, IsInROEVariables)
  {
    StoreArray<Particle> myParticles{};
    auto part = myParticles[2];  // B0
    StoreArray<RestOfEvent> myROEs{};

    DataStore::StoreEntry& roeobjptr = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at("RestOfEvent");
    roeobjptr.object = myROEs[0];
    roeobjptr.ptr = myROEs[0];

    auto partROE1 = myParticles[3];  // gamma from ROE
    auto partROE2 = myParticles[5];  // pi from ROE
    auto partROE3 = myParticles[10];   // K_L0 from ROE
    auto partROEnotFromMask = myParticles[7];  // pi from ROE not from mask
    partROEnotFromMask->print(); // gamma

    auto partNotROE = myParticles[0];  // e NOT from ROE

    auto* var = Manager::Instance().getVariable("isInRestOfEvent");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROE1), 1.0);

    var = Manager::Instance().getVariable("isInRestOfEvent");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROE2), 1.0);

    var = Manager::Instance().getVariable("isInRestOfEvent");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROE3), 1.0);

    var = Manager::Instance().getVariable("isInRestOfEvent");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partNotROE), 0.0);

    var = Manager::Instance().getVariable("isInRestOfEvent");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partNotROE), 0.0);

    var = Manager::Instance().getVariable("passesROEMask(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROE1), 1.0);

    var = Manager::Instance().getVariable("passesROEMask(my_mask)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROEnotFromMask), 0.0);

    var = Manager::Instance().getVariable("nROE_ParticlesInList(pi0:vartest)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("currentROEIsInList(B0:vartest)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROE1), 1.0);

    var = Manager::Instance().getVariable("currentROEIsInList(pi0:vartest)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROE1), 0.0);

    var = Manager::Instance().getVariable("particleRelatedToCurrentROE(PDG)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(partROE1), 511.0);

    // Clear ptr at the end
    roeobjptr.object = nullptr;
    roeobjptr.ptr = nullptr;
  }
}
