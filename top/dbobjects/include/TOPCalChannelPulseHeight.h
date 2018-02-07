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
   * Pulse height parameterizations for all 512 channels of 16 modules.
   * Parameterization is: P(x) = (x/x0)^p1 * exp(-(x/x0)^p2), x0 > 0, p1 >= 0, p2 > 0
   * From channel gain/efficiency calibration
   */
  class TOPCalChannelPulseHeight: public TObject {
  public:

    /**
     * Parameters of the pulse height distribution P(x) = (x/x0)^p1 * exp(-(x/x0)^p2)
     */
    struct PulseHeightPar {
      float x0 = 0; /**< distribution parameter x0 [ADC counts] */
      float p1 = 0; /**< distribution parameter p1 */
      float p2 = 0; /**< distribution parameter p2 */
    };

    /**
     * Default constructor
     */
    TOPCalChannelPulseHeight()
    {}

    /**
     * Sets calibration for a single channel
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
        B2ERROR("TOPCalChannelPulseHeight: not set, invalid slot number " << moduleID);
        return;
      }
      if (channel >= c_numChannels) {
        B2ERROR("TOPCalChannelPulseHeight: not set, invalid channel " << channel);
        return;
      }
      if (x0 <= 0) {
        B2ERROR("TOPCalChannelPulseHeight: not set, invalid parameter value x0 (" << x0
                << ") for slot " << moduleID << " channel " << channel);
        return;
      }
      if (p1 < 0) {
        B2ERROR("TOPCalChannelPulseHeight: not set, invalid parameter value p1 (" << p1
                << ") for slot " << moduleID << " channel " << channel);
        return;
      }
      if (p2 <= 0) {
        B2ERROR("TOPCalChannelPulseHeight: not set, invalid parameter value p2 (" << p2
                << ") for slot " << moduleID << " channel " << channel);
        return;
      }
      m_par[module][channel].x0 = x0;
      m_par[module][channel].p1 = p1;
      m_par[module][channel].p2 = p2;
      m_status[module][channel] = true;
    }

    /**
     * Returns pulse height parameters for a given slot and channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return parameters of pulse heigth distribution
     */
    const PulseHeightPar& getParameters(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2ERROR("TOPCalChannelPulseHeight: invalid slot number " << moduleID
                << ", returning parameters of slot 0 channel 0");
        return m_par[0][0];
      }
      if (channel >= c_numChannels) {
        B2ERROR("TOPCalChannelPulseHeight: invalid channel " << channel
                << ", returning parameters of slot " << moduleID << " channel 0");
        return m_par[module][0];
      }
      return m_par[module][channel];
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
    enum {
      c_numModules = 16,  /**< number of modules */
      c_numChannels = 512 /**< number of channels per module */
    };

    PulseHeightPar m_par[c_numModules][c_numChannels]; /**< calibration constants */
    bool m_status[c_numModules][c_numChannels] = {{false}}; /**< calibration status */

    ClassDef(TOPCalChannelPulseHeight, 1); /**< ClassDef */

  };

} // end namespace Belle2

