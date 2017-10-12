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


#include <tracking/trackFindingVXD/filterTools/SelectionVariableFactory.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>

#include <tracking/vxdCaTracking/XHitFilterFactory.h>

using namespace std;

using namespace Belle2;

namespace SelFarFactoryTests {

  /** just a small proto-container storing coordinates */
  class DummyHit {
  public:
    /** std constructor */
    DummyHit(): m_x(0), m_y(0), m_z(0) {};

    /** constructor*/
    DummyHit(double x, double y, double z): m_x(x), m_y(y), m_z(z) {};

    /** destructor */
    virtual ~DummyHit() {};

    double X() const { return m_x; }/**< return x */
    double Y() const { return m_y; }/**< return y */
    double Z() const { return m_z; }/**< return z */

  private:
    double m_x; /**< x */
    double m_y; /**< y */
    double m_z; /**< z */
  };


  /** Test class for SelectionVariableFactory object. */
  class SelVarFactoryTest : public ::testing::Test {
  protected:
  };


  /** tests if I can get the correct results from a SelectionVariable I asked for */
  TEST_F(SelVarFactoryTest, TestSelVarFactory)
  {
    // home hits with coordinates:
    auto hit1 = DummyHit(1, 1, 1);
    auto hit2 = DummyHit(2, 2, 3);
    auto hit3 = DummyHit(2, 3, 4);
    auto hit4 = DummyHit(23, 42, 5);
    auto hitIP = DummyHit(0, 0, 0);
    // some SelectionVariables:
    auto d3D = Distance3DSquared<DummyHit>();
    auto a3D = Angle3DSimple<DummyHit>();
    auto dcRad = DeltaCircleRadius<DummyHit>();
    auto dDcc = DeltaDistCircleCenter<DummyHit>();

    auto aFactory = SelectionVariableFactory<DummyHit>();


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

  }



  /** tests if I can get the correct results from a FilterID I asked for */
  TEST_F(SelVarFactoryTest, XHitFilterFactory)
  {
    // home hits with coordinates:
    auto hit1 = DummyHit(1, 1, 1);
    auto thit1 = TVector3(1, 1, 1);
    auto hit2 = DummyHit(2, 2, 3);
    auto thit2 = TVector3(2, 2, 3);
    auto hit3 = DummyHit(2, 3, 4);
    auto thit3 = TVector3(2, 3, 4);
    auto hit4 = DummyHit(23, 42, 5);
    auto thit4 = TVector3(23, 42, 5);
    auto hitIP = DummyHit(0, 0, 0);
    auto thitIP = TVector3(0, 0, 0);

    // some Filters:
    TwoHitFilters twoHitFilters;
    ThreeHitFilters threeHitFilters;
    FourHitFilters fourHitFilters;

    auto aFactory = XHitFilterFactory<DummyHit>();


    // 2 hits:
    auto dist3D = aFactory.get2HitInterface("distance3D");
    twoHitFilters.resetValues(thit1, thit2);
    EXPECT_DOUBLE_EQ(dist3D(hit1, hit2) , twoHitFilters.calcDist3D());

    auto distXY = aFactory.get2HitInterface("distanceXY");
    twoHitFilters.resetValues(thit1, thit2);
    EXPECT_DOUBLE_EQ(distXY(hit1, hit2) , twoHitFilters.calcDistXY());


    auto a3DHiOc = aFactory.get2HitInterface("HioCangles3D");
    threeHitFilters.resetValues(thit1, thit2, thitIP);
    EXPECT_DOUBLE_EQ(a3DHiOc(hit1, hit2), threeHitFilters.calcAngle3D());

    auto notExisting2 = aFactory.get2HitInterface("bla");
    EXPECT_DOUBLE_EQ(notExisting2(hit1, hit2) , 0.0);


    // 3 hits:
    auto ang3D = aFactory.get3HitInterface("angles3D");
    threeHitFilters.resetValues(thit1, thit2, thit3);
    EXPECT_DOUBLE_EQ(ang3D(hit1, hit2, hit3) , threeHitFilters.calcAngle3D());

    auto dpTHiOc = aFactory.get3HitInterface("HioCdeltaPt");
    fourHitFilters.resetValues(thit1, thit2, thit3, thitIP);
    EXPECT_DOUBLE_EQ(dpTHiOc(hit1, hit2, hit3), fourHitFilters.calcDeltapT());

    auto notExisting3 = aFactory.get3HitInterface("bla");
    EXPECT_DOUBLE_EQ(notExisting3(hit1, hit2, hit3) , 0.0);


    // 4 hits:
    auto dpT = aFactory.get4HitInterface("deltaPt");
    fourHitFilters.resetValues(thit1, thit2, thit3, thit4);
    EXPECT_DOUBLE_EQ(dpT(hit1, hit2, hit3, hit4), fourHitFilters.calcDeltapT());
    auto notExisting4 = aFactory.get4HitInterface("bla");
    EXPECT_DOUBLE_EQ(notExisting4(hit1, hit2, hit3, hit4) , 0.0);

  }
}
