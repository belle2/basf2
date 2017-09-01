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

/// Clear all results
void SoftwareTriggerResult::clear()
{
  m_results.clear();
}