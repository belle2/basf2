/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/EventExtraInfo.h>
#include <framework/utilities/HTML.h>
#include <stdexcept>

using namespace Belle2;


float EventExtraInfo::getExtraInfo(const std::string& name) const
{
  return eventExtraInfo.at(name);
}

bool EventExtraInfo::hasExtraInfo(const std::string& name) const
{
  return not(eventExtraInfo.find(name) == eventExtraInfo.end());
}


void EventExtraInfo::removeExtraInfo()
{
  eventExtraInfo.clear();
}

void EventExtraInfo::addExtraInfo(const std::string& name, float value)
{
  if (hasExtraInfo(name)) {
    throw std::out_of_range(std::string("Key with name ") + name + " already exists in EventExtraInfo.");
  }
  eventExtraInfo[name] = value;
}

void EventExtraInfo::setExtraInfo(const std::string& name, float value)
{
  eventExtraInfo[name] = value;
}

std::string EventExtraInfo::getInfoHTML() const
{
  std::string s;
  for (const auto& pair : eventExtraInfo) {
    s += HTML::escape(pair.first) + " = " + std::to_string(pair.second) + "<br />";
  }
  return s;
}

std::vector<std::string> EventExtraInfo::getNames() const
{
  std::vector<std::string> out;
  for (const auto& pair : eventExtraInfo)
    out.push_back(pair.first);
  return out;
}
