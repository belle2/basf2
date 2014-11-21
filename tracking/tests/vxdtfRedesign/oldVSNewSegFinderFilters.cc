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
#include <vxd/geometry/SensorInfoBase.h>
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance2DXYSquared.h"

#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"

#include <tracking/vxdCaTracking/VXDSector.h>
#include <tracking/vxdCaTracking/SegFinderFilters.h>

#include <tuple>
#include <iostream>
#include <math.h>

using namespace std;

using namespace Belle2;

namespace VXDTFsegFinderTest {

  /** We want to verify that the old and the new filters provide the same results. */
  class OldVSNewSegFinderFiltersTest : public ::testing::Test {
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


  /** a function to create a mockup for a VXDSector */
  VXDSector provideVXDSectorDummy(int cutOffType, unsigned int friendName, std::pair<double, double> cutValues)
  {
    VXDSector thisSector(123);

    thisSector.addCutoff(cutOffType, friendName, cutValues);

    return thisSector;
  }


  /** shows how to use a filter in a simple case */
  TEST_F(OldVSNewSegFinderFiltersTest, BasicFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));

  }


  /** compares old and new distance3D-behavior */
  TEST_F(OldVSNewSegFinderFiltersTest, CompareDistance3D)
  {
    // prepare old stuff:
    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit1(2, 3, 4);
    TVector3 outerHit2(2, 3, 3.999);
    TVector3 outerHit3(2, 2, 4.001);
    TVector3 outerHit4(2, 2, 4);
    VXDSector testSector = provideVXDSectorDummy(FilterID::distance3D, 11, {2., 3.});
    SegFinderFilters segFinderBox(outerHit1, innerHit, &testSector, 11);

    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared, Range<double, double>, VoidObserver > filter(Range<double, double>(2., 3.));

    SpacePoint innerSP = provideSpacePointDummy(innerHit.X() , innerHit.Y(), innerHit.Z());
    SpacePoint outerSP1 = provideSpacePointDummy(outerHit1.X() , outerHit1.Y(), outerHit1.Z());
    SpacePoint outerSP2 = provideSpacePointDummy(outerHit2.X() , outerHit2.Y(), outerHit2.Z());
    SpacePoint outerSP3 = provideSpacePointDummy(outerHit3.X() , outerHit3.Y(), outerHit3.Z());
    SpacePoint outerSP4 = provideSpacePointDummy(outerHit4.X() , outerHit4.Y(), outerHit4.Z());

    // show behavior of new filter:
    EXPECT_FALSE(filter.accept(innerSP, outerSP1));
    EXPECT_TRUE(filter.accept(innerSP, outerSP2));
    EXPECT_TRUE(filter.accept(innerSP, outerSP3));
    EXPECT_FALSE(filter.accept(innerSP, outerSP4));

    // show behavior of old filter and directly afterwards a direct comparison of the new and the old filter:
    segFinderBox.resetValues(outerHit1, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(innerSP, outerSP1));

    segFinderBox.resetValues(outerHit2, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(innerSP, outerSP2));

    segFinderBox.resetValues(outerHit3, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(innerSP, outerSP3));

    segFinderBox.resetValues(outerHit4, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(innerSP, outerSP4));

  }


  /** Constructor. use this one, when having a sectormap (e.g. during track finding), use TwoHitFilters when no sectormap is available */

}
