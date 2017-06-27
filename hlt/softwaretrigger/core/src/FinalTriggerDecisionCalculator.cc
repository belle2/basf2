/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    bool FinalTriggerDecisionCalculator::getFinalTriggerDecision(const SoftwareTriggerResult& result)
    {
      for (const auto& cutResultWithName : result.getResults()) {
        const std::string& resultName = cutResultWithName.first;
        const SoftwareTriggerCutResult& cutResult = static_cast<SoftwareTriggerCutResult>(cutResultWithName.second);

        if (SoftwareTriggerDBHandler::isTotalCutName(resultName) and cutResult == SoftwareTriggerCutResult ::c_reject) {
          return false;
        }
      }

      return true;
    }
  }
}