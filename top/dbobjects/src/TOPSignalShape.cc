/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPSignalShape.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <math.h>

using namespace std;

namespace Belle2 {

  TOPSignalShape::TOPSignalShape(std::vector<double> shape, double timeBin, double tau,
                                 double pole1, double pole2):
    m_shape(shape), m_tau(tau), m_pole1(pole1), m_pole2(pole2)
  {
    m_tmax = m_tmin + (shape.size() - 1) * timeBin;

    // find peaking value (positive pulse maximum!)

    int samplePeak = 0;
    for (unsigned i = 0; i < shape.size(); i++) {
      if (shape[i] > shape[samplePeak]) samplePeak = i;
    }

    TSpline5 spline("spline", m_tmin, m_tmax, shape.data(), shape.size());
    double t1 = m_tmin + (samplePeak - 1) * timeBin;
    double t2 = m_tmin + (samplePeak + 1) * timeBin;
    for (int i = 0; i < 20; i++) {
      double t = (t1 + t2) / 2;
      if (spline.Derivative(t) > 0) {
        t1 = t;
      } else {
        t2 = t;
      }
    }
    double vPeak = spline.Eval((t1 + t2) / 2);

    // normalize waveform

    for (auto& value : m_shape) value /= vPeak;

    // find 50% CF crossing time (positive pulse!)

    auto sampleRise = samplePeak;
    while (sampleRise >= 0 and m_shape[sampleRise] > 0.5) sampleRise--;
    t1 = m_tmin + sampleRise * timeBin;
    t2 = m_tmin + (sampleRise + 1) * timeBin;
    for (int i = 0; i < 20; i++) {
      double t = (t1 + t2) / 2;
      if (spline.Eval(t) < vPeak / 2) {
        t1 = t;
      } else {
        t2 = t;
      }
    }
    double crossingTime = (t1 + t2) / 2;

    // set 50% CF crossing to happen at t = 0

    m_tmin -= crossingTime;
    m_tmax -= crossingTime;

  }


  double TOPSignalShape::getValue(double t) const
  {
    if (m_shape.empty()) {
      B2ERROR("TOPSignalShape::getValue: object not initialized");
      return 0;
    }
    if (t < m_tmin) return 0;
    if (t > m_tmax) return m_shape.back() * exp(-(t - m_tmax) / m_tau);
    if (!m_interpolator) {
      std::vector<double> shape(m_shape); // since argument in TSpline5 is not const!
      m_interpolator = new TSpline5("signalShape", m_tmin, m_tmax,
                                    shape.data(), shape.size());
    }
    return m_interpolator->Eval(t);
  }

} // end Belle2 namespace
