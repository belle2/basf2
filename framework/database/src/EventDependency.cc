/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/EventDependency.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace Belle2;

void EventDependency::add(unsigned int event, TObject* object)
{
  m_objects.Add(object);
  m_eventNumbers.push_back(event);
}


int EventDependency::getIndex(const EventMetaData& event) const
{
  int result = 0;
  for (unsigned int eventNumber : m_eventNumbers) {
    if (eventNumber > event.getEvent()) return result;
    result++;
  }
  return result;
}

