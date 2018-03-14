/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/utility/Binning.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  TEST(BinningTest, Binning)
  {
    MVA::Binning binning(3);

    EXPECT_EQ(binning.m_signal_pdf.size(), 3);
    EXPECT_EQ(binning.m_signal_cdf.size(), 3);
    EXPECT_EQ(binning.m_bckgrd_pdf.size(), 3);
    EXPECT_EQ(binning.m_bckgrd_cdf.size(), 3);
    EXPECT_EQ(binning.m_boundaries.size(), 4);

    EXPECT_FLOAT_EQ(binning.m_signal_yield, 0.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_yield, 0.0);

    binning.m_signal_pdf[0] = 1.0;
    binning.m_signal_pdf[1] = 2.0;
    binning.m_signal_pdf[2] = 3.0;

    binning.m_bckgrd_pdf[0] = 3.0;
    binning.m_bckgrd_pdf[1] = 2.0;
    binning.m_bckgrd_pdf[2] = 1.0;

    binning.m_boundaries = {1.0, 2.0, 3.0, 4.0};

    binning.normalizePDFs();

    EXPECT_FLOAT_EQ(binning.m_signal_pdf[0], 1.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[1], 2.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[2], 3.0 / 6.0 * 3.0);

    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[0], 3.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[1], 2.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[2], 1.0 / 6.0 * 3.0);

    EXPECT_FLOAT_EQ(binning.m_signal_yield, 6.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_yield, 6.0);

    binning.calculateCDFsFromPDFs();

    EXPECT_FLOAT_EQ(binning.m_signal_pdf[0], 1.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[1], 2.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[2], 3.0 / 6.0 * 3.0);

    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[0], 3.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[1], 2.0 / 6.0 * 3.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[2], 1.0 / 6.0 * 3.0);

    EXPECT_FLOAT_EQ(binning.m_signal_yield, 6.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_yield, 6.0);

    EXPECT_FLOAT_EQ(binning.m_signal_cdf[0], 1.0 / 6.0);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[1], 3.0 / 6.0);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[2], 6.0 / 6.0);

    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[0], 3.0 / 6.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[1], 5.0 / 6.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[2], 6.0 / 6.0);

    binning.m_boundaries[0] = 1.0;
    binning.m_boundaries[1] = 2.0;
    binning.m_boundaries[2] = 3.0;
    binning.m_boundaries[3] = 4.0;

    EXPECT_EQ(binning.getBin(0.0), 0);
    EXPECT_EQ(binning.getBin(1.0), 0);
    EXPECT_EQ(binning.getBin(1.5), 0);
    EXPECT_EQ(binning.getBin(2.0), 1);
    EXPECT_EQ(binning.getBin(2.5), 1);
    EXPECT_EQ(binning.getBin(3.0), 2);
    EXPECT_EQ(binning.getBin(3.5), 2);
    EXPECT_EQ(binning.getBin(4.5), 2);

  }

  TEST(BinningTest, EqualFrequencyBinning)
  {
    std::vector<float> data(100);
    std::vector<float> weights(100);
    std::vector<bool> isSignal(100);

    for (unsigned int i = 0; i < 100; ++i) {
      data[i] = i;
      weights[i] = static_cast<int>(i / 10) + 1;
      isSignal[i] = (i % 2) == 1;
    }

    MVA::Binning binning = MVA::Binning::CreateEqualFrequency(data, weights, isSignal, 3);

    EXPECT_FLOAT_EQ(binning.m_boundaries.size(), 4);
    EXPECT_FLOAT_EQ(binning.m_boundaries[0], 0.0);
    EXPECT_FLOAT_EQ(binning.m_boundaries[1], 55.0);
    EXPECT_FLOAT_EQ(binning.m_boundaries[2], 80.0);
    EXPECT_FLOAT_EQ(binning.m_boundaries[3], 99.0);

    EXPECT_FLOAT_EQ(binning.m_signal_pdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[0] * binning.m_signal_yield * 55.0 / 99.0, 87.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[1] * binning.m_signal_yield * 25.0 / 99.0, 93.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[2] * binning.m_signal_yield * 19.0 / 99.0, 95.0);

    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[0] * binning.m_bckgrd_yield * 55.0 / 99.0, 93.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[1] * binning.m_bckgrd_yield * 25.0 / 99.0, 87.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[2] * binning.m_bckgrd_yield * 19.0 / 99.0, 95.0);

    EXPECT_FLOAT_EQ(binning.m_signal_cdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[0] * binning.m_signal_yield, 87.0);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[1] * binning.m_signal_yield, 180.0);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[2] * binning.m_signal_yield, 275.0);

    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[0] * binning.m_bckgrd_yield, 93.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[1] * binning.m_bckgrd_yield, 180.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[2] * binning.m_bckgrd_yield, 275.0);

  }

  TEST(BinningTest, EquidistantBinning)
  {
    std::vector<float> data(100);
    std::vector<float> weights(100);
    std::vector<bool> isSignal(100);

    for (unsigned int i = 0; i < 100; ++i) {
      data[i] = i;
      weights[i] = static_cast<int>(i / 10) + 1;
      isSignal[i] = (i % 2) == 1;
    }

    MVA::Binning binning = MVA::Binning::CreateEquidistant(data, weights, isSignal, 3);

    EXPECT_FLOAT_EQ(binning.m_boundaries.size(), 4);
    EXPECT_FLOAT_EQ(binning.m_boundaries[0], 0.0);
    EXPECT_FLOAT_EQ(binning.m_boundaries[1], 33.0);
    EXPECT_FLOAT_EQ(binning.m_boundaries[2], 66.0);
    EXPECT_FLOAT_EQ(binning.m_boundaries[3], 99.0);

    EXPECT_FLOAT_EQ(binning.m_signal_pdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[0] * binning.m_signal_yield / 3.0, 34.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[1] * binning.m_signal_yield / 3.0, 92.0);
    EXPECT_FLOAT_EQ(binning.m_signal_pdf[2] * binning.m_signal_yield / 3.0, 149.0);

    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[0] * binning.m_bckgrd_yield / 3.0, 38.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[1] * binning.m_bckgrd_yield / 3.0, 88.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_pdf[2] * binning.m_bckgrd_yield / 3.0, 149.0);

    EXPECT_FLOAT_EQ(binning.m_signal_cdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[0] * binning.m_signal_yield, 34.0);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[1] * binning.m_signal_yield, 126.0);
    EXPECT_FLOAT_EQ(binning.m_signal_cdf[2] * binning.m_signal_yield, 275.0);

    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf.size(), 3);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[0] * binning.m_bckgrd_yield, 38.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[1] * binning.m_bckgrd_yield, 126.0);
    EXPECT_FLOAT_EQ(binning.m_bckgrd_cdf[2] * binning.m_bckgrd_yield, 275.0);

  }

}
