/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPCalTimebase.h>
#include <iostream>

using namespace std;
using namespace Belle2;


const TOPSampleTimes* TOPCalTimebase::getSampleTimes(unsigned scrodID,
                                                     unsigned channel) const
{
  if (m_map.empty()) createMap();

  unsigned key = (scrodID << 16) + (channel % 128);
  typedef std::map<unsigned, const TOPSampleTimes*>::const_iterator Iterator;
  Iterator it = m_map.find(key);
  if (it == m_map.end()) return m_sampleTime;
  return it->second;
}


void TOPCalTimebase::createMap() const
{
  for (const auto& sampleTime : m_sampleTimes) {
    unsigned key = (sampleTime.getScrodID() << 16) + sampleTime.getChannel();
    m_map[key] = &sampleTime;
  }
  if (!m_sampleTime) m_sampleTime = new TOPSampleTimes(0, 0, m_syncTimeBase);

  B2DEBUG(100, "Map created, size = " << m_map.size());
}
