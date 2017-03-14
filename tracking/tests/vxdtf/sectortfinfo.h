#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/SectorTFInfo.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class SectorTFInfoTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(SectorTFInfoTest, testEmptyFilter)
  {
    SectorTFInfo aSector = SectorTFInfo();

    ASSERT_EQ(0., aSector.getSectorID());

  }

  /** Tests the points of the sector */
  TEST_F(SectorTFInfoTest, testPoints)
  {
    TVector3 testPoint(2, 1, 3);

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setSectorID(12);
    aSector.setPoint(0, testPoint);

    ASSERT_EQ(12., aSector.getSectorID());
    ASSERT_EQ(testPoint, aSector.getPoint(0));

  }

  /** Tests the friends of the sector */
  TEST_F(SectorTFInfoTest, testFriends)
  {
    std::vector<unsigned int> testFriends = {1, 2, 3};
    bool false_item = false;

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setAllFriends(testFriends);
    aSector.setIsOnlyFriend(false_item);

    ASSERT_EQ(1., aSector.getFriends().at(0));
    ASSERT_EQ(false_item, aSector.getIsOnlyFriend());
    ASSERT_EQ(3., aSector.sizeFriends());

  }


  /** Tests the overlapped logic of a sector*/
  TEST_F(SectorTFInfoTest, testOverlapped)
  {
    bool false_item = false;
    bool true_item = true;

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setSectorID(12);

    aSector.setUseCounter(1);
    aSector.changeUseCounter(1);

    ASSERT_EQ(true_item, aSector.isOverlapped());

    aSector.changeUseCounter(-1);

    ASSERT_EQ(false_item, aSector.isOverlapped());
    ASSERT_EQ(false_item, aSector.isOverlapped());

    ASSERT_EQ(2., aSector.getMaxCounter());
    ASSERT_EQ(1., aSector.getUseCounter());

  }


  /** Tests the display options of a sector */
  TEST_F(SectorTFInfoTest, testDisplay)
  {
    TVector3 testPoint(2, 1, 3);

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setSectorID(12);
    aSector.setPoint(0, testPoint);

    ASSERT_EQ(12., aSector.getSectorID());
    ASSERT_EQ(testPoint, aSector.getPoint(0));
    ASSERT_EQ(testPoint, aSector.getCoordinates()[0]);
    ASSERT_EQ(testPoint, aSector.getCoordinates()[4]);
    ASSERT_EQ(5., aSector.getCoordinates().size());

    aSector.getDisplayAlternativeBox();
    aSector.getDisplayInformation();

  }



}
