/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#include <analysis/dbobjects/PIDDetectorWeights.h>

#include <gtest/gtest.h>
#include <random>



namespace Belle2 {

  /** Test the calculation of the track helix-based isolation score per particle. */
  class TrackIsoScoreCalculatorTest : public ::testing::Test {

  public:

    /**
     * The charged particle hypothesis to test.
     */
    Const::ChargedStable m_testHypo = Const::electron;

    /**
     * The detector to test.
     */
    Const::EDetector m_detector = Const::ECL;

    /**
     * The detector layer to test.
     */
    int m_layer = 0;

    /**
     * Dummy ROOT file name.
     * Files are created at the start of the test,
     * and deleted at the end.
     */
    std::string m_dummyFile = "dummyFile.root";

  protected:

    /**
     * Create a ROOT::TTree for the detector weights w/ just one row.
     * This replicates the structure of the serialised csv weights file in the payload.
     */
    void createDummyTTree()
    {
      ROOT::RDataFrame rdf(1);

      auto pdgId = static_cast<double>(m_testHypo.getPDGCode());
      std::string regLabel = "Barrel";

      rdf.Define("pdgId", [&]() { return pdgId; })
      .Define("p_min", []() { return 1.0; })
      .Define("p_max", []() { return 1.5; })
      .Define("theta_min", []() { return 0.56; })
      .Define("theta_max", []() { return 2.23; })
      .Define("p_bin_idx", [&]() { return 1.0; })
      .Define("theta_bin_idx", [&]() { return 1.0; })
      .Define("reg_label", [&]() { return regLabel; })
      .Define("indiv_s_SVD", []() { return 0.2; })
      .Define("ablat_s_SVD", []() { return 0.; })
      .Define("indiv_s_CDC", []() { return 0.886; })
      .Define("ablat_s_CDC", []() { return -0.084; })
      .Define("indiv_s_TOP", []() { return 0.; })
      .Define("ablat_s_TOP", []() { return 0.; })
      .Define("indiv_s_ARICH", []() { return 0.; })
      .Define("ablat_s_ARICH", []() { return 0.; })
      .Define("indiv_s_ECL", []() { return 0.959; })
      .Define("ablat_s_ECL", []() { return -0.118; })
      .Define("indiv_s_KLM", []() { return 0.2; })
      .Define("ablat_s_KLM", []() { return 0.; })
      .Snapshot("tree", m_dummyFile);
    };

    /**
     * Prepare resources for the tests.
     */
    void SetUp() override
    {
      createDummyTTree();
    }

    /**
     * Release all resources.
     */
    void TearDown() override
    {
      // Delete all dummy files.
      if (remove(m_dummyFile.c_str())) {
        B2ERROR("Couldn't remove file: " << m_dummyFile);
      }
    }

  };

  /**
   * Test correct retrieval of information from the
   * database representation inner structure.
   */
  TEST_F(TrackIsoScoreCalculatorTest, TestDBRep)
  {

    PIDDetectorWeights dbrep("tree", m_dummyFile);

    auto newThresh = 30.;
    dbrep.setDistThreshold(m_detector, m_layer, newThresh);
    EXPECT_EQ(newThresh, dbrep.getDistThreshold(m_detector, m_layer));

    auto pdg = dbrep.getWeightsRDF().Take<double>("pdgId").GetValue()[0];
    EXPECT_EQ(pdg, m_testHypo.getPDGCode());

    auto p = 1.23; // GeV/c
    auto theta = 1.34; // rad
    auto weight = dbrep.getWeight(m_testHypo, m_detector, p, theta);
    EXPECT_EQ(weight, -0.118);

  }

}  // namespace
