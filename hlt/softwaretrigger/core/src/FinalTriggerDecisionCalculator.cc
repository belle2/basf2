/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>

using namespace Belle2;
using namespace SoftwareTrigger;

bool
FinalTriggerDecisionCalculator::getFinalTriggerDecision(const SoftwareTriggerResult& result, bool forgetTotalResult)
{
  const auto& results = result.getResults();

  // Handle different revisions of the trigger menu
  // Revision 2: if there is a final decision already stored, just use it (if not forgetTotalResult is set)
  const std::string& allTotalResultName = SoftwareTriggerDBHandler::makeTotalResultName();

  auto allTotalResultIterator = results.find(allTotalResultName);
  if (allTotalResultIterator != results.end() and not forgetTotalResult) {
    auto allTotalResult = static_cast<SoftwareTriggerCutResult>(allTotalResultIterator->second);
    return allTotalResult == SoftwareTriggerCutResult::c_accept;
  }

  // Revision 2: filters are called "filter" and "skim". skim does not change the result.
  // if "filter" rejected the event, the event is rejected
  const std::string& filterTotalResultName = SoftwareTriggerDBHandler::makeTotalResultName("filter");

  auto filterTotalResultIterator = results.find(filterTotalResultName);

  if (filterTotalResultIterator != results.end()) {
    auto filterTotalResult = static_cast<SoftwareTriggerCutResult>(filterTotalResultIterator->second);
    if (filterTotalResult == SoftwareTriggerCutResult::c_reject) {
      return false;
    }
  }

  // Revision 1: filters are called "fast_reco", "hlt" and "calib". calib does not change result.
  // if any of fast_reco or hlt rejected the event, the event was rejected at all
  const std::string& fastRecoTotalResultName = SoftwareTriggerDBHandler::makeTotalResultName("fast_reco");
  const std::string& hltTotalResultName = SoftwareTriggerDBHandler::makeTotalResultName("hlt");

  auto fastRecoTotalResultIterator = results.find(fastRecoTotalResultName);
  auto hltTotalResultIterator = results.find(hltTotalResultName);

  if (fastRecoTotalResultIterator != results.end()) {
    B2WARNING("You are using an old trigger result with a newer version of the software. Make sure this is what you want.");
    auto fastRecoTotalResult = static_cast<SoftwareTriggerCutResult>(fastRecoTotalResultIterator->second);
    if (fastRecoTotalResult == SoftwareTriggerCutResult::c_reject) {
      return false;
    }
  }
  if (hltTotalResultIterator != results.end()) {
    B2WARNING("You are using an old trigger result with a newer version of the software. Make sure this is what you want.");
    auto hltTotalResult = static_cast<SoftwareTriggerCutResult>(hltTotalResultIterator->second);
    if (hltTotalResult == SoftwareTriggerCutResult::c_reject) {
      return false;
    }
  }

  // If there is no reject information (or no total information at all), accept the event :-)
  return true;
}


SoftwareTriggerCutResult FinalTriggerDecisionCalculator::getModuleResult(const SoftwareTriggerResult& result,
    const std::string& baseIdentifier,
    bool acceptOverridesReject)
{
  bool hasOneAcceptCut = false;
  bool hasOneRejectCut = false;

  for (const auto& resultWithName : result.getResults()) {
    const std::string& cutName = resultWithName.first;

    if (not SoftwareTriggerDBHandler::hasBaseIdentifier(cutName, baseIdentifier)) {
      continue;
    }

    if (cutName == SoftwareTriggerDBHandler::makeTotalResultName(baseIdentifier)) {
      B2WARNING("The store object already includes a result for this module. Will overwrite it.");
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