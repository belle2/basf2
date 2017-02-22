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
   * The Channel condition is stored as the enum EStatus with the convention:
   * c_Dead = 0 dead channel
   * c_Active = 1  active channel
   * c_Noisy = 2 noisy channel.
   * A simple method isActive has been implemented to return false if
   * the channel has been masked for whatever reason and true if
   * the channel is active.
   */
  class TOPCalChannelMask: public TObject {
  public:

    /**
     * Status of the channel
     */
    enum EStatus {c_Dead = 0,
                  c_Active = 1,
                  c_Noisy = 2
                 };

    /**
     * Default constructor. All channels are set to active
     */
    TOPCalChannelMask()
    {
      for (unsigned m = 0; m < c_numModules; m++) {
        for (unsigned c = 0; c < c_numChannels; c++) {
          m_status[m][c] = c_Active;
        }
      }
    }

    /**
     * Sets the status for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param status channel status (1: active 0: dead -1: noisy )
     */
    void setStatus(int moduleID, unsigned channel, EStatus status)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16");
        return;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511");
        return;
      }
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
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16");
        return;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511");
        return;
      }
      m_status[module][channel] = c_Active;
    }

    /**
     * Sets a specific channel as dead
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setDead(int moduleID, unsigned channel)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16");
        return;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511");
        return;
      }
      m_status[module][channel] = c_Dead;
    }

    /**
     * Sets a specific channel as noisy
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setNoisy(int moduleID, unsigned channel)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16");
        return;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511");
        return;
      }
      m_status[module][channel] = c_Noisy;
    }

    /**
     * Returns the status of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return status channel status (1: active 0: dead -1: noisy )
     */
    EStatus getStatus(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16. Returning dead channel value");
        return c_Dead;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511. Returning dead channel value");
        return c_Dead;
      }
      return m_status[module][channel];
    }

    /**
     * Returns false if the channel is dead or noisy,
     * and true is the channel is active
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    bool isActive(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Module number greater than 16. Returning false");
        return false;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Channel number greater than 511. Returning false");
        return false;
      }
      return (m_status[module][channel] == c_Active);
    }




  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    EStatus m_status[c_numModules][c_numChannels];    /**< channel status */

    ClassDef(TOPCalChannelMask, 1); /**< ClassDef */

  };

} // end namespace Belle2

