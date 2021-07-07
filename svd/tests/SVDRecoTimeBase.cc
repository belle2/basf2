/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDRecoTimeBase.h>
#include <vector>
#include <string>
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
    }

    /**
     * Check object creation and data getters
     */
    TEST(SVDRecoTimeBase, ConstructFromContainer)
    {
      // Create an arbitrary recotimeBase
      VxdID sensorID(3, 4, 1);
      std::vector<float> init_bins({ -5.0, -3.0, -1.0, 1.0, 3.0, 5.0});
      SVDRecoTimeBase timeBase(sensorID, false, init_bins);
      // Test getters
      EXPECT_EQ(sensorID, timeBase.getSensorID());
      EXPECT_FALSE(timeBase.isUStrip());
      EXPECT_EQ(init_bins.size() - 1, timeBase.getNBins());
      const SVDRecoTimeBase::BinEdgesArray& bins = timeBase.getBins();
      for (size_t ib = 0; ib < bins.size(); ++ib)
        EXPECT_EQ(init_bins[ib], bins[ib]);
    }
    /**
     * Check object printout
     */
    TEST(SVDRecoTimeBase, toString)
    {
      // Create an arbitrary recotimeBase
      VxdID sensorID(3, 4, 1);
      std::vector<float> init_bins({ -5.0, -3.0, -1.0, 1.0, 3.0, 5.0});
      SVDRecoTimeBase timeBase(sensorID, false, init_bins);
      // Test toString method
      EXPECT_EQ("VXDID : 25632 = 3.4.1 side: V bins: -5 -3 -1 1 3 5 ", timeBase.toString());
    }

  } // namespace SVD
}  // namespace Belle2
