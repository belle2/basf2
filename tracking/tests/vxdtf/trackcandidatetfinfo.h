#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/TrackCandidateTFInfo.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class TrackCandidateTFInfoTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(TrackCandidateTFInfoTest, testEmptyFilter)
  {

    bool false_item = false;


    TrackCandidateTFInfo atc = TrackCandidateTFInfo();

    ASSERT_EQ(-1, atc.getOwnID());
    ASSERT_EQ(false_item, atc.isFitPossible());
    ASSERT_EQ(0, atc.getProbValue());
    ASSERT_EQ(-1, atc.getAssignedGFTC());
    ASSERT_EQ(0, atc.getIsReal());
  }

  TEST_F(TrackCandidateTFInfoTest, testFilledFilter)
  {
    bool false_item = false;

    TrackCandidateTFInfo atc = TrackCandidateTFInfo();

    atc.setOwnID(7);
    atc.fitIsPossible(false_item);
    atc.setProbValue(1.7);
    atc.assignGFTC(3);
    atc.setIsReal(1);

    ASSERT_EQ(7, atc.getOwnID());
    ASSERT_EQ(false_item, atc.isFitPossible());
    ASSERT_EQ(1.7, atc.getProbValue());
    ASSERT_EQ(3, atc.getAssignedGFTC());
    ASSERT_EQ(1, atc.getIsReal());


  }


  /** Tests partice Information */
  TEST_F(TrackCandidateTFInfoTest, testParticle)
  {
    bool false_item = false;
    bool true_item = true;

    TrackCandidateTFInfo atc = TrackCandidateTFInfo();

    atc.push_back_UsedParticles(std::make_pair(11, 0.5));
    atc.push_back_UsedParticles(std::make_pair(13, 0.4));

    atc.setIsReal(1);

    atc.containsParticle(11);

    ASSERT_EQ(true_item, atc.containsParticle(11));
    ASSERT_EQ(false_item, atc.containsParticle(12));
    ASSERT_EQ(true_item, atc.containsParticle(13));

    ASSERT_EQ(11, atc.getMainParticle().first);
    ASSERT_EQ(0.5, atc.getMainParticle().second);

    ASSERT_EQ(11, atc.getInfoParticle(11).first);
    ASSERT_EQ(0.5, atc.getInfoParticle(11).second);

    ASSERT_EQ(true_item, atc.getIsReal());
  }



  /** Tests the display options of a sector */
  TEST_F(TrackCandidateTFInfoTest, testDisplay)
  {

    TVector3 cell1(1, 2, 3);
    TVector3 cell2(2, 3, 4);

    TrackCandidateTFInfo atc = TrackCandidateTFInfo();

    std::vector<TVector3> newCoordinates;
    newCoordinates.push_back(cell1);
    newCoordinates.push_back(cell2);

    atc.push_back_AssignedCell(1, newCoordinates);


    atc.push_back_UsedParticles(std::make_pair(11, 0.5));
    atc.push_back_UsedParticles(std::make_pair(13, 0.4));

    std::vector<TVector3> allPosistions = atc.getCoordinates();

    ASSERT_EQ(2., allPosistions.size());

    ASSERT_EQ(cell1, allPosistions.at(0));
    ASSERT_EQ(cell2, allPosistions.at(1));

    atc.getDisplayAlternativeBox();
    atc.getDisplayInformation();

  }


}  // namespace





