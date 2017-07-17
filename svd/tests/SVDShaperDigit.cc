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
#include <vector>
#include <limits>
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
      EXPECT_EQ(0, digit.getPipelineAddress());
    }

    /**
     * Check object creation and data getters using a stl container of samples.
     */
    TEST(SVDShaperDigit, ConstructFromContainer)
    {
      // Create an arbitrary 6-digit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;
      char digitTime(-16);
      unsigned char digitPipelineAddress(3);

      std::vector<int> init_samples({0, 5, 10, 9, 6, 5});
      SVDShaperDigit digit(sensorID, false, cellID, init_samples, digitTime, digitPipelineAddress);
      // Test getters
      EXPECT_EQ(sensorID, digit.getSensorID());
      EXPECT_FALSE(digit.isUStrip());
      EXPECT_EQ(cellID, digit.getCellID());
      const SVDShaperDigit::APVFloatSamples& samples = digit.getSamples();
      for (size_t isample = 0; isample < SVDShaperDigit::c_nAPVSamples; ++isample)
        EXPECT_EQ(static_cast<SVDShaperDigit::APVFloatSampleType>(init_samples[isample]),
                  samples[isample]);
      EXPECT_EQ(static_cast<float>(digitTime), digit.getTime());
      EXPECT_EQ(static_cast<unsigned short>(digitPipelineAddress), digit.getPipelineAddress());
    }

    /**
     * Check object creation and data getters using a c-array of samples.
     */
    TEST(SVDShaperDigit, ConstructFromCArray)
    {
      // Create an arbitrary 6-digit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;
      char digitTime(-16);
      unsigned char digitPipelineAddress(3);

      // floats must work, too.
      float init_samples[SVDShaperDigit::c_nAPVSamples] = {0, 5, 10, 9, 6, 5};
      SVDShaperDigit digit(
        sensorID, false, cellID, init_samples, digitTime, digitPipelineAddress
      );
      // Test getters
      EXPECT_EQ(sensorID, digit.getSensorID());
      EXPECT_FALSE(digit.isUStrip());
      EXPECT_EQ(cellID, digit.getCellID());
      const SVDShaperDigit::APVFloatSamples& samples = digit.getSamples();
      for (size_t isample = 0; isample < SVDShaperDigit::c_nAPVSamples; ++isample)
        EXPECT_EQ(static_cast<SVDShaperDigit::APVFloatSampleType>(init_samples[isample]),
                  samples[isample]);
      EXPECT_EQ(static_cast<float>(digitTime), digit.getTime());
      EXPECT_EQ(static_cast<unsigned short>(digitPipelineAddress), digit.getPipelineAddress());
    }
    /**
     * Check standard object creation without FADC time and pipeline addresss.
     */
    TEST(SVDShaperDigit, ConstructWoTime)
    {
      // Create an arbitrary 6-digit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;

      double init_samples[6] = {0, 5, 10, 9, 6, 5};
      SVDShaperDigit digit(sensorID, false, cellID, init_samples);
      // Test that time and error are set correctly.
      EXPECT_EQ(0.0, digit.getTime());
      EXPECT_EQ(0, digit.getPipelineAddress());
    }

    TEST(SVDShaperDigit, SampleTrimming)
    {

      // Create an arbitrary 6-digit
      VxdID sensorID(3, 4, 1);
      short int cellID = 132;

      auto sampleMin =
        static_cast<SVDShaperDigit::APVFloatSampleType>(std::numeric_limits<SVDShaperDigit::APVRawSampleType>::lowest());
      auto sampleMax =
        static_cast<SVDShaperDigit::APVFloatSampleType>(std::numeric_limits<SVDShaperDigit::APVRawSampleType>::max());

      std::vector<float> init_samples({0, -10, 255, 256, 500, -1});
      SVDShaperDigit digit(sensorID, false, cellID, init_samples);
      const auto& samples = digit.getSamples();
      for (size_t isample = 0; isample < SVDShaperDigit::c_nAPVSamples; ++isample) {
        auto trimmedSample = std::max(sampleMin, std::min(sampleMax, init_samples[isample]));
        EXPECT_EQ(trimmedSample, samples[isample]);
      }
    }

    TEST(SVDShaperDigit, TrimmingFunction)
    {

      // Create an arbitrary 6-digit
      auto sampleMin =
        static_cast<SVDShaperDigit::APVFloatSampleType>(std::numeric_limits<SVDShaperDigit::APVRawSampleType>::lowest());
      auto sampleMax =
        static_cast<SVDShaperDigit::APVFloatSampleType>(std::numeric_limits<SVDShaperDigit::APVRawSampleType>::max());

      std::vector<float> init_samples({0, -10, 255, 256, 500, -1});
      for (size_t isample = 0; isample < SVDShaperDigit::c_nAPVSamples; ++isample) {
        auto trimmedSample = std::max(sampleMin, std::min(sampleMax, init_samples[isample]));
        EXPECT_EQ(trimmedSample, SVDShaperDigit::trimToSampleRange(init_samples[isample]));
      }
    }

  } // namespace SVD
}  // namespace Belle2
