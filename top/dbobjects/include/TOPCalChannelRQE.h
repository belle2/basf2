/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
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
   * Class to store relative quantum efficiency of channels w.r.t initial one measured in PMT QA
   * QE is expected to decrease during the experiment due to aging induced by acuumulated charge
   * Default is set to 1.0
   */
  class TOPCalChannelRQE: public TObject {
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
     */
    TOPCalChannelRQE() {}

    /**
     * Sets the relative QE for a single channel and switches status to calibrated
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param relQE relative quantum efficiency w.r.t initial one (absolute value, not in %)
     */
    void setRQE(int moduleID, unsigned channel, double relQE)
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
      m_relQE[module][channel] = relQE;
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
     * Returns the relative QE for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return relative quantum efficieny (absolute value, not in %)
     */
    double getRQE(int moduleID, unsigned channel) const
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
      return m_relQE[module][channel];
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

    float m_relQE[c_numModules][c_numChannels] = {{1.0}};    /**< relative quantum efficiency */
    EStatus m_status[c_numModules][c_numChannels] = {{c_Default}}; /**< calibration status */

    ClassDef(TOPCalChannelRQE, 1); /**< ClassDef */

  };

} // end namespace Belle2

