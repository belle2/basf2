/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Calibration constants for time-walk correction and for tuning of electronic time
   * resolution in digitization.
   */
  class TOPCalTimeWalk: public TObject {

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
    TOPCalTimeWalk()
    {}

    /**
     * Sets all the parameters and switches status to calibrated
     * @param timeWalkParams parameters of time-walk calibration curve (polynomial) [ns]
     * @param a electronic time resolution: noise term excess parameter [ns]
     * @param b electronic time resolution: quadratic term parameter [ns]
     */
    void set(const std::vector<double>& timeWalkParams, double a, double b);

    /**
     * Switches calibration status to unusable to flag badly calibrated constant
     */
    void setUnusable() {m_status = c_Unusable;}

    /**
     * Returns time walk parameters of the calibration curve
     * @return parameters of a polynomial [ns]
     */
    const std::vector<double>& getTimeWalkParams() const {return m_timeWalkParams;}

    /**
     * Returns noise term excess parameter of electronic time resolution
     * @terurn noise term excess parameter [ns]
     */
    double getA() const {return m_a;}

    /**
     * Returns quadratic term parameter of electronic time resolution
     * @terurn quadratic term parameter [ns]
     */
    double getB() const {return m_b;}

    /**
     * Returns time-walk at given pulse height
     * @param pulseHeight pulse height
     * @return time-walk [ns]
     */
    double getTimeWalk(int pulseHeight) const;

    /**
     * Returns an excess of electronic time resolution at given pulse height
     * @param pulseHeight pulse height
     * @return excess sigma squared [ns^2]
     */
    double getSigmaSq(int pulseHeight) const;

    /**
     * Returns an excess of electronic time resolution at given pulse height
     * @param pulseHeight pulse height
     * @return excess sigma [ns]
     */
    double getSigma(int pulseHeight) const;

    /**
     * Returns calibration status
     * @return true, if good calibrated
     */
    bool isCalibrated() const {return m_status == c_Calibrated;}

    /**
     * Returns calibration status
     * @return true, if default (not calibrated)
     */
    bool isDefault() const {return m_status == c_Default;};

    /**
     * Returns calibration status
     * @return true, if bad calibrated
     */
    bool isUnusable() const {return m_status == c_Unusable;};

  private:

    std::vector<double> m_timeWalkParams; /**< parameters of calibration curve [ns] */
    double m_a = 0; /**< noise term excess parameter [ns] */
    double m_b = 0; /**< quadratic term parameter [ns] */
    EStatus m_status = c_Default; /**< calibration status */

    ClassDef(TOPCalTimeWalk, 1); /**< ClassDef */

  };


} // end namespace Belle2
