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
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/SlopeRZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DNormed.h"

#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"

#include <tracking/vxdCaTracking/TwoHitFilters.h>

#include <tuple>
#include <iostream>
#include <math.h>

using namespace std;

using namespace Belle2;

namespace VXDTFtwoHitFilterTest {

  /** Test class for these new and shiny two-hit-filters. */
  class TwoHitFilterTest : public ::testing::Test {
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


  /** a tiny counter class for counting stuff */
  template < class T>
  class counter {
  public:
    static int N;
    counter() {};
    ~counter() {};
  };


  /** a counter to be used by Distance3DSquared */
  template<>
  int counter< Distance3DSquared >::N(0);


  /** a counter to be used by Distance2DXYSquared */
  template<>
  int counter< Distance2DXYSquared >::N(0);


  /** this observer does simply count the number of times, the attached Filter was used */
  class Observer : public VoidObserver {
  public:
    template<class Var>
    static void notify(const typename Var::argumentType& ,
                       const typename Var::argumentType& ,
                       const Var& ,
                       typename Var::variableType) {
      counter<Var>::N ++ ;
    }

  };


  /** presents the functionality of the SpacePoint-creating function written above */
  TEST_F(TwoHitFilterTest, SpacePointCreation)
  {
    SpacePoint testSP = provideSpacePointDummy(1.2, 2.3, 4.2);
    EXPECT_EQ(1.2, testSP.getPosition().X()) ;
    EXPECT_EQ(2.3, testSP.getPosition().Y()) ;
    EXPECT_EQ(4.2, testSP.getPosition().Z()) ;

  }


  /** shows the functionality of the auto naming capability of the Filter */
  TEST_F(TwoHitFilterTest, SelectionVariableName)
  {

    EXPECT_EQ("Belle2::Distance3DSquared" , Distance3DSquared().name());
    EXPECT_EQ("Belle2::Distance2DXYSquared" , Distance2DXYSquared().name());
    EXPECT_EQ("Belle2::Distance1DZ" , Distance1DZ().name());
    EXPECT_EQ("Belle2::SlopeRZ" , SlopeRZ().name());

  }


  /** shows how to use the filter Distance3DSquared in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance3DSquared)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(.5 , 0., 0.);
    SpacePoint x3 = provideSpacePointDummy(2. , 0., 0.);

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));

  }


  /** shows how to use the filter Distance3DSquared in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance2DXYSquared)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance2DXYSquared, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(.5 , 0., 0.);
    SpacePoint x3 = provideSpacePointDummy(2. , 0., 0.);
    SpacePoint x4 = provideSpacePointDummy(0. , 0., 2.);

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_TRUE(filter.accept(x2, x4));

  }


  /** shows how to use the filter Distance1DZ in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance1DZ)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance1DZ, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(0. , 0., .5);
    SpacePoint x3 = provideSpacePointDummy(.25 , .25, 0.);
    SpacePoint x4 = provideSpacePointDummy(0. , 0., 1.);

    EXPECT_TRUE(filter.accept(x2, x1));
    EXPECT_FALSE(filter.accept(x1, x2)); // the input order is relevant
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_FALSE(filter.accept(x1, x4));

  }


  /** shows how to use the filter SlopeRZ and its expected behavior for some tricky cases */
  TEST_F(TwoHitFilterTest, BasicFilterTestSlopeRZ)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SlopeRZ, Range<float, float>, VoidObserver > filter(Range<float, float>(atan(2.), atan(3.)));

    SpacePoint innerSP = provideSpacePointDummy(1 , 2, 3);
    SpacePoint outerSP1 = provideSpacePointDummy(1 , 4, 4);
    SpacePoint outerSP2 = provideSpacePointDummy(1 , 4, 3.95);
    SpacePoint outerSP3 = provideSpacePointDummy(1 , 4, 4.05);
    SpacePoint outerSP4 = provideSpacePointDummy(1 , 3, 3.45);
    SpacePoint outerSP5 = provideSpacePointDummy(1 , 3, 3.55);
    SpacePoint outerSP6 = provideSpacePointDummy(1 , 4, 3);
    SpacePoint outerSP7 = provideSpacePointDummy(1 , 0, 4);

    EXPECT_FALSE(filter.accept(outerSP1, innerSP));
    EXPECT_TRUE(filter.accept(outerSP2, innerSP));
    EXPECT_FALSE(filter.accept(innerSP, outerSP2)); // reverse order not same result (because of z)
    EXPECT_FALSE(filter.accept(outerSP3, innerSP));
    EXPECT_TRUE(filter.accept(outerSP4, innerSP));
    EXPECT_FALSE(filter.accept(outerSP5, innerSP));
    EXPECT_EQ(filter.accept(outerSP1, innerSP), filter.accept(outerSP7, innerSP)); // (direction of r-vector not relevant, only its length)


    EXPECT_FLOAT_EQ(0., SlopeRZ().value(innerSP, innerSP));
    EXPECT_FLOAT_EQ(atan(2.), SlopeRZ().value(outerSP1, innerSP));
    EXPECT_FLOAT_EQ(atan(2. / 0.95), SlopeRZ().value(outerSP2, innerSP));
    EXPECT_FLOAT_EQ(- SlopeRZ().value(outerSP2, innerSP), SlopeRZ().value(innerSP, outerSP2)); // reverse order changes sign
    EXPECT_FLOAT_EQ(atan(2. / 1.05), SlopeRZ().value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(atan(1. / 0.45), SlopeRZ().value(outerSP4, innerSP));
    EXPECT_FLOAT_EQ(atan(1. / 0.55), SlopeRZ().value(outerSP5, innerSP));
    EXPECT_FLOAT_EQ(M_PI * 0.5, SlopeRZ().value(outerSP6, innerSP)); // no problem with division by 0 in Z
    EXPECT_FLOAT_EQ(atan(2. / 1.05), SlopeRZ().value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(SlopeRZ().value(outerSP1, innerSP), SlopeRZ().value(outerSP7, innerSP)); // (direction of r-vector not relevant, only its length)

  }


  /** shows how to use the filter Distance3DNormed in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance3DNormed)
  {
    // Very verbose declaration, the old normed distance 3D has only an upper cut, no lower one:
    Filter< Distance3DNormed, UpperBoundedSet<float>, VoidObserver > filter(UpperBoundedSet<float>(1.));

    // prepare spacePoints for new stuff
    SpacePoint innerSP = provideSpacePointDummy(1 , 2, 3);
    SpacePoint outerSP1 = provideSpacePointDummy(2 , 3, 4);
    SpacePoint outerSP2 = provideSpacePointDummy(1 , 2, 4);
    SpacePoint outerSP3 = provideSpacePointDummy(2 , 3, 3);

    EXPECT_FLOAT_EQ(2. / 3., Distance3DNormed().value(outerSP1, innerSP));
    EXPECT_FLOAT_EQ(0., Distance3DNormed().value(outerSP2, innerSP));
    EXPECT_FLOAT_EQ(1., Distance3DNormed().value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(0., Distance3DNormed().value(innerSP, innerSP));

  }


  /** shows how to attach an observer to a filter of interest */
  TEST_F(TwoHitFilterTest, ObservedFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(0., 1.));

    Filter< Distance3DSquared, Range<double, double>, Observer > filter(unobservedFilter);
    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
    counter< Distance3DSquared >::N = 0;

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_EQ(2 , counter< Distance3DSquared >::N);
  }


  /** shows how to bypass a filter which itself was not initially planned to be bypassed */
  TEST_F(TwoHitFilterTest, BypassableFilter)
  {
    bool bypassControl(false);
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared, Range<double, double>, Observer > nonBypassableFilter(Range<double, double>(0., 1.));
    auto filter = nonBypassableFilter.bypass(bypassControl);
    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
    counter< Distance3DSquared >::N = 0;

    EXPECT_FALSE(filter.accept(x1, x2));
    EXPECT_EQ(1 , counter< Distance3DSquared >::N);

    bypassControl = true;
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_EQ(2 , counter< Distance3DSquared >::N);

  }


  /** shows how to write compact code using the new filter design */
  TEST_F(TwoHitFilterTest, Shortcuts)
  {

    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

    auto filterSup = (Distance3DSquared() < 1.) ;
    EXPECT_TRUE(filterSup.accept(x1, x2));
    EXPECT_FALSE(filterSup.accept(x1, x3));

    auto filterSup2 = (1 > Distance3DSquared()) ;
    EXPECT_TRUE(filterSup2.accept(x1, x2));
    EXPECT_FALSE(filterSup2.accept(x1, x3));

    auto filterInf = (Distance3DSquared() > 1.) ;
    EXPECT_TRUE(filterInf.accept(x1, x3));
    EXPECT_FALSE(filterInf.accept(x1, x2));

    auto filterInf2 = (1 < Distance3DSquared()) ;
    EXPECT_TRUE(filterInf2.accept(x1, x3));
    EXPECT_FALSE(filterInf2.accept(x1, x2));

    auto filterRange = (0.1 < Distance3DSquared() < 1);
    EXPECT_FALSE(filterRange.accept(x1, x1));
    EXPECT_TRUE(filterRange.accept(x1, x2));
    EXPECT_FALSE(filterRange.accept(x1, x3));

  }


  /** tests compatibility of filters with boolean operations for easy coupling of filters */
  TEST_F(TwoHitFilterTest, BooleanOperations)
  {

    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(1.0f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

    auto filter = !(Distance3DSquared() > 1.);
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_TRUE(filter.accept(x1, x1));
    EXPECT_FALSE(filter.accept(x1, x3));

    auto filter2 =
      !(Distance3DSquared() > 1.) &&
      !(Distance3DSquared() < 1);
    // i.e. Distance3DSquared == 1
    EXPECT_TRUE(filter2.accept(x1, x2));
    EXPECT_FALSE(filter2.accept(x1, x1));
    EXPECT_FALSE(filter2.accept(x1, x3));


    auto filter3 =
      (Distance3DSquared() > 1.) ||
      (Distance3DSquared() < 1);
    // i.e. Distance3DSquared != 1
    EXPECT_FALSE(filter3.accept(x1, x2));
    EXPECT_TRUE(filter3.accept(x1, x1));
    EXPECT_TRUE(filter3.accept(x1, x3));

  }


  /** evaluating compatibility of filters with lazy evaluation */
  TEST_F(TwoHitFilterTest, ShortCircuitsEvaluation)
  {
    auto filter(
      (Distance2DXYSquared() < 1).observe(Observer()) &&
      (Distance3DSquared()   < 1).observe(Observer())
    );

    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(1.0f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

    counter< Distance3DSquared   >::N = 0;
    counter< Distance2DXYSquared >::N = 0;

    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_EQ(1 , counter< Distance2DXYSquared >::N);
    EXPECT_EQ(0 , counter< Distance3DSquared >::N);

    EXPECT_TRUE(filter.accept(x1, x1));
    EXPECT_EQ(2 , counter< Distance2DXYSquared >::N);
    EXPECT_EQ(1 , counter< Distance3DSquared >::N);

  }

}
