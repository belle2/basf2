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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <gtest/gtest.h>

namespace Belle2 {
  namespace SVD {

    /**
     * Check empty object creation and data getters.
     */
    TEST(SVDShaperDigit, ConstructEmpty)
    {
      // Create an empty 6-digit
      SVDShaperDigit digit;
      // Test getters
      EXPECT_EQ(0, digit.getSensorID());
      EXPECT_TRUE(digit.isUStrip());
      EXPECT_EQ(0, digit.getCellID());
      const SVDShaperDigit::APVFloatSamples& samples = digit.getSamples();
      for (auto sample : samples)
        EXPECT_EQ(SVDShaperDigit::APVFloatSampleType(0), sample);
      EXPECT_EQ(0.0, digit.getTime());
      EXPECT_EQ(100.0, digit.getTimeError());
    }

    /**
     * Check standard object creation and data getters.
     */
    TEST(SVDShaperDigit, ConstructStandard)
    {
      // Create an arbitrary 6-digit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;
      float digitTime = -16.22;
      float digitTimeError = 5.0;

      SVDShaperDigit::APVRawSamples init_samples({0, 5, 10, 9, 6, 5});
      SVDShaperDigit digit(sensorID, false, cellID, init_samples, digitTime, digitTimeError);
      // Test getters
      EXPECT_EQ(sensorID, digit.getSensorID());
      EXPECT_FALSE(digit.isUStrip());
      EXPECT_EQ(cellID, digit.getCellID());
      const SVDShaperDigit::APVFloatSamples& samples = digit.getSamples();
      for (size_t isample = 0; isample < SVDShaperDigit::c_nAPVSamples; ++isample)
        EXPECT_EQ(static_cast<SVDShaperDigit::APVFloatSampleType>(init_samples[isample]),
                  samples[isample]);
      EXPECT_EQ(digitTime, digit.getTime());
      EXPECT_EQ(digitTimeError, digit.getTimeError());
    }
    /**
     * Check standard object creation without FADC time/error.
     */
    TEST(SVDShaperDigit, ConstructWoTime)
    {
      // Create an arbitrary 6-digit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;

      SVDShaperDigit::APVRawSamples init_samples({0, 5, 10, 9, 6, 5});
      SVDShaperDigit digit(sensorID, false, cellID, init_samples);
      // Test that time and error are set correctly.
      EXPECT_EQ(0.0, digit.getTime());
      EXPECT_EQ(100.0, digit.getTimeError());
    }
  } // namespace SVD
}  // namespace Belle2
