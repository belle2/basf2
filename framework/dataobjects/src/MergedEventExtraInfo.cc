/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/MergedEventExtraInfo.h>
#include <framework/utilities/HTML.h>
#include <stdexcept>

using namespace Belle2;


float MergedEventExtraInfo::getExtraInfo(const std::string& name) const
{
  if (hasExtraInfo(name)) return m_mergedEventExtraInfo.at(name);
  else throw std::runtime_error("MergedEventExtraInfo::getExtraInfo: You try to access the MergedEventExtraInfo '" + name +
                                  "', but it doesn't exist.");
}

bool MergedEventExtraInfo::hasExtraInfo(const std::string& name) const
{
  return not(m_mergedEventExtraInfo.find(name) == m_mergedEventExtraInfo.end());
}


void MergedEventExtraInfo::removeExtraInfo()
{
  m_mergedEventExtraInfo.clear();
}

void MergedEventExtraInfo::addExtraInfo(const std::string& name, float value)
{
  if (hasExtraInfo(name)) {
    throw std::out_of_range(std::string("Key with name ") + name + " already exists in MergedEventExtraInfo.");
  }
  m_mergedEventExtraInfo[name] = value;
}

void MergedEventExtraInfo::setExtraInfo(const std::string& name, float value)
{
  m_mergedEventExtraInfo[name] = value;
}

std::string MergedEventExtraInfo::getInfoHTML() const
{
  std::string s;
  for (const auto& pair : m_mergedEventExtraInfo) {
    s += HTML::escape(pair.first) + " = " + std::to_string(pair.second) + "<br />";
  }
  return s;
}

std::vector<std::string> MergedEventExtraInfo::getNames() const
{
  std::vector<std::string> out;
  for (const auto& pair : m_mergedEventExtraInfo)
    out.push_back(pair.first);
  return out;
}
