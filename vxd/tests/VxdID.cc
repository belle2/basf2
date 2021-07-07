/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <vxd/dataobjects/VxdID.h>
#include <gtest/gtest.h>
#include <string>

using namespace std;

namespace Belle2 {

  /** Check that both constructors yield the same result */
  TEST(VxdID, Constructor)
  {
    VxdID s("1.1.1");
    VxdID l(1, 1, 1);
    EXPECT_EQ(l, s);
    EXPECT_EQ((string)l, "1.1.1");
  }

  /** Check the string conversion of the VxdID in both directions */
  TEST(VxdID, FromString)
  {
    //Check that all possible values can be converted back and forth
    for (int layer = 0; layer <= VxdID::MaxLayer; ++layer) {
      for (int ladder = 0; ladder <= VxdID::MaxLadder; ++ladder) {
        for (int sensor = 0; sensor <= VxdID::MaxSensor; ++sensor) {
          for (int segment = 0; segment <= VxdID::MaxSegment; ++segment) {
            string sid = (string)VxdID(layer, ladder, sensor, segment);
            VxdID id(layer, ladder, sensor, segment);
            EXPECT_EQ(id, VxdID(sid));
            EXPECT_EQ(id.getLayerNumber(), layer);
            EXPECT_EQ(id.getLadderNumber(), ladder);
            EXPECT_EQ(id.getSensorNumber(), sensor);
            EXPECT_EQ(id.getSegmentNumber(), segment);
          }
        }
      }
    }

    //Check some invalid ids
    EXPECT_THROW(VxdID("1.f"), invalid_argument);
    EXPECT_THROW(VxdID("1.1.f"), invalid_argument);
    EXPECT_THROW(VxdID("1.*.f"), invalid_argument);
    EXPECT_THROW(VxdID("1.1.1 and some more"), invalid_argument);
  }
}  // namespace
