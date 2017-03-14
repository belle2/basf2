#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/CellTFInfo.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class CellTFInfoTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(CellTFInfoTest, testEmptyFilter)
  {
    CellTFInfo aCell = CellTFInfo();

    ASSERT_EQ(0., aCell.getState());

    ASSERT_EQ(-1., aCell.getPassIndex());

    ASSERT_EQ("", aCell.getDiedAt());

  }

  /** tests filled filters. */
  TEST_F(CellTFInfoTest, testFilledFilter)
  {
    bool false_item = false;

    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit(2, 3, 4);

    CellTFInfo aCell = CellTFInfo();

    aCell.setState(1);
    aCell.changeState(1);

    aCell.setPassIndex(7);
    aCell.setDiedAt("Hallo");

    ASSERT_EQ(2., aCell.getState());

    ASSERT_EQ(false_item, aCell.getActive());

    ASSERT_EQ(7., aCell.getPassIndex());

    ASSERT_EQ("Hallo", aCell.getDiedAt());

  }


  /** tests assigned hits. */
  TEST_F(CellTFInfoTest, testAssignedHits)
  {
    bool false_item = false;

    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit(2, 3, 4);

    CellTFInfo aCell = CellTFInfo();
    aCell.push_back_AssignedHits(1, outerHit);
    aCell.push_back_AssignedHits(2, innerHit);


    aCell.setState(1);
    aCell.changeState(1);

    aCell.setPassIndex(7);
    aCell.setDiedAt("Hallo");

    EXPECT_DOUBLE_EQ(3., aCell.getCoordinates().at(1).Z());
    EXPECT_DOUBLE_EQ(2., aCell.getCoordinates().at(1).Y());
    EXPECT_DOUBLE_EQ(1., aCell.getCoordinates().at(1).X());

    EXPECT_DOUBLE_EQ(4., aCell.getCoordinates().at(0).Z());
    EXPECT_DOUBLE_EQ(3., aCell.getCoordinates().at(0).Y());
    EXPECT_DOUBLE_EQ(2., aCell.getCoordinates().at(0).X());

    ASSERT_EQ(2., aCell.getState());

    ASSERT_EQ(false_item, aCell.getActive());

    ASSERT_EQ(7., aCell.getPassIndex());

    ASSERT_EQ("Hallo", aCell.getDiedAt());
  }



  /** tests neighbours. */
  TEST_F(CellTFInfoTest, testNeighbours)
  {
//     bool false_item = false;

    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit(2, 3, 4);

    std::vector<int> allNeighbours = {12, 13, 14, 15, 16};

    CellTFInfo aCell = CellTFInfo();
    aCell.push_back_Neighbours(1);
    aCell.insert_Neighbours(allNeighbours);

    aCell.setPassIndex(7);
    aCell.setDiedAt("Hallo");

    ASSERT_EQ(6., aCell.sizeNeighbours());
    ASSERT_EQ(12., aCell.getNeighbours().at(1));
  }



  /** Tests partice Information */
  TEST_F(CellTFInfoTest, testParticle)
  {
    bool false_item = false;
    bool true_item = true;

    CellTFInfo aCell = CellTFInfo();

    aCell.push_back_UsedParticles(std::make_pair(11, 0.5));
    aCell.push_back_UsedParticles(std::make_pair(13, 0.4));

    aCell.setIsReal(1);

    aCell.containsParticle(11);

    ASSERT_EQ(true_item, aCell.containsParticle(11));
    ASSERT_EQ(false_item, aCell.containsParticle(12));
    ASSERT_EQ(true_item, aCell.containsParticle(13));

    ASSERT_EQ(11, aCell.getMainParticle().first);
    ASSERT_EQ(0.5, aCell.getMainParticle().second);

    ASSERT_EQ(11, aCell.getInfoParticle(11).first);
    ASSERT_EQ(0.5, aCell.getInfoParticle(11).second);

    ASSERT_EQ(true_item, aCell.getIsReal());
  }


  /** Tests the overlapped logic of a sector*/
  TEST_F(CellTFInfoTest, testOverlapped)
  {
    bool false_item = false;
    bool true_item = true;

    CellTFInfo aCell = CellTFInfo();

    aCell.setUseCounter(1);
    aCell.changeUseCounter(1);

    ASSERT_EQ(true_item, aCell.isOverlappedByTC());

    aCell.changeUseCounter(-1);

    ASSERT_EQ(false_item, aCell.isOverlappedByTC());

    ASSERT_EQ(2., aCell.getMaxCounter());
    ASSERT_EQ(1., aCell.getUseCounter());

  }


  /** Tests the display options of a sector */
  TEST_F(CellTFInfoTest, testDisplay)
  {

    TVector3 innerHit(1, 2, 3);
    TVector3 outerHit(2, 3, 4);

    CellTFInfo aCell = CellTFInfo();
    aCell.push_back_AssignedHits(1, outerHit);
    aCell.push_back_AssignedHits(2, innerHit);

    aCell.push_back_UsedParticles(std::make_pair(11, 0.5));
    aCell.push_back_UsedParticles(std::make_pair(13, 0.4));

    std::vector<TVector3> allPosistions = aCell.getCoordinates();

    ASSERT_EQ(2., allPosistions.size());

    ASSERT_EQ(outerHit, allPosistions.at(0));
    ASSERT_EQ(innerHit, allPosistions.at(1));

    aCell.getDisplayAlternativeBox();
    aCell.getDisplayInformation();

  }

}  // namespace
