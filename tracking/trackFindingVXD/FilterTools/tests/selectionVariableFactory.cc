/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbihler (jakob.lettenbichler@oeaw.ac.at)      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>


#include <tracking/trackFindingVXD/FilterTools/SelectionVariableFactory.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/vectorTools/B2Vector3.h>

#include <tracking/vxdCaTracking/XHitFilterFactory.h>

using namespace std;

using namespace Belle2;

namespace SelFarFactoryTests {

  /** just a small proto-container storing coordinates */
  class DummyHit {
  public:
    /** std constructor */
    DummyHit(): x(0), y(0), z(0) {};

    /** constructor*/
    DummyHit(double x, double y, double z): x(x), y(y), z(z) {};

    /** destructor */
    virtual ~DummyHit() {};

    double X() const { return x; }/**< return x */
    double Y() const { return y; }/**< return y */
    double Z() const { return z; }/**< return z */

  private:
    double x; /**< x */
    double y; /**< y */
    double z; /**< z */
  };


  /** Test class for SelectionVariableFactory object. */
  class SelVarFactoryTest : public ::testing::Test {
  protected:
  };


  /** tests if I can get the correct results from a SelectionVariable I asked for */
  TEST_F(SelVarFactoryTest, TestFactory)
  {
    // home hits with coordinates:
    auto hit1 = DummyHit(1, 1, 1);
    auto hit2 = DummyHit(2, 2, 3);
    auto hit3 = DummyHit(2, 3, 4);
    auto hit4 = DummyHit(23, 42, 5);
    auto hitIP = DummyHit(0, 0, 0);
    // some SelectionVariables:
    auto d3D = Distance3DSquared<DummyHit>();
//  auto sRZ = SlopeRZ<DummyHit>();
    auto a3D = Angle3DSimple<DummyHit>();
//  auto pT = Pt<DummyHit>();
    auto dcRad = DeltaCircleRadius<DummyHit>();
    auto dDcc = DeltaDistCircleCenter<DummyHit>();

    auto aFactory = SelectionVariableFactory<DummyHit>();

//     EXPECT_DOUBLE_EQ(aFactory.getResult2Hit("Distance3DSquared", hit1, hit2) , d3D.value(hit1, hit2));
//  EXPECT_DOUBLE_EQ(aFactory.getResult2Hit("SlopeRZ", hit1, hit2) , sRZ.value(hit1, hit2));
//     EXPECT_DOUBLE_EQ(aFactory.getResult3Hit("Angle3DSimple", hit1, hit2, hit3), a3D.value(hit1, hit2, hit3));
//  EXPECT_DOUBLE_EQ(aFactory.getResult3Hit("Pt", hit1, hit2, hit3), pT.value(hit1, hit2, hit3));
//     EXPECT_DOUBLE_EQ(aFactory.getResult4Hit("DeltaPt", hit1, hit2, hit3, hit4), dPT.value(hit1, hit2, hit3, hit4));
//  EXPECT_DOUBLE_EQ(aFactory.getResult4Hit("DeltaDistCircleCenter", hit1, hit2, hit3, hit4), dDcc.value(hit1, hit2, hit3, hit4));

    // 2 hits:
    auto dist3D = aFactory.get2HitInterface("Distance3DSquared");
    EXPECT_DOUBLE_EQ(dist3D(hit1, hit2) , d3D.value(hit1, hit2));
    auto a3DHiOc = aFactory.get2HitInterface("Angle3DSimpleHighOccupancy");
    EXPECT_DOUBLE_EQ(a3DHiOc(hit1, hit2) , a3D.value(hit1, hit2, hitIP));
    auto notExisting2 = aFactory.get2HitInterface("bla");
    EXPECT_DOUBLE_EQ(notExisting2(hit1, hit2) , 0.0);
    // 3 hits:
    auto ang3D = aFactory.get3HitInterface("Angle3DSimple");
    EXPECT_DOUBLE_EQ(ang3D(hit1, hit2, hit3) , a3D.value(hit1, hit2, hit3));
    auto cRadHiOc = aFactory.get3HitInterface("DeltaCircleRadiusHighOccupancy");
    EXPECT_DOUBLE_EQ(cRadHiOc(hit1, hit2, hit3) , dcRad.value(hit1, hit2, hit3, hitIP));
    auto notExisting3 = aFactory.get3HitInterface("bla");
    EXPECT_DOUBLE_EQ(notExisting3(hit1, hit2, hit3) , 0.0);
    // 4 hits:
    auto ddcc = aFactory.get4HitInterface("DeltaDistCircleCenter");
    EXPECT_DOUBLE_EQ(ddcc(hit1, hit2, hit3, hit4) , dDcc.value(hit1, hit2, hit3, hit4));
    auto notExisting4 = aFactory.get4HitInterface("bla");
    EXPECT_DOUBLE_EQ(notExisting4(hit1, hit2, hit3, hit4) , 0.0);



    auto oldFilterFactory = XHitFilterFactory<DummyHit>();
    auto dist3Dold = oldFilterFactory.get2HitInterface("distance3D");
    auto distXYold = oldFilterFactory.get2HitInterface("distanceXY");
    EXPECT_NE(dist3Dold(hit1, hit2) , distXYold(hit1, hit2));

  }

}
