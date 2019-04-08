/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#include <analysis/dbobjects/ChargedPidMVAWeights.h>

#include <utility>
#include <gtest/gtest.h>

#include <TH2F.h>

namespace Belle2 {

  /** Test the MVA-based charged PID. */
  class ChargedParticleIdentificatorTest : public ::testing::Test {

  public:

    /**
     * Database representation of MVA weightfiles.
     */
    ChargedPidMVAWeights m_dbrep;

    /**
     * The signal charged particle hypothesis to test.
     */
    Const::ChargedStable m_testHypo = Const::electron;

    /**
     * The (clusterTheta, p) grid for which xml files are stored in the payload.
     */
    std::unique_ptr<TH2F> m_grid;

    /**
     * The clusterTheta bin edges.
     */
    std::vector<float> m_thetabins = {0.2164208, 0.5480334, 0.561996, 2.2462387, 2.2811453, 2.7070057};

    /**
     * The p bin edges.
     */
    std::vector<float> m_pbins = {0.0, 0.5, 0.75, 1.0, 3.0, 100.0};

    /**
     * The clusterRegion bins.
     */
    std::vector<int> m_eclregbins = {1, 11, 2, 13, 3};

    std::string m_basename = "dummy_weightfile";

    /**
     * List of dummy xml file names.
     * Files are created at the start of the test,
     * and deleted at the end.
     */
    std::vector<std::string> m_dummyfiles;

  protected:

    /**
     * Prepare resources for the tests.
     */
    virtual void SetUp()
    {

      m_grid = std::make_unique<TH2F>("theta_p_binsgrid",
                                      ";ECL cluster #theta;p_{lab}",
                                      m_thetabins.size() - 1,
                                      m_thetabins.data(),
                                      m_pbins.size() - 1,
                                      m_pbins.data());

      m_dbrep.storeClusterThetaPGrid(m_testHypo.getPDGCode(), m_grid.get());

      m_dbrep.setAngularUnit(Unit::rad);
      m_dbrep.setEnergyUnit(Unit::GeV);

      for (unsigned int ip(0); ip < m_pbins.size() - 1; ip++) {
        for (unsigned int jth(0); jth < m_thetabins.size() - 1; jth++) {
          auto fname = m_basename
                       + "__p__" + std::to_string(m_pbins.at(ip)) + "_" + std::to_string(m_pbins.at(ip + 1))
                       + "__clusterTheta__" + std::to_string(m_thetabins.at(jth)) + "_" + std::to_string(m_thetabins.at(jth + 1));
          std::replace(fname.begin(), fname.end(), '.', '_');
          fname += ".xml";
          std::ofstream dummyfile(fname);
          dummyfile.close();
          m_dummyfiles.push_back(fname);
        }
      }
      m_dbrep.storeMVAWeights(m_testHypo.getPDGCode(), m_dummyfiles);

    }

    /**
     * Release all resources.
     */
    virtual void TearDown()
    {

      // Delete all dummy files.
      for (const auto& fname : m_dummyfiles) {
        if (remove(fname.c_str())) {
          B2ERROR("Couldn't remove file: " << fname);
        }
      }

    }

  };

  /**
   * Test correct storage of weightfiles in the
   * database representation inner structure.
   */
  TEST_F(ChargedParticleIdentificatorTest, TestDBRep)
  {

    // Pick a value for (clusterTheta, p) in the grid.
    int binx = 3;
    int biny = 4;
    auto theta = m_grid->GetXaxis()->GetBinCenter(binx);
    auto p = m_grid->GetYaxis()->GetBinCenter(biny);

    int jth, ip;
    auto ij = m_dbrep.getMVAWeightIdx(m_testHypo, theta, p, jth, ip);

    EXPECT_EQ(jth, binx);
    EXPECT_EQ(ip, biny);

    auto thisfname = m_basename
                     + "__p__" + std::to_string(m_pbins.at(ip - 1)) + "_" + std::to_string(m_pbins.at(ip))
                     + "__clusterTheta__" + std::to_string(m_thetabins.at(jth - 1)) + "_" + std::to_string(m_thetabins.at(jth));
    std::replace(thisfname.begin(), thisfname.end(), '.', '_');
    thisfname += ".xml";

    EXPECT_EQ(thisfname, m_dummyfiles.at(ij));

    auto matchitr = std::find(m_dummyfiles.begin(), m_dummyfiles.end(), thisfname);
    auto thisidx = std::distance(m_dummyfiles.begin(), matchitr);

    EXPECT_EQ(thisidx, ij);

  }

}  // namespace
