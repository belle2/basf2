/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {
    /** Test basic functionality. */
    TEST(FinalTriggerDecisionCalculatorTest, basic)
    {
      SoftwareTriggerResult result;

      // No results stored
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // First result: an accept result
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne"), SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne")));
      EXPECT_THROW(result.getResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutTwo")), std::out_of_range);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // Another first result: a reject result
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne"), SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne")));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // Another first result: a don't know result
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne"), SoftwareTriggerCutResult::c_noResult);

      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne")));
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // Add an unrelated result: should not change anything
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("other_test", "cutOne"), SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult(SoftwareTriggerDBHandler::makeFullCutName("other_test", "cutOne")));

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // Three result
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutTwo"), SoftwareTriggerCutResult::c_noResult);
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutThree"), SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, result.getResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne")));
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, result.getResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutTwo")));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, result.getResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutThree")));

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // Add another accept result: should not change anything
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutFour"), SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // More accept than reject: should be the same
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutTwo"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutThree"), SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // More reject than accept: should be the same
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutOne"), SoftwareTriggerCutResult::c_reject);
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutTwo"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutThree"), SoftwareTriggerCutResult::c_reject);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));

      // Add two other accept result: should not change anything
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutSix"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("test", "cutSeven"), SoftwareTriggerCutResult::c_accept);

      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, FinalTriggerDecisionCalculator::getModuleResult(result, "test", true));
      EXPECT_EQ(SoftwareTriggerCutResult::c_reject, FinalTriggerDecisionCalculator::getModuleResult(result, "test", false));
    }

    /** Test full result functionality. */
    TEST(FinalTriggerDecisionCalculatorTest, result)
    {
      SoftwareTriggerResult result;

      // No results stored -> ne event is accepted.
      EXPECT_EQ(true, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // No total results should not change anything.
      result.addResult(SoftwareTriggerDBHandler::makeFullCutName("filter", "some_cut"), SoftwareTriggerCutResult::c_reject);
      EXPECT_EQ(true, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // Revision 2
      // filter has rejected -> reject
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("filter"), SoftwareTriggerCutResult::c_reject);
      EXPECT_EQ(false, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // filter has accepted and skim also -> accept
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("filter"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("skim"), SoftwareTriggerCutResult::c_accept);
      EXPECT_EQ(true, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // filter has rejected and skim should not change anything -> reject
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("filter"), SoftwareTriggerCutResult::c_reject);
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("skim"), SoftwareTriggerCutResult::c_accept);
      EXPECT_EQ(false, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // filter has accepted, skim does not matter -> accept
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("filter"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("skim"), SoftwareTriggerCutResult::c_reject);
      EXPECT_EQ(true, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // Revision 1
      // fast reco has rejected -> reject
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("fast_reco"), SoftwareTriggerCutResult::c_reject);
      EXPECT_EQ(false, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // fast reco has accepted and hlt also -> accept
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("fast_reco"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("hlt"), SoftwareTriggerCutResult::c_accept);
      EXPECT_EQ(true, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // fast reco has rejected and hlt should not change anything -> reject
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("fast_reco"), SoftwareTriggerCutResult::c_reject);
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("hlt"), SoftwareTriggerCutResult::c_accept);
      EXPECT_EQ(false, FinalTriggerDecisionCalculator::getFinalTriggerDecision(result));

      // fast reco and hlt have accepted, calib does not matter -> accept
      result.clear();
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("fast_reco"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("hlt"), SoftwareTriggerCutResult::c_accept);
      result.addResult(SoftwareTriggerDBHandler::makeTotalResultName("calib"), SoftwareTriggerCutResult::c_reject);

    }
  }
}