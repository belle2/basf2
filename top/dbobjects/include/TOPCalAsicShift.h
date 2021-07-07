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
   * Calibration constants for ASIC shifts of all 16 modules.
   * These are discrete shifts, usually in steps of a system clock (roughly 8 ns).
   */
  class TOPCalAsicShift: public TObject {
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
    TOPCalAsicShift()
    {}

    /**
     * Sets calibration for a single ASIC and switches status to calibrated
     * @param moduleID module ID (1-based)
     * @param asic ASIC number within a module (0 - 63)
     * @param T0 shift in time [ns]
     */
    void setT0(int moduleID, unsigned asic, double T0);

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     * @param moduleID module ID (1-based)
     * @param asic ASIC number within a module (0 - 63)
     */
    void setUnusable(int moduleID, unsigned asic);

    /**
     * Returns T0 of a single asic
     * @param moduleID module ID (1-based)
     * @param asic ASIC number within a module (0 - 63)
     * @return T0
     */
    double getT0(int moduleID, unsigned asic) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param asic ASIC number within a module (0 - 63)
     * @return true, if good calibrated
     */
    bool isCalibrated(int moduleID, unsigned asic) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param asic ASIC number within a module (0 - 63)
     * @return true, if default (not calibrated)
     */
    bool isDefault(int moduleID, unsigned asic) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @param asic ASIC number within a module (0 - 63)
     * @return true, if bad calibrated
     */
    bool isUnusable(int moduleID, unsigned asic) const;

  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numAsics = 64 /**< number of ASIC's per module */
         };

    float m_T0[c_numModules][c_numAsics] = {{0}};    /**< calibration constants */
    EStatus m_status[c_numModules][c_numAsics] = {{c_Default}}; /**< calibration status */

    ClassDef(TOPCalAsicShift, 1); /**< ClassDef */

  };

} // end namespace Belle2

