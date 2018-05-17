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
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/filterMap/trackletVariables/ZiggZaggXY.h>
#include <tracking/trackFindingVXD/filterMap/trackletVariables/ZiggZaggXYWithSigma.h>
#include <tracking/trackFindingVXD/filterMap/trackletVariables/ZiggZaggRZ.h>
#include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <math.h>


using namespace std;
using namespace Belle2;

namespace VXDTFtrackletFilterTest {

  /** Test class for these new and shiny two-hit-filters. */
  class TrackletFilterTest : public ::testing::Test {
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
    // also need the reco-transform
    sensorInfoBase.setTransformation(transform, true);

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
  int lastResult = 0;



  /** takes result, prints it and stores it to lastResult */
  class ResultsObserver : public VoidObserver {
  public:
    /** notify function is called by the filter, this one takes result, prints it and stores it to lastResult. */
    template<class Var, typename ... otherTypes>
    static void notify(const Var& filterType,
                       typename Var::variableType fResult,
                       otherTypes ...)
    {
      B2INFO("ResultsObserver: Filter " << filterType.name() << " got result of " << fResult);
      lastResult = fResult;
    }

  };


  /** presents the functionality of the SpacePoint-creating function written above */
  TEST_F(TrackletFilterTest, SpacePointCreation)
  {
    SpacePoint testSP = provideSpacePointDummy(1.2, 2.3, 4.2);
    EXPECT_FLOAT_EQ(1.2, testSP.getPosition().X()) ;
    EXPECT_FLOAT_EQ(2.3, testSP.getPosition().Y()) ;
    EXPECT_FLOAT_EQ(4.2, testSP.getPosition().Z()) ;
  }


  /** shows the functionality of the auto naming capability of the Filter */
  TEST_F(TrackletFilterTest, SelectionVariableName)
  {
    auto ziggZaggXYChecker = ZiggZaggXY<SpacePoint, SpacePointTrackCand>();
    EXPECT_EQ("ZiggZaggXY" , ziggZaggXYChecker.name());

    auto ziggZaggXYWithSigmaChecker = ZiggZaggXYWithSigma<SpacePoint, SpacePointTrackCand>();
    EXPECT_EQ("ZiggZaggXYWithSigma" , ziggZaggXYWithSigmaChecker.name());

    auto ziggZaggRZChecker = ZiggZaggRZ<SpacePoint, SpacePointTrackCand>();
    EXPECT_EQ("ZiggZaggRZ" , ziggZaggRZChecker.name());
  }


  /** test ziggZagg */
  TEST_F(TrackletFilterTest, TestZiggZagg)
  {
    // Info: the actual position is not important, the naming just indicates the way one should add hits to the containers!
    SpacePoint outerSP = provideSpacePointDummy(1.1, 1., 1.); // R: 1.48660687
    SpacePoint outerCenterSP = provideSpacePointDummy(2., 2., 2.); // R: 2.82842712
    SpacePoint innerCenterSP = provideSpacePointDummy(3., 2.1, 3.); // R: 3.66196668
    SpacePoint innerSP = provideSpacePointDummy(4., 3., 3.); // R: 5
    SpacePoint innermostSP = provideSpacePointDummy(5., 3, 6.); // R: 5.83095189

    /// concept works for vector:
    const std::vector<const Belle2::SpacePoint*> aSpacePointVec = { &outerSP, &outerCenterSP, &innerCenterSP, &innerSP };

    Filter< ZiggZaggXY<SpacePoint, std::vector<const Belle2::SpacePoint*>>, ClosedRange<int, int>, ResultsObserver >
        ziggZaggXYChecker4VecSPs(
          ClosedRange<int, int>(1, 1));
    EXPECT_FALSE(ziggZaggXYChecker4VecSPs.accept(aSpacePointVec));
    EXPECT_EQ(2, lastResult);

    /// concept works for SpacePointTrackCand (intended main use):
    // should be ziggZagg:
    SpacePointTrackCand aSpacePointTC = SpacePointTrackCand(aSpacePointVec, 11, -1, 23);

    Filter< ZiggZaggXY<SpacePoint, SpacePointTrackCand>, ClosedRange<int, int>, ResultsObserver > ziggZaggXYChecker4SPTC(
      ClosedRange<int, int>(1, 1));
    EXPECT_FALSE(ziggZaggXYChecker4SPTC.accept(aSpacePointTC));
    EXPECT_EQ(2, lastResult);
    Filter< ZiggZaggXYWithSigma<SpacePoint, SpacePointTrackCand>, ClosedRange<int, int>, ResultsObserver >
    ziggZaggXYWithSigmaChecker4SPTC(
      ClosedRange<int, int>(0, 1));
    EXPECT_FALSE(ziggZaggXYWithSigmaChecker4SPTC.accept(aSpacePointTC));
    EXPECT_EQ(2, lastResult);
    Filter< ZiggZaggRZ<SpacePoint, SpacePointTrackCand>, ClosedRange<int, int>, ResultsObserver > ziggZaggRZChecker4SPTC(
      ClosedRange<int, int>(1, 1));
    EXPECT_FALSE(ziggZaggRZChecker4SPTC.accept(aSpacePointTC));
    EXPECT_EQ(2, lastResult);


    // should be ziggZagg:
    const std::vector<const Belle2::SpacePoint*> aSpacePointVec2 = { &outerSP, &outerCenterSP, &innerCenterSP, &innerSP, &innerSP };
    SpacePointTrackCand aSpacePointTC2 = SpacePointTrackCand(aSpacePointVec2, 11, -1, 23);

    EXPECT_FALSE(ziggZaggXYChecker4SPTC.accept(aSpacePointTC2));
    EXPECT_EQ(3, lastResult); // last two hits are identical, will be detected in Filter.
    EXPECT_FALSE(ziggZaggXYWithSigmaChecker4SPTC.accept(aSpacePointTC2));
    EXPECT_EQ(2, lastResult);
    EXPECT_FALSE(ziggZaggRZChecker4SPTC.accept(aSpacePointTC2));
    EXPECT_EQ(3, lastResult);


    // should _not_ be ziggZagg:
    const std::vector<const Belle2::SpacePoint*> aSpacePointVec3 = { &outerSP, &outerCenterSP, &innerSP, &innermostSP };
    SpacePointTrackCand aSpacePointTC3 = SpacePointTrackCand(aSpacePointVec3, 11, -1, 23);

    EXPECT_TRUE(ziggZaggXYChecker4SPTC.accept(aSpacePointTC3));
    EXPECT_EQ(1, lastResult);
    EXPECT_TRUE(ziggZaggXYWithSigmaChecker4SPTC.accept(aSpacePointTC3));
    EXPECT_EQ(1, lastResult);
    EXPECT_FALSE(ziggZaggRZChecker4SPTC.accept(aSpacePointTC3));
    EXPECT_EQ(2, lastResult);


    //  /** Difference between ZiggZaggs (no RZ-test here) **/
    SpacePoint testSP1 = provideSpacePointDummy(1.001, 1., 0.); // sets to positionSigma of ~0.1 in x,y,z!
    SpacePoint testSP2 = provideSpacePointDummy(2., 2.0002, 0.); // sets to positionSigma of ~0.1 in x,y,z!
    SpacePoint testSP3 = provideSpacePointDummy(3.0005, 3., 0.); // sets to positionSigma of ~0.1 in x,y,z!
    SpacePoint testSP4 = provideSpacePointDummy(4., 4.0003, 0.); // sets to positionSigma of ~0.1 in x,y,z!
    SpacePoint testSP5 = provideSpacePointDummy(5.001, 5., 0.); // sets to positionSigma of ~0.1 in x,y,z!
    const std::vector<const Belle2::SpacePoint*> aSpacePointVec4 = { &testSP1, &testSP2, &testSP3, &testSP4, &testSP5 };
    SpacePointTrackCand aSpacePointTC4 = SpacePointTrackCand(aSpacePointVec4, 11, -1, 23);

    EXPECT_FALSE(ziggZaggXYChecker4SPTC.accept(aSpacePointTC4));
    EXPECT_EQ(2, lastResult);
    EXPECT_TRUE(ziggZaggXYWithSigmaChecker4SPTC.accept(aSpacePointTC4)); // does not ziggZagg due to positionSigma != 0
    EXPECT_EQ(0, lastResult);

  }

}

