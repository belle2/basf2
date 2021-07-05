/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Class to store the threshold efficiency (i.e. the efficiency for the
   * pulse identification, function of the threshold being used in the CFD
   * or template fit algorithm ) for all 512 channels of 16 modules.
   * From laser scans.
   */
  class TOPCalChannelThresholdEff: public TObject {
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
     * Threshold efficiencies are set to 0 by default.
     */
    TOPCalChannelThresholdEff() {}

    /**
     * Sets the threshold efficiency and correspolding threshold for a single channel and switches status to calibrated
     * (efficiency definition : ratio of integral of fit function over a range [threshold, +inf] to integral over the full range [0, +inf])
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param ThrEff channel threshold efficiency
     * @param offlineThreshold threshold ADC counts used in efficiency evaluation
     */
    void setThrEff(int moduleID, unsigned channel, float ThrEff, short offlineThreshold)
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
      m_ThrEff[module][channel] = ThrEff;
      m_offlineThreshold[module][channel] = offlineThreshold;
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
     * Returns the threshold efficiency of a single channel (1.0 if status is c_Default)
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return ThrEff
     */
    float getThrEff(int moduleID, unsigned channel) const
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
      if (m_status[module][channel] == c_Default) return 1.0;
      return m_ThrEff[module][channel];
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

    /**
     * Returns the threshold value used for efficiency evaluation
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return offlineThreshold (in a unit of ADC counts)
     */
    short getOfflineThreshold(int moduleID, unsigned channel) const
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
      return m_offlineThreshold[module][channel];
    }

  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };


    float m_ThrEff[c_numModules][c_numChannels] = {{0}};    /**< threshold efficiency value. 0 by default. */
    EStatus m_status[c_numModules][c_numChannels] = {{c_Default}}; /**< calibration status */
    short m_offlineThreshold[c_numModules][c_numChannels] = {{0}}; /**< threshold value used for efficiency evaluation */

    ClassDef(TOPCalChannelThresholdEff, 3); /**< ClassDef */

  };

} // end namespace Belle2

