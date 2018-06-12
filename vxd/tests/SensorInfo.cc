#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/SensorPlane.h>
#include <framework/utilities/TestHelpers.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace VXD {

    /**
     * Check wether all the getters work for a normal, rectangular sensor
     * with only one pixel/strip size.
     */
    TEST(SensorInfoBase, Rectangular)
    {
      SensorInfoBase rect(SensorInfoBase::PXD, VxdID(1, 2, 3), 1.0, 2.0, 3.0, 2, 4);
      EXPECT_EQ(rect.getType(), SensorInfoBase::PXD);
      EXPECT_EQ(rect.getID(), VxdID(1, 2, 3));
      EXPECT_EQ(rect.getWidth(), 1.0);
      EXPECT_EQ(rect.getLength(), 2.0);
      EXPECT_EQ(rect.getThickness(), 3.0);
      EXPECT_EQ(rect.getWidth(), rect.getUSize());
      EXPECT_EQ(rect.getLength(), rect.getVSize());
      EXPECT_EQ(rect.getThickness(), rect.getWSize());
      EXPECT_EQ(rect.getUPitch(), 0.5);
      EXPECT_EQ(rect.getVPitch(), 0.5);
      EXPECT_EQ(rect.getUCellPosition(0), -0.25);
      EXPECT_EQ(rect.getUCellPosition(1), 0.25);
      EXPECT_EQ(rect.getVCellPosition(0), -0.75);
      EXPECT_EQ(rect.getVCellPosition(1), -0.25);
      EXPECT_EQ(rect.getVCellPosition(2), 0.25);
      EXPECT_EQ(rect.getVCellPosition(3), 0.75);
      for (double u = -1.0 + 0.05; u <= 1.0; u += 0.1) {
        EXPECT_EQ(rect.getUCellID(u), (int)((u / 1.0 + 0.5) * 2));
        if (u < -0.5) {EXPECT_EQ(rect.getUCellID(u, 0, true), 0); }
        if (u > 0.5) {EXPECT_EQ(rect.getUCellID(u, 0, true), rect.getUCells() - 1);}
        for (double v = -2.0 + 0.11; v <= 2.0; v += 0.2) {
          EXPECT_EQ(rect.getVCellID(v), (int)((v / 2.0 + 0.5) * 4));
          if (v < -1.0) {EXPECT_EQ(rect.getVCellID(v, true), 0);}
          if (v > 1.0) {EXPECT_EQ(rect.getVCellID(v, true), rect.getVCells() - 1);}
          EXPECT_EQ(rect.inside(u, v), fabs(u) <= 0.5 && fabs(v) <= 1.0);
        }
      }
      EXPECT_EQ(rect.getUCells(), 2);
      EXPECT_EQ(rect.getVCells(), 4);
    }

    /**
     * Check wether all the getters work for a trapezoidal sensor
     * with only one pixel/strip size.
     */
    TEST(SensorInfoBase, Trapezoidal)
    {
      SensorInfoBase rect(SensorInfoBase::PXD, VxdID(3, 2, 1), 2.0, 2.0, 1.0, 2, 4, 1.0);
      EXPECT_EQ(rect.getType(), SensorInfoBase::PXD);
      EXPECT_EQ(rect.getID(), VxdID(3, 2, 1));
      for (double v = -1.0; v < 1.0; v += 0.1) {
        EXPECT_DOUBLE_EQ(rect.getWidth(v), 2.0 - (v / 2.0 + 0.5) * 1.0);
        EXPECT_EQ(rect.getWidth(), rect.getUSize());
      }
      EXPECT_EQ(rect.getLength(), 2.0);
      EXPECT_EQ(rect.getThickness(), 1.0);
      EXPECT_EQ(rect.getLength(), rect.getVSize());
      EXPECT_EQ(rect.getThickness(), rect.getWSize());
      EXPECT_EQ(rect.getUPitch(-1.0), 1.0);
      EXPECT_EQ(rect.getUPitch(0.0), 0.75);
      EXPECT_EQ(rect.getUPitch(1.0), 0.5);
      EXPECT_EQ(rect.getVPitch(), 0.5);
      for (double u = -2.0 + 0.1; u <= 2.0; u += 0.2) {
        for (double v = -2.0 + 0.11; v <= 2.0; v += 0.2) {
          EXPECT_EQ(rect.getUCellID(u, v), (int)((u / rect.getWidth(v) + 0.5) * 2));
          EXPECT_EQ(rect.getVCellID(v), (int)((v / 2.0 + 0.5) * 4));
          EXPECT_EQ(rect.inside(u, v), fabs(u) <= rect.getWidth(v) / 2.0 && fabs(v) <= 1.0);
        }
      }
      EXPECT_EQ(rect.getUCells(), 2);
      EXPECT_EQ(rect.getVCells(), 4);
    }

    /**
     * Check wether all the getters work for a trapezoidal sensor with two
     * pixel sizes.
     */
    TEST(SensorInfoBase, Segmented)
    {
      SensorInfoBase rect(SensorInfoBase::PXD, VxdID(3, 2, 1), 2.0, 2.0, 1.0, 2, 2, 1.0, 1.0, 4);
      for (double v = -1.0; v < 1.0; v += 0.1) {
        EXPECT_DOUBLE_EQ(rect.getWidth(v), 2.0 - (v / 2.0 + 0.5) * 1.0);
        EXPECT_EQ(rect.getWidth(), rect.getUSize());
        EXPECT_EQ(rect.getVPitch(v), v >= 0 ? 0.25 : 0.5);
      }
      EXPECT_EQ(rect.getLength(), 2.0);
      EXPECT_EQ(rect.getThickness(), 1.0);
      EXPECT_EQ(rect.getLength(), rect.getVSize());
      EXPECT_EQ(rect.getThickness(), rect.getWSize());
      EXPECT_EQ(rect.getUPitch(-1.0), 1.0);
      EXPECT_EQ(rect.getUPitch(0.0), 0.75);
      EXPECT_EQ(rect.getUPitch(1.0), 0.5);
      EXPECT_EQ(rect.getVCellPosition(0), -0.75);
      EXPECT_EQ(rect.getVCellPosition(1), -0.25);
      EXPECT_EQ(rect.getVCellPosition(2), 0.125);
      EXPECT_EQ(rect.getVCellPosition(3), 0.375);
      EXPECT_EQ(rect.getVCellPosition(4), 0.625);
      EXPECT_EQ(rect.getVCellPosition(5), 0.875);
      for (double u = -2.0 + 0.1; u <= 2.0; u += 0.2) {
        for (double v = -2.0 + 0.11; v <= 2.0; v += 0.2) {
          EXPECT_EQ(rect.getUCellID(u, v), (int)((u / rect.getWidth(v) + 0.5) * 2));
          if (v <= 0) {
            EXPECT_EQ(rect.getVCellID(v), (int)((v / 2.0 + 0.5) * 4));
          } else {
            EXPECT_EQ(rect.getVCellID(v), (int)((v / 2.0 + 0.5) * 8) - 2);
          }
          EXPECT_EQ(rect.inside(u, v), fabs(u) <= rect.getWidth(v) / 2.0 && fabs(v) <= 1.0);
          double iu(u), iv(v);
          rect.forceInside(iu, iv);
          EXPECT_TRUE(rect.inside(iu, iv));
        }
      }
      EXPECT_EQ(rect.getUCells(), 2);
      EXPECT_EQ(rect.getVCells(), 6);
    }

    /**
     * Check that a SensorPlane bails if it cannot find the SensorInfo associated
     * with the plane
     * Disabled temporarily, 23/08/2016, P. Kvasnicka
     */
    TEST(SensorPlane, DISABLED_NotFound)
    {
      SensorPlane plane(VxdID(1, 1, 1));
      EXPECT_B2FATAL(plane.isInActive(0, 0));
    }

  } // vxd namespace
}  // Belle2 namespace
