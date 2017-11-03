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
#include <svd/dataobjects/SVDRecoTimeBase.h>
#include <vector>
#include <string>
#include <cmath>
#include <gtest/gtest.h>

namespace Belle2 {
  namespace SVD {

    /**
     * Check empty object creation and data getters.
     */
    TEST(SVDRecoTimeBase, ConstructEmpty)
    {
      // Create an empty time base
      SVDRecoTimeBase timeBase;
      // Test getters
      EXPECT_EQ(0, timeBase.getSensorID());
      EXPECT_TRUE(timeBase.isUStrip());
      const SVDRecoTimeBase::BinEdgesArray bins = timeBase.getBins();
      EXPECT_EQ(0, timeBase.getNBins());
      for (auto  bin : bins)
        EXPECT_EQ(SVDRecoTimeBase::BinnedDataType(0.0), bin);
      EXPECT_EQ(SVDModeByte::c_DefaultID, timeBase.getModeByte().getID());
      EXPECT_EQ("0-suppr/global/6 samples/???", std::string(timeBase.getModeByte()));
    }

    /**
     * Check object creation and data getters
     */
    TEST(SVDRecoTimeBase, ConstructFromContainer)
    {
      // Create an arbitrary recotimeBase
      VxdID sensorID(3, 4, 1);
      SVDModeByte timeBaseModeByte(151);
      std::vector<float> init_bins({ -5.0, -3.0, -1.0, 1.0, 3.0, 5.0});
      SVDRecoTimeBase timeBase(sensorID, false, init_bins, timeBaseModeByte);
      // Test getters
      EXPECT_EQ(sensorID, timeBase.getSensorID());
      EXPECT_FALSE(timeBase.isUStrip());
      EXPECT_EQ(init_bins.size() - 1, timeBase.getNBins());
      const SVDRecoTimeBase::BinEdgesArray& bins = timeBase.getBins();
      for (size_t ib = 0; ib < bins.size(); ++ib)
        EXPECT_EQ(init_bins[ib], bins[ib]);
      EXPECT_EQ(timeBaseModeByte, timeBase.getModeByte());
      EXPECT_EQ("0-suppr/global/6 samples/???", std::string(timeBase.getModeByte()));
    }
    /**
     * Check object printout
     */
    TEST(SVDRecoTimeBase, toString)
    {
      // Create an arbitrary recotimeBase
      VxdID sensorID(3, 4, 1);
      SVDModeByte timeBaseModeByte(151);
      std::vector<float> init_bins({ -5.0, -3.0, -1.0, 1.0, 3.0, 5.0});
      SVDRecoTimeBase timeBase(sensorID, false, init_bins, timeBaseModeByte);
      // Test toString method
      EXPECT_EQ("VXDID : 25632 = 3.4.1 side: V bins: -5 -3 -1 1 3 5  mode: 0-suppr/global/6 samples/???\n", timeBase.toString());
    }

  } // namespace SVD
}  // namespace Belle2
