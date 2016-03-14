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
#include <tracking/trackFindingVXD/FourHitFilters/DeltaPt.h>
#include <tracking/trackFindingVXD/FourHitFilters/DeltaDistCircleCenter.h>
#include <tracking/trackFindingVXD/FilterTools/SelectionVariableHelper.h>

#include <tracking/trackFindingVXD/FilterTools/Shortcuts.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <math.h>


using namespace std;
using namespace Belle2;

namespace VXDTFfourHitFilterTest {

  /** Test class for these new and shiny two-hit-filters. */
  class FourHitFilterTest : public ::testing::Test {
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
  TEST_F(FourHitFilterTest, SpacePointCreation)
  {
    SpacePoint testSP = provideSpacePointDummy(1.2, 2.3, 4.2);
    EXPECT_FLOAT_EQ(1.2, testSP.getPosition().X()) ;
    EXPECT_FLOAT_EQ(2.3, testSP.getPosition().Y()) ;
    EXPECT_FLOAT_EQ(4.2, testSP.getPosition().Z()) ;
  }


  /** shows the functionality of the auto naming capability of the Filter */
  TEST_F(FourHitFilterTest, SelectionVariableName)
  {
    EXPECT_EQ("Belle2__DeltaDistCircleCenter{Belle2__SpacePoint}" , DeltaDistCircleCenter<SpacePoint>().name());
    EXPECT_EQ("Belle2__DeltaPt{Belle2__SpacePoint}" , DeltaPt<SpacePoint>().name());
  }


  TEST_F(FourHitFilterTest, TestDeltaPtAndDeltaDistCircleCenter)
  {
    SpacePoint outerSP = provideSpacePointDummy(0, 0, 0.);
    SpacePoint outerCenterSP = provideSpacePointDummy(-2, 0, 0.);
    SpacePoint innerCenterSP = provideSpacePointDummy(0, 2, 0.);
    SpacePoint innerSP = provideSpacePointDummy(2, 0, 0.);


    Filter< DeltaDistCircleCenter<SpacePoint>, Range<double, double>, ResultsObserver > filterDeltaDistCircleCenter(
      Range<double, double>(1.41, 1.42));
    EXPECT_TRUE(filterDeltaDistCircleCenter.accept(outerSP, outerCenterSP, innerCenterSP, innerSP));
    EXPECT_FLOAT_EQ(sqrt(2), lastResult);

    Filter< DeltaPt<SpacePoint>, Range<double, double>, ResultsObserver > filteDeltaPt(Range<double, double>(0.002, 0.003));
    EXPECT_TRUE(filteDeltaPt.accept(outerSP, outerCenterSP, innerCenterSP, innerSP));
    EXPECT_FLOAT_EQ(0.00263349, lastResult);

  }

}

