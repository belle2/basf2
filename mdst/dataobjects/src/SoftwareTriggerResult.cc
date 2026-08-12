/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <boost/algorithm/string/replace.hpp>

#include <TROOT.h>
#include <TColor.h>

using namespace Belle2;

/// Add a new cut result to the storage or override the result with the same name.
void SoftwareTriggerResult::addResult(const std::string& triggerIdentifier, SoftwareTriggerCutResult result,
                                      SoftwareTriggerCutResult nonPrescalesResult)
{
  m_results[triggerIdentifier] = std::make_pair(static_cast<int>(result), static_cast<int>(nonPrescalesResult));
}

std::pair<SoftwareTriggerCutResult, SoftwareTriggerCutResult> SoftwareTriggerResult::getResultPair(
  const std::string& triggerIdentifier) const
{
  auto pair = m_results.at(triggerIdentifier);
  return {static_cast<SoftwareTriggerCutResult>(pair.first), static_cast<SoftwareTriggerCutResult>(pair.second)};
}

SoftwareTriggerCutResult SoftwareTriggerResult::getResult(const std::string& triggerIdentifier) const
{
  return getResultPair(triggerIdentifier).first;
}

/// Return the cut result with the given name or throw an error if no result is there.
SoftwareTriggerCutResult SoftwareTriggerResult::getNonPrescaledResult(const std::string& triggerIdentifier) const
{
  return getResultPair(triggerIdentifier).second;
}

std::map<std::string, int> SoftwareTriggerResult::getResults() const
{
  std::map<std::string, int> result;
  for (const auto& [key, valuePair] : m_results) {
    result[key] = valuePair.first;
  }
  return result;
}

std::map<std::string, int> SoftwareTriggerResult::getNonPrescaledResults() const
{
  std::map<std::string, int> result;
  for (const auto& [key, valuePair] : m_results) {
    result[key] = valuePair.second;
  }
  return result;
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

  const std::string colorNeutral = gROOT->GetColor(kWhite)->AsHexString();
  const std::string colorReject = gROOT->GetColor(kRed)->AsHexString();
  const std::string colorAccept = gROOT->GetColor(kGreen)->AsHexString();

  for (const auto& result : m_results) {
    out << "<tr>";
    std::string name = result.first;
    boost::replace_all(name, "software_trigger_cut&", "");
    boost::replace_all(name, "&", "/");
    const int value = result.second.first;

    auto thisColor = colorNeutral;
    if (value > 0) {
      thisColor = colorAccept;
    } else if (value < 0) {
      thisColor = colorReject;
    }

    out << "<td>" << name << "</td>";
    out << "<td bgcolor=\"" << thisColor << "\">" << value << "</td>";
    out << "</tr>";
  }
  out << "</table>";
  return out.str();
}
