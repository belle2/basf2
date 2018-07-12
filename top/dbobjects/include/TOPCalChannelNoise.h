/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
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
   * r.m.s. of noise for all 512 channels of 16 modules.
   *
   * The noise for masked channels is undefined.
   * It is the caller's responsibility to check for masked channels
   */
  class TOPCalChannelNoise: public TObject {
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
     * Default constructor.
     * Noises are set to 0 by default.
     */
    TOPCalChannelNoise() {}

    /**
     * Sets the noise r.m.s for a single channel and switches status to calibrated.
     * If data for a given channel not available set noise to 0 (or just skip the call)
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param rmsNoise r.m.s. of noise [ADC counts]
     */
    void setNoise(int moduleID, unsigned channel, double rmsNoise)
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
      m_rmsNoise[module][channel] = rmsNoise;
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
     * Returns the noise r.m.s of a single channel (0 or negative: data not available)
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return r.m.s. of noise [ADC counts]
     */
    double getNoise(int moduleID, unsigned channel) const
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
      return m_rmsNoise[module][channel];
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
    enum {
      c_numModules = 16,  /**< number of modules */
      c_numChannels = 512 /**< number of channels per module */
    };

    float m_rmsNoise[c_numModules][c_numChannels] = {{0}};    /**< noise [ADC counts] */
    EStatus m_status[c_numModules][c_numChannels] = {{c_Default}}; /**< calibration status */

    ClassDef(TOPCalChannelNoise, 2); /**< ClassDef */

  };

} // end namespace Belle2


