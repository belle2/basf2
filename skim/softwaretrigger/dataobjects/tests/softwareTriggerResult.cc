/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/dataobjects/SoftwareTriggerResult.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {
    /** Test basic functionality. */
    TEST(SoftwareTriggerResultTest, basic)
    {
      SoftwareTriggerResult result;

      // No results stored
      EXPECT_EQ(0, result.getTotalResult());
      EXPECT_EQ(0, result.getTotalResult(true));
      EXPECT_EQ(0, result.getTotalResult(false));

      // First result: an accept result
      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult("cutOne"));
      EXPECT_THROW(result.getResult("cutTwo"), std::out_of_range);

      EXPECT_EQ(1, result.getTotalResult());
      EXPECT_EQ(1, result.getTotalResult(true));
      EXPECT_EQ(1, result.getTotalResult(false));

      // Another first result: a reject result
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult("cutOne"));
      EXPECT_EQ(-1, result.getTotalResult());
      EXPECT_EQ(-1, result.getTotalResult(true));
      EXPECT_EQ(-1, result.getTotalResult(false));

      // Another first result: a don't know result
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_noResult);

      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult("cutOne"));
      EXPECT_EQ(0, result.getTotalResult());
      EXPECT_EQ(0, result.getTotalResult(true));
      EXPECT_EQ(0, result.getTotalResult(false));

      // Three result
      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept);
      result.addResult("cutTwo", SoftwareTriggerCutResult::c_noResult);
      result.addResult("cutThree", SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult("cutOne"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult("cutTwo"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult("cutThree"));

      EXPECT_EQ(-1, result.getTotalResult());
      EXPECT_EQ(1, result.getTotalResult(true));
      EXPECT_EQ(-1, result.getTotalResult(false));

      // Add another accept result: should not change anything
      result.addResult("cutFour", SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(-1, result.getTotalResult());
      EXPECT_EQ(1, result.getTotalResult(true));
      EXPECT_EQ(-1, result.getTotalResult(false));

      // More accept than reject: should be the same
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept);
      result.addResult("cutTwo", SoftwareTriggerCutResult::c_accept);
      result.addResult("cutThree", SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(-1, result.getTotalResult());
      EXPECT_EQ(1, result.getTotalResult(true));
      EXPECT_EQ(-1, result.getTotalResult(false));

      // More reject than accept: should be the same
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_reject);
      result.addResult("cutTwo", SoftwareTriggerCutResult::c_accept);
      result.addResult("cutThree", SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(-1, result.getTotalResult());
      EXPECT_EQ(1, result.getTotalResult(true));
      EXPECT_EQ(-1, result.getTotalResult(false));

      // Add two other accept result: should not change anything
      result.addResult("cutSix", SoftwareTriggerCutResult::c_accept);
      result.addResult("cutSeven", SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(-1, result.getTotalResult());
      EXPECT_EQ(1, result.getTotalResult(true));
      EXPECT_EQ(-1, result.getTotalResult(false));
    }
  }
}
