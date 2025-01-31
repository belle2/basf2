/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/utilities/HTML.h>
#include <stdexcept>

using namespace Belle2;


float EventExtraInfo::getExtraInfo(const std::string& name) const
{
  if (hasExtraInfo(name)) return eventExtraInfo.at(name);
  else throw std::runtime_error("EventExtraInfo::getExtraInfo: You try to access the EventExtraInfo '" + name +
                                  "', but it doesn't exist.");
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



std::string EventExtraInfo::getExtraStringInfo(const std::string& name) const
{
  if (hasExtraStringInfo(name)) return eventExtraStringInfo.at(name);
  else throw std::runtime_error("EventExtraInfo::getExtraStringInfo: You try to access the EventExtraInfo '" + name +
                                  "', but it doesn't exist.");
}

bool EventExtraInfo::hasExtraStringInfo(const std::string& name) const
{
  return not(eventExtraStringInfo.find(name) == eventExtraStringInfo.end());
}


void EventExtraInfo::removeExtraStringInfo()
{
  eventExtraStringInfo.clear();
}

void EventExtraInfo::addExtraStringInfo(const std::string& name, const std::string& value)
{
  if (hasExtraStringInfo(name)) {
    throw std::out_of_range(std::string("Key with name ") + name + " already exists in EventExtraInfo.");
  }
  eventExtraStringInfo[name] = value;
}

void EventExtraInfo::setExtraStringInfo(const std::string& name, const std::string& value)
{
  eventExtraStringInfo[name] = value;
}

std::vector<std::string> EventExtraInfo::getStringInfoNames() const
{
  std::vector<std::string> out;
  for (const auto& pair : eventExtraStringInfo)
    out.push_back(pair.first);
  return out;
}

std::string EventExtraInfo::getEventType() const
{
  if (hasExtraStringInfo(std::string("eventType")))
    return getExtraStringInfo(std::string("eventType"));
  else
    return std::string("");
}
