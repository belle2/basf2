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
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance3DSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance2DXYSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance1DZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/SlopeRZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance3DNormed.h>

#include <tracking/trackFindingVXD/sectorMap/filterFramework/Shortcuts.h>

#include <tracking/vxdCaTracking/VXDSector.h>
#include <tracking/vxdCaTracking/SegFinderFilters.h>
#include <tracking/vxdCaTracking/TwoHitFilters.h>

#include <boost/math/special_functions/fpclassify.hpp> // isnan and isinf

#include <tuple>
#include <iostream>
#include <math.h> // sqrt, atan, isinf, ...

using namespace std;

using namespace Belle2;

namespace VXDTFsegFinderTest {

  /** We want to verify that the old and the new filters provide the same results.
   *
   * This code is dependent of the old VXDTF design and therefore should be kept or removed with the old code
   */
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
  VXDSector provideVXDSectorDummy(int cutOffType, unsigned int friendName, std::pair<float, float> cutValues)
  {
    VXDSector thisSector(123);

    thisSector.addCutoff(cutOffType, friendName, cutValues);

    return thisSector;
  }



  /** shows how to use a filter in a simple case */
  TEST_F(OldVSNewSegFinderFiltersTest, BasicFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared<SpacePoint>, Range<float, float>, VoidObserver > filter(Range<float, float>(0., 1.));

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

    // Very verbose declaration:
    Filter< Distance3DSquared<SpacePoint>, Range<float, float>, VoidObserver > filter(Range<float, float>(2., 3.));

    // prepare spacePoints for new stuff
    SpacePoint innerSP = provideSpacePointDummy(innerHit.X() , innerHit.Y(), innerHit.Z());
    SpacePoint outerSP1 = provideSpacePointDummy(outerHit1.X() , outerHit1.Y(), outerHit1.Z());
    SpacePoint outerSP2 = provideSpacePointDummy(outerHit2.X() , outerHit2.Y(), outerHit2.Z());
    SpacePoint outerSP3 = provideSpacePointDummy(outerHit3.X() , outerHit3.Y(), outerHit3.Z());
    SpacePoint outerSP4 = provideSpacePointDummy(outerHit4.X() , outerHit4.Y(), outerHit4.Z());

    // show behavior of new filter:
    EXPECT_FALSE(filter.accept(outerSP1, innerSP));
    EXPECT_TRUE(filter.accept(outerSP2, innerSP));
    EXPECT_TRUE(filter.accept(outerSP3, innerSP));
    EXPECT_FALSE(filter.accept(outerSP4, innerSP));

    // show behavior of old filter and directly afterwards a direct comparison of the new and the old filter:
    segFinderBox.resetValues(outerHit1, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(outerSP1, innerSP));

    segFinderBox.resetValues(outerHit2, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(outerSP2, innerSP));

    segFinderBox.resetValues(outerHit3, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(outerSP3, innerSP));

    segFinderBox.resetValues(outerHit4, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::distance3D));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::distance3D), filter.accept(outerSP4, innerSP));
  }



  /** compares old and new distancexy-behavior */
  TEST_F(OldVSNewSegFinderFiltersTest, CompareDistanceXY)
  {
    // prepare old stuff:
    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit1(2, 3, 4);
    TVector3 outerHit2(2, 2.999, 123);
    TVector3 outerHit3(2, 3.001, 423);
    TVector3 outerHit4(2, 3.4, 4);
    TVector3 outerHit5(2, 3.42, 4);
    VXDSector testSector = provideVXDSectorDummy(FilterID::distanceXY, 11, {2., 3.});
    SegFinderFilters segFinderBox(outerHit1, innerHit, &testSector, 11);

    // Very verbose declaration:
    Filter< Distance2DXYSquared<SpacePoint>, Range<float, float>, VoidObserver > filter(Range<float, float>(2., 3.));

    // prepare spacePoints for new stuff
    SpacePoint innerSP = provideSpacePointDummy(innerHit.X() , innerHit.Y(), innerHit.Z());
    SpacePoint outerSP1 = provideSpacePointDummy(outerHit1.X() , outerHit1.Y(), outerHit1.Z());
    SpacePoint outerSP2 = provideSpacePointDummy(outerHit2.X() , outerHit2.Y(), outerHit2.Z());
    SpacePoint outerSP3 = provideSpacePointDummy(outerHit3.X() , outerHit3.Y(), outerHit3.Z());
    SpacePoint outerSP4 = provideSpacePointDummy(outerHit4.X() , outerHit4.Y(), outerHit4.Z());
    SpacePoint outerSP5 = provideSpacePointDummy(outerHit5.X() , outerHit5.Y(), outerHit5.Z());

    // show behavior of new filter:
    EXPECT_FALSE(filter.accept(outerSP1, innerSP));
    EXPECT_FALSE(filter.accept(outerSP2, innerSP));
    EXPECT_TRUE(filter.accept(outerSP3, innerSP));
    EXPECT_TRUE(filter.accept(outerSP4, innerSP));
    EXPECT_FALSE(filter.accept(outerSP5, innerSP));

    // show behavior of old filter and directly afterwards a direct comparison of the new and the old filter:
    segFinderBox.resetValues(outerHit1, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDistXY(FilterID::distanceXY));
    EXPECT_EQ(segFinderBox.checkDistXY(FilterID::distanceXY), filter.accept(outerSP1, innerSP));

    segFinderBox.resetValues(outerHit2, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDistXY(FilterID::distanceXY));
    EXPECT_EQ(segFinderBox.checkDistXY(FilterID::distanceXY), filter.accept(outerSP2, innerSP));

    segFinderBox.resetValues(outerHit3, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDistXY(FilterID::distanceXY));
    EXPECT_EQ(segFinderBox.checkDistXY(FilterID::distanceXY), filter.accept(outerSP3, innerSP));

    segFinderBox.resetValues(outerHit4, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDistXY(FilterID::distanceXY));
    EXPECT_EQ(segFinderBox.checkDistXY(FilterID::distanceXY), filter.accept(outerSP4, innerSP));

    segFinderBox.resetValues(outerHit5, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDistXY(FilterID::distanceXY));
    EXPECT_EQ(segFinderBox.checkDistXY(FilterID::distanceXY), filter.accept(outerSP5, innerSP));
  }



  /** compares old and new distanceZ-behavior */
  TEST_F(OldVSNewSegFinderFiltersTest, CompareDistanceZ)
  {
    // prepare old stuff:
    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit1(2, 3, 4);
    TVector3 outerHit2(2, 3, 3.999);
    TVector3 outerHit3(2, 2, 4.001);
    TVector3 outerHit4(1, 2, 4.001);
    VXDSector testSector = provideVXDSectorDummy(FilterID::distanceZ, 11, {1., 2.});
    SegFinderFilters segFinderBox(outerHit1, innerHit, &testSector, 11);

    // Very verbose declaration:
    Filter< Distance1DZ<SpacePoint>, Range<float, float>, VoidObserver > filter(Range<float, float>(1., 2.));

    // prepare spacePoints for new stuff
    SpacePoint innerSP = provideSpacePointDummy(innerHit.X() , innerHit.Y(), innerHit.Z());
    SpacePoint outerSP1 = provideSpacePointDummy(outerHit1.X() , outerHit1.Y(), outerHit1.Z());
    SpacePoint outerSP2 = provideSpacePointDummy(outerHit2.X() , outerHit2.Y(), outerHit2.Z());
    SpacePoint outerSP3 = provideSpacePointDummy(outerHit3.X() , outerHit3.Y(), outerHit3.Z());
    SpacePoint outerSP4 = provideSpacePointDummy(outerHit4.X() , outerHit4.Y(), outerHit4.Z());


    // show behavior of new filter:
    EXPECT_FALSE(filter.accept(outerSP1, innerSP));
    EXPECT_FALSE(filter.accept(outerSP2, innerSP));
    EXPECT_TRUE(filter.accept(outerSP3, innerSP));
    EXPECT_TRUE(filter.accept(outerSP4, innerSP));
    EXPECT_FALSE(filter.accept(innerSP, outerSP4)); // reverse order not same result


    // show behavior of old filter and directly afterwards a direct comparison of the new and the old filter:
    segFinderBox.resetValues(outerHit1, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDistZ(FilterID::distanceZ));
    EXPECT_EQ(segFinderBox.checkDistZ(FilterID::distanceZ), filter.accept(outerSP1, innerSP));

    segFinderBox.resetValues(outerHit2, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDistZ(FilterID::distanceZ));
    EXPECT_EQ(segFinderBox.checkDistZ(FilterID::distanceZ), filter.accept(outerSP2, innerSP));

    segFinderBox.resetValues(outerHit3, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDistZ(FilterID::distanceZ));
    EXPECT_EQ(segFinderBox.checkDistZ(FilterID::distanceZ), filter.accept(outerSP3, innerSP));

    segFinderBox.resetValues(outerHit4, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDistZ(FilterID::distanceZ));
    EXPECT_EQ(segFinderBox.checkDistZ(FilterID::distanceZ), filter.accept(outerSP4, innerSP));

    segFinderBox.resetValues(innerHit, outerHit4, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDistZ(FilterID::distanceZ));
    EXPECT_EQ(segFinderBox.checkDistZ(FilterID::distanceZ), filter.accept(innerSP, outerSP4));
  }


  /** compares old and new slopeRZ-behavior */
  TEST_F(OldVSNewSegFinderFiltersTest, CompareSlopeRZ)
  {
    bool remindMeThatIAmNotDoneYet(false);
    EXPECT_FALSE(
      remindMeThatIAmNotDoneYet); /// meta test, which tests whether I have approved my own test (if not done yet = true -> test fails)

    // prepare old stuff:
    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit1(1, 4, 4);
    TVector3 outerHit2(1, 4, 3.95);
    TVector3 outerHit3(1, 4, 4.05);
    TVector3 outerHit4(1, 3, 3.45);
    TVector3 outerHit5(1, 3, 3.55);
    TVector3 outerHit6(1, 4, 3);
    TVector3 outerHit7(1, 0, 4);
    VXDSector testSector = provideVXDSectorDummy(FilterID::slopeRZ, 11, {atan(2.), atan(3.)});
    SegFinderFilters segFinderBox(outerHit1, innerHit, &testSector, 11);
    TwoHitFilters twoHitFilterBox(outerHit1, innerHit);

    // Very verbose declaration, see below for convenient shortcuts
    Filter< SlopeRZ<SpacePoint>, Range<float, float>, VoidObserver > filter(Range<float, float>(atan(2.), atan(3.)));
    auto sRZ = SlopeRZ<SpacePoint>();

    // prepare spacePoints for new stuff
    SpacePoint innerSP = provideSpacePointDummy(innerHit.X() , innerHit.Y(), innerHit.Z());
    SpacePoint outerSP1 = provideSpacePointDummy(outerHit1.X() , outerHit1.Y(), outerHit1.Z());
    SpacePoint outerSP2 = provideSpacePointDummy(outerHit2.X() , outerHit2.Y(), outerHit2.Z());
    SpacePoint outerSP3 = provideSpacePointDummy(outerHit3.X() , outerHit3.Y(), outerHit3.Z());
    SpacePoint outerSP4 = provideSpacePointDummy(outerHit4.X() , outerHit4.Y(), outerHit4.Z());
    SpacePoint outerSP5 = provideSpacePointDummy(outerHit5.X() , outerHit5.Y(), outerHit5.Z());
    SpacePoint outerSP6 = provideSpacePointDummy(outerHit6.X() , outerHit6.Y(), outerHit6.Z());
    SpacePoint outerSP7 = provideSpacePointDummy(outerHit7.X() , outerHit7.Y(), outerHit7.Z());



    // show behavior of old filter and directly afterwards a direct comparison of the new and the old filter:

    EXPECT_FALSE(filter.accept(outerSP1, innerSP));
    segFinderBox.resetValues(outerHit1, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::slopeRZ));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::slopeRZ), filter.accept(outerSP1, innerSP));

    twoHitFilterBox.resetValues(outerHit1, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcSlopeRZ(), sRZ.value(outerSP1, innerSP));


    EXPECT_TRUE(filter.accept(outerSP2, innerSP));
    EXPECT_FALSE(filter.accept(innerSP, outerSP2)); // reverse order not same result (because of z)
    segFinderBox.resetValues(outerHit2, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::slopeRZ)); // !
    EXPECT_NE(segFinderBox.checkDist3D(FilterID::slopeRZ), filter.accept(outerSP2,
              innerSP)); // ATTENTION: rounding errors! the old filtering calculates in double precission, the new one in float precission -> float value is lower than cut,  therefore not accepted

    twoHitFilterBox.resetValues(outerHit2, innerHit);
//  B2WARNING(" outerHit2 vs innerHit old/new: " << twoHitFilterBox.calcSlopeRZ() << "/" << SlopeRZ().value(outerSP2, innerSP))
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcSlopeRZ(), sRZ.value(outerSP2, innerSP));
    EXPECT_EQ(twoHitFilterBox.calcSlopeRZ(), sRZ.value(outerSP2,
                                                       innerSP)); // they are float equal but not double equal


    EXPECT_FALSE(filter.accept(outerSP3, innerSP));
    segFinderBox.resetValues(outerHit3, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::slopeRZ));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::slopeRZ), filter.accept(outerSP3, innerSP));

    twoHitFilterBox.resetValues(outerHit3, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcSlopeRZ(), sRZ.value(outerSP3, innerSP));


    EXPECT_TRUE(filter.accept(outerSP4, innerSP));
    segFinderBox.resetValues(outerHit4, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkDist3D(FilterID::slopeRZ));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::slopeRZ), filter.accept(outerSP4, innerSP));

    twoHitFilterBox.resetValues(outerHit4, innerHit);
//  B2WARNING(" outerHit4 vs innerHit old/new: " << twoHitFilterBox.calcSlopeRZ() << "/" << SlopeRZ().value(outerSP4, innerSP))
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcSlopeRZ(), sRZ.value(outerSP4, innerSP));


    EXPECT_FALSE(filter.accept(outerSP5, innerSP));
    segFinderBox.resetValues(outerHit5, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::slopeRZ));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::slopeRZ), filter.accept(outerSP5, innerSP));

    twoHitFilterBox.resetValues(outerHit5, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcSlopeRZ(), sRZ.value(outerSP5, innerSP));


    EXPECT_EQ(filter.accept(outerSP1, innerSP), filter.accept(outerSP7,
                                                              innerSP)); // (direction of r-vector not relevant, only its length)
    segFinderBox.resetValues(outerHit7, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkDist3D(FilterID::slopeRZ));
    EXPECT_EQ(segFinderBox.checkDist3D(FilterID::slopeRZ), filter.accept(outerSP7, innerSP));

    twoHitFilterBox.resetValues(outerHit7, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcSlopeRZ(), sRZ.value(outerSP7, innerSP));
  }


  /** compares old and new normedDist3D-behavior */
  TEST_F(OldVSNewSegFinderFiltersTest, CompareNormedDistance3D)
  {
    bool remindMeThatIAmNotDoneYet(false);
    EXPECT_FALSE(
      remindMeThatIAmNotDoneYet); /// meta test, which tests whether I have approved my own test (if not done yet = true -> test fails)

    // prepare old stuff:
    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit1(2, 3, 4);
    TVector3 outerHit2(1, 2, 4);
    TVector3 outerHit3(2, 3, 3);

    VXDSector testSector = provideVXDSectorDummy(FilterID::normedDistance3D, 11, {0., 1.});
    SegFinderFilters segFinderBox(outerHit1, innerHit, &testSector, 11);
    TwoHitFilters twoHitFilterBox(outerHit1, innerHit);

    // Very verbose declaration, the old normed distance 3D has only an upper cut, no lower one:
    Filter< Distance3DNormed<SpacePoint>, UpperBoundedSet<float>, VoidObserver > filter(UpperBoundedSet<float>(1.));
    auto d3Dn = Distance3DNormed<SpacePoint>();

    // prepare spacePoints for new stuff
    SpacePoint innerSP = provideSpacePointDummy(innerHit.X() , innerHit.Y(), innerHit.Z());
    SpacePoint outerSP1 = provideSpacePointDummy(outerHit1.X() , outerHit1.Y(), outerHit1.Z());
    SpacePoint outerSP2 = provideSpacePointDummy(outerHit2.X() , outerHit2.Y(), outerHit2.Z());
    SpacePoint outerSP3 = provideSpacePointDummy(outerHit3.X() , outerHit3.Y(), outerHit3.Z());



    // show behavior of old filter and directly afterwards a direct comparison of the new and the old filter:

    EXPECT_TRUE(filter.accept(outerSP1, innerSP));
    segFinderBox.resetValues(outerHit1, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D));
    EXPECT_EQ(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D), filter.accept(outerSP1, innerSP));
    twoHitFilterBox.resetValues(outerHit1, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcNormedDist3D(), d3Dn.value(outerSP1, innerSP));
    EXPECT_FLOAT_EQ(2. / 3., d3Dn.value(outerSP1, innerSP));
    EXPECT_FLOAT_EQ(2. / 3., twoHitFilterBox.calcNormedDist3D());


    EXPECT_TRUE(filter.accept(outerSP2, innerSP));
    segFinderBox.resetValues(outerHit2, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D));
    EXPECT_EQ(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D), filter.accept(outerSP2, innerSP));
    twoHitFilterBox.resetValues(outerHit2, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcNormedDist3D(), d3Dn.value(outerSP2, innerSP));
    EXPECT_FLOAT_EQ(0., d3Dn.value(outerSP2, innerSP));
    EXPECT_FLOAT_EQ(0., twoHitFilterBox.calcNormedDist3D());


    EXPECT_FALSE(filter.accept(outerSP3, innerSP));
    segFinderBox.resetValues(outerHit3, innerHit, &testSector, 11);
    EXPECT_FALSE(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D));
    EXPECT_EQ(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D), filter.accept(outerSP3, innerSP));
    twoHitFilterBox.resetValues(outerHit3, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcNormedDist3D(), d3Dn.value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(1., d3Dn.value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(1., twoHitFilterBox.calcNormedDist3D());


    // if division by 0 is provoked, value is 0, which leads in this example to a rejected filter!
    EXPECT_TRUE(filter.accept(innerSP, innerSP));
    segFinderBox.resetValues(innerHit, innerHit, &testSector, 11);
    EXPECT_TRUE(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D));
    EXPECT_EQ(segFinderBox.checkNormedDist3D(FilterID::normedDistance3D), filter.accept(innerSP, innerSP));
    twoHitFilterBox.resetValues(innerHit, innerHit);
    EXPECT_FLOAT_EQ(twoHitFilterBox.calcNormedDist3D(), d3Dn.value(innerSP, innerSP));
    EXPECT_FLOAT_EQ(0., d3Dn.value(innerSP, innerSP));
    EXPECT_FLOAT_EQ(0., twoHitFilterBox.calcNormedDist3D());
  }
}
