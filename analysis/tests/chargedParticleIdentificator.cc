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

#include <gtest/gtest.h>
#include <random>

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
     * The (clusterTheta, p, charge) grid for which xml files are stored in the payload.
     */
    std::unique_ptr<TH3F> m_grid;

    /**
     * The clusterTheta bin edges in [rad].
     */
    std::vector<float> m_thetabins = {0.2164208, 0.5480334, 0.561996, 2.2462387, 2.2811453, 2.7070057};

    /**
     * The p bin edges in [GeV/c].
     */
    std::vector<float> m_pbins = {0.2, 0.6, 0.75, 1.0, 3.0, 7.0};

    /**
     * The charge bin edges.
     */
    std::vector<float> m_chbins = { -1.5, -0.5, 0.5, 1.5};

    /**
     * Base common name for all dummy weight files.
     */
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
    void SetUp() override
    {

      m_grid = std::make_unique<TH3F>("theta_p_charge_binsgrid",
                                      ";ECL cluster #theta;p_{lab}; Q",
                                      m_thetabins.size() - 1,
                                      m_thetabins.data(),
                                      m_pbins.size() - 1,
                                      m_pbins.data(),
                                      m_chbins.size() - 1,
                                      m_chbins.data());

      m_dbrep.setWeightCategories(m_grid.get());

      std::vector<std::tuple<double, double, double>> gridBinCentres;

      for (unsigned int kch(0); kch < m_chbins.size() - 1; kch++) {
        auto ch_bin_centre = (m_chbins.at(kch) + m_chbins.at(kch + 1)) / 2.0;
        for (unsigned int ip(0); ip < m_pbins.size() - 1; ip++) {
          auto p_bin_centre = (m_pbins.at(ip) + m_pbins.at(ip + 1)) / 2.0;
          for (unsigned int jth(0); jth < m_thetabins.size() - 1; jth++) {
            auto th_bin_centre = (m_thetabins.at(jth) + m_thetabins.at(jth + 1)) / 2.0;
            auto fname = m_basename
                         + "__clusterTheta__" + std::to_string(m_thetabins.at(jth)) + "_" + std::to_string(m_thetabins.at(jth + 1))
                         + "__p__" + std::to_string(m_pbins.at(ip)) + "_" + std::to_string(m_pbins.at(ip + 1))
                         + "__charge__" + std::to_string(ch_bin_centre);

            std::replace(fname.begin(), fname.end(), '.', '_');
            fname += ".xml";
            std::ofstream dummyfile(fname);
            dummyfile.close();
            m_dummyfiles.push_back(fname);

            auto centre = std::make_tuple(th_bin_centre, p_bin_centre, ch_bin_centre);
            gridBinCentres.push_back(centre);
          }
        }
      }
      m_dbrep.storeMVAWeights(m_testHypo.getPDGCode(), m_dummyfiles, gridBinCentres);

    }

    /**
     * Release all resources.
     */
    void TearDown() override
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

    // Pick a random index in the clusterTheta, p, charge bins arrays.
    // Exclude underflow and overflow.
    std::random_device rd; // non-deterministic uniform int rand generator.
    std::uniform_int_distribution<int> binx_idx_distr(1, m_thetabins.size() - 1);
    std::uniform_int_distribution<int> biny_idx_distr(1, m_pbins.size() - 1);
    std::uniform_int_distribution<int> binz_idx_distr(1, m_chbins.size() - 1);
    int binx = binx_idx_distr(rd);
    int biny = biny_idx_distr(rd);
    int binz = binz_idx_distr(rd);

    // Pick each axis' bin centre as a test value for (clusterTheta, p, charge).
    auto theta = m_grid->GetXaxis()->GetBinCenter(binx);
    auto p = m_grid->GetYaxis()->GetBinCenter(biny);
    auto charge = m_grid->GetZaxis()->GetBinCenter(binz);

    int jth, ip, kch;
    auto jik = m_dbrep.getMVAWeightIdx(theta, p, charge, jth, ip, kch);

    EXPECT_EQ(jth, binx);
    EXPECT_EQ(ip, biny);
    EXPECT_EQ(kch, binz);

    auto thisfname = m_basename
                     + "__clusterTheta__" + std::to_string(m_thetabins.at(jth - 1)) + "_" + std::to_string(m_thetabins.at(jth))
                     + "__p__" + std::to_string(m_pbins.at(ip - 1)) + "_" + std::to_string(m_pbins.at(ip))
                     + "__charge__" + std::to_string(charge);
    std::replace(thisfname.begin(), thisfname.end(), '.', '_');
    thisfname += ".xml";

    EXPECT_EQ(thisfname, m_dummyfiles.at(jik));

    auto matchitr = std::find(m_dummyfiles.begin(), m_dummyfiles.end(), thisfname);
    auto thisidx = std::distance(m_dummyfiles.begin(), matchitr);

    EXPECT_EQ(thisidx, jik);

  }

}  // namespace
