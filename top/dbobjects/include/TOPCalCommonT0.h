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
   * Common T0 calibration constant. Calibration can be intra run dependent.
   * Done usually with di-muon events.
   */
  class TOPCalCommonT0: public TObject {
  public:

    /**
     * Calibration status of a constant
     *
     * Notes:
     * - TOPDigits are alowed to be calibrated only if status = c_Calibrated
     * - value for status = c_roughlyCalibrated is used only to set initial value of
     *   bunch offset running average in bunch finder
     */
    enum EStatus {
      c_Default = 0,    /**< uncalibrated default value */
      c_Calibrated = 1, /**< good calibrated value */
      c_Unusable = 2,    /**< bad calibrated value */
      c_roughlyCalibrated = 3  /**< only roughly calibrated value (for HLT/expressreco) */
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
     * Switches calibration status to roughly calibrated (usefull at HLT/express reco)
      */
    void setRoughlyCalibrated() {m_status = c_roughlyCalibrated;}

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

    /**
     * Returns calibration status
     * @return true, if roughly calibrated
     */
    bool isRoughlyCalibrated() const {return m_status == c_roughlyCalibrated;}

  private:

    float m_T0 = 0;    /**< calibration constant */
    float m_errT0 = 0; /**< error on constant */
    EStatus m_status = c_Default; /**< calibration status */

    ClassDef(TOPCalCommonT0, 3); /**< ClassDef */

  };

} // end namespace Belle2

