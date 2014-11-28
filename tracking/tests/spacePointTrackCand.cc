/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// google test framework
#include <gtest/gtest.h>

// class to be tested
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <boost/concept_check.hpp>

// stuff needed for setting up the tests

using namespace std;

namespace Belle2 {

  class SpacePointTrackCandTest : public ::testing::Test {
  public:
    /** this is a small helper function to create a sensorInfo to be used */
    VXD::SensorInfoBase createSensorInfo(VxdID aVxdID, double width = 1., double length = 1., double width2 = -1.) {
      // (SensorType type, VxdID id, double width, double length, double thickness, int uCells, int vCells, double width2=-1, double splitLength=-1, int vCells2=0)
      VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, width, length, 0.3, 2, 4, width2);

      TGeoRotation r1;
      r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
      TGeoTranslation t1(-10, 10, 1);
      TGeoCombiTrans c1(t1, r1);
      TGeoHMatrix transform = c1;
      sensorInfoBase.setTransformation(transform);

      return sensorInfoBase;
    }
  protected:
  };

  /**
   * Test the Constructor, that takes a vector of SpacePoint* as argument
   */
  TEST_F(SpacePointTrackCandTest, testConstructorFromVector)
  {
    // set up some SpacePoints and group them to a vector
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase aSensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);
    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    const SpacePoint aPXDSpacePoint = SpacePoint(&aCluster, &aSensorInfoBase);

    VxdID anotherVxdID = VxdID(3, 3, 3);
    VXD::SensorInfoBase anotherSensorInfoBase = createSensorInfo(anotherVxdID, 2.3, 4.2);
    SVDCluster aUCluster = SVDCluster(anotherVxdID, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1);
    SVDCluster aVCluster = SVDCluster(anotherVxdID, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1);
    std::vector<const SVDCluster*> a2HitCluster = { &aUCluster, &aVCluster };
    const SpacePoint aSVDSpacePoint = SpacePoint(a2HitCluster, &anotherSensorInfoBase);

    SVDCluster anotherUCluster = SVDCluster(anotherVxdID, true, 0.23, 0.1, 0.01, 0.001, 1, 1, 1);
    std::vector<const SVDCluster*> a1HitCluster = { &anotherUCluster };
    const SpacePoint anotherSVDSpacePoint = SpacePoint(a1HitCluster, &anotherSensorInfoBase);

    const std::vector<const Belle2::SpacePoint*> aSpacePointVec = { &aPXDSpacePoint, &aSVDSpacePoint, &anotherSVDSpacePoint };


    // construct SpacePointTrackCand with (more or less arbitrary) values for pdg code, charge and MC-TrackID
    SpacePointTrackCand aSpacePointTC = SpacePointTrackCand(aSpacePointVec, 11, -1, 23);

    // do some checks
    // check if all SpacePoints are actually in the TrackCand
    EXPECT_EQ(aSpacePointTC.getNHits(), aSpacePointVec.size());
    // check pdg, charge, etc...
    EXPECT_EQ(aSpacePointTC.getPdgCode(), 11);
    EXPECT_DOUBLE_EQ(aSpacePointTC.getChargeSeed(), -1);
    EXPECT_EQ(aSpacePointTC.getMcTrackID(), 23);

    // get SpacePoints and compare them with the original vector
    const std::vector<const SpacePoint*> returnSPVector = aSpacePointTC.getHits();
    for (unsigned int i = 0; i < aSpacePointTC.getNHits(); i++) {
      EXPECT_TRUE(returnSPVector[i] == aSpacePointVec[i]);
    }
  }

  /**
   * Test operator == of SpacePointTrackCand
   */
  TEST_F(SpacePointTrackCandTest, testEqualityOperator)
  {
    // set up some set up some space points and add them to SpacePointTrackCands
    // many parts copied from spacePoint.cc (tracking/tests/)

    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);
    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    const SpacePoint aPXDSpacePoint = SpacePoint(&aCluster, &sensorInfoBase);

    VxdID anotherVxdID = VxdID(3, 3, 3);
    VXD::SensorInfoBase anotherSensorInfoBase = createSensorInfo(anotherVxdID, 2.3, 4.2);
    SVDCluster aUCluster = SVDCluster(anotherVxdID, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1);
    SVDCluster aVCluster = SVDCluster(anotherVxdID, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1);
    std::vector<const SVDCluster*> a2HitCluster = { &aUCluster, &aVCluster };
    const SpacePoint aSVDSpacePoint = SpacePoint(a2HitCluster, &anotherSensorInfoBase);

    SVDCluster anotherUCluster = SVDCluster(anotherVxdID, true, 0.23, 0.1, 0.01, 0.001, 1, 1, 1);
    std::vector<const SVDCluster*> a1HitCluster = { &anotherUCluster };
    const SpacePoint anotherSVDSpacePoint = SpacePoint(a1HitCluster, &anotherSensorInfoBase);

    const std::vector<const Belle2::SpacePoint*> aSpacePointVec = { &aPXDSpacePoint, &aSVDSpacePoint };
    const std::vector<const Belle2::SpacePoint*> sameSpacePointVec = { &aPXDSpacePoint, &aSVDSpacePoint };
    const std::vector<const Belle2::SpacePoint*> anotherSpacePointVec = { &aPXDSpacePoint, &anotherSVDSpacePoint };

    // set up two SpacePointTrackCands from the SpacePoint* vectors above
    Belle2::SpacePointTrackCand aSpacePointTC = SpacePointTrackCand(aSpacePointVec, 1);
    Belle2::SpacePointTrackCand anotherSpacePointTC = SpacePointTrackCand(sameSpacePointVec);

    // getting undefined reference here!
    EXPECT_TRUE(aSpacePointTC == anotherSpacePointTC);

    // change pdg code and compare again (should not have any influence)
    anotherSpacePointTC.setPdgCode(-11);
    EXPECT_TRUE(aSpacePointTC == anotherSpacePointTC);

    // add a space point and compare again, should now give false
    anotherSpacePointTC.addSpacePoint(&anotherSVDSpacePoint);

    EXPECT_FALSE(aSpacePointTC == anotherSpacePointTC);

    // create another TC with different SVD SpacePoint and compare again with the previously created TCs.
    Belle2::SpacePointTrackCand changedSVDSpacePointTC = SpacePointTrackCand(anotherSpacePointVec, 2, 3, 4);

    EXPECT_FALSE(aSpacePointTC == changedSVDSpacePointTC);
    EXPECT_FALSE(changedSVDSpacePointTC == anotherSpacePointTC);

    // create an empty TC and add SpacePoints by hand and compare again with the other TCs
    Belle2::SpacePointTrackCand newSpacePointTC;
    newSpacePointTC.addSpacePoint(&aPXDSpacePoint);
    newSpacePointTC.addSpacePoint(&aSVDSpacePoint);

    EXPECT_TRUE(newSpacePointTC == aSpacePointTC);

    newSpacePointTC.addSpacePoint(&anotherSVDSpacePoint);

    EXPECT_TRUE(newSpacePointTC == anotherSpacePointTC);

  }

  /**
   * Test setPdgCode method, by comparing its results with the expected values for the according particles
   */
  TEST_F(SpacePointTrackCandTest, testSetPdgCode)
  {
    SpacePointTrackCand aSpacePointTC;

    // pdg code of electron is 11
    aSpacePointTC.setPdgCode(11);
    EXPECT_DOUBLE_EQ(aSpacePointTC.getChargeSeed(), -1);
    // positron
    aSpacePointTC.setPdgCode(-11);
    EXPECT_DOUBLE_EQ(aSpacePointTC.getChargeSeed(), 1);

    // B+ Meson
    aSpacePointTC.setPdgCode(521);
    EXPECT_DOUBLE_EQ(aSpacePointTC.getChargeSeed(), 1);
  }
}