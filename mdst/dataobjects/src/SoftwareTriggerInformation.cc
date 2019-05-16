#include <mdst/dataobjects/SoftwareTriggerInformation.h>
#include <boost/algorithm/string/replace.hpp>

#include <TROOT.h>
#include <TColor.h>

using namespace Belle2;

void SoftwareTriggerInformation::addNonPrescaledResult(const std::string& triggerIdentifier, const SoftwareTriggerCutResult& result)
{
  m_nonPrescaledResults[triggerIdentifier] = static_cast<int>(result);
}

void SoftwareTriggerInformation::addPrescaling(const std::string& triggerIdentifier, const std::vector<unsigned int>& prescaling)
{
  m_prescalings[triggerIdentifier] = prescaling;
}

SoftwareTriggerCutResult SoftwareTriggerInformation::getNonPrescaledResult(const std::string& triggerIdentifier) const
{
  return static_cast<SoftwareTriggerCutResult>(m_nonPrescaledResults.at(triggerIdentifier));
}

const std::vector<unsigned int>& SoftwareTriggerInformation::getPrescaling(const std::string& triggerIdentifier) const
{
  return m_prescalings.at(triggerIdentifier);
}

void SoftwareTriggerInformation::clear()
{
  m_nonPrescaledResults.clear();
  m_prescalings.clear();
}

