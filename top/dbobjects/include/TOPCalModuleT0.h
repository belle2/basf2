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
    void setT0(int moduleID, double T0, double errT0)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2ERROR("Invalid module number, constant not set (" << ClassName() << ")");
        return;
      }
      m_T0[module] = T0;
      m_errT0[module] = errT0;
      m_status[module] = c_Calibrated;
    }

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     * @param moduleID module ID (1-based)
     */
    void setUnusable(int moduleID)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2ERROR("Invalid module number, status not set (" << ClassName() << ")");
        return;
      }
      m_status[module] = c_Unusable;
    }

    /**
     * Returns T0 of a module
     * @param moduleID module ID (1-based)
     * @return T0
     */
    double getT0(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
        return 0;
      }
      return m_T0[module];
    }

    /**
     * Returns error on T0 of a module
     * @param moduleID module ID (1-based)
     * @return error on T0
     */
    double getT0Error(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) {
        B2WARNING("Invalid module number, returning 0 (" << ClassName() << ")");
        return 0;
      }
      return m_errT0[module];
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if good calibrated
     */
    bool isCalibrated(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      return m_status[module] == c_Calibrated;
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if default (not calibrated)
     */
    bool isDefault(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      return m_status[module] == c_Default;
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if bad calibrated
     */
    bool isUnusable(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      return m_status[module] == c_Unusable;
    }


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

