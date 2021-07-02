/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept, SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult("cutOne"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getNonPrescaledResult("cutOne"));
      EXPECT_THROW(result.getResult("cutTwo"), std::out_of_range);

      // Another first result: a reject result
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_reject, SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult("cutOne"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getNonPrescaledResult("cutOne"));

      // Another first result: a don't know result
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_noResult);

      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult("cutOne"));

      // The default for non-prescaled should be noResult
      result.clear();
      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult("cutOne"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getNonPrescaledResult("cutOne"));

      // Three result
      result.clear();

      result.addResult("cutOne", SoftwareTriggerCutResult::c_accept, SoftwareTriggerCutResult::c_reject);
      result.addResult("cutTwo", SoftwareTriggerCutResult::c_noResult, SoftwareTriggerCutResult::c_accept);
      result.addResult("cutThree", SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult("cutOne"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getNonPrescaledResult("cutOne"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult("cutTwo"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getNonPrescaledResult("cutTwo"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult("cutThree"));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getNonPrescaledResult("cutThree"));
    }
  }
}
