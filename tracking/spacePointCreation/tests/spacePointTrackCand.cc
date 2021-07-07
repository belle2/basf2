/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/Const.h>

// google test framework
#include <gtest/gtest.h>

// class to be tested
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

// stuff needed for setting up the tests

using namespace std;

namespace Belle2 {

  /**
   * Test class for the SpacePointTrackCand class
   */
  class SpacePointTrackCandTest : public ::testing::Test {
  public:
    /** this is a small helper function to create a sensorInfo to be used */
    VXD::SensorInfoBase createSensorInfo(VxdID aVxdID, double width = 1., double length = 1., double width2 = -1.)
    {
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
    SVDCluster aUCluster = SVDCluster(anotherVxdID, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    SVDCluster aVCluster = SVDCluster(anotherVxdID, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> a2HitCluster = { &aUCluster, &aVCluster };
    const SpacePoint aSVDSpacePoint = SpacePoint(a2HitCluster, &anotherSensorInfoBase);

    SVDCluster anotherUCluster = SVDCluster(anotherVxdID, true, 0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> a1HitCluster = { &anotherUCluster };
    const SpacePoint anotherSVDSpacePoint = SpacePoint(a1HitCluster, &anotherSensorInfoBase);

    const std::vector<const Belle2::SpacePoint*> aSpacePointVec = { &aPXDSpacePoint, &aSVDSpacePoint, &anotherSVDSpacePoint };


    // construct SpacePointTrackCand with (more or less arbitrary) values for pdg code, charge and MC-TrackID
    SpacePointTrackCand aSpacePointTC = SpacePointTrackCand(aSpacePointVec, 11, -1, 23);

    // do some checks
    // check if all SpacePoints are actually in the TrackCand
    EXPECT_EQ(aSpacePointTC.getNHits(), aSpacePointVec.size());
    // check pdg, charge, etc...
    EXPECT_EQ(aSpacePointTC.getPdgCode(), Const::electron.getPDGCode());
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
    SVDCluster aUCluster = SVDCluster(anotherVxdID, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    SVDCluster aVCluster = SVDCluster(anotherVxdID, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> a2HitCluster = { &aUCluster, &aVCluster };
    const SpacePoint aSVDSpacePoint = SpacePoint(a2HitCluster, &anotherSensorInfoBase);

    SVDCluster anotherUCluster = SVDCluster(anotherVxdID, true, 0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
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
    anotherSpacePointTC.setPdgCode(-Const::electron.getPDGCode());
    EXPECT_TRUE(aSpacePointTC == anotherSpacePointTC);

    // add a space point with arbitrary sorting parameter and compare again, should now give false
    anotherSpacePointTC.addSpacePoint(&anotherSVDSpacePoint, 1.0);

    EXPECT_FALSE(aSpacePointTC == anotherSpacePointTC);

    // create another TC with different SVD SpacePoint and compare again with the previously created TCs.
    Belle2::SpacePointTrackCand changedSVDSpacePointTC = SpacePointTrackCand(anotherSpacePointVec, 2, 3, 4);

    EXPECT_FALSE(aSpacePointTC == changedSVDSpacePointTC);
    EXPECT_FALSE(changedSVDSpacePointTC == anotherSpacePointTC);

    // create an empty TC and add SpacePoints by hand and compare again with the other TCs
    Belle2::SpacePointTrackCand newSpacePointTC;
    newSpacePointTC.addSpacePoint(&aPXDSpacePoint, 1.0);
    newSpacePointTC.addSpacePoint(&aSVDSpacePoint, 1.1);

    EXPECT_TRUE(newSpacePointTC == aSpacePointTC);

    newSpacePointTC.addSpacePoint(&anotherSVDSpacePoint, 1.2);

    EXPECT_TRUE(newSpacePointTC == anotherSpacePointTC);

  }

  /**
   * Test setPdgCode method, by comparing its results with the expected values for the according particles
   */
  TEST_F(SpacePointTrackCandTest, testSetPdgCode)
  {
    SpacePointTrackCand aSpacePointTC;

    // electron
    aSpacePointTC.setPdgCode(Const::electron.getPDGCode());
    EXPECT_DOUBLE_EQ(aSpacePointTC.getChargeSeed(), -1);
    // positron
    aSpacePointTC.setPdgCode(-Const::electron.getPDGCode());
    EXPECT_DOUBLE_EQ(aSpacePointTC.getChargeSeed(), 1);

    // B+ Meson
    aSpacePointTC.setPdgCode(521);
    EXPECT_DOUBLE_EQ(aSpacePointTC.getChargeSeed(), 1);
  }

  /**
   * Test the get hits in range method
   */
  TEST_F(SpacePointTrackCandTest, testGetHitsInRange)
  {
    SpacePointTrackCand fullTrackCand;

    // set up some SpacePoints and add them to a SpacePointTrackCand
    VxdID aVxdID1 = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase1 = createSensorInfo(aVxdID1, 2.3, 4.2);
    PXDCluster aCluster = PXDCluster(aVxdID1, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    const SpacePoint aPXDSpacePoint = SpacePoint(&aCluster, &sensorInfoBase1);
    fullTrackCand.addSpacePoint(&aPXDSpacePoint, 1.23); // add a SpacePoint with an arbitrary sorting parameter

    VxdID aVxdID2 = VxdID(2, 2, 2);
    VXD::SensorInfoBase sensorInfoBase2 = createSensorInfo(aVxdID2, 2.3, 4.2);
    SVDCluster aUCluster = SVDCluster(aVxdID2, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    SVDCluster aVCluster = SVDCluster(aVxdID2, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> UVClusterVec1 = { &aUCluster, &aVCluster };
    std::vector<const SVDCluster*> VClusterVec = { &aVCluster };
    std::vector<const SVDCluster*> UClusterVec = { &aUCluster };
    const SpacePoint SVDSpacePoint1 = SpacePoint(UVClusterVec1, &sensorInfoBase2);
    const SpacePoint SVDSpacePoint2 = SpacePoint(VClusterVec, &sensorInfoBase2);
    const SpacePoint SVDSpacePoint3 = SpacePoint(UClusterVec, &sensorInfoBase2);
    fullTrackCand.addSpacePoint(&SVDSpacePoint1, 2.34);
    fullTrackCand.addSpacePoint(&SVDSpacePoint2, 3.45);
    fullTrackCand.addSpacePoint(&SVDSpacePoint3, 4.56);

    VxdID aVxdId3 = VxdID(3, 3, 3);
    VXD::SensorInfoBase sensorInfoBase3 = createSensorInfo(aVxdId3, 2.3, 4.2);
    SVDCluster aUCluster2 = SVDCluster(aVxdId3, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    SVDCluster aVCluster2 = SVDCluster(aVxdId3, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> UVClusterVec2 = { &aUCluster2, &aVCluster2 };
    std::vector<const SVDCluster*> VClusterVec2 = { &aVCluster2 };
    std::vector<const SVDCluster*> UClusterVec2 = { &aUCluster2 };
    const SpacePoint SVDSpacePoint4 = SpacePoint(UVClusterVec2, &sensorInfoBase3);
    const SpacePoint SVDSpacePoint5 = SpacePoint(VClusterVec2, &sensorInfoBase3);
    const SpacePoint SVDSpacePoint6 = SpacePoint(UClusterVec2, &sensorInfoBase3);
    fullTrackCand.addSpacePoint(&SVDSpacePoint4, 5.67);
    fullTrackCand.addSpacePoint(&SVDSpacePoint5, 6.78);
    fullTrackCand.addSpacePoint(&SVDSpacePoint6, 7.89);

    // set up some 'shorter' SpacePointTrackCands
    SpacePointTrackCand shortTC1;
    shortTC1.addSpacePoint(&aPXDSpacePoint, 1.23);
    shortTC1.addSpacePoint(&SVDSpacePoint1, 2.34);
    shortTC1.addSpacePoint(&SVDSpacePoint2, 3.45);

    SpacePointTrackCand shortTC2;
    shortTC2.addSpacePoint(&SVDSpacePoint2, 3.45);
    shortTC2.addSpacePoint(&SVDSpacePoint3, 4.56);
    shortTC2.addSpacePoint(&SVDSpacePoint4, 5.67);
    shortTC2.addSpacePoint(&SVDSpacePoint5, 6.78);

    SpacePointTrackCand subTrackCand1 = SpacePointTrackCand(fullTrackCand.getHitsInRange(0, 3));
    EXPECT_TRUE(subTrackCand1 == shortTC1);
    std::vector<double> sortParams1 = { 1.23, 2.34, 3.45 };
    std::vector<double> sortParamsTC1 = fullTrackCand.getSortingParametersInRange(0, 3);
    EXPECT_EQ(sortParams1.size(), sortParamsTC1.size());
    for (unsigned int i = 0; i < sortParamsTC1.size(); ++i) { EXPECT_DOUBLE_EQ(sortParams1.at(i), sortParamsTC1.at(i)); }

    SpacePointTrackCand subTrackCand2 = SpacePointTrackCand(fullTrackCand.getHitsInRange(2, 6));
    EXPECT_TRUE(subTrackCand2 == shortTC2);
    std::vector<double> sortParams2 = { 3.45, 4.56, 5.67, 6.78 };
    std::vector<double> sortParamsTC2 = fullTrackCand.getSortingParametersInRange(2, 6);
    EXPECT_EQ(sortParams2.size(), sortParamsTC2.size());
    for (unsigned int i = 0; i < sortParamsTC2.size(); ++i) { EXPECT_DOUBLE_EQ(sortParams2.at(i), sortParamsTC2.at(i)); }

    // test throwing of exceptions
    unsigned int nHits = fullTrackCand.getNHits();
    ASSERT_THROW(fullTrackCand.getHitsInRange(0, nHits + 1),
                 SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to too high final index
    ASSERT_NO_THROW(fullTrackCand.getHitsInRange(nHits - 2, nHits));
    ASSERT_THROW(fullTrackCand.getHitsInRange(-1, 3),
                 SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to negative starting index
    ASSERT_THROW(fullTrackCand.getHitsInRange(0, 10), SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to too high final index
    ASSERT_THROW(fullTrackCand.getHitsInRange(10, 0),
                 SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to too high starting index
    ASSERT_THROW(fullTrackCand.getHitsInRange(2, -1), SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to too low final index
    ASSERT_THROW(fullTrackCand.getSortingParametersInRange(-1, 2),
                 SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to negative starting index
    ASSERT_THROW(fullTrackCand.getSortingParametersInRange(0, nHits + 1),
                 SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to too high final index
    ASSERT_THROW(fullTrackCand.getSortingParametersInRange(12, 3),
                 SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to too high starting index
    ASSERT_THROW(fullTrackCand.getSortingParametersInRange(2, -2),
                 SpacePointTrackCand::SPTCIndexOutOfBounds); // throw due to too low final index
    ASSERT_NO_THROW(fullTrackCand.getSortingParametersInRange(0, nHits));
  }

  /**
   * Test the setRefereeStatus and getRefereeStatus methods
   */
  TEST_F(SpacePointTrackCandTest, testRefereeStatus)
  {
    SpacePointTrackCand trackCand; // default constructor -> should initialize the referee status to c_initialState

    // currently c_isActive is the initialState:
    EXPECT_EQ(trackCand.getRefereeStatus(), SpacePointTrackCand::c_isActive);
    EXPECT_EQ(SpacePointTrackCand::c_initialState, SpacePointTrackCand::c_isActive);

    unsigned short int initialStatus = trackCand.getRefereeStatus();
    EXPECT_EQ(initialStatus, SpacePointTrackCand::c_initialState);

    // design an arbitrary status and set it, then test if the returned status is the set status
    unsigned short int newStatus = 0;
    newStatus += SpacePointTrackCand::c_checkedByReferee;
    newStatus += SpacePointTrackCand::c_checkedTrueHits;
    trackCand.setRefereeStatus(newStatus);
    EXPECT_EQ(trackCand.getRefereeStatus(), newStatus);

    // clear the status and test if its 0
    trackCand.clearRefereeStatus();
    EXPECT_EQ(trackCand.getRefereeStatus(), 0);

    // reset the status and test if it's the initial state
    trackCand.resetRefereeStatus();
    EXPECT_EQ(trackCand.getRefereeStatus(), SpacePointTrackCand::c_initialState);


    trackCand.clearRefereeStatus(); // enforce empty refereeStatus
    // add several statusses one by one and test if the overall status is as expected (once with 'overall status' and once with status one by one)
    trackCand.addRefereeStatus(SpacePointTrackCand::c_checkedClean);
    EXPECT_TRUE(trackCand.hasRefereeStatus(SpacePointTrackCand::c_checkedClean));
    trackCand.addRefereeStatus(SpacePointTrackCand::c_hitsOnSameSensor);
    EXPECT_TRUE(trackCand.hasRefereeStatus(SpacePointTrackCand::c_hitsOnSameSensor));
    EXPECT_FALSE(trackCand.hasRefereeStatus(SpacePointTrackCand::c_hitsLowDistance));

    trackCand.addRefereeStatus(SpacePointTrackCand::c_checkedByReferee);
    unsigned short int expectedStatus = SpacePointTrackCand::c_checkedClean + SpacePointTrackCand::c_hitsOnSameSensor +
                                        SpacePointTrackCand::c_checkedByReferee;
    EXPECT_EQ(trackCand.getRefereeStatus(), expectedStatus);


    // remove one status and test if it actuall gets removed
    trackCand.removeRefereeStatus(SpacePointTrackCand::c_checkedClean);
    expectedStatus -= SpacePointTrackCand::c_checkedClean;
    EXPECT_EQ(trackCand.getRefereeStatus(), expectedStatus);
    EXPECT_FALSE(trackCand.hasRefereeStatus(SpacePointTrackCand::c_checkedClean));
    trackCand.removeRefereeStatus(SpacePointTrackCand::c_checkedClean); // remove again and check if nothing changes
    EXPECT_EQ(trackCand.getRefereeStatus(), expectedStatus);

    EXPECT_TRUE(trackCand.hasRefereeStatus(expectedStatus));

    // test if the status that remains is still set
    EXPECT_TRUE(trackCand.hasRefereeStatus(SpacePointTrackCand::c_hitsOnSameSensor));
    EXPECT_TRUE(trackCand.hasHitsOnSameSensor()); // echeck the wrapper function


    // check if adding of the same status twice works
    trackCand.clearRefereeStatus();
    trackCand.addRefereeStatus(SpacePointTrackCand::c_checkedSameSensors);
    trackCand.addRefereeStatus(SpacePointTrackCand::c_checkedSameSensors);
    EXPECT_TRUE(trackCand.hasRefereeStatus(SpacePointTrackCand::c_checkedSameSensors));
    EXPECT_TRUE(trackCand.checkedSameSensors());

    trackCand.addRefereeStatus(SpacePointTrackCand::c_curlingTrack);
    EXPECT_TRUE(trackCand.isCurling());


    trackCand.clearRefereeStatus();
    // test that at least n bits are present for storing stuff in the m_refereeStatus (update this test if you add a new flag to the RefereeStatusBit)
    // WARNING: hardcoded -> make sure to keep this thing up to date!
    int n = 13;
    int maxUsedStatus = pow(2, n);
    trackCand.addRefereeStatus(maxUsedStatus);
    EXPECT_TRUE(trackCand.hasRefereeStatus(maxUsedStatus));

    int maxN = 15; // unsigned short int has 2 bytes (-> 16 bits) to store information
    int maxPossibleStatus = pow(2, maxN); // maximum possible status to store
    trackCand.addRefereeStatus(maxPossibleStatus);
    EXPECT_TRUE(trackCand.hasRefereeStatus(maxPossibleStatus));

    int impossibleStatus = maxPossibleStatus * 2; // create an integer that is too big to be stored in a unsigned short integer
    trackCand.clearRefereeStatus();
    trackCand.setRefereeStatus(impossibleStatus); // this should lead to an overflow in SpacePointTrackCand::m_refereeStatus
    EXPECT_NE(trackCand.getRefereeStatus(), impossibleStatus); // -> overflow leads this test to fail

    trackCand.clearRefereeStatus();
    int wrongStatus = 7; // set a wrong status, that causes some of the checks to be true although they shouldn't
    trackCand.addRefereeStatus(wrongStatus);
    EXPECT_TRUE(trackCand.hasRefereeStatus(1));
    EXPECT_TRUE(trackCand.hasRefereeStatus(2));
    EXPECT_TRUE(trackCand.hasRefereeStatus(4));
    wrongStatus = 15;
    trackCand.addRefereeStatus(wrongStatus);
    EXPECT_TRUE(trackCand.hasRefereeStatus(8));
  }

  /**
   * Test the removeSpacePoint method
   */
  TEST_F(SpacePointTrackCandTest, testRemoveSpacePoints)
  {
    // set up SpacePointTrackCand
    SpacePointTrackCand fullTrackCand;

    // set up some SpacePoints and add them to a SpacePointTrackCand
    VxdID aVxdID1 = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase1 = createSensorInfo(aVxdID1, 2.3, 4.2);
    PXDCluster aCluster = PXDCluster(aVxdID1, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    const SpacePoint aPXDSpacePoint = SpacePoint(&aCluster, &sensorInfoBase1);
    fullTrackCand.addSpacePoint(&aPXDSpacePoint, 1.23); // add a SpacePoint with an arbitrary sorting parameter

    VxdID aVxdID2 = VxdID(2, 2, 2);
    VXD::SensorInfoBase sensorInfoBase2 = createSensorInfo(aVxdID2, 2.3, 4.2);
    SVDCluster aUCluster = SVDCluster(aVxdID2, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    SVDCluster aVCluster = SVDCluster(aVxdID2, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> UVClusterVec1 = { &aUCluster, &aVCluster };
    std::vector<const SVDCluster*> VClusterVec = { &aVCluster };
    std::vector<const SVDCluster*> UClusterVec = { &aUCluster };
    const SpacePoint SVDSpacePoint1 = SpacePoint(UVClusterVec1, &sensorInfoBase2);
    const SpacePoint SVDSpacePoint2 = SpacePoint(VClusterVec, &sensorInfoBase2);
    const SpacePoint SVDSpacePoint3 = SpacePoint(UClusterVec, &sensorInfoBase2);
    fullTrackCand.addSpacePoint(&SVDSpacePoint1, 2.34);
    fullTrackCand.addSpacePoint(&SVDSpacePoint2, 3.45);
    fullTrackCand.addSpacePoint(&SVDSpacePoint3, 4.56);

    // set up a second TC, but ommit one SpacePoint
    SpacePointTrackCand expectedTC;
    expectedTC.addSpacePoint(&aPXDSpacePoint, 1.23);
    expectedTC.addSpacePoint(&SVDSpacePoint2, 3.45);
    expectedTC.addSpacePoint(&SVDSpacePoint3, 4.56);

    // remove SpcePoint from first TrackCand amd compare it with the expected TrackCand
    fullTrackCand.removeSpacePoint(1);
    EXPECT_TRUE(expectedTC == fullTrackCand);

    std::vector<double> expectedSortParams = { 1.23, 3.45, 4.56 };
    std::vector<double> sortParams = fullTrackCand.getSortingParameters();
    EXPECT_EQ(expectedSortParams.size(), sortParams.size());
    for (unsigned int i = 0; i < sortParams.size(); ++i) { EXPECT_DOUBLE_EQ(sortParams.at(i), expectedSortParams.at(i)); }

    // try to remove a SpacePoint that is out of bounds
    ASSERT_THROW(fullTrackCand.removeSpacePoint(fullTrackCand.getNHits()), SpacePointTrackCand::SPTCIndexOutOfBounds);
  }

  /**
   * Test setPdgCode method, by comparing its results with the expected values for the according particles
   */
  TEST_F(SpacePointTrackCandTest, testGetSortedHits)
  {
    // set up some SpacePoints and add them to a SpacePointTrackCand
    VxdID aVxdID1 = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase1 = createSensorInfo(aVxdID1, 2.3, 4.2);
    PXDCluster aCluster = PXDCluster(aVxdID1, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    const SpacePoint aPXDSpacePoint = SpacePoint(&aCluster, &sensorInfoBase1);

    VxdID aVxdID2 = VxdID(2, 2, 2);
    VXD::SensorInfoBase sensorInfoBase2 = createSensorInfo(aVxdID2, 2.3, 4.2);
    SVDCluster aUCluster = SVDCluster(aVxdID2, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    SVDCluster aVCluster = SVDCluster(aVxdID2, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> UVClusterVec1 = { &aUCluster, &aVCluster };
    std::vector<const SVDCluster*> VClusterVec = { &aVCluster };
    std::vector<const SVDCluster*> UClusterVec = { &aUCluster };
    const SpacePoint SVDSpacePoint1 = SpacePoint(UVClusterVec1, &sensorInfoBase2);
    const SpacePoint SVDSpacePoint2 = SpacePoint(VClusterVec, &sensorInfoBase2);
    const SpacePoint SVDSpacePoint3 = SpacePoint(UClusterVec, &sensorInfoBase2);

    VxdID aVxdId3 = VxdID(3, 3, 3);
    VXD::SensorInfoBase sensorInfoBase3 = createSensorInfo(aVxdId3, 2.3, 4.2);
    SVDCluster aUCluster2 = SVDCluster(aVxdId3, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    SVDCluster aVCluster2 = SVDCluster(aVxdId3, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1, 1.0);
    std::vector<const SVDCluster*> UVClusterVec2 = { &aUCluster2, &aVCluster2 };
    std::vector<const SVDCluster*> VClusterVec2 = { &aVCluster2 };
    std::vector<const SVDCluster*> UClusterVec2 = { &aUCluster2 };
    const SpacePoint SVDSpacePoint4 = SpacePoint(UVClusterVec2, &sensorInfoBase3);
    const SpacePoint SVDSpacePoint5 = SpacePoint(VClusterVec2, &sensorInfoBase3);
    const SpacePoint SVDSpacePoint6 = SpacePoint(UClusterVec2, &sensorInfoBase3);


    SpacePointTrackCand unsortedConstructed;
    // add a SpacePoint with sorting parameter
    unsortedConstructed.addSpacePoint(&aPXDSpacePoint, 1.0);
    // add some SVD SpacePoints in logical order
    unsortedConstructed.addSpacePoint(&SVDSpacePoint1, 2.34);
    unsortedConstructed.addSpacePoint(&SVDSpacePoint2, 3.45);
    unsortedConstructed.addSpacePoint(&SVDSpacePoint3, 4.56);
    // add some SpacePoints in 'wrong' order
    unsortedConstructed.addSpacePoint(&SVDSpacePoint4, 5.67);
    unsortedConstructed.addSpacePoint(&SVDSpacePoint6, 7.89);
    unsortedConstructed.addSpacePoint(&SVDSpacePoint5, 6.78);

    // set up a SpacePointTrackCands already properly sorted
    SpacePointTrackCand sortedConstructed;
    sortedConstructed.addSpacePoint(&aPXDSpacePoint, 1.23);
    sortedConstructed.addSpacePoint(&SVDSpacePoint1, 2.34);
    sortedConstructed.addSpacePoint(&SVDSpacePoint2, 3.45);
    sortedConstructed.addSpacePoint(&SVDSpacePoint3, 4.56);
    sortedConstructed.addSpacePoint(&SVDSpacePoint4, 5.67);
    sortedConstructed.addSpacePoint(&SVDSpacePoint5, 6.78);
    sortedConstructed.addSpacePoint(&SVDSpacePoint6, 7.89);

    // test sorting
    SpacePointTrackCand sorted = SpacePointTrackCand(unsortedConstructed.getSortedHits());
    // == compares the spacepoints
    EXPECT_TRUE(sorted == sortedConstructed);

    SpacePointTrackCand nothingChanged = SpacePointTrackCand(sortedConstructed.getSortedHits());
    EXPECT_TRUE(nothingChanged == sortedConstructed);


    std::vector<double> sortParams1 = { 1.0, 2.0, 3.0 , 4.0, 5.0, 7.0, 6.0};
    sortedConstructed.setSortingParameters(sortParams1);

    // verify that sorting changed according to new sortParams
    SpacePointTrackCand unsorted = SpacePointTrackCand(sortedConstructed.getSortedHits());
    EXPECT_TRUE(unsorted == unsortedConstructed);
  }
}
