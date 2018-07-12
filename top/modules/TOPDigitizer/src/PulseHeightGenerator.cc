/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPDigitizer/PulseHeightGenerator.h>
#include <framework/logging/Logger.h>
#include <iostream>

using namespace std;

namespace Belle2 {
  namespace TOP {

    PulseHeightGenerator::PulseHeightGenerator(double x0, double p1, double p2,
                                               double xmax):
      m_x0(x0), m_p1(p1), m_p2(p2), m_xmax(xmax)
    {
      if (x0 <= 0)
        B2FATAL("TOP::PulseHeightGenerator: parameter x0 must be positive");
      if (p1 < 0)
        B2FATAL("TOP::PulseHeightGenerator: parameter p1 must be non-negative");
      if (p2 <= 0)
        B2FATAL("TOP::PulseHeightGenerator: parameter p2 must be positive");

      double xPeak = pow((p1 / p2), 1 / p2) * x0;
      if (m_xmax < xPeak) xPeak = m_xmax;
      m_vPeak = getValue(xPeak);

    }


    double PulseHeightGenerator::generate() const
    {
      if (m_xmax <= 0) return 0;
      while (true) {
        double x = gRandom->Uniform(m_xmax);
        if (gRandom->Uniform(m_vPeak) < getValue(x)) return x;
      }
    }

  } // TOP
} // Belle2
