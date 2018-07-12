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
   * c_Active = 0 active channel
   * c_Dead   = 1 dead channel
   * c_Noisy  = 2 noisy channel.
   * A simple method isActive has been implemented to return false if
   * the channel has been masked for whatever reason and true if
   * the channel is active.
   */
  class TOPCalChannelMask: public TObject {
  public:

    /**
     * Status of the channel
     */
    enum EStatus {c_Active = 0,
                  c_Dead = 1,
                  c_Noisy = 2
                 };

    /**
     * Default constructor.
     */
    TOPCalChannelMask() {}

    /**
     * Sets the status for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param status channel status (0: active 1: dead 2: noisy )
     */
    void setStatus(int moduleID, unsigned channel, EStatus status)
    {
      int module = moduleID - 1;
      if (!check(module, channel)) {
        B2WARNING("Channel status not set");
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
      int module = moduleID - 1;
      if (!check(module, channel)) {
        B2WARNING("Channel status 'active' not set");
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
      int module = moduleID - 1;
      if (!check(module, channel)) {
        B2WARNING("Channel status 'dead' not set");
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
      int module = moduleID - 1;
      if (!check(module, channel)) {
        B2WARNING("Channel status 'noisy' not set");
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
      int module = moduleID - 1;
      if (!check(module, channel)) {
        B2WARNING("Returning dead channel value");
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
      int module = moduleID - 1;
      if (!check(module, channel)) {
        B2WARNING("Returning false");
        return false;
      }
      return (m_status[module][channel] == c_Active);
    }

  private:
    /**
     * Check input module and channel arguments are sane
     */
    bool check(const int module, const unsigned channel) const
    {
      if (module >= c_numModules) {
        B2ERROR("Invalid module number (" << ClassName() << ")");
        return false;
      }
      if (module < 0) {
        B2ERROR("Invalid module number (" << ClassName() << ")");
        return false;
      }
      if (channel >= c_numChannels) {
        B2ERROR("Invalid module number (" << ClassName() << ")");
        return false;
      }
      return true;
    }

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    EStatus m_status[c_numModules][c_numChannels] = {{ c_Active }};    /**< channel status. c_Active by default.*/
    // instantiating an array with too few entries fills with zeroes (==active)

    ClassDef(TOPCalChannelMask, 1); /**< ClassDef */

  };

} // end namespace Belle2

