/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

