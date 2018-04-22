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
   * Common T0 calibration constant. Calibration can be intra run dependent.
   * Done probably online using di-muon events.
   */
  class TOPCalCommonT0: public TObject {
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
    TOPCalCommonT0()
    {}

    /**
     * Full constructor, sets status to calibrated
     * @param T0 common T0
     * @param errT0 error on T0
     */
    TOPCalCommonT0(double T0, double errT0): m_T0(T0), m_errT0(errT0), m_status(c_Calibrated)
    {}

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
      */
    void setUnusable() {m_status = c_Unusable;}

    /**
     * Returns T0
     * @return T0
     */
    double getT0() const {return m_T0;}

    /**
     * Returns error on T0
     * @return error on T0
     */
    double getT0Error() const {return m_errT0;}

    /**
     * Returns calibration status
     * @return true, if good calibrated
     */
    bool isCalibrated() const {return m_status == c_Calibrated;}

    /**
     * Returns calibration status
     * @return true, if default (not calibrated)
     */
    bool isDefault() const {return m_status == c_Default;}

    /**
     * Returns calibration status
     * @return true, if bad calibrated
     */
    bool isUnusable() const {return m_status == c_Unusable;}

  private:

    float m_T0 = 0;    /**< calibration constant */
    float m_errT0 = 0; /**< error on constant */
    EStatus m_status = c_Default; /**< calibration status */

    ClassDef(TOPCalCommonT0, 3); /**< ClassDef */

  };

} // end namespace Belle2

