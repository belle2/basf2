#include <tracking/vxdCaTracking/TwoHitFilters.h>
#include <gtest/gtest.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class TwoHitFiltersTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters by filling zeroes */
  TEST_F(TwoHitFiltersTest, TestEmptyFilter)
  {
    TwoHitFilters aFilter = TwoHitFilters();

    EXPECT_DOUBLE_EQ(0., aFilter.calcDist3D());

    EXPECT_DOUBLE_EQ(0., aFilter.fullDist3D());

    EXPECT_DOUBLE_EQ(0., aFilter.calcDistXY());

    EXPECT_DOUBLE_EQ(0., aFilter.fullDistXY());

    EXPECT_DOUBLE_EQ(0., aFilter.calcDistZ());

    EXPECT_DOUBLE_EQ(0., aFilter.calcSlopeRZ());

    EXPECT_DOUBLE_EQ(0., aFilter.calcNormedDist3D());

    EXPECT_DOUBLE_EQ(42., aFilter.filterNan(42.));

    EXPECT_DOUBLE_EQ(42, aFilter.filterNan(42));

    EXPECT_DOUBLE_EQ(42., aFilter.filterNan(42));

    EXPECT_DOUBLE_EQ(0., aFilter.filterNan(1. / 0.));

  }

  /** Test simple Setters and Getters by filling non-zero-values */
  TEST_F(TwoHitFiltersTest, TestFilledFilter)
  {
    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit(2, 3, 4);

    TwoHitFilters aFilter = TwoHitFilters(outerHit, innerHit); // correct order

    EXPECT_DOUBLE_EQ(3., aFilter.calcDist3D()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(sqrt(3.), aFilter.fullDist3D()); // does calc dist (outer - innerHit)

    EXPECT_DOUBLE_EQ(2., aFilter.calcDistXY()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(sqrt(2.), aFilter.fullDistXY()); // does calc dist (outer - innerHit)calcDistXY

    EXPECT_DOUBLE_EQ(1., aFilter.calcDistZ());

    EXPECT_DOUBLE_EQ(atan(sqrt(2.)),
                     aFilter.calcSlopeRZ()); // normal slope is fullDistXY/calcDistZ = sqrt(2), here atan of slope is calculated

    EXPECT_DOUBLE_EQ(2. / 3., aFilter.calcNormedDist3D());


    TwoHitFilters bFilter =
      TwoHitFilters(); // initialising an empty filter first to check whether the resetting function is doing its job...

    bFilter.resetValues(innerHit, outerHit); // wrong order

    EXPECT_DOUBLE_EQ(aFilter.calcDist3D(), bFilter.calcDist3D()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(aFilter.fullDist3D(), bFilter.fullDist3D()); // does calc dist (outer - innerHit)

    EXPECT_DOUBLE_EQ(aFilter.calcDistXY(), bFilter.calcDistXY()); // does calc dist (outer - innerHit)^2!

    EXPECT_DOUBLE_EQ(aFilter.fullDistXY(), bFilter.fullDistXY()); // does calc dist (outer - innerHit)

    EXPECT_DOUBLE_EQ(-aFilter.calcDistZ(), bFilter.calcDistZ());

    EXPECT_DOUBLE_EQ(-aFilter.calcSlopeRZ(), bFilter.calcSlopeRZ());

    EXPECT_DOUBLE_EQ(aFilter.calcNormedDist3D(), bFilter.calcNormedDist3D());
  }



  /** Test simple Setters and Getters by filling extreme values */
  TEST_F(TwoHitFiltersTest, testLargeFilter)
  {
    TVector3 innerHit(1e150, 0, 0);
    TVector3 outerHit(0, 0, 0);
    TVector3 innerHit2(1, 2, 3);
    TVector3 outerHit2(2, 3, 4);

    TwoHitFilters aFilter = TwoHitFilters(outerHit, innerHit); // correct order

    EXPECT_DOUBLE_EQ(1e300, aFilter.calcDist3D()); // does calc dist (outer - innerHit)^2!

    aFilter.resetValues(innerHit2, outerHit2);

    // does currently give an error at the clang-build:
//     EXPECT_DOUBLE_EQ(-atan(sqrt(2.)), aFilter.calcSlopeRZ());
  }


  /** testing out of range behavior */
  TEST_F(TwoHitFiltersTest, testOutOfRangeFilter) //approx 1.8e308 ... largest possible value of a double
  {
    TVector3 innerHit(1e300, 0, 0);
    TVector3 outerHit(0, 0, 0);

    TwoHitFilters aFilter = TwoHitFilters(outerHit, innerHit); // correct order

//     EXPECT_DOUBLE_EQ(1e600, aFilter.calcDist3D()); // does calc dist (outer - innerHit)^2!
    EXPECT_DOUBLE_EQ(innerHit * innerHit, aFilter.calcDist3D()); // does calc dist (outer - innerHit)^2!

  }

  /** And now possibly the only case where TwoHitFilters produces wrong results. However, (1e300, 0, 1e300) will never happen. */
  TEST_F(TwoHitFiltersTest,
         TestOutOfRangeNormedDistFilter) //FAILS, because both calcDistXY() and calcDist3D() are too large to be stored in a double.
  {
    TVector3 innerHit(1e300, 0, 1e300);
    TVector3 outerHit(0, 0, 0);
    double correctResult = 1. / 2.; // this should be the result which is analytically correct
    double wrongResult = 0.; // this is the result because of out of range of double precission

    TwoHitFilters aFilter = TwoHitFilters(outerHit, innerHit); // correct order

    // however, the values exceed the range of double, therefore the result is NOT EQUAL to the correct value:
    EXPECT_NE(correctResult, aFilter.calcNormedDist3D());
    EXPECT_DOUBLE_EQ(wrongResult, aFilter.calcNormedDist3D());
  }
}  // namespace
