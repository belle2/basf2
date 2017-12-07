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
     * Default constructor
     */
    TOPCalChannelT0()
    {
      for (unsigned m = 0; m < c_numModules; m++) {
        for (unsigned c = 0; c < c_numChannels; c++) {
          m_T0[m][c] = 0;
          m_errT0[m][c] = 0;
          m_status[m][c] = false;
        }
      }
    }

    /**
     * Sets calibration for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param T0 channel T0
     * @param errT0 error on T0
     */
    void setT0(int moduleID, unsigned channel, double T0, double errT0)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      if (channel >= c_numChannels) return;
      m_T0[module][channel] = T0;
      m_errT0[module][channel] = errT0;
      m_status[module][channel] = true;
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
      if (module >= c_numModules) return 0;
      if (channel >= c_numChannels) return 0;
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
      if (module >= c_numModules) return 0;
      if (channel >= c_numChannels) return 0;
      return m_errT0[module][channel];
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return true, if calibrated
     */
    bool isCalibrated(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      if (channel >= c_numChannels) return false;
      return m_status[module][channel];
    }

  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };

    float m_T0[c_numModules][c_numChannels];    /**< calibration constants */
    float m_errT0[c_numModules][c_numChannels]; /**< errors on constants */
    bool m_status[c_numModules][c_numChannels]; /**< calibration status */

    ClassDef(TOPCalChannelT0, 2); /**< ClassDef */

  };

} // end namespace Belle2

