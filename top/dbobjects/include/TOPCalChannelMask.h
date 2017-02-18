/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Channel status for all 512 channels of 16 modules.
   * The CHannle condition is stored as an int with the convention:
   * status = 1  active channel
   * status = 0 dead channel
   * status = -1 noisy channel.
   * A simple method isActive has been implemented to return 0 if
   * the channel has been masked for whatever reason and 1 if
   * the channel is active.
   */
  class TOPCalChannelMask: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPCalChannelMask()
    {
      for (unsigned m = 0; m < c_numModules; m++) {
        for (unsigned c = 0; c < c_numChannels; c++) {
          m_status[m][c] = 1;
        }
      }
    }

    /**
     * Sets the status for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param status channel status (1: active 0: dead -1: noisy )
     */
    void setStatus(int moduleID, unsigned channel, double status)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      if (channel >= c_numChannels) return;
      m_status[module][channel] = status;
    }

    /**
     * Sets a specific channel as active
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setActive(int moduleID, unsigned channel)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      if (channel >= c_numChannels) return;
      m_status[module][channel] = 1;
    }

    /**
     * Sets a specific channel as dead
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setDead(int moduleID, unsigned channel)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      if (channel >= c_numChannels) return;
      m_status[module][channel] = 0;
    }

    /**
     * Sets a specific channel as noisy
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setNoisy(int moduleID, unsigned channel)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      if (channel >= c_numChannels) return;
      m_status[module][channel] = -1;
    }


    /**
     * Returns the status of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return status channel status (1: active 0: dead -1: noisy )
     */
    double getStatus(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return 0;
      if (channel >= c_numChannels) return 0;
      return m_status[module][channel];
    }


    /**
     * Returns 0 if the channel is dead or noisy,
     * and 1 is the channel is active
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    int isActive(int moduleID, unsigned channel)
    {
      unsigned activeStatus = 0;
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      if (channel >= c_numChannels) return;
      if (m_status[module][channel] == 1) activeStatus = 1;
      return activeStatus;
    }


  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    int m_status[c_numModules][c_numChannels];    /**< channel status */

    ClassDef(TOPCalChannelMask, 1); /**< ClassDef */

  };

} // end namespace Belle2

