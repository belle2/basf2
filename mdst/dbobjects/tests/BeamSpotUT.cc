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
    EXPECT_EQ(bs.getVertex().X() , 0.);
    EXPECT_EQ(bs.getVertex().Y() , 0.);
    EXPECT_EQ(bs.getVertex().Z() , 0.);

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

    bs.setSize(size);
    bs.setVertex(position, positionError);

    TVector3 testPosition = bs.getVertex();
    EXPECT_EQ(testPosition.X() , 1.);
    EXPECT_EQ(testPosition.Y() , 2.);
    EXPECT_EQ(testPosition.Z() , 3.);

    TMatrixDSym testError = bs.getPositionError();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        EXPECT_EQ(testError(i, j) , errorElement(i, j));

    TMatrixDSym testSize = bs.getSize();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        EXPECT_EQ(testSize(i, j) , sizeElement(i, j));

    TMatrixDSym testSize2 = bs.getCovVertex();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        EXPECT_EQ(testSize2(i, j) , sizeElement(i, j));

    EXPECT_EQ(bs.getGlobalUniqueID() , 1);
    BeamSpot bs2(bs);
    EXPECT_EQ(bs == bs2, true);

    BeamSpot bs3;
    bs3 = bs;
    EXPECT_EQ(bs == bs3, true);

    BeamSpot bs4;
    bs4 = bs3;
    size(2, 2) = 0.;

    bs3.setSize(size);
    size(2, 2) = std::numeric_limits<float>::min();
    bs4.setSize(size);

    EXPECT_EQ(bs4 == bs3, false);
  }

}
