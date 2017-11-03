/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDModeByte.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <gtest/gtest.h>

namespace Belle2 {
  namespace SVD {

    /**
     * Check empty object creation and data getters.
     */
    TEST(SVDRecoDigit, ConstructEmpty)
    {
      // Create an empty 6-digit
      SVDRecoDigit digit;
      // Test getters
      EXPECT_EQ(0, digit.getSensorID());
      EXPECT_TRUE(digit.isUStrip());
      EXPECT_EQ(0, digit.getCellID());
      EXPECT_EQ(0, digit.getAmplitude());
      EXPECT_EQ(10, digit.getAmplitudeError());
      EXPECT_EQ(0, digit.getTime());
      EXPECT_EQ(100, digit.getTimeError());
      const SVDRecoDigit::OutputProbArray pdf = digit.getProbabilities();
      for (auto prob : pdf)
        EXPECT_EQ(SVDRecoDigit::OutputProbType(1.0), prob);
      EXPECT_EQ(100.0, digit.getChi2Ndf());
      EXPECT_EQ(SVDModeByte::c_DefaultID, digit.getModeByte().getID());
      EXPECT_EQ("0-suppr/global/6 samples/???", std::string(digit.getModeByte()));
    }

    /**
     * Check object creation and data getters using a stl container of probabilities.
     */
    TEST(SVDRecoDigit, ConstructFromContainer)
    {
      // Create an arbitrary recodigit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;
      SVDModeByte digitModeByte(151);
      float init_charge = 23456;
      float init_chargeErr = 1234;
      float init_time = -16;
      float init_timeErr = 3.21;
      float init_chi2 = 2.34;

      std::vector<float> init_probs({0.0, 0.01, 0.10, 0.79, 0.06, 0.04});
      SVDRecoDigit digit(sensorID, false, cellID, init_charge, init_chargeErr, init_time,
                         init_timeErr, init_probs, init_chi2, digitModeByte);
      // Test getters
      EXPECT_EQ(sensorID, digit.getSensorID());
      EXPECT_FALSE(digit.isUStrip());
      EXPECT_EQ(cellID, digit.getCellID());
      EXPECT_EQ(init_charge, digit.getCharge());
      EXPECT_EQ(init_chargeErr, digit.getAmplitudeError());
      EXPECT_EQ(init_time, digit.getTime());
      EXPECT_EQ(init_timeErr, digit.getTimeError());
      const SVDRecoDigit::OutputProbArray pdf = digit.getProbabilities();
      for (size_t ip = 0; ip < pdf.size(); ++ip)
        EXPECT_LE(fabs(static_cast<SVDRecoDigit::OutputProbType>(init_probs[ip]) - pdf[ip]), 1.0 / SVDRecoDigit::storedProbArrayNorm);
      // Also, norm must be 1
      float outputNorm = std::accumulate(pdf.begin(), pdf.end(), 0.0);
      EXPECT_LE(fabs(1.0 - outputNorm), 1.0e-6);
      EXPECT_EQ(init_chi2, digit.getChi2Ndf());
      EXPECT_EQ(digitModeByte, digit.getModeByte());
      EXPECT_EQ("0-suppr/global/6 samples/???", std::string(digit.getModeByte()));
    }
    /**
     * Check object creation and data getters using a stl container of probabilities.
     */
    TEST(SVDRecoDigit, ToString)
    {
      // Create an arbitrary recodigit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;
      SVDModeByte digitModeByte(151);
      float init_charge = 23456;
      float init_chargeErr = 1234;
      float init_time = -16;
      float init_timeErr = 3.21;
      float init_chi2 = 2.34;

      std::vector<float> init_probs({0.0, 0.01, 0.10, 0.79, 0.06, 0.04});
      SVDRecoDigit digit(sensorID, false, cellID, init_charge, init_chargeErr, init_time,
                         init_timeErr, init_probs, init_chi2, digitModeByte);
      // Test getters
      std::string
      digitString("VXDID : 25632 = 3.4.1 strip: V-132 Amplitude: 23456 +/- 1234 Time: -16 +/- 3.21\n probabilities: 0 655 6553 51772 3932 2621 Chi2/ndf: 2.34 0-suppr/global/6 samples/???\n");
      EXPECT_EQ(digitString, digit.toString());
    }

  } // namespace SVD
}  // namespace Belle2
