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
#include <framework/logging/Logger.h>

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
    TOPASICChannel()
    {
      for (unsigned i = 0; i < c_TimeAxisSize; i++) m_timeAxis[i] = 0;
    }

    /**
     * Constructor with module ID, hardware channel number and number of ASIC windows
     * @param barID module ID
     * @param channel hardware channel number
     * @param numWindows number of active windows per ASIC
     */
    TOPASICChannel(int barID, unsigned channel, int numWindows):
      m_barID(barID), m_channel(channel)
    {
      for (int i = 0; i < numWindows; i++) m_pedestals.push_back(NULL);
      setTimeAxis();
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
      unsigned i = pedestals.getASICWindow();
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
      unsigned i = gains.getASICWindow();
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
     * Set equidistant time axis.
     * Time axis is differential: each time is with respect to the previous sample,
     * the first one is with respect to the last sample of previous ASIC window.
     * @param timeBin time bin
     */
    void setTimeAxis(float timeBin = 1 / 2.72)
    {
      for (unsigned i = 0; i < c_TimeAxisSize; i++) m_timeAxis[i] = timeBin;
    }

    /**
     * Set time axis from calibration.
     * Time axis is differential: each time is with respect to the previous sample,
     * the first one is with respect to the last sample of previous ASIC window.
     * @param timsDiffs vector of 256 elements of time differences btw. samples
     */
    void setTimeAxis(std::vector<float> timeDiffs)
    {
      for (unsigned i = 0; i < c_TimeAxisSize; i++) {
        if (i < timeDiffs.size()) m_timeAxis[i] = timeDiffs[i];
      }
      if (timeDiffs.size() < c_TimeAxisSize)
        B2WARNING("TOPASICChannel::setTimeAxis: vector too short")
      }

    /**
     * Return module ID
     * @return module ID
     */
    int getBarID() const {return m_barID;}

    /**
     * Return hardware channel number
     * @return channel number
     */
    unsigned getChannel() const {return m_channel;}


    /**
     * Return sample time in respect to previous sample
     * @param window ASIC window number
     * @param i sample number within ASIC window
     */
    float getSampleTime(unsigned window, unsigned i) const
    {
      unsigned k = (window % 4) * TOPASICPedestals::c_WindowSize +
                   (i % TOPASICPedestals::c_WindowSize);
      return m_timeAxis[k];
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
     * Return pedestals of an ASIC window
     * @param window ASIC window number
     * @return pointer to pedestals or NULL
     */
    const TOPASICPedestals* getPedestals(unsigned window) const
    {
      if (window < m_pedestals.size()) return m_pedestals[window];
      return NULL;
    }

    /**
     * Return gains of an ASIC window
     * @param window ASIC window number
     * @return pointer to gains or NULL
     */
    const TOPASICGains* getGains(unsigned window) const
    {
      if (window < m_gains.size()) return m_gains[window];
      if (window < m_pedestals.size()) return &m_defaultGain;
      return NULL;
    }

    /**
     * Return number of good ASIC windows (e.g. those with defined pedestals)
     * @return number of good windows
     */
    unsigned getNumofGoodWindows() const
    {
      unsigned n = 0;
      for (auto& pedestal : m_pedestals) if (pedestal) n++;
      return n;
    }


  private:

    int m_barID = 0;          /**< bar ID */
    unsigned m_channel = 0;   /**< hardware channel number */
    float m_timeAxis[c_TimeAxisSize]; /**< differential time axis */
    std::vector<TOPASICPedestals*> m_pedestals; /**< pedestals */
    std::vector<TOPASICGains*> m_gains;         /**< gains */

    TOPASICGains m_defaultGain; /**< default gain */

    ClassDef(TOPASICChannel, 2); /**< ClassDef */

  };

} // end namespace Belle2

