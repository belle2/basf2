/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/SignalPDF.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SignalPDF::SignalPDF(int pixelID, const TOPNominalTTS& tts):
      m_pixelID(pixelID), m_tts(&tts)
    {}

    double SignalPDF::getPDFValue(double time, double timeErr, double sigt) const
    {
      double wid0 = timeErr * timeErr + sigt * sigt;
      double f = 0;
      for (const auto& peak : m_peaks) {
        for (const auto& gaus : m_tts->getTTS()) {
          double sigma = peak.wid + gaus.sigma * gaus.sigma + wid0; // sigma squared!
          double x = pow(time - peak.t0 - gaus.position, 2) / sigma;
          if (x > 10) continue; // TODO
          sigma = sqrt(2 * M_PI * sigma);
          f += peak.nph * gaus.fraction / sigma * exp(-x / 2);
        }
      }
      return f;
    }

  } // namespace TOP
} // namespace Belle2

