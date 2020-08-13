/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPCalTimeWalk.h>
#include <math.h>

namespace Belle2 {

  void TOPCalTimeWalk::set(const std::vector<double>& timeWalkParams,
                           double noise, double quadratic)
  {
    m_timeWalkParams = timeWalkParams;
    m_noise = noise;
    m_quadratic = quadratic;
    m_status = c_Calibrated;
  }

  double TOPCalTimeWalk::getTimeWalk(int pulseHeight) const
  {
    /* time walk parameters are coefficients in a polynomial
     * that is evaluated at x = pulseHeight here */

    double f = 0;
    double x = 1;
    for (auto p : m_timeWalkParams) {
      f += p * x;
      x *= pulseHeight;
    }
    return f;
  }

  double TOPCalTimeWalk::getSigmaSq(int pulseHeight) const
  {
    if (pulseHeight <= 0) return 0;
    double x = pulseHeight;
    return pow(m_noise / x, 2) + pow(m_quadratic * x * x, 2);
  }

  double TOPCalTimeWalk::getSigma(int pulseHeight) const
  {
    return sqrt(getSigmaSq(pulseHeight));
  }


} // end Belle2 namespace
