#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/vxdCaTracking/ThreeHitFilters.h>
#include <gtest/gtest.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class ThreeHitFiltersTest : public ::testing::Test {
  protected:
  };


  /** Test simple Setters and Getters. */
  TEST_F(ThreeHitFiltersTest, simpleTest)
  {
    // testing deltaPt-calculator:
    TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHitEvil(6., 3., 0.), outerHitSimple(6., 4., 0.);

    ThreeHitFilters aFilter = ThreeHitFilters(outerHitSimple, centerHit, innerHit);

//    B2INFO("now comparing values of calcCircleDist2IP using simple outerHit: \n calcPt: " << aFilter.calcCircleDist2IP())
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
    EXPECT_FLOAT_EQ(0.44499173338941, aFilter.calcCircleDist2IP());

    aFilter.resetValues(outerHitEvil, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using evil outerHit: \n calcPt: " << aFilter.calcPt())
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
    EXPECT_FLOAT_EQ(0.719806016136754, aFilter.calcCircleDist2IP());

//    B2INFO("now tests with 0.976T (and reset between each step): \n")
    aFilter.resetMagneticField(0.976);
    aFilter.resetValues(outerHitSimple, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using simple outerHit: \n calcPt: " << aFilter.calcCircleDist2IP() )
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
    EXPECT_FLOAT_EQ(0.44499173338941, aFilter.calcCircleDist2IP());

    aFilter.resetValues(outerHitEvil, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using evil outerHit: \n calcPt: " << aFilter.calcPt() )
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
    EXPECT_FLOAT_EQ(0.719806016136754, aFilter.calcCircleDist2IP());
  }


  /** the correctness of the magneticField-values (important for pT-related calculations) */
  TEST_F(ThreeHitFiltersTest, TestMagneticField)
  {
    TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHitEvil(6., 3., 0.), outerHitSimple(6., 4., 0.);

    ThreeHitFilters aFilter = ThreeHitFilters(outerHitSimple, centerHit, innerHit);

    EXPECT_DOUBLE_EQ(1.5, aFilter.getMagneticField()); // standard case

    aFilter.resetMagneticField(1);
    EXPECT_DOUBLE_EQ(1., aFilter.getMagneticField());

    EXPECT_DOUBLE_EQ(26.5650511770779893515721937204532946712042, aFilter.fullAngle3D());   //angle in degrees
  }


  /** the correctness of the angle calculators */
  TEST_F(ThreeHitFiltersTest, TestAngles)
  {
//     TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHitEvil(6., 3., 0.), outerHitSimple(6., 4., 0.);
    TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHit(6., 4., 1.);
    TVector3 cent_inner = centerHit - innerHit, outer_center = outerHit - centerHit;

//    B2INFO("now tests with 1T \n")
    ThreeHitFilters aFilter = ThreeHitFilters(outerHit, centerHit, innerHit);

    EXPECT_DOUBLE_EQ(31.4821541052938556040832384555411729852856, aFilter.fullAngle3D());       //angle in degrees
    EXPECT_DOUBLE_EQ(0.090909090909090909091, aFilter.calcAngle3D());

    EXPECT_DOUBLE_EQ(26.5650511770779893515721937204532946712042, aFilter.fullAngleXY());       //angle in degrees
    EXPECT_DOUBLE_EQ(.1, aFilter.calcAngleXY());

    EXPECT_FLOAT_EQ(17.54840061379229806435203716652846677620, aFilter.fullAngleRZ());
    EXPECT_FLOAT_EQ(cos(17.54840061379229806435203716652846677620 * M_PI / 180.), aFilter.calcAngleRZ());

    EXPECT_DOUBLE_EQ(0.4636476090008061162142562314612144020285, aFilter.fullAngle2D(outer_center, cent_inner));  //angle in radians
    EXPECT_DOUBLE_EQ(0.89442719099991586, aFilter.calcAngle2D(outer_center, cent_inner));
  }


  /** test sign, helixFit and calcDeltaSlopeRZ filters */
  TEST_F(ThreeHitFiltersTest, TestSignAndOtherFilters)
  {
    TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHit(6., 4., 1.), sigma(.01, .01, .01), unrealsigma(2, 2, 2),
             outerhighHit(4., 6., 1.);

    ThreeHitFilters aFilter = ThreeHitFilters(outerHit, centerHit, innerHit);

    EXPECT_DOUBLE_EQ(0.30627736916966945608, aFilter.calcDeltaSlopeRZ());

    EXPECT_DOUBLE_EQ(0., aFilter.calcHelixFit());

    EXPECT_DOUBLE_EQ(1., aFilter.calcSign(outerHit, centerHit, innerHit, sigma, sigma, sigma));
    EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(outerhighHit, centerHit, innerHit, sigma, sigma, sigma));
    EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(innerHit, centerHit, outerHit, sigma, sigma, sigma));
    EXPECT_DOUBLE_EQ(0., aFilter.calcSign(outerHit, centerHit, innerHit, unrealsigma, unrealsigma,
                                          unrealsigma)); //for very large sigma, this track is approximately straight.

    EXPECT_LT(0., aFilter.calcSign(outerHit, centerHit, innerHit));
    EXPECT_GT(0., aFilter.calcSign(outerhighHit, centerHit, innerHit));
    EXPECT_GT(0., aFilter.calcSign(innerHit, centerHit, outerHit));
    EXPECT_LT(0., aFilter.calcSign(outerHit, centerHit, innerHit));

    EXPECT_DOUBLE_EQ(1., aFilter.calcSign(outerHit, centerHit, innerHit));
    EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(outerhighHit, centerHit, innerHit));
    EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(innerHit, centerHit, outerHit));
  }


  /** test DeltaSOverZ */
  TEST_F(ThreeHitFiltersTest, TestDeltaSOverZ)
  {
    TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 1.), outerHit(6., 4., 3.);
    TVector3 cent_inner = centerHit - innerHit, outer_center = outerHit - centerHit;
    ThreeHitFilters aFilter = ThreeHitFilters(outerHit, centerHit, innerHit);

    EXPECT_FLOAT_EQ(0.31823963, aFilter.calcDeltaSlopeZOverS());

    outerHit.RotateZ(.4);
    centerHit.RotateZ(.4);
    innerHit.RotateZ(.4);
    aFilter.resetValues(outerHit, centerHit, innerHit); //calcDeltaSOverZV2 is invariant under rotations in the r-z plane

    EXPECT_FLOAT_EQ(0.31823963, aFilter.calcDeltaSlopeZOverS());
  }


  /** test cramer method in calcPt */
  TEST_F(ThreeHitFiltersTest, TestCalcPt)
  {
    // calcCircleCenterV2 had problems when x_1==x_2 or y_1==y_2
    TVector3 innerHit(1., 2., 0.), centerHit(3., 2., 1.), outerHit(3., 4., 3.);
    TVector3 cent_inner = centerHit - innerHit, outer_center = outerHit - centerHit;
    ThreeHitFilters aFilter = ThreeHitFilters();
    TVector3 innerHit2(1., 1., 0.), centerHit2(3., 3., 0.), outerHitEvil(6., 3., 0.);

    double pt = 0, ptTrue = 0;

    aFilter.resetValues(outerHit, centerHit, innerHit);
    ptTrue = aFilter.calcPt(1.414213562373095048801688724209698078570);
    aFilter.resetValues(outerHit, centerHit, innerHit);
    pt = aFilter.calcPt();
    EXPECT_DOUBLE_EQ(ptTrue, pt);

    ptTrue = 0.017118925181688543;
    aFilter.resetValues(outerHitEvil, centerHit2, innerHit2);
    pt = aFilter.calcPt();
    EXPECT_DOUBLE_EQ(ptTrue, pt);

    aFilter.resetValues(outerHit, outerHit, innerHit);
    //B2WARNING("MUST produce errors: 2 hits are the same: " << ptTrue << ", Pt: " << pt );
    ptTrue = aFilter.calcPt(1.414213562373095048801688724209698078570);
    aFilter.resetValues(outerHit, outerHit, innerHit);
    EXPECT_ANY_THROW(aFilter.calcPt());
  }
}  // namespace
