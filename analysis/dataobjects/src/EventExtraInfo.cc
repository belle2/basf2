/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
  for (auto pair : eventExtraInfo) {
    s += HTML::escape(pair.first) + " = " + std::to_string(pair.second) + "<br />";
  }
  return s;
}
