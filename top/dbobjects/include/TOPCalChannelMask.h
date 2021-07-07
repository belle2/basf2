/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

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
    void setStatus(int moduleID, unsigned channel, EStatus status);

    /**
     * Sets a specific channel as active
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setActive(int moduleID, unsigned channel);

    /**
     * Sets a specific channel as dead
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setDead(int moduleID, unsigned channel);

    /**
     * Sets a specific channel as noisy
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setNoisy(int moduleID, unsigned channel);

    /**
     * Returns the status of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return status channel status (1: active 0: dead -1: noisy )
     */
    EStatus getStatus(int moduleID, unsigned channel) const;

    /**
     * Returns false if the channel is dead or noisy,
     * and true is the channel is active
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    bool isActive(int moduleID, unsigned channel) const;

    /**
     * Returns number of all channels
     */
    int getNumOfChannels() const {return c_numChannels * c_numModules;}

    /**
     * Returns number of channels in a module
     */
    int getNumOfModuleChannels() const {return c_numChannels;}

    /**
     * Returns number of active channels
     */
    int getNumOfActiveChannels() const {return getNumOf(c_Active);}

    /**
     * Returns number of dead channels
     */
    int getNumOfDeadChannels() const {return getNumOf(c_Dead);}

    /**
     * Returns number of noisy channels
     */
    int getNumOfNoisyChannels() const {return getNumOf(c_Noisy);}

    /**
     * Returns fraction of active channels
     */
    double getActiveFraction() const
    {
      return static_cast<double>(getNumOfActiveChannels()) / c_numModules / c_numChannels;
    }

    /**
     * Returns number of module active channels
     */
    int getNumOfActiveChannels(int moduleID) const {return getNumOf(c_Active, moduleID);}

    /**
     * Returns number of module dead channels
     */
    int getNumOfDeadChannels(int moduleID) const {return getNumOf(c_Dead, moduleID);}

    /**
     * Returns number of module noisy channels
     */
    int getNumOfNoisyChannels(int moduleID) const {return getNumOf(c_Noisy, moduleID);}

    /**
     * Returns fraction of module active channels
     */
    double getActiveFraction(int moduleID) const
    {
      return static_cast<double>(getNumOfActiveChannels(moduleID)) / c_numChannels;
    }


  private:

    /**
     * Check input module and channel arguments are sane
     */
    bool check(const int module, const unsigned channel) const;

    /**
     * Counts and returns the number of channels having a given status
     * @param check status to be checked
     * @return number of channels of this status
     */
    int getNumOf(EStatus check) const;

    /**
     * Counts and returns the number of channels od a module having a given status
     * @param check status to be checked
     * @param moduleID module (slot) ID (1-based)
     * @return number of channels of this status
     */
    int getNumOf(EStatus check, int moduleID) const;

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    EStatus m_status[c_numModules][c_numChannels] = {{ c_Active }}; /**< channel status */
    // instantiating an array with too few entries fills with zeroes (==active)

    ClassDef(TOPCalChannelMask, 1); /**< ClassDef */

  };

} // end namespace Belle2

