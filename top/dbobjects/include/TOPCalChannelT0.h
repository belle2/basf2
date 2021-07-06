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
    void setT0(int moduleID, unsigned channel, double T0, double errT0);

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     */
    void setUnusable(int moduleID, unsigned channel);

    /**
     * Subtracts arithmetic average of a module from constants whose status is not c_Default.
     * Arithmetic average is calculated from those whose status is c_Calibrated.
     */
    void suppressAverage();

    /**
     * Returns T0 of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return T0
     */
    double getT0(int moduleID, unsigned channel) const;

    /**
     * Returns error on T0 of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return error on T0
     */
    double getT0Error(int moduleID, unsigned channel) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return true, if good calibrated
     */
    bool isCalibrated(int moduleID, unsigned channel) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return true, if default (not calibrated)
     */
    bool isDefault(int moduleID, unsigned channel) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return true, if bad calibrated
     */
    bool isUnusable(int moduleID, unsigned channel) const;

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

