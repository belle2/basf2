/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalTimebase.h>
#include <framework/logging/Logger.h>
#include <cmath>

using namespace std;

namespace Belle2 {

  typedef map<unsigned, size_t>::const_iterator Iterator; /**< Iteratior for m_map */


  void TOPCalTimebase::append(unsigned scrodID, unsigned channel,
                              const vector<double>& sampleTimes,
                              bool replace)
  {
    // sanitary checks for NaN's
    for (double t : sampleTimes) {
      if (isnan(t)) {
        B2ERROR("TOPCalTimebase::append: sampleTimes contain NaN's - constants rejected."
                << LogVar("scrodID", scrodID)
                << LogVar("channel", channel));
        return;
      }
    }

    // append
    unsigned key = (scrodID << 16) + (channel % 128);
    Iterator it = m_map.find(key);
    if (it == m_map.end()) { // constants not appended yet
      m_sampleTimes.push_back(TOPSampleTimes(scrodID, channel, m_syncTimeBase));
      m_sampleTimes.back().setTimeAxis(sampleTimes, m_syncTimeBase);
      m_map[key] = m_sampleTimes.size() - 1;
    } else { // constants already there
      if (replace) {
        m_sampleTimes[it->second].setTimeAxis(sampleTimes, m_syncTimeBase);
        B2WARNING("TOPCalTimebase::append: constants replaced."
                  << LogVar("scrodID", scrodID)
                  << LogVar("channel", channel));
      } else {
        B2WARNING("TOPCalTimebase::append old constants kept."
                  << LogVar("scrodID", scrodID)
                  << LogVar("channel", channel));
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
    return &m_sampleTimes[it->second];
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
    for (size_t i = 0; i < m_sampleTimes.size(); i++) {
      const auto& sampleTime = m_sampleTimes[i];
      unsigned key = (sampleTime.getScrodID() << 16) + sampleTime.getChannel();
      m_map[key] = i;
    }

    B2DEBUG(29, "TOPCalTimebase: map created, size = " << m_map.size());
  }

} // end Belle2 namespace
