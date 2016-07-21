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
      EXPECT_FALSE(result.getTotalResult());

      // First result
      result.addResult("cutOne", false);
      EXPECT_FALSE(result.getResult("cutOne"));
      EXPECT_FALSE(result.getTotalResult());
      EXPECT_THROW(result.getResult("cutTwo"), std::out_of_range);

      // Second result
      result.addResult("cutTwo", true);
      EXPECT_TRUE(result.getResult("cutTwo"));
      EXPECT_FALSE(result.getResult("cutOne"));
      EXPECT_TRUE(result.getTotalResult());
    }
  }
}
