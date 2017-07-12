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

using namespace Belle2;
using namespace SoftwareTrigger;

bool FinalTriggerDecisionCalculator::getFinalTriggerDecision(const SoftwareTriggerResult& result)
{
  const std::string& fastRecoTotalResultName = SoftwareTriggerDBHandler::makeTotalCutName("fast_reco");
  const std::string& hltTotalResultName = SoftwareTriggerDBHandler::makeTotalCutName("hlt");

  for (const auto& cutResultWithName : result.getResults()) {
    const std::string& resultName = cutResultWithName.first;
    const SoftwareTriggerCutResult& cutResult = static_cast<SoftwareTriggerCutResult>(cutResultWithName.second);

    if ((resultName == fastRecoTotalResultName or resultName == hltTotalResultName)
        and cutResult == SoftwareTriggerCutResult::c_reject) {
      return false;
    }
  }

  return true;
}


SoftwareTriggerCutResult FinalTriggerDecisionCalculator::getModuleResult(const SoftwareTriggerResult& result,
    const std::string& baseIdentifier, bool acceptOverridesReject)
{
  bool hasOneAcceptCut = false;
  bool hasOneRejectCut = false;

  for (const auto& resultWithName : result.getResults()) {
    const std::string& cutName = resultWithName.first;

    if (not SoftwareTriggerDBHandler::hasBaseIdentifier(cutName, baseIdentifier)) {
      continue;
    }

    const SoftwareTriggerCutResult cutResult = static_cast<SoftwareTriggerCutResult>(resultWithName.second);

    if (cutResult == SoftwareTriggerCutResult::c_accept) {
      hasOneAcceptCut = true;
    } else if (cutResult == SoftwareTriggerCutResult::c_reject) {
      hasOneRejectCut = true;
    }
  }

  if (acceptOverridesReject) {
    if (hasOneAcceptCut or (not hasOneRejectCut)) {
      return SoftwareTriggerCutResult::c_accept;
    } else {
      return SoftwareTriggerCutResult::c_reject;
    }
  } else {
    if (hasOneAcceptCut and (not hasOneRejectCut)) {
      return SoftwareTriggerCutResult::c_accept;
    } else {
      return SoftwareTriggerCutResult::c_reject;
    }
  }
}