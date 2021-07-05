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
   * Module T0 calibration constants for all 16 modules.
   * From module alignment.
   */
  class TOPCalModuleT0: public TObject {
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
    TOPCalModuleT0()
    {}

    /**
     * Sets calibration for a single module and switches status to calibrated
     * @param moduleID module ID (1-based)
     * @param T0 module T0
     * @param errT0 error on T0
     */
    void setT0(int moduleID, double T0, double errT0);

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     * @param moduleID module ID (1-based)
     */
    void setUnusable(int moduleID);

    /**
     * Subtracts arithmetic average from constants whose status is not c_Default.
     * Arithmetic average is calculated from those whose status is c_Calibrated.
     */
    void suppressAverage();

    /**
     * Returns T0 of a module
     * @param moduleID module ID (1-based)
     * @return T0
     */
    double getT0(int moduleID) const;

    /**
     * Returns error on T0 of a module
     * @param moduleID module ID (1-based)
     * @return error on T0
     */
    double getT0Error(int moduleID) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if good calibrated
     */
    bool isCalibrated(int moduleID) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if default (not calibrated)
     */
    bool isDefault(int moduleID) const;

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if bad calibrated
     */
    bool isUnusable(int moduleID) const;

  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
         };

    float m_T0[c_numModules] = {0};    /**< calibration constants */
    float m_errT0[c_numModules] = {0}; /**< errors on constants */
    EStatus m_status[c_numModules] = {c_Default}; /**< calibration status */

    ClassDef(TOPCalModuleT0, 3); /**< ClassDef */

  };

} // end namespace Belle2

