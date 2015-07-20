/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni (eugenio.paoloni@pi.infn.it)             *
 *               Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/Angle3DSimple.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/Angle3DFull.h>

#include <tracking/trackFindingVXD/FilterTools/Shortcuts.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <math.h>


using namespace std;
using namespace Belle2;

namespace VXDTFthreeHitFilterTest {

  /** Test class for these new and shiny two-hit-filters. */
  class ThreeHitFilterTest : public ::testing::Test {
  protected:
  };



  /** this is a small helper function to create a sensorInfo to be used */
  VXD::SensorInfoBase createSensorInfo(VxdID aVxdID, double globalX = 0., double globalY = 0., double globalZ = -0.)
  {
    // (SensorType type, VxdID id, double width, double length, double thickness, int uCells, int vCells, double width2=-1, double splitLength=-1, int vCells2=0)
    VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 2.3, 4.2, 0.3, 2, 4, -1);

    TGeoRotation r1;
    r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
    TGeoTranslation t1(globalX, globalY, globalZ);
    TGeoCombiTrans c1(t1, r1);
    TGeoHMatrix transform = c1;
    sensorInfoBase.setTransformation(transform);

    return sensorInfoBase;
  }



  /** when given global coordinates, a SpacePoint lying there will be returned */
  SpacePoint provideSpacePointDummy(double X, double Y, double Z)
  {
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, X, Y, Z);

    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    SpacePoint testPoint = SpacePoint(&aCluster, &sensorInfoBase);

    return SpacePoint(&aCluster, &sensorInfoBase);
  }



  /** simply storing last result here to be able to use it for some tests */
  double lastResult = 0.;



  /** takes result, prints it and stores it to lastResult */
  class ResultsObserver : public VoidObserver {
  public:
    template<class Var, typename ... otherTypes>
    static void notify(const Var& filterType,
                       typename Var::variableType fResult,
                       otherTypes ...)
    {
      B2INFO(" Filter " << filterType.name() << " got result of " << fResult)
      lastResult = fResult;
    }

  };


  /** presents the functionality of the SpacePoint-creating function written above */
  TEST_F(ThreeHitFilterTest, SpacePointCreation)
  {
    SpacePoint testSP = provideSpacePointDummy(1.2, 2.3, 4.2);
    EXPECT_FLOAT_EQ(1.2, testSP.getPosition().X()) ;
    EXPECT_FLOAT_EQ(2.3, testSP.getPosition().Y()) ;
    EXPECT_FLOAT_EQ(4.2, testSP.getPosition().Z()) ;
  }


  /** shows the functionality of the auto naming capability of the Filter */
  TEST_F(ThreeHitFilterTest, SelectionVariableName)
  {
    EXPECT_EQ("Belle2__Angle3DSimple{Belle2__SpacePoint}" , Angle3DSimple<SpacePoint>().name());
  }


  /** shows how to use the filter Angle3DSimple in a simple case */
  TEST_F(ThreeHitFilterTest, BasicFilterTestAngle3DSimple)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Angle3DSimple<SpacePoint>, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(.5 , 0., 0.);
    SpacePoint x3 = provideSpacePointDummy(2. , 0., 0.);

    EXPECT_FALSE(filter.accept(x1, x2, x3));

  }


  /** the correctness of the angle calculators */
  TEST_F(ThreeHitFilterTest, TestAngles)
  {
    SpacePoint outerSP = provideSpacePointDummy(6., 4., 1.);
    SpacePoint centerSP = provideSpacePointDummy(3., 3., 0.);
    SpacePoint innerSP = provideSpacePointDummy(1., 1., 0.);

//  EXPECT_DOUBLE_EQ(31.4821541052938556040832384555411729852856, aFilter.fullAngle3D());       //angle in degrees
//  EXPECT_DOUBLE_EQ(0.090909090909090909091, aFilter.calcAngle3D());
    Filter< Angle3DSimple<SpacePoint>, Range<double, double>, ResultsObserver > filterAngle3D(Range<double, double>(0.09, 0.091));
    EXPECT_TRUE(filterAngle3D.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(0.090909090909090909091, lastResult); // last result is what filterAngle3D had calculated


    Filter< Angle3DFull<SpacePoint>, Range<double, double>, ResultsObserver > filterAngle3Dfull(Range<double, double>(31.48, 31.49));
    EXPECT_TRUE(filterAngle3Dfull.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(31.4821541052938556040832384555411729852856, lastResult); // last result is what filterFullAngle3D had calculated

//  EXPECT_DOUBLE_EQ(26.5650511770779893515721937204532946712042, aFilter.fullAngleXY());       //angle in degrees
//  EXPECT_DOUBLE_EQ(.1, aFilter.calcAngleXY());

//  EXPECT_FLOAT_EQ(17.54840061379229806435203716652846677620, aFilter.fullAngleRZ());
//  EXPECT_FLOAT_EQ(cos(17.54840061379229806435203716652846677620 * M_PI / 180.), aFilter.calcAngleRZ());

//  EXPECT_DOUBLE_EQ(0.4636476090008061162142562314612144020285, aFilter.fullAngle2D(outer_center, cent_inner));  //angle in radians
//  EXPECT_DOUBLE_EQ(0.89442719099991586, aFilter.calcAngle2D(outer_center, cent_inner));
  }



  /** test sign, helixFit and calcDeltaSlopeRZ filters */
  TEST_F(ThreeHitFilterTest, TestSignAndOtherFilters)
  {
//  TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHit(6., 4., 1.), sigma(.01, .01, .01), unrealsigma(2, 2, 2),
//  outerhighHit(4., 6., 1.);
//
//  ThreeHitFilters aFilter = ThreeHitFilters(outerHit, centerHit, innerHit);
//
//  EXPECT_DOUBLE_EQ(0.30627736916966945608, aFilter.calcDeltaSlopeRZ());
//
//  EXPECT_DOUBLE_EQ(0., aFilter.calcHelixFit());
//
//  EXPECT_DOUBLE_EQ(1., aFilter.calcSign(outerHit, centerHit, innerHit, sigma, sigma, sigma));
//  EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(outerhighHit, centerHit, innerHit, sigma, sigma, sigma));
//  EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(innerHit, centerHit, outerHit, sigma, sigma, sigma));
//  EXPECT_DOUBLE_EQ(0., aFilter.calcSign(outerHit, centerHit, innerHit, unrealsigma, unrealsigma,
//                      unrealsigma)); //for very large sigma, this track is approximately straight.
//
//  EXPECT_LT(0., aFilter.calcSign(outerHit, centerHit, innerHit));
//  EXPECT_GT(0., aFilter.calcSign(outerhighHit, centerHit, innerHit));
//  EXPECT_GT(0., aFilter.calcSign(innerHit, centerHit, outerHit));
//  EXPECT_LT(0., aFilter.calcSign(outerHit, centerHit, innerHit));
//
//  EXPECT_DOUBLE_EQ(1., aFilter.calcSign(outerHit, centerHit, innerHit));
//  EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(outerhighHit, centerHit, innerHit));
//  EXPECT_DOUBLE_EQ(-1., aFilter.calcSign(innerHit, centerHit, outerHit));
  }



  /** test DeltaSOverZ */
  TEST_F(ThreeHitFilterTest, TestDeltaSOverZ)
  {
//  TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 1.), outerHit(6., 4., 3.);
//  TVector3 cent_inner = centerHit - innerHit, outer_center = outerHit - centerHit;
//  ThreeHitFilters aFilter = ThreeHitFilters(outerHit, centerHit, innerHit);
//
//  EXPECT_FLOAT_EQ(0.31823963, aFilter.calcDeltaSlopeZOverS());
//
//  outerHit.RotateZ(.4);
//  centerHit.RotateZ(.4);
//  innerHit.RotateZ(.4);
//  aFilter.resetValues(outerHit, centerHit, innerHit); //calcDeltaSOverZV2 is invariant under rotations in the r-z plane
//
//  EXPECT_FLOAT_EQ(0.31823963, aFilter.calcDeltaSlopeZOverS());
  }



  /** test cramer method in calcPt */
  TEST_F(ThreeHitFilterTest, TestCalcPt)
  {
//  // calcCircleCenterV2 had problems when x_1==x_2 or y_1==y_2
//  TVector3 innerHit(1., 2., 0.), centerHit(3., 2., 1.), outerHit(3., 4., 3.);
//  TVector3 cent_inner = centerHit - innerHit, outer_center = outerHit - centerHit;
//  ThreeHitFilters aFilter = ThreeHitFilters();
//  TVector3 innerHit2(1., 1., 0.), centerHit2(3., 3., 0.), outerHitEvil(6., 3., 0.);
//
//  double pt = 0, ptTrue = 0;
//
//  aFilter.resetValues(outerHit, centerHit, innerHit);
//  ptTrue = aFilter.calcPt(1.414213562373095048801688724209698078570);
//  aFilter.resetValues(outerHit, centerHit, innerHit);
//  pt = aFilter.calcPt();
//  EXPECT_DOUBLE_EQ(ptTrue, pt);
//
//  ptTrue = 0.017118925181688543;
//  aFilter.resetValues(outerHitEvil, centerHit2, innerHit2);
//  pt = aFilter.calcPt();
//  EXPECT_DOUBLE_EQ(ptTrue, pt);
//
//  aFilter.resetValues(outerHit, outerHit, innerHit);
//  //B2WARNING("MUST produce errors: 2 hits are the same: " << ptTrue << ", Pt: " << pt );
//  ptTrue = aFilter.calcPt(1.414213562373095048801688724209698078570);
//  aFilter.resetValues(outerHit, outerHit, innerHit);
//  EXPECT_ANY_THROW(aFilter.calcPt());
  }











  /** shows how to attach an observer to a filter of interest */
  TEST_F(ThreeHitFilterTest, ObservedFilter)
  {
//  // Very verbose declaration, see below for convenient shortcuts
//  Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(0., 1.));
//
//  Filter< Distance3DSquared<SpacePoint>, Range<double, double>, CountingObserver > filter(unobservedFilter);
//  SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
//  SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
//  SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
//  counter< Distance3DSquared<SpacePoint> >::resetCounter();
//
//  EXPECT_TRUE(filter.accept(x1, x2));
//  EXPECT_FALSE(filter.accept(x1, x3));
//  EXPECT_EQ(2 , counter< Distance3DSquared<SpacePoint> >::used);
  }


  /** shows how to bypass a filter which itself was not initially planned to be bypassed */
  TEST_F(ThreeHitFilterTest, BypassableFilter)
  {
//  bool bypassControl(false);
//  // Very verbose declaration, see below for convenient shortcuts
//  Filter< Distance3DSquared<SpacePoint>, Range<double, double>, CountingObserver > nonBypassableFilter(Range<double, double>(0., 1.));
//  auto filter = nonBypassableFilter.bypass(bypassControl);
//  SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
//  SpacePoint x2 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
//  counter< Distance3DSquared<SpacePoint> >::resetCounter();
//
//  EXPECT_FALSE(filter.accept(x1, x2));
//  EXPECT_EQ(1 , counter< Distance3DSquared<SpacePoint> >::used);
//
//  bypassControl = true;
//  EXPECT_TRUE(filter.accept(x1, x2));
//  EXPECT_EQ(2 , counter< Distance3DSquared<SpacePoint> >::used);
//
  }


  /** shows how to write compact code using the new filter design */
  TEST_F(ThreeHitFilterTest, Shortcuts)
  {
    /*
      SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
      SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
      SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

      auto filterSup = (Distance3DSquared<SpacePoint>() < 1.) ;
      EXPECT_TRUE(filterSup.accept(x1, x2));
      EXPECT_FALSE(filterSup.accept(x1, x3));

      auto filterSup2 = (1 > Distance3DSquared<SpacePoint>()) ;
      EXPECT_TRUE(filterSup2.accept(x1, x2));
      EXPECT_FALSE(filterSup2.accept(x1, x3));

      auto filterInf = (Distance3DSquared<SpacePoint>() > 1.) ;
      EXPECT_TRUE(filterInf.accept(x1, x3));
      EXPECT_FALSE(filterInf.accept(x1, x2));

      auto filterInf2 = (1 < Distance3DSquared<SpacePoint>()) ;
      EXPECT_TRUE(filterInf2.accept(x1, x3));
      EXPECT_FALSE(filterInf2.accept(x1, x2));

      auto filterRange = (0.1 < Distance3DSquared<SpacePoint>() < 1);
      EXPECT_FALSE(filterRange.accept(x1, x1));
      EXPECT_TRUE(filterRange.accept(x1, x2));
      EXPECT_FALSE(filterRange.accept(x1, x3));
      */
  }


  /** tests compatibility of filters with boolean operations for easy coupling of filters */
  TEST_F(ThreeHitFilterTest, BooleanOperations)
  {
    /*
      SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
      SpacePoint x2 = provideSpacePointDummy(1.0f , 0.0f, 0.0f);
      SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

      auto filter = !(Distance3DSquared<SpacePoint>() > 1.);
      EXPECT_TRUE(filter.accept(x1, x2));
      EXPECT_TRUE(filter.accept(x1, x1));
      EXPECT_FALSE(filter.accept(x1, x3));

      auto filter2 =
      !(Distance3DSquared<SpacePoint>() > 1.) &&
      !(Distance3DSquared<SpacePoint>() < 1);
      // i.e. Distance3DSquared == 1
      EXPECT_TRUE(filter2.accept(x1, x2));
      EXPECT_FALSE(filter2.accept(x1, x1));
      EXPECT_FALSE(filter2.accept(x1, x3));


      auto filter3 =
      (Distance3DSquared<SpacePoint>() > 1.) ||
      (Distance3DSquared<SpacePoint>() < 1);
      // i.e. Distance3DSquared != 1
      EXPECT_FALSE(filter3.accept(x1, x2));
      EXPECT_TRUE(filter3.accept(x1, x1));
      EXPECT_TRUE(filter3.accept(x1, x3));
      */
  }


  /** evaluating compatibility of filters with lazy evaluation */
  TEST_F(ThreeHitFilterTest, ShortCircuitsEvaluation)
  {
//  auto filter(
//    (Distance2DXYSquared<SpacePoint>() < 1).observe(CountingObserver()) &&
//    (Distance3DSquared<SpacePoint>()   < 1).observe(CountingObserver())
//  );
//
//  SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
//  SpacePoint x2 = provideSpacePointDummy(1.0f , 0.0f, 0.0f);
//  SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
//
//  counter< Distance3DSquared<SpacePoint>   >::used = 0;
//  counter< Distance2DXYSquared<SpacePoint> >::used = 0;
//
//  EXPECT_FALSE(filter.accept(x1, x3));
//  EXPECT_EQ(1 , counter< Distance2DXYSquared<SpacePoint> >::used);
//  EXPECT_EQ(0 , counter< Distance3DSquared<SpacePoint> >::used);
//
//  EXPECT_TRUE(filter.accept(x1, x1));
//  EXPECT_EQ(2 , counter< Distance2DXYSquared<SpacePoint> >::used);
//  EXPECT_EQ(1 , counter< Distance3DSquared<SpacePoint> >::used);
//
  }

}
