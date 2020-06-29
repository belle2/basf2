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
      DataStore::Instance().setInitializeActive(true);
      pi0ParticleList.registerInDataStore(DataStore::c_DontWriteOut);
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
      // Add mask, which should have 1 gamma, 1 pi+, 1 K_L0
      savedROE->updateMaskWithCuts("my_mask",  chargedSelection,  photonSelection);
      savedROE->print("my_mask");

      roeobjptr.assign(savedROE->Clone());
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
    StoreArray<Particle> myParticles;
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

  }
  /*
   * Test ROE track/cluster composition variables
   */
  TEST_F(ROEVariablesTest, ROETrackClusterCompositionVariables)
  {
    StoreArray<Particle> myParticles;
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
    EXPECT_FLOAT_EQ(var->function(part), roe4VecCMS.Vect().Mag());

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


  }
  /*
   * Test isInROE variables
   */
  TEST_F(ROEVariablesTest, IsInROEVariables)
  {
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;

    StoreObjPtr<RestOfEvent> roeobjptr;
    B2INFO("Ptr: " << roeobjptr << " valid: " << roeobjptr.isValid());

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

  }

  TEST_F(ROEVariablesTest, ROEVariablesOld)
  {
    //Gearbox& gearbox = Gearbox::getInstance();
    //gearbox.setBackends({std::string("file:")});
    //gearbox.close();
    //gearbox.open("geometry/Belle2.xml", false);
    StoreObjPtr<ParticleList> pi0ParticleList("pi0:vartest");
    StoreArray<ECLCluster> myECLClusters;
    StoreArray<KLMCluster> myKLMClusters;
    StoreArray<TrackFitResult> myTFRs;
    StoreArray<Track> myTracks;
    StoreArray<Particle> myParticles;
    StoreArray<RestOfEvent> myROEs;
    StoreArray<PIDLikelihood> myPIDLikelihoods;

    pi0ParticleList.create();
    pi0ParticleList->initialize(111, "pi0:vartest");

    // Neutral ECLCluster on reconstructed side
    ECLCluster myECL;
    myECL.setIsTrack(false);
    float eclREC = 0.5;
    myECL.setEnergy(eclREC);
    myECL.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    ECLCluster* savedECL = myECLClusters.appendNew(myECL);

    // Particle on reconstructed side from ECLCluster
    Particle p(savedECL);
    Particle* part = myParticles.appendNew(p);

    // Create ECLCluster on ROE side
    ECLCluster myROEECL;
    myROEECL.setIsTrack(false);
    float eclROE = 1.0;
    myROEECL.setEnergy(eclROE);
    myROEECL.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    ECLCluster* savedROEECL = myECLClusters.appendNew(myROEECL);
    Particle* roeECLParticle = myParticles.appendNew(savedROEECL);
    // Create KLMCluster on ROE side
    KLMCluster myROEKLM;
    KLMCluster* savedROEKLM = myKLMClusters.appendNew(myROEKLM);
    Particle* roeKLMParticle = myParticles.appendNew(savedROEKLM);

    // Create Track on ROE side
    // - create TFR

    const float pValue = 0.5;
    const float bField = 1.5;
    const int charge = 1;
    TMatrixDSym cov6(6);

    TVector3 position(1.0, 0, 0);
    TVector3 momentum(0, 1.0, 0);

    auto CDCValue = static_cast<unsigned long long int>(0x300000000000000);

    myTFRs.appendNew(position, momentum, cov6, charge, Const::muon, pValue, bField, CDCValue, 16777215);

    // - create Track
    Track myROETrack;
    myROETrack.setTrackFitResultIndex(Const::muon, 0);
    Track* savedROETrack = myTracks.appendNew(myROETrack);
    // - create PID information, add relation
    PIDLikelihood myPID;
    myPID.setLogLikelihood(Const::TOP, Const::muon, 0.15);
    myPID.setLogLikelihood(Const::ARICH, Const::muon, 0.152);
    myPID.setLogLikelihood(Const::ECL, Const::muon, 0.154);
    myPID.setLogLikelihood(Const::CDC, Const::muon, 0.156);
    myPID.setLogLikelihood(Const::SVD, Const::muon, 0.158);
    myPID.setLogLikelihood(Const::TOP, Const::pion, 0.5);
    myPID.setLogLikelihood(Const::ARICH, Const::pion, 0.52);
    myPID.setLogLikelihood(Const::ECL, Const::pion, 0.54);
    myPID.setLogLikelihood(Const::CDC, Const::pion, 0.56);
    myPID.setLogLikelihood(Const::SVD, Const::pion, 0.58);
    PIDLikelihood* savedPID = myPIDLikelihoods.appendNew(myPID);

    savedROETrack->addRelationTo(savedPID);
    Particle* roeTrackParticle = myParticles.appendNew(savedROETrack, Const::muon);

    // Create ROE object, append tracks, clusters, add relation to particle
    //TODO: make particles
    RestOfEvent roe;
    vector<const Particle*> roeParticlesToAdd;
    roeParticlesToAdd.push_back(roeTrackParticle);
    roeParticlesToAdd.push_back(roeECLParticle);
    roeParticlesToAdd.push_back(roeKLMParticle);
    roe.addParticles(roeParticlesToAdd);
    RestOfEvent* savedROE = myROEs.appendNew(roe);
    /*
    std::map<std::string, std::map<unsigned int, bool>> tMasks;
    std::map<std::string, std::map<unsigned int, bool>> cMasks;
    std::map<std::string, std::vector<double>> fracs;

    std::map<unsigned int, bool> tMask1;
    std::map<unsigned int, bool> tMask2;
    tMask1[savedROETrack->getArrayIndex()] = true;
    tMask2[savedROETrack->getArrayIndex()] = false;

    std::map<unsigned int, bool> cMask1;
    std::map<unsigned int, bool> cMask2;
    cMask1[savedROEECL->getArrayIndex()] = true;
    cMask2[savedROEECL->getArrayIndex()] = false;

    std::vector<double> frac1 = {0, 0, 1, 0, 0, 0};
    std::vector<double> frac2 = {1, 1, 1, 1, 1, 1};

    tMasks["mask1"] = tMask1;
    tMasks["mask2"] = tMask2;

    cMasks["mask1"] = cMask1;
    cMasks["mask2"] = cMask2;

    fracs["mask1"] = frac1;
    fracs["mask2"] = frac2;

    savedROE->appendTrackMasks(tMasks);
    savedROE->appendECLClusterMasks(cMasks);
    savedROE->appendChargedStableFractionsSet(fracs);
    */
    savedROE->initializeMask("mask1", "test");
    std::shared_ptr<Variable::Cut> trackSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 2"));
    std::shared_ptr<Variable::Cut> eclSelection = std::shared_ptr<Variable::Cut>(Variable::Cut::compile("p > 2"));
    savedROE->updateMaskWithCuts("mask1");
    savedROE->initializeMask("mask2", "test");
    savedROE->updateMaskWithCuts("mask2",  trackSelection,  eclSelection);
    part->addRelationTo(savedROE);

    // ROE variables
    PCmsLabTransform T;
    float E0 = T.getCMSEnergy() / 2;
    B2INFO("E0 is " << E0);
    //*/
    TLorentzVector pTrack_ROE_Lab(momentum, TMath::Sqrt(Const::muon.getMass()*Const::muon.getMass() + 1.0 /*momentum.Mag2()*/));
    pTrack_ROE_Lab = roeTrackParticle->get4Vector();
    TLorentzVector pECL_ROE_Lab(0, 0, eclROE, eclROE);
    TLorentzVector pECL_REC_Lab(0, 0, eclREC, eclREC);

    TLorentzVector rec4vec;
    rec4vec.SetE(pECL_REC_Lab.E());
    rec4vec.SetVect(pECL_REC_Lab.Vect());

    TLorentzVector roe4vec;
    roe4vec.SetE(pTrack_ROE_Lab.E() + pECL_ROE_Lab.E());
    roe4vec.SetVect(pTrack_ROE_Lab.Vect() + pECL_ROE_Lab.Vect());

    TLorentzVector rec4vecCMS = T.rotateLabToCms() * rec4vec;
    TLorentzVector roe4vecCMS = T.rotateLabToCms() * roe4vec;

    TVector3 pB = - roe4vecCMS.Vect();
    pB.SetMag(0.340);

    TLorentzVector m4v0;
    m4v0.SetE(2 * E0 - (rec4vecCMS.E() + roe4vecCMS.E()));
    m4v0.SetVect(- (rec4vecCMS.Vect() + roe4vecCMS.Vect()));

    TLorentzVector m4v1;
    m4v1.SetE(E0 - rec4vecCMS.E());
    m4v1.SetVect(- (rec4vecCMS.Vect() + roe4vecCMS.Vect()));

    TLorentzVector m4v2;
    m4v2.SetE(E0 - rec4vecCMS.E());
    m4v2.SetVect(- rec4vecCMS.Vect());

    TLorentzVector m4v3;
    m4v3.SetE(E0 - rec4vecCMS.E());
    m4v3.SetVect(pB - rec4vecCMS.Vect());

    TLorentzVector neutrino4vecCMS;
    neutrino4vecCMS.SetVect(- (roe4vecCMS.Vect() + rec4vecCMS.Vect()));
    neutrino4vecCMS.SetE(neutrino4vecCMS.Vect().Mag());

    TLorentzVector corrRec4vecCMS = rec4vecCMS + neutrino4vecCMS;
    B2INFO("roe4vecCMS.E() = " << roe4vecCMS.E());
    // TESTS FOR ROE STRUCTURE
    //EXPECT_B2FATAL(savedROE->getTrackMask("noSuchMask"));
    //EXPECT_B2FATAL(savedROE->getECLClusterMask("noSuchMask"));
    //double fArray[6];
    //EXPECT_B2FATAL(savedROE->fillFractions(fArray, "noSuchMask"));
    EXPECT_B2FATAL(savedROE->updateMaskWithCuts("noSuchMask"));
    EXPECT_B2FATAL(savedROE->updateMaskWithV0("noSuchMask", part));
    EXPECT_B2FATAL(savedROE->hasParticle(part, "noSuchMask"));

    // TESTS FOR ROE VARIABLES

    const Manager::Var* var = Manager::Instance().getVariable("nROE_Charged(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask1, 13)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Charged(mask1, 211)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_Photons(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Photons(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_NeutralHadrons(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Tracks(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_Tracks(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_ECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("nROE_NeutralECLClusters(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("nROE_ParticlesInList(pi0:vartest)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("roeCharge(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 1.0);

    var = Manager::Instance().getVariable("roeCharge(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("roeEextra(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), savedROEECL->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons));

    var = Manager::Instance().getVariable("roeEextra(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), 0.0);

    var = Manager::Instance().getVariable("roeDeltae(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), roe4vecCMS.E() - E0);

    var = Manager::Instance().getVariable("roeDeltae(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), -E0);

    var = Manager::Instance().getVariable("roeMbc(mask1)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - roe4vecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("roeMbc(mask2)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), E0);

    var = Manager::Instance().getVariable("weDeltae(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), corrRec4vecCMS.E() - E0);

    var = Manager::Instance().getVariable("weDeltae(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), rec4vecCMS.E() + rec4vecCMS.Vect().Mag() - E0);

    var = Manager::Instance().getVariable("weMbc(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), TMath::Sqrt(E0 * E0 - corrRec4vecCMS.Vect().Mag2()));

    var = Manager::Instance().getVariable("weMbc(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), E0);

    var = Manager::Instance().getVariable("weMissM2(mask1,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), m4v0.Mag2());

    var = Manager::Instance().getVariable("weMissM2(mask2,0)");
    ASSERT_NE(var, nullptr);
    EXPECT_FLOAT_EQ(var->function(part), (2 * E0 - rec4vecCMS.E()) * (2 * E0 - rec4vecCMS.E()) - rec4vecCMS.Vect().Mag2());

  }


}
