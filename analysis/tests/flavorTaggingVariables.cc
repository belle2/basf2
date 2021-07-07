/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/utility/MCMatching.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <gtest/gtest.h>

#include <TMatrixFSym.h>
#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

namespace {

  class FlavorTaggingVariablesTest : public ::testing::Test {
  protected:
    /** register Particle array + ParticleExtraInfoMap object. */
    void SetUp() override
    {
      DataStore::Instance().setInitializeActive(true);
      StoreArray<ECLCluster> testsECLClusters;
      StoreArray<KLMCluster> testsKLMClusters;
      StoreArray<TrackFitResult> testsTFRs;
      StoreArray<Track> testsTracks;
      StoreArray<Particle> testsParticles;
      StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
      StoreArray<MCParticle> testsMCParticles;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      StoreArray<RestOfEvent> testsROEs;
      StoreArray<PIDLikelihood> testsPIDLikelihoods;
      testsECLClusters.registerInDataStore();
      testsKLMClusters.registerInDataStore();
      testsTFRs.registerInDataStore();
      testsTracks.registerInDataStore();
      testsParticles.registerInDataStore();
      extraInfoMap.registerInDataStore();
      testsMCParticles.registerInDataStore();
      roe.registerInDataStore();
      testsROEs.registerInDataStore();
      testsPIDLikelihoods.registerInDataStore();
      testsParticles.registerRelationTo(testsROEs);
      testsParticles.registerRelationTo(testsMCParticles);
      testsTracks.registerRelationTo(testsPIDLikelihoods);
      testsTracks.registerRelationTo(testsECLClusters);
      testsTracks.registerRelationTo(testsMCParticles);
      testsECLClusters.registerRelationTo(testsTracks);
      DataStore::Instance().setInitializeActive(false);
    }

    /** clear datastore */
    void TearDown() override
    {
      DataStore::Instance().reset();
    }
  };

  /** Test flavor tagging variables running on each ROE. */
  TEST_F(FlavorTaggingVariablesTest, VariablesRunningForEachROE)
  {

    /**In this function we test some of the variables used as input by the flavor tagger module.
    *  Specifically, we test those variables that use information from the RestOfEvent dataobject.
    *  In the flavor tagger module, these variables run within a loop for each ROE. */


    Gearbox& gearbox = Gearbox::getInstance();
    gearbox.setBackends({std::string("file:")});
    gearbox.close();
    gearbox.open("geometry/Belle2.xml", false);


    StoreArray<ECLCluster> testsECLClusters;
    StoreArray<TrackFitResult> testsTFRs;
    StoreArray<Track> testsTracks;
    StoreArray<Particle> testsParticles;


    /** Neutral particles to be added to the ROE:  we will add 5 photons. */
    vector<const Particle*> roeNeutralParticles;

    /** Vector containing the ConnectedRegionId and ClusterId for each neutral ROE ECLCluster */
    vector<vector<int>> roeNeutralECLClusterIds{{4, 1}, {6, 1}, {9, 1}, {12, 1}, {17, 1}};

    /** Vector containing the energy, theta, phi and radius values for each neutral ROE ECLCluster */
    vector<vector<double>> roeNeutralECLClusterProperties{{0.0487526, 0.493606, -2.65695, 239.246},
      {0.485431, 0.768093, 0.905049, 199.805},
      {0.436428, 0.99517, 1.6799, 167.271},
      {0.0230045, 1.05471, -1.20024, 161.013},
      {0.174332, 1.49141, 1.3059, 141.107}};

    /** Create neutral particles from ECLClusters for ROE */
    for (unsigned i = 0; i < roeNeutralECLClusterIds.size(); ++i) {

      ECLCluster ROEECL;
      ROEECL.setIsTrack(false);
      ROEECL.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      ROEECL.setConnectedRegionId(roeNeutralECLClusterIds[i][0]);
      ROEECL.setClusterId(roeNeutralECLClusterIds[i][1]);
      ROEECL.setEnergy(roeNeutralECLClusterProperties[i][0]);
      ROEECL.setTheta(roeNeutralECLClusterProperties[i][1]);
      ROEECL.setPhi(roeNeutralECLClusterProperties[i][2]);
      ROEECL.setR(roeNeutralECLClusterProperties[i][3]);
      ECLCluster* savedROEECL = testsECLClusters.appendNew(ROEECL);
      Particle* roeECLParticle = testsParticles.appendNew(savedROEECL);
      roeNeutralParticles.push_back(roeECLParticle);

    }

    /** Charged particles to be added to the ROE: we will add 8 charged pions. */
    vector<const Particle*> roeChargedParticles;
    const float bField = 1.5;
    TMatrixDSym cov6(6);

    /** Vector containing the x, y, z, px, py, pz, and p-value for each ROE TrackFitResult */
    vector<vector<float>> roeTFRProperties{{ -0.578196, 0.252548, 0.158642, -0.520087, -1.19071, 0.417451, 0.139206},
      {0.00434622, -0.0191058, 0.231542, 0.664887, 0.15125, 0.463762, 0.0313268},
      {0.0738595, -0.00294903, 0.242319, 0.0186749, 0.467721, 0.288627, 0.0415814},
      {1.3887, -2.43016, 1.00615, -0.27093, -0.15482, 0.0639155, 0.0132331},
      {0.0041096, -0.0152487, 0.264326, 0.294061, 0.079251, 0.0389014, 0.00200114},
      { -0.00371803, -5.22544e-05, 0.210148, 0.0019418, -0.138163, -0.0287232, 0.000622186},
      {1.02296, 0.608721, 0.32273, -0.0505521, 0.0849532, 0.0839057, 0},
      {6.49062, 3.20227, 117.684, 0.0469579, -0.0951783, -0.0706371, 0.117562}};

    /** Vector containing the charges for each ROE TrackFitResult */
    vector<short int> roeTRFCharges{1, 1, 1, -1, -1, -1, 1, -1};

    /** Vector containing the CDC initialization values for each ROE TrackFitResult */
    vector<uint64_t> roeTRFCDCValues{3098476543630901248, 3026418949592973312, 3170534137668829184, 3386706919782612992,
                                     3242591731706757120, 2954361355555045376, 1080863910568919040, 504403158265495552};

    /** Vector containing the VXD initialization values for each ROE TrackFitResult */
    vector<uint32_t> roeTRFVXDValues{5570560, 5592320, 5592320, 5570560, 5592320, 5264640, 328960, 0};


    /** Vector containing the ConnectedRegionId and ClusterId for each charged ROE ECLCluster */
    vector<vector<int>> roeChargedECLClusterIds{{14, 1}, {10, 1}, {7, 1}, {8, 1}, {15, 1}};

    /** Vector containing the energy, theta, phi and radius values for each charged ROE ECLCluster */
    vector<vector<double>> roeChargedECLClusterProperties{{0.964336, 1.23481, -2.25428, 148.729},
      {0.214864, 0.965066,   -0.232973, 170.008},
      {0.0148855, 0.914396,  1.01693  , 175.861},
      {0.524092, 0.956389,   0.854331 , 171.378},
      {0.230255, 1.33317,    -1.45326 , 144.849}};

    unsigned int chargedECLCLusterCounter = 0;

    /** Create charged particles from tracks for first ROE. */
    for (unsigned i = 0; i < roeTRFCharges.size(); ++i) {

      TVector3 position(roeTFRProperties[i][0], roeTFRProperties[i][1], roeTFRProperties[i][2]);
      TVector3 momentum(roeTFRProperties[i][3], roeTFRProperties[i][4], roeTFRProperties[i][5]);

      testsTFRs.appendNew(position, momentum, cov6, roeTRFCharges[i], Const::pion, roeTFRProperties[i][6], bField, roeTRFCDCValues[i],
                          roeTRFVXDValues[i], 0);

      Track ROETrack;
      ROETrack.setTrackFitResultIndex(Const::pion, i);
      Track* savedROETrack = testsTracks.appendNew(ROETrack);

      /** Here we add charged ECL clusters for some tracks */
      if (i == 0 || i == 1 || i == 2 || i == 3) {
        ECLCluster ROEChargedECL;
        ROEChargedECL.setIsTrack(true);
        ROEChargedECL.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
        ROEChargedECL.setConnectedRegionId(roeChargedECLClusterIds[chargedECLCLusterCounter][0]);
        ROEChargedECL.setClusterId(roeChargedECLClusterIds[chargedECLCLusterCounter][1]);
        ROEChargedECL.setEnergy(roeChargedECLClusterProperties[chargedECLCLusterCounter][0]);
        ROEChargedECL.setTheta(roeChargedECLClusterProperties[chargedECLCLusterCounter][1]);
        ROEChargedECL.setPhi(roeChargedECLClusterProperties[chargedECLCLusterCounter][2]);
        ROEChargedECL.setR(roeChargedECLClusterProperties[chargedECLCLusterCounter][3]);
        ECLCluster* savedROEChargedECL = testsECLClusters.appendNew(ROEChargedECL);
        savedROEChargedECL->addRelationTo(savedROETrack);
        chargedECLCLusterCounter++;

        if (i == 2) {
          /** We add a second Cluster to the third ROE track */
          ECLCluster ROEChargedECL2;
          ROEChargedECL2.setIsTrack(true);
          ROEChargedECL2.setHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
          ROEChargedECL2.setConnectedRegionId(roeChargedECLClusterIds[chargedECLCLusterCounter][0]);
          ROEChargedECL2.setClusterId(roeChargedECLClusterIds[chargedECLCLusterCounter][1]);
          ROEChargedECL2.setEnergy(roeChargedECLClusterProperties[chargedECLCLusterCounter][0]);
          ROEChargedECL2.setTheta(roeChargedECLClusterProperties[chargedECLCLusterCounter][1]);
          ROEChargedECL2.setPhi(roeChargedECLClusterProperties[chargedECLCLusterCounter][2]);
          ROEChargedECL2.setR(roeChargedECLClusterProperties[chargedECLCLusterCounter][3]);
          ECLCluster* savedROEChargedECL2 = testsECLClusters.appendNew(ROEChargedECL2);
          savedROETrack->addRelationTo(savedROEChargedECL2);
          chargedECLCLusterCounter++;

        }
      }
      Particle* roeTrackParticle = testsParticles.appendNew(savedROETrack, Const::pion);
      roeChargedParticles.push_back(roeTrackParticle);
    }

    /** Create ROE object, append tracks, clusters */
    StoreObjPtr<RestOfEvent> roe("RestOfEvent");
    roe.create();
    roe->addParticles(roeNeutralParticles);
    roe->addParticles(roeChargedParticles);

    B2INFO("Is the ROE valid? " << roe.isValid());
    ASSERT_TRUE(roe.isValid());


    /** Test if we created the ROE ECLCLusters correctly */
    TLorentzVector roe1FourVectorECLClusters = roe -> get4VectorNeutralECLClusters();

    B2INFO("The total four momentum of the neutral clusters in the first test ROE is = ("
           << roe1FourVectorECLClusters.E() << ", "
           << roe1FourVectorECLClusters.X() << ", "
           << roe1FourVectorECLClusters.Y() << ", "
           << roe1FourVectorECLClusters.Z() << ")");

    EXPECT_NEAR(roe1FourVectorECLClusters.E(), 1.16795, 0.00005);
    EXPECT_NEAR(roe1FourVectorECLClusters.X(), 0.20075, 0.00005);
    EXPECT_NEAR(roe1FourVectorECLClusters.Y(), 0.76747, 0.00005);
    EXPECT_NEAR(roe1FourVectorECLClusters.Z(), 0.65482, 0.00005);

    /** Now we test the Flavor Tagger Input Variables */
    const Manager::Var* var = Manager::Instance().getVariable("BtagToWBosonVariables(recoilMass)");

    /** In this kind of fatal assertion (see google tests docu), we check that the variable manager is
    * not returning a null pointer instead of a pointer to the requested variable. */
    ASSERT_NE(var, nullptr);

    /** This vector contains the expected output values of the variable "BtagToWBosonVariables(recoilMass)" for each
     *  of the tracks in the ROE (reference values). */
    vector<double> refsBtagToWBosonRecoilMass{3.2093, 4.1099, 4.3019, 4.4436, 4.4924, 4.5995, 4.6493, 4.5963};

    /** For all tracks in the ROE we want to check that the variable "BtagToWBosonVariables(recoilMass)"
       returns the expected value.*/
    for (unsigned i = 0; i < roeChargedParticles.size(); i++) {

      double output = var -> function(roeChargedParticles[i]);

      /** In this non-fatal assertion we compare the output of the variable "BtagToWBosonVariables(recoilMass)"
      * with the reference value for the ith ROE track.
      * The absolute difference should not exceed 0.0005.  */
      EXPECT_NEAR(output, refsBtagToWBosonRecoilMass[i], 0.0005);

    }

    /** In the following, we repeat the procedure for the other flavor tagging variables that run for each ROE. */

    var = Manager::Instance().getVariable("BtagToWBosonVariables(recoilMassSqrd)");
    ASSERT_NE(var, nullptr);

    vector<double> refsBtagToWBosonRecoilMassSqrd{10.300, 16.891, 18.506, 19.746, 20.182, 21.155, 21.616, 21.126};

    for (unsigned i = 0; i < roeChargedParticles.size(); i++) {

      double output = var -> function(roeChargedParticles[i]);

      EXPECT_NEAR(output, refsBtagToWBosonRecoilMassSqrd[i], 0.0005);

    }

    var = Manager::Instance().getVariable("BtagToWBosonVariables(pMissCMS)");
    ASSERT_NE(var, nullptr);

    double refsBtagToWBosonPMissCMS = 0.542734;

    for (auto& roeChargedParticle : roeChargedParticles) {

      double output = var -> function(roeChargedParticle);

      EXPECT_NEAR(output, refsBtagToWBosonPMissCMS, 0.000005);

    }

    var = Manager::Instance().getVariable("BtagToWBosonVariables(cosThetaMissCMS)");
    ASSERT_NE(var, nullptr);

    vector<double> refsBtagToWBosonCosThetaMissCMS{0.0621, -0.6164, -0.2176, 0.3516, -0.1165, 0.4694, -0.0754, 0.6302};

    for (unsigned i = 0; i < roeChargedParticles.size(); i++) {

      double output = var -> function(roeChargedParticles[i]);

      EXPECT_NEAR(output, refsBtagToWBosonCosThetaMissCMS[i], 0.0005);

    }


    var = Manager::Instance().getVariable("BtagToWBosonVariables(EW90)");
    ASSERT_NE(var, nullptr);

    vector<double> refsBtagToWBosonEW90{0.3020, 1.63517, 1.09619, 0.96434, 0.17433, 1.19459, 1.13867, 1.36892};

    for (unsigned i = 0; i < roeChargedParticles.size(); i++) {

      double output = var -> function(roeChargedParticles[i]);

      EXPECT_NEAR(output, refsBtagToWBosonEW90[i], 0.0005);

    }

  }


  /** Test isSignal variable. */
  TEST_F(FlavorTaggingVariablesTest, isSignalVariable)
  {

    /**In this function we test the isSignal variable because it is used in target variables which are essential
    * for the flavor tagger module. The variables that use isSignal are:
    * isRelatedRestOfEventB0Flavor, isRestOfEventB0Flavor (qrCombined), isRelatedRestOfEventMajorityB0Flavor,
    * isRestOfEventMajorityB0Flavor, McFlavorOfTagSide.  */

    /** In the following we will test the output of isSignal for the 12 possible basic mc error flags.*/

    StoreArray<Particle> testsParticles;
    StoreArray<MCParticle> testsMCParticles;

    /** Here we create a mc B0 particle*/
    MCParticle MCB0;
    MCB0.setEnergy(5.68161);
    MCB0.setPDG(511);
    MCB0.setMassFromPDG();
    MCB0.setMomentum(-0.12593, -0.143672, 2.09072);
    MCParticle* savedMCB0 = testsMCParticles.appendNew(MCB0);

    /** Here we create a B0 particle and set its relation to the mc B0 particle.*/
    Particle B0({ -0.129174, -0.148899, 2.09292, 5.67644}, 511);
    Particle* savedB0 = testsParticles.appendNew(B0);
    savedB0->addRelationTo(savedMCB0);

    /** Here we test the isSignal variable */
    const Manager::Var* var = Manager::Instance().getVariable("isSignal");
    ASSERT_NE(var, nullptr);

    /** Here we set the mc error flag corresponding to a perfectly matched particle,
    * we expect isSignal to return 1.0 .*/
    savedB0->addExtraInfo(MCMatching::c_extraInfoMCErrors, 0);
    double output1 = var -> function(savedB0);
    ASSERT_EQ(output1, 1.0);

    /** We consider also as correctly matched those particles with the mc error flags
    * We considered manually set flags MissFSR, MissingResonance and MissPHOTOS. 1 + 2 + 1024 = 1027,
    * as signal. Now this is tested in mcmatching.cc.
    * if c_isIgnoreRadiatedPhotons and c_isIgnoreIntermediate are set to the properties, 2 + 4 = 6  */
    savedB0->setExtraInfo(MCMatching::c_extraInfoMCErrors, 0);
    savedB0->setProperty(6);
    double output2 = var -> function(savedB0);
    ASSERT_EQ(output2, 1.0);

    /** We do not consider as signal those particles with other mc error flags. See definition in
    * analysis/utility/include/MCMatching.h */
    vector<int> notAcceptedMCErrorFlags{4, 8, 16, 32, 64, 128, 256, 512};

    for (int notAcceptedMCErrorFlag : notAcceptedMCErrorFlags) {

      savedB0->setExtraInfo(MCMatching::c_extraInfoMCErrors, notAcceptedMCErrorFlag);
      double output = var -> function(savedB0);
      ASSERT_EQ(output, 0);

    }

  }

}
