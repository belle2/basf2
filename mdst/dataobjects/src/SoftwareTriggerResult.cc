#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <boost/algorithm/string/replace.hpp>

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

std::string SoftwareTriggerResult::getInfoHTML() const
{
  std::stringstream out;
  out << "<table>";
  for (const auto& result : m_results) {
    out << "<tr>";
    std::string name = result.first;
    boost::replace_all(name, "software_trigger_cut&", "");
    boost::replace_all(name, "&", "/");
    const int value = result.second;
    out << "<td>" << name << "</td>";
    out << "<td>" << value << "</td>";
    out << "</tr>";
  }
  out << "</table>";
  return out.str();
}
