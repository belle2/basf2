#include <mdst/dataobjects/SoftwareTriggerResult.h>

using namespace Belle2;

/// Add a new cut result to the storage or override the result with the same name.
void SoftwareTriggerResult::addResult(const std::string& triggerIdentifier, const SoftwareTriggerCutResult& result)
{
  m_results[triggerIdentifier] = static_cast<int>(result);
}

/// Return the cut result with the given name or throw an error if no result is there.
SoftwareTriggerCutResult SoftwareTriggerResult::getResult(const std::string& triggerIdentifier) const
{
  return static_cast<SoftwareTriggerCutResult>(m_results.at(triggerIdentifier));
}

/**
 * Return the "total result" of this event. See the SoftwareTriggerModule for a description on
 * when what is returned.
 */
int SoftwareTriggerResult::getTotalResult(bool acceptOverridesReject) const
{
  bool hasOneAcceptCut = false;
  bool hasOneRejectCut = false;

  for (const auto& identifierWithResult : m_results) {
    const auto& result = static_cast<SoftwareTriggerCutResult>(identifierWithResult.second);
    if (result == SoftwareTriggerCutResult::c_accept) {
      hasOneAcceptCut = true;
    } else if (result == SoftwareTriggerCutResult::c_reject) {
      hasOneRejectCut = true;
    }
  }

  if (acceptOverridesReject) {
    if (hasOneAcceptCut) {
      return 1;
    } else if (hasOneRejectCut) {
      return -1;
    } else {
      return 0;
    }
  } else {
    if (hasOneRejectCut) {
      return -1;
    } else if (hasOneAcceptCut) {
      return 1;
    } else {
      return 0;
    }
  }
}

/// Clear all results
void SoftwareTriggerResult::clear()
{
  m_results.clear();
}