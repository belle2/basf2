/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <mdst/dbobjects/BeamSpot.h>
#include <gtest/gtest.h>

#include <limits>
#include <functional>

namespace Belle2 {
  /** */
  class BeamSpotTest : public ::testing::Test {
  protected:
  };

  /** Constructor, copy constructor, comparison, setters and getters */
  TEST_F(BeamSpotTest, Basic)
  {
    BeamSpot bs;
    EXPECT_EQ(bs.getIPPosition().X() , 0.);
    EXPECT_EQ(bs.getIPPosition().Y() , 0.);
    EXPECT_EQ(bs.getIPPosition().Z() , 0.);

    std::function<float (int, int)> sizeElement = [ & sizeElement](int i, int j) {
      return j >= i ? (i + 1) + 10 * (j + 1) : sizeElement(j, i) ;
    };

    std::function<float (int, int)> errorElement = [& errorElement](int i, int j) {
      return j >= i ? 100 + (i + 1) + 10 * (j + 1) : errorElement(j, i);
    };

    TMatrixDSym size(3), positionError(3);
    for (int i = 0; i < 3; i++)
      for (int j = i; j < 3; j++) {
        size(i, j) = size(j, i) = sizeElement(i, j);
        positionError(i, j) = positionError(j, i) = errorElement(i, j);
      }

    TVector3 position;
    position.SetXYZ(1., 2., 3.);

    bs.setSizeCovMatrix(size);
    bs.setIP(position, positionError);

    TVector3 testPosition = bs.getIPPosition();
    EXPECT_EQ(testPosition.X() , 1.);
    EXPECT_EQ(testPosition.Y() , 2.);
    EXPECT_EQ(testPosition.Z() , 3.);

    TMatrixDSym testError = bs.getIPPositionCovMatrix();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        EXPECT_EQ(testError(i, j) , errorElement(i, j));

    TMatrixDSym testSize = bs.getSizeCovMatrix();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        EXPECT_EQ(testSize(i, j) , sizeElement(i, j));


    TMatrixDSym testCovVertex = bs.getCovVertex();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        EXPECT_EQ(testCovVertex(i, j) , (sizeElement(i, j) + errorElement(i, j)));

    EXPECT_EQ(bs.getGlobalUniqueID() , 1);
    BeamSpot bs2(bs);
    EXPECT_EQ(bs == bs2, true);

    BeamSpot bs3;
    bs3 = bs;
    EXPECT_EQ(bs == bs3, true);

    BeamSpot bs4;
    bs4 = bs3;
    size(2, 2) = 0.;

    bs3.setSizeCovMatrix(size);
    size(2, 2) = std::numeric_limits<double>::min();
    bs4.setSizeCovMatrix(size);

    EXPECT_EQ(bs4 == bs3, false);
  }

}
