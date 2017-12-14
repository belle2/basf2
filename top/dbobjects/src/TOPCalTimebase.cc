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

namespace Belle2 {

  typedef map<unsigned, const TOPSampleTimes*>::const_iterator Iterator; /**< Iteratior for m_map */


  void TOPCalTimebase::append(unsigned scrodID, unsigned channel,
                              const vector<double>& sampleTimes,
                              bool replace)
  {
    unsigned key = (scrodID << 16) + (channel % 128);
    Iterator it = m_map.find(key);
    if (it == m_map.end()) { // constants not appended yet
      m_sampleTimes.push_back(TOPSampleTimes(scrodID, channel, m_syncTimeBase));
      m_sampleTimes.back().setTimeAxis(sampleTimes, m_syncTimeBase);
      m_map[key] = &m_sampleTimes.back();
    } else { // constants already there
      if (replace) {
        const_cast<TOPSampleTimes*>(it->second)->setTimeAxis(sampleTimes, m_syncTimeBase);
        B2WARNING("Constants for scrod " << scrodID << " channel " << channel << ": replaced");
      } else {
        B2WARNING("Constants for scrod " << scrodID << " channel " << channel << ": previous kept");
      }
    }

  }


  const TOPSampleTimes* TOPCalTimebase::getSampleTimes(unsigned scrodID,
                                                       unsigned channel) const
  {
    if (m_map.empty()) createMap();

    unsigned key = (scrodID << 16) + (channel % 128);
    Iterator it = m_map.find(key);
    if (it == m_map.end()) {
      if (!m_sampleTime) m_sampleTime = new TOPSampleTimes(0, 0, m_syncTimeBase);
      return m_sampleTime;
    }
    return it->second;
  }


  bool TOPCalTimebase::isAvailable(unsigned scrodID, unsigned channel) const
  {
    if (m_map.empty()) createMap();

    unsigned key = (scrodID << 16) + (channel % 128);
    Iterator it = m_map.find(key);
    return (it != m_map.end());

  }


  void TOPCalTimebase::createMap() const
  {
    for (const auto& sampleTime : m_sampleTimes) {
      unsigned key = (sampleTime.getScrodID() << 16) + sampleTime.getChannel();
      m_map[key] = &sampleTime;
    }

    B2DEBUG(100, "Map created, size = " << m_map.size());
  }

} // end Belle2 namespace
