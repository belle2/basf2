/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <top/dbobjects/TOPASICPedestals.h>
#include <top/dbobjects/TOPASICGains.h>
#include <vector>

namespace Belle2 {

  /**
   * Calibration constants of a singe ASIC channel: pedestals, gains and time axis
   */
  class TOPASICChannel: public TObject {
  public:

    /**
     * time axis size (time axis is for 4 ASIC windows!)
     */
    enum {c_TimeAxisSize = TOPASICPedestals::c_WindowSize * 4};

    /**
     * Default constructor
     */
    TOPASICChannel(): m_barID(0), m_channel(0), m_timeInterval(0)
    {
      for (unsigned i = 0; i < c_TimeAxisSize; i++)
        m_timeAxis[i] = 0;
    }

    /**
     * Constructor with barID, hardware channel number and number of ASIC windows
     * @param barID bar ID
     * @param channel hardware channel number
     * @param numWindows number of active windows per ASIC
     */
    TOPASICChannel(int barID, unsigned channel, int numWindows):
      m_barID(barID), m_channel(channel)
    {
      for (int i = 0; i < numWindows; i++) m_pedestals.push_back(NULL);
      float timeBin = 1 / 2.77; //TODO get correct value
      setTimeAxis(timeBin);
      setTimeInterval(timeBin * c_TimeAxisSize);
    }

    /**
     * Destructor
     */
    ~TOPASICChannel()
    {
      for (auto& window : m_pedestals) {
        if (window) delete window;
      }
      for (auto& window : m_gains) {
        if (window) delete window;
      }
    }

    /**
     * Set pedestals of a single ASIC window.
     * @param pedestals ASIC window pedestals
     * @return true on success
     */
    bool setPedestals(const TOPASICPedestals& pedestals)
    {
      unsigned i = pedestals.getASICWindowID();
      if (i < m_pedestals.size()) {
        if (m_pedestals[i]) {
          *m_pedestals[i] = pedestals;
        } else {
          m_pedestals[i] = new TOPASICPedestals(pedestals);
        }
        return true;
      }
      return false;
    }

    /**
     * Set gains of a single ASIC window.
     * @param gains ASIC window gains
     * @return true on success
     */
    bool setGains(const TOPASICGains& gains)
    {
      if (m_gains.empty()) {
        for (unsigned i = 0; i < m_pedestals.size(); i++) m_gains.push_back(NULL);
      }
      unsigned i = gains.getASICWindowID();
      if (i < m_gains.size()) {
        if (m_gains[i]) {
          *m_gains[i] = gains;
        } else {
          m_gains[i] = new TOPASICGains(gains);
        }
        return true;
      }
      return false;
    }

    /**
     * Set time interval to the next four ASIC windows
     * @param timeInterval time interval
     */
    void setTimeInterval(float timeInterval) {m_timeInterval = timeInterval;}

    /**
     * Set equidistant time axis
     * @param timeBin time bin
     */
    void setTimeAxis(float timeBin)
    {
      float t = 0;
      for (unsigned i = 0; i < c_TimeAxisSize; i++) {
        m_timeAxis[i] = t;
        t += timeBin;
      }
    }

    /**
     * Return bar ID
     * @return bar ID
     */
    int getBarID() const {return m_barID;}

    /**
     * Return hardware channel number
     * @return channel number
     */
    unsigned getChannel() const {return m_channel;}

    /**
     * Return time interval to the next four ASIC windows
     * @return time interval
     */
    float getTimeInterval() const {return m_timeInterval;}

    /**
     * Return time axis with an offset added
     * @param offset offset to be added
     * @return time axis
     */
    std::vector<float> getTimeAxis(float offset = 0) const
    {
      std::vector<float> timeAxis;
      for (unsigned i = 0; i < c_TimeAxisSize; i++) {
        timeAxis.push_back(m_timeAxis[i] + offset);
      }
      return timeAxis;
    }

    /**
     * Return number of ASIC windows
     * @return number of windows
     */
    unsigned getNumofWindows() const
    {
      return m_pedestals.size();
    }

    /**
     * Return pedestals of i-th ASIC window
     * @param i ASIC window number
     * @return pointer to pedestals or NULL
     */
    const TOPASICPedestals* getPedestals(unsigned i) const
    {
      if (i < m_pedestals.size()) return m_pedestals[i];
      return NULL;
    }

    /**
     * Return gains of i-th ASIC window
     * @param i ASIC window number
     * @return pointer to gains or NULL
     */
    const TOPASICGains* getGains(unsigned i) const
    {
      if (i < m_gains.size()) return m_gains[i];
      return NULL;
    }

    /**
     * Return number of good ASIC windows (e.g. those with defined pedestals)
     * @return number of good windows
     */
    unsigned getNumofGoodWindows() const
    {
      unsigned n = 0;
      for (auto& window : m_pedestals) if (window) n++;
      return n;
    }


  private:

    int m_barID;          /**< bar ID */
    unsigned m_channel;   /**< hardware channel ID */
    float m_timeInterval; /**< time interval between ASIC windows */
    float m_timeAxis[c_TimeAxisSize]; /**< time axis */
    std::vector<TOPASICPedestals*> m_pedestals; /**< pedestals */
    std::vector<TOPASICGains*> m_gains;         /**< gains */

    ClassDef(TOPASICChannel, 1); /**< ClassDef */

  };

} // end namespace Belle2

