/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
     * Default constructor
     */
    TOPASICChannel()
    {
    }

    /**
     * Constructor with module ID, hardware channel number and number of ASIC windows
     * @param moduleID module ID
     * @param channel hardware channel number
     * @param numWindows number of active windows per ASIC
     */
    TOPASICChannel(int moduleID, unsigned channel, int numWindows):
      m_moduleID(moduleID), m_channel(channel)
    {
      for (int i = 0; i < numWindows; i++) m_pedestals.push_back(NULL);
    }

    /**
     * Copy constructor
     */
    TOPASICChannel(const TOPASICChannel& chan): TObject()
    {
      *this = chan;
    }

    /**
     * Assignment operator
     */
    TOPASICChannel& operator=(const TOPASICChannel& chan)
    {
      if (this != &chan) {
        m_moduleID = chan.getModuleID();
        m_channel = chan.getChannel();
        for (auto& pedestals : m_pedestals) {
          if (pedestals) delete pedestals;
        }
        m_pedestals = chan.getPedestals();
        for (auto& pedestals : m_pedestals) {
          if (pedestals) pedestals = new TOPASICPedestals(*pedestals);
        }
        for (auto& gains : m_gains) {
          if (gains) delete gains;
        }
        m_gains = chan.getGains();
        for (auto& gains : m_gains) {
          if (gains) gains = new TOPASICGains(*gains);
        }
      }
      return *this;
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
     * Return module ID
     * @return module ID
     */
    int getModuleID() const {return m_moduleID;}

    /**
     * Return hardware channel number
     * @return channel number
     */
    unsigned getChannel() const {return m_channel;}

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
     * Returns a vector of pedestals
     * @return pedestals
     */
    const std::vector<TOPASICPedestals*>& getPedestals() const {return m_pedestals;}

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
     * Returns a vector of gains
     * @return gains
     */
    const std::vector<TOPASICGains*>& getGains() const {return m_gains;}

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

    int m_moduleID = 0;                         /**< module ID */
    unsigned m_channel = 0;                     /**< hardware channel number */
    std::vector<TOPASICPedestals*> m_pedestals; /**< pedestals */
    std::vector<TOPASICGains*> m_gains;         /**< gains */

    TOPASICGains m_defaultGain;                 /**< default gain */

    ClassDef(TOPASICChannel, 4); /**< ClassDef */

  };

} // end namespace Belle2

