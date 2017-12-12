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
     * Default constructor
     */
    TOPCalCommonT0()
    {}

    /**
     * Full constructor
     * @param T0 common T0
     * @param errT0 error on T0
     */
    TOPCalCommonT0(double T0, double errT0): m_T0(T0), m_errT0(errT0), m_status(true)
    {}

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
     * @return true, if calibrated
     */
    bool isCalibrated() const {return m_status;}

  private:

    float m_T0 = 0;    /**< calibration constant */
    float m_errT0 = 0; /**< error on constant */
    bool m_status = false; /**< calibration status */

    ClassDef(TOPCalCommonT0, 2); /**< ClassDef */

  };

} // end namespace Belle2

