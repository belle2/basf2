/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/Angle3DSimple.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/Angle3DFull.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleXYSimple.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleXYFull.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleRZSimple.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleRZFull.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/CircleDist2IP.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/DeltaSlopeRZ.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/DeltaSlopeZoverS.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/DeltaSoverZ.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/HelixParameterFit.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/Pt.h>
#include <tracking/trackFindingVXD/FilterTools/SelectionVariableHelper.h>

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
      B2INFO("ResultsObserver: Filter " << filterType.name() << " got result of " << fResult)
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


    Filter< Angle3DSimple<SpacePoint>, Range<double, double>, ResultsObserver > filterAngle3D(Range<double, double>(0.09, 0.091));
    EXPECT_TRUE(filterAngle3D.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(0.090909090909090909091, lastResult); // last result is what filterAngle3D had calculated

    Filter< Angle3DFull<SpacePoint>, Range<double, double>, ResultsObserver > filterAngle3Dfull(Range<double, double>(31.48, 31.49));
    EXPECT_TRUE(filterAngle3Dfull.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(31.4821541052938556040832384555411729852856, lastResult);


    Filter< AngleXYFull<SpacePoint>, Range<double, double>, ResultsObserver > filterAngleXYfull(Range<double, double>(26.5, 26.6));
    EXPECT_TRUE(filterAngleXYfull.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(26.5650511770779893515721937204532946712042, lastResult);

    Filter< AngleXYSimple<SpacePoint>, Range<double, double>, ResultsObserver > filterAngleXY(Range<double, double>(0.099, 0.101));
    EXPECT_TRUE(filterAngleXY.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(0.1, lastResult);


    Filter< AngleRZFull<SpacePoint>, Range<double, double>, ResultsObserver > filterAngleRZfull(Range<double, double>(17.5, 17.6));
    EXPECT_TRUE(filterAngleRZfull.accept(outerSP, centerSP, innerSP));
    EXPECT_NEAR(17.548400613792298064, lastResult, 0.001);

    Filter< AngleRZSimple<SpacePoint>, Range<double, double>, ResultsObserver > filterAngleRZ(Range<double, double>(0.94, 0.96));
    EXPECT_TRUE(filterAngleRZ.accept(outerSP, centerSP, innerSP));
    EXPECT_NEAR(cos(17.54840061379229806435203716652846677620 * M_PI / 180.), lastResult, 0.001);
  }



  /** test sign, helixFit and calcDeltaSlopeRZ filters */
  TEST_F(ThreeHitFilterTest, TestSignAndOtherFilters)
  {
    SpacePoint outerSP = provideSpacePointDummy(6., 4., 1.);
    SpacePoint centerSP = provideSpacePointDummy(3., 3., 0.);
    SpacePoint innerSP = provideSpacePointDummy(1., 1., 0.);
    SpacePoint outerHighSP = provideSpacePointDummy(4., 6., 1.);
    B2Vector3<float> sigma(.01, .01, .01), unrealsigma(2, 2, 2);

    typedef SelVarHelper<SpacePoint, float> Helper;


    Filter< DeltaSlopeRZ<SpacePoint>, Range<double, double>, ResultsObserver > filterDeltaSlopeRZ(Range<double, double>(0.3, 0.31));
    EXPECT_TRUE(filterDeltaSlopeRZ.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(0.30627736916966945608, lastResult);

    Filter< HelixParameterFit<SpacePoint>, Range<double, double>, ResultsObserver > filterHelixFit(Range<double, double>(-0.01, 0.01));
    EXPECT_TRUE(filterHelixFit.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(0., lastResult);

    EXPECT_DOUBLE_EQ(1., Helper::calcSign(outerSP, centerSP, innerSP, sigma, sigma, sigma));
    EXPECT_DOUBLE_EQ(-1., Helper::calcSign(outerHighSP, centerSP, innerSP, sigma, sigma, sigma));
    EXPECT_DOUBLE_EQ(-1., Helper::calcSign(innerSP, centerSP, outerSP, sigma, sigma, sigma));
    //for very large sigma, this track is approximately straight:
    EXPECT_DOUBLE_EQ(0., Helper::calcSign(outerSP, centerSP, innerSP, unrealsigma, unrealsigma, unrealsigma));

    EXPECT_LT(0., Helper::calcSign(outerSP, centerSP, innerSP));
    EXPECT_GT(0., Helper::calcSign(outerHighSP, centerSP, innerSP));
    EXPECT_GT(0., Helper::calcSign(innerSP, centerSP, outerSP));
    EXPECT_LT(0., Helper::calcSign(outerSP, centerSP, innerSP));

    EXPECT_DOUBLE_EQ(1., Helper::calcSign(outerSP, centerSP, innerSP));
    EXPECT_DOUBLE_EQ(-1., Helper::calcSign(outerHighSP, centerSP, innerSP));
    EXPECT_DOUBLE_EQ(-1., Helper::calcSign(innerSP, centerSP, outerSP));
  }



  /** test DeltaSOverZ, DeltaSlopeZoverS */
  TEST_F(ThreeHitFilterTest, TestDeltaSOverZ)
  {
    lastResult = 0;
    B2Vector3<float> iVec(1., 1., 0.), cVec(3., 3., 1.), oVec(6., 4., 3.);
    SpacePoint outerSP = provideSpacePointDummy(oVec.X(), oVec.Y(), oVec.Z());
    SpacePoint centerSP = provideSpacePointDummy(cVec.X(), cVec.Y(), cVec.Z());
    SpacePoint innerSP = provideSpacePointDummy(iVec.X(), iVec.Y(), iVec.Z());

    Filter< DeltaSlopeZoverS<SpacePoint>, Range<double, double>, ResultsObserver > filterDeltaSlopeZOverS(Range<double, double>(0.31,
        0.32));
    EXPECT_TRUE(filterDeltaSlopeZOverS.accept(outerSP, centerSP, innerSP));
    EXPECT_NEAR(0.31823963, lastResult, 0.00001);

    Filter< DeltaSoverZ<SpacePoint>, Range<double, double>, ResultsObserver > filterDeltaSOverZ(Range<double, double>(-0.39, -0.38));
    EXPECT_TRUE(filterDeltaSOverZ.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(-0.38471845, lastResult);


    //calcDeltaSOverZV2 is invariant under rotations in the r-z plane:
    oVec.RotateZ(.4);
    cVec.RotateZ(.4);
    iVec.RotateZ(.4);
    SpacePoint outerSP2 = provideSpacePointDummy(oVec.X(), oVec.Y(), oVec.Z());
    SpacePoint centerSP2 = provideSpacePointDummy(cVec.X(), cVec.Y(), cVec.Z());
    SpacePoint innerSP2 = provideSpacePointDummy(iVec.X(), iVec.Y(), iVec.Z());

    EXPECT_TRUE(filterDeltaSlopeZOverS.accept(outerSP2, centerSP2, innerSP2));
    EXPECT_NEAR(0.31823963, lastResult, 0.00001);

    EXPECT_TRUE(filterDeltaSOverZ.accept(outerSP2, centerSP2, innerSP2));
    EXPECT_FLOAT_EQ(-0.38471845, lastResult);
  }



  /** test cramer method in calcPt */
  TEST_F(ThreeHitFilterTest, TestCalcPt)
  {
    typedef SelVarHelper<SpacePoint, float> Helper;

    SpacePoint outerSP = provideSpacePointDummy(3., 4., 3.);
    SpacePoint centerSP = provideSpacePointDummy(3., 2., 1.);
    SpacePoint innerSP = provideSpacePointDummy(1., 2., 0.);

    SpacePoint outerSPEvil = provideSpacePointDummy(6., 3., 0.);
    SpacePoint centerSP2 = provideSpacePointDummy(3., 3., 0.);
    SpacePoint innerSP2 = provideSpacePointDummy(1., 1., 0.);

    double ptTrue = 0;

    ptTrue = Helper::calcPt(1.414213562373095048801688724209698078570);
    Filter< Pt<SpacePoint>, Range<double, double>, ResultsObserver > filtePt(Range<double, double>(0.005, 0.05));
    EXPECT_TRUE(filtePt.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(ptTrue, lastResult);

    ptTrue = 0.017118925181688543;
    EXPECT_TRUE(filtePt.accept(outerSPEvil, centerSP2, innerSP2));
    EXPECT_FLOAT_EQ(ptTrue, lastResult);

    //B2WARNING("MUST produce errors: 2 hits are the same: " << ptTrue << ", Pt: " << pt );
    EXPECT_ANY_THROW(filtePt.accept(outerSP, outerSP, innerSP));

    Helper::resetMagneticField(0.976);
    double ptTrue1p5 = ptTrue;
    ptTrue = Helper::calcPt(1.414213562373095048801688724209698078570);
    EXPECT_FALSE(filtePt.accept(outerSP, centerSP, innerSP));
    EXPECT_FLOAT_EQ(ptTrue, lastResult);
    EXPECT_LT(ptTrue, ptTrue1p5);

    Helper::resetMagneticField(1.5);
  }



  /** Test CircleDist2IP. */
  TEST_F(ThreeHitFilterTest, TestCircleDist2IP)
  {
    SpacePoint outerSPEvil = provideSpacePointDummy(6., 3., 0.);
    SpacePoint outerSPSimple = provideSpacePointDummy(6., 4., 0.);
    SpacePoint centerSP = provideSpacePointDummy(3., 3., 0.);
    SpacePoint innerSP = provideSpacePointDummy(1., 1., 0.);
//     TVector3 innerHit(1., 1., 0.), centerHit(3., 3., 0.), outerHitEvil(6., 3., 0.), outerHitSimple(6., 4., 0.);

//     ThreeHitFilters aFilter = ThreeHitFilters(outerHitSimple, centerHit, innerHit);
    Filter< CircleDist2IP<SpacePoint>, Range<double, double>, ResultsObserver > filteCircleDist2IP(Range<double, double>(0.44, 0.45));
    EXPECT_TRUE(filteCircleDist2IP.accept(outerSPSimple, centerSP, innerSP));
    EXPECT_NEAR(0.44499173338941, lastResult, 0.00001);

//    B2INFO("now comparing values of calcCircleDist2IP using simple outerHit: \n calcPt: " << aFilter.calcCircleDist2IP())
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
//     EXPECT_FLOAT_EQ(0.44499173338941, aFilter.calcCircleDist2IP());

//     aFilter.resetValues(outerHitEvil, centerHit, innerHit);
    EXPECT_FALSE(filteCircleDist2IP.accept(outerSPEvil, centerSP, innerSP));
    EXPECT_FLOAT_EQ(0.719806016136754, lastResult);
//    B2INFO("now comparing values of calcCircleDist2IP using evil outerHit: \n calcPt: " << aFilter.calcPt())
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
//     EXPECT_FLOAT_EQ(0.719806016136754, aFilter.calcCircleDist2IP());

//    B2INFO("now tests with 0.976T (and reset between each step): \n")
    typedef SelVarHelper<SpacePoint, float> Helper;
    Helper::resetMagneticField(0.976);

    EXPECT_TRUE(filteCircleDist2IP.accept(outerSPSimple, centerSP, innerSP));
    EXPECT_NEAR(0.44499173338941, lastResult, 0.00001);

//     aFilter.resetMagneticField(0.976);
//     aFilter.resetValues(outerHitSimple, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using simple outerHit: \n calcPt: " << aFilter.calcCircleDist2IP() )
//    aFilter.resetValues(outerHitSimple,centerHit,innerHit);
//     EXPECT_FLOAT_EQ(0.44499173338941, aFilter.calcCircleDist2IP());


    EXPECT_FALSE(filteCircleDist2IP.accept(outerSPEvil, centerSP, innerSP));
    EXPECT_FLOAT_EQ(0.719806016136754, lastResult);

//     aFilter.resetValues(outerHitEvil, centerHit, innerHit);
//    B2INFO("now comparing values of calcCircleDist2IP using evil outerHit: \n calcPt: " << aFilter.calcPt() )
//    aFilter.resetValues(outerHitEvil,centerHit,innerHit);
//     EXPECT_FLOAT_EQ(0.719806016136754, aFilter.calcCircleDist2IP());

    Helper::resetMagneticField(1.5);
  }

}
