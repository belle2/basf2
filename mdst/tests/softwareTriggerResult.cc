/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {
    /** Test basic functionality. */
    TEST(SoftwareTriggerResultTest, basic)
    {
      SoftwareTriggerResult result;

      // First result: an accept result
      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult("cutOne"));
      EXPECT_THROW(result.getResult("cutTwo"), std::out_of_range);

      // Another first result: a reject result
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult("cutOne"));

      // Another first result: a don't know result
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_noResult);

      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult("cutOne"));
      // Three result
      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept);
      result.addResult("cutTwo", SoftwareTriggerCutResult::c_noResult);
      result.addResult("cutThree", SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult("cutOne"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult("cutTwo"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult("cutThree"));
    }
  }
}
