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
     * Default constructor
     */
    TOPCalModuleT0()
    {
      for (unsigned m = 0; m < c_numModules; m++) {
        m_T0[m] = 0;
        m_errT0[m] = 0;
        m_status[m] = false;
      }
    }

    /**
     * Sets calibration for a single module
     * @param moduleID module ID (1-based)
     * @param T0 module T0
     * @param errT0 error on T0
     */
    void setT0(int moduleID, double T0, double errT0)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      m_T0[module] = T0;
      m_errT0[module] = errT0;
      m_status[module] = true;
    }

    /**
     * Returns T0 of a module
     * @param moduleID module ID (1-based)
     * @return T0
     */
    double getT0(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return 0;
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
      if (module >= c_numModules) return 0;
      return m_errT0[module];
    }

    /**
     * Returns calibration status
     * @param moduleID module ID (1-based)
     * @return true, if calibrated
     */
    bool isCalibrated(int moduleID) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return false;
      return m_status[module];
    }


  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
         };

    float m_T0[c_numModules];    /**< calibration constants */
    float m_errT0[c_numModules]; /**< errors on constants */
    bool m_status[c_numModules]; /**< calibration status */

    ClassDef(TOPCalModuleT0, 2); /**< ClassDef */

  };

} // end namespace Belle2

