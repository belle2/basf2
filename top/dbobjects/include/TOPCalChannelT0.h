/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Channel T0 calibration constants for all 512 channels of 16 modules.
   * From calibration with laser system.
   */
  class TOPCalChannelT0: public TObject {
  public:

    /**
     * Calibration status of a constant
     */
    enum EStatus {
      c_Default = 0,    /**< uncalibrated default value */
      c_Calibrated = 1, /**< good calibrated value */
      c_Unusable = 2    /**< bad calibrated value */
    };


    /**
     * Default constructor
     */
    TOPCalChannelT0()
    {}

    /**
     * Sets calibration for a single channel and switches status to calibrated
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param T0 channel T0
     * @param errT0 error on T0
     */
    void setT0(int moduleID, unsigned channel, double T0, double errT0)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
        return;
      }
      if (channel >= c_numChannels) {
        B2ERROR("Invalid channel number, constant not set (" << ClassName() << ")");
        return;
      }
      m_T0[module][channel] = T0;
      m_errT0[module][channel] = errT0;
      m_status[module][channel] = c_Calibrated;
    }

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setUnusable(int moduleID, unsigned channel)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
        return;
      }
      if (channel >= c_numChannels) {
        B2ERROR("Invalid channel number, status not set (" << ClassName() << ")");
        return;
      }
      m_status[module][channel] = c_Unusable;
    }

    /**
     * Returns T0 of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return T0
     */
    double getT0(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
        return 0;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Invalid channel number, returning 0 (" << ClassName() << ")");
        return 0;
      }
      return m_T0[module][channel];
    }

    /**
     * Returns error on T0 of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return error on T0
     */
    double getT0Error(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
        return 0;
      }
      if (channel >= c_numChannels) {
        B2WARNING("Invalid channel number, returning 0 (" << ClassName() << ")");
        return 0;
      }
      return m_errT0[module][channel];
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return true, if good calibrated
     */
    bool isCalibrated(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      if (channel >= c_numChannels) return false;
      return m_status[module][channel] == c_Calibrated;
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return true, if default (not calibrated)
     */
    bool isDefault(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      if (channel >= c_numChannels) return false;
      return m_status[module][channel] == c_Default;
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return true, if bad calibrated
     */
    bool isUnusable(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      if (channel >= c_numChannels) return false;
      return m_status[module][channel] == c_Unusable;
    }

  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    float m_T0[c_numModules][c_numChannels] = {{0}};    /**< calibration constants */
    float m_errT0[c_numModules][c_numChannels] = {{0}}; /**< errors on constants */
    EStatus m_status[c_numModules][c_numChannels] = {{c_Default}}; /**< calibration status */

    ClassDef(TOPCalChannelT0, 3); /**< ClassDef */

  };

} // end namespace Belle2

