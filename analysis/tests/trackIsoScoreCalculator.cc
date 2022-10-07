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
#include <framework/utilities/TestHelpers.h>

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
     * Create a ROOT::TTree for the detector weights w/ a few entries.
     * This replicates the structure of the CSV weights file that are serialised in the payload.
     */
    void createDummyTTree()
    {

      unsigned int nEntries(6);

      ROOT::RDataFrame rdf(nEntries);

      std::vector<double> pdgIds(nEntries, static_cast<double>(m_testHypo.getPDGCode()));
      std::vector<double> pMinEdges = {1.0, 1.0, 1.0, 1.5, 1.5, 1.5};
      std::vector<double> pMaxEdges = {1.5, 1.5, 1.5, 3.0, 3.0, 3.0};
      std::vector<double> thetaMinEdges = {0.22, 0.56, 2.23, 0.22, 0.56, 2.23};
      std::vector<double> thetaMaxEdges = {0.56, 2.23, 2.71, 0.56, 2.23, 2.71};
      std::vector<double> pBinIdxs = {1.0, 1.0, 1.0, 2.0, 2.0, 2.0};
      std::vector<double> thetaBinIdxs = {1.0, 2.0, 3.0, 1.0, 2.0, 3.0};
      std::vector<std::string> regLabels = {"FWD", "Barrel", "BWD", "FWD", "Barrel", "BWD"};
      std::vector<double> weights_SVD = {0., 0., -0.003, 0.031, 0.017, 0.077};
      std::vector<double> weights_CDC = {-0.061, -0.084, -0.162, -0.073, -0.073, -0.05, -0.121};
      std::vector<double> weights_TOP = {0., 0., 0., 0., 0., 0.};
      std::vector<double> weights_ARICH = {0., 0., 0., -0.004, 0., 0.};
      std::vector<double> weights_ECL = {-0.062, -0.118, -0.136, -0.22, -0.255, -0.377};
      std::vector<double> weights_KLM = {0., 0., 0.006, -0.003, 0.003, 0.002};

      unsigned int iEntry(0);
      rdf.Define("pdgId", [&]() { auto x = pdgIds[iEntry]; return x; })
      .Define("p_min", [&]() { auto x = pMinEdges[iEntry]; return x; })
      .Define("p_max", [&]() { auto x = pMaxEdges[iEntry]; return x; })
      .Define("theta_min", [&]() { auto x = thetaMinEdges[iEntry]; return x; })
      .Define("theta_max", [&]() { auto x = thetaMaxEdges[iEntry]; return x; })
      .Define("p_bin_idx", [&]() { auto x = pBinIdxs[iEntry]; return x; })
      .Define("theta_bin_idx", [&]() { auto x = thetaBinIdxs[iEntry]; return x; })
      .Define("reg_label", [&]() { auto x = regLabels[iEntry]; return x; })
      .Define("ablat_s_SVD", [&]() { auto x = weights_SVD[iEntry]; return x; })
      .Define("ablat_s_CDC", [&]() { auto x = weights_CDC[iEntry]; return x; })
      .Define("ablat_s_TOP", [&]() { auto x = weights_TOP[iEntry]; return x; })
      .Define("ablat_s_ARICH", [&]() { auto x = weights_ARICH[iEntry]; return x; })
      .Define("ablat_s_ECL", [&]() { auto x = weights_ECL[iEntry]; return x; })
      .Define("ablat_s_KLM", [&]() { auto x = weights_KLM[iEntry]; ++iEntry; return x; }) // Only the last call in the chain must increment the entry counter!
      .Snapshot("tree", m_dummyFile);

    };

    /**
     * Create a ROOT::TTree  with a "broken" bin edges structure, i.e. non-contiguous bin edges.
     * This will throw a FATAL error, and indicate the CSV weights file has problems.
     */
    void createDummyBrokenTTree()
    {

      unsigned int nEntries(6);

      ROOT::RDataFrame rdf(nEntries);

      std::vector<double> pdgIds(nEntries, static_cast<double>(m_testHypo.getPDGCode()));
      std::vector<double> pMinEdges = {1.0, 1.0, 1.0, 1.5, 1.5, 1.5};
      std::vector<double> pMaxEdges = {1.8, 1.8, 1.8, 3.0, 3.0, 3.0}; // note the non-contiguous edges wrt. pMinEdges
      std::vector<double> thetaMinEdges = {0.22, 0.56, 2.23, 0.22, 0.56, 2.23};
      std::vector<double> thetaMaxEdges = {0.56, 2.23, 2.71, 0.56, 2.23, 2.71};
      std::vector<double> pBinIdxs = {1.0, 1.0, 1.0, 2.0, 2.0, 2.0};
      std::vector<double> thetaBinIdxs = {1.0, 2.0, 3.0, 1.0, 2.0, 3.0};

      // Tell snapshot to update file.
      ROOT::RDF::RSnapshotOptions opt;
      opt.fMode = "UPDATE";

      unsigned int iEntry(0);
      rdf.Define("pdgId", [&]() { auto x = pdgIds[iEntry]; return x; })
      .Define("p_min", [&]() { auto x = pMinEdges[iEntry]; return x; })
      .Define("p_max", [&]() { auto x = pMaxEdges[iEntry]; return x; })
      .Define("theta_min", [&]() { auto x = thetaMinEdges[iEntry]; return x; })
      .Define("theta_max", [&]() { auto x = thetaMaxEdges[iEntry]; return x; })
      .Define("p_bin_idx", [&]() { auto x = pBinIdxs[iEntry]; return x; })
      .Define("theta_bin_idx", [&]() { auto x = thetaBinIdxs[iEntry]; ++iEntry; return x; })
      .Snapshot("tree_broken", m_dummyFile, "", opt);

    };

    /**
     * Prepare resources for the tests.
     */
    void SetUp() override
    {
      createDummyTTree();
      createDummyBrokenTTree();
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

    // Test for correct filling of the RDataFrame.
    auto pdgIds = dbrep.getWeightsRDF().Take<double>("pdgId").GetValue();
    for (const auto& pdgId : pdgIds) {
      EXPECT_EQ(pdgId, m_testHypo.getPDGCode());
    }

    // Retrieve weight for a (p, theta) pair in the available weights range.
    auto p = 1.23; // GeV/c
    auto theta = 1.34; // rad
    auto weight = dbrep.getWeight(m_testHypo, m_detector, p, theta);
    EXPECT_EQ(weight, -0.118);

    // Test for weight in case of out-of-range p and/or theta values.
    p = 1.46;
    theta = 0.12;
    weight = dbrep.getWeight(m_testHypo, m_detector, p, theta);
    EXPECT_TRUE(std::isnan(weight));

    // Trigger a FATAL if reading an ill-defined source table.
    EXPECT_B2FATAL(PIDDetectorWeights("tree_broken", m_dummyFile));

  }

}  // namespace
