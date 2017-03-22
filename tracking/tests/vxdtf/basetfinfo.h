#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/displayInterfaceTF/BaseTFInfo.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

namespace Belle2 {

  /** Set up a few arrays and objects in the datastore */
  class BaseTFInfoTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters. */
  TEST_F(BaseTFInfoTest, testEmptyFilter)
  {
    BaseTFInfo aBase = BaseTFInfo();

    ASSERT_EQ("", aBase.getDiedAt());

    EXPECT_DOUBLE_EQ(0., aBase.sizeAccepted());
    EXPECT_DOUBLE_EQ(0., aBase.sizeRejected());

    EXPECT_DOUBLE_EQ(-1., aBase.getPassIndex());
  }

  /** Tests setter/getter and the Accepted/Rejected Filters of BaseTFInfoTest Object */
  TEST_F(BaseTFInfoTest, testFilledFilter)
  {
    BaseTFInfo aBase = BaseTFInfo();
    aBase.setDiedAt("Hallo");
    aBase.setPassIndex(1);

    std::vector<int> allAccepted = {12, 13, 14, 15, 16};
    std::vector<int> allRejected = {11, 12, 13, 14};

    aBase.insert_Accepted(allAccepted);
    aBase.insert_Rejected(allRejected);

    ASSERT_EQ("Hallo", aBase.getDiedAt());

    ASSERT_EQ(14, aBase.getAccepted()[2]);
    ASSERT_EQ(12, aBase.getRejected()[1]);

    ASSERT_EQ(1, aBase.getPassIndex());
  }
}  // namespace
