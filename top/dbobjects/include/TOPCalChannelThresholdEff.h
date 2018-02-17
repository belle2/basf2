/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it), Marko Staric,      *
 *               Maeda Yosuke                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
     * Sets the threshold efficiency for a single channel and switches status to calibrated
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param ThrEff channel threshold efficiency
     */
    void setThrEff(int moduleID, unsigned channel, float ThrEff)
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
     * Sets threshold value used for efficiency evaluation
     * (efficiency definition : ratio of integral of fit function over a range [threshold, +inf] to integral over the full range [0, +inf])
     * @param threshold threshold value (in a unit of ADC count, common for all channels)
     */
    void setOfflineThreshold(float offlineThreshold)
    {
      m_offlineThreshold = offlineThreshold;
    }


    /**
     * Returns the threshold efficiency of a single channel
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
     * @return offlineThreshold, common for all channels
     */
    float getOfflineThreshold() const
    {
      return m_offlineThreshold;
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
    float m_offlineThreshold = 0; /**< threshold value used for efficiency evaluation, common for all channels */

    ClassDef(TOPCalChannelThresholdEff, 3); /**< ClassDef */

  };

} // end namespace Belle2

