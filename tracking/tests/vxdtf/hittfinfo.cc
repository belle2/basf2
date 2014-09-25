#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/HitTFInfo.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {




  /** Set up a few arrays and objects in the datastore */
  class HitTFInfoTest : public ::testing::Test {
  protected:
  };

  /** Test empty HitTFInfo */
  TEST_F(HitTFInfoTest, testEmptyFilter)
  {
    HitTFInfo aHit = HitTFInfo();

    ASSERT_EQ(0., aHit.getSectorID());

  }

  /** Test simple Setters and Getters of a HitTFInfo Object */
  TEST_F(HitTFInfoTest, testHitInfos)
  {
    TVector3 testPoint(2, 1, 3);
    TVector3 testPoint2(0.1, 0.1, 0.3);

    HitTFInfo aHit = HitTFInfo();
    aHit.setSectorID(12);
    aHit.setPosition(testPoint);
    aHit.setHitSigma(testPoint2);
    aHit.setIsReal(1);

    ASSERT_EQ(12., aHit.getSectorID());
    ASSERT_EQ(testPoint, aHit.getPosition());
    ASSERT_EQ(testPoint2, aHit.getHitSigma());
    ASSERT_EQ(1, aHit.getIsReal());

  }

  /** Test assigned Cluster Logic of a Hit */
  TEST_F(HitTFInfoTest, testAssignedClusters)
  {

    HitTFInfo aHit = HitTFInfo();
    aHit.setSectorID(12);

    aHit.push_back_AssignedCluster(11);
    aHit.push_back_AssignedCluster(13);

    ASSERT_EQ(2, aHit.sizeAssignedCluster());

  }

  /** Test used counter Cell Logic of a Hit */
  TEST_F(HitTFInfoTest, testUseCounterCell)
  {

    HitTFInfo aHit = HitTFInfo();
    aHit.setSectorID(12);

    aHit.push_back_UseCounterCell(18);
    aHit.push_back_UseCounterCell(19);

    ASSERT_EQ(2, aHit.sizeUseCounterCell());

  }

  /** Test used counter TCID Logic of a Hit */
  TEST_F(HitTFInfoTest, testUseCounterTCIDs)
  {

    HitTFInfo aHit = HitTFInfo();
    aHit.setSectorID(12);

    aHit.push_back_UseCounterTCIDs(55);
    aHit.push_back_AllCounterTCIDs(55);

    aHit.push_back_UseCounterTCIDs(44);
    aHit.push_back_AllCounterTCIDs(44);

    // same TCID => no change
    aHit.push_back_AllCounterTCIDs(44);

    ASSERT_EQ(2, aHit.sizeUseCounterTCIDs());

    aHit.remove_UseCounterTCIDs(44);

    ASSERT_EQ(1, aHit.sizeUseCounterTCIDs());
    ASSERT_EQ(2, aHit.sizeAllCounterTCIDs());


  }

  /** Tests partice Information */
  TEST_F(HitTFInfoTest, testParticle)
  {

    bool false_item = false;
    bool true_item = true;

    HitTFInfo aHit = HitTFInfo();
    aHit.setSectorID(12);

    aHit.push_back_UsedParticles(make_pair(11, 0.5));
    aHit.push_back_UsedParticles(make_pair(13, 0.4));

    aHit.containsParticle(11);

    ASSERT_EQ(true_item, aHit.containsParticle(11));
    ASSERT_EQ(false_item, aHit.containsParticle(12));
    ASSERT_EQ(true_item, aHit.containsParticle(13));

    ASSERT_EQ(11, aHit.getMainParticle().first);
    ASSERT_EQ(0.5, aHit.getMainParticle().second);

    ASSERT_EQ(11, aHit.getInfoParticle(11).first);
    ASSERT_EQ(0.5, aHit.getInfoParticle(11).second);

  }


  /** Tests overlapped logic of a HitTFInfo-Object */
  TEST_F(HitTFInfoTest, testOverlapped)
  {
    bool false_item = false;
//     bool true_item = true;

    HitTFInfo aHit = HitTFInfo();
    aHit.setSectorID(12);

    aHit.setUseCounter(1);
    aHit.changeUseCounter(1);

    ASSERT_EQ(false_item, aHit.isOverlappedByCellsOrTCs());

    ASSERT_EQ(2., aHit.getMaxCounter());
    ASSERT_EQ(2., aHit.getUseCounter());

  }


}  // namespace
