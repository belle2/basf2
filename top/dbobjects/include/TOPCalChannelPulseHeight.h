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
#include <top/dbobjects/TOPPulseHeightPar.h>

namespace Belle2 {

  /**
   * Pulse height parameterizations for all 512 channels of 16 modules.
   * Parameterization is: P(x) = (x/x0)^p1 * exp(-(x/x0)^p2), x0 > 0, p1 >= 0, p2 > 0
   * From channel gain/efficiency calibration
   */
  class TOPCalChannelPulseHeight: public TObject {
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
    TOPCalChannelPulseHeight()
    {}

    /**
     * Sets calibration for a single channel and switches status to calibrated
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param x0 distribution parameter (x0 > 0)
     * @param p1 distribution parameter (p1 >= 0)
     * @param p2 distribution parameter (p2 > 0)
     */
    void setParameters(int moduleID, unsigned channel, double x0, double p1, double p2)
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
      if (x0 <= 0) {
        B2ERROR("Invalid parameter value x0 (" << x0 << ") for slot " << moduleID
                << " channel " << channel << ", constant not set (" << ClassName() << ")");
        return;
      }
      if (p1 < 0) {
        B2ERROR("Invalid parameter value p1 (" << p1 << ") for slot " << moduleID
                << " channel " << channel << ", constant not set (" << ClassName() << ")");
        return;
      }
      if (p2 <= 0) {
        B2ERROR("Invalid parameter value p2 (" << p2 << ") for slot " << moduleID
                << " channel " << channel << ", constant not set (" << ClassName() << ")");
        return;
      }
      m_par[module][channel].x0 = x0;
      m_par[module][channel].p1 = p1;
      m_par[module][channel].p2 = p2;
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
     * Returns pulse height parameters for a given slot and channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return parameters of pulse heigth distribution
     */
    const TOPPulseHeightPar& getParameters(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Invalid slot number " << moduleID
                  << ", returning parameters of slot 0 channel 0 (" << ClassName() << ")");
        return m_par[0][0];
      }
      if (channel >= c_numChannels) {
        B2WARNING("Invalid channel " << channel
                  << ", returning parameters of slot " << moduleID << " channel 0 ("
                  << ClassName() << ")");
        return m_par[module][0];
      }
      return m_par[module][channel];
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

    TOPPulseHeightPar m_par[c_numModules][c_numChannels]; /**< calibration constants */
    EStatus m_status[c_numModules][c_numChannels] = {{c_Default}}; /**< calibration status */

    ClassDef(TOPCalChannelPulseHeight, 3); /**< ClassDef */

  };

} // end namespace Belle2

