/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/SignalPDF.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SignalPDF::SignalPDF(int pixelID, const TOPNominalTTS& tts):
      m_pixelID(pixelID), m_tts(&tts)
    {
      // the largest TTS component width and offset, used to skip the peaks that are too far in time
      for (const auto& gaus : tts.getTTS()) {
        m_maxSigmaSq = std::max(m_maxSigmaSq, static_cast<double>(gaus.sigma * gaus.sigma));
        m_maxAbsPos = std::max(m_maxAbsPos, std::abs(static_cast<double>(gaus.position)));
      }
    }

    double SignalPDF::getPDFValue(double time, double timeErr, double sigt) const
    {
      double wid0 = timeErr * timeErr + sigt * sigt;
      double f = 0;
      for (const auto& peak : m_peaks) {
        // a peak whose every term is rejected by the x > 100 cut below cannot contribute:
        // the 101 keeps this necessary condition on the safe side of rounding
        double t = time - peak.t0;
        double d = std::abs(t) - m_maxAbsPos;
        if (d > 0 and d * d > 101 * (peak.wid + m_maxSigmaSq + wid0)) continue;
        for (const auto& gaus : m_tts->getTTS()) {
          double sigma = peak.wid + gaus.sigma * gaus.sigma + wid0; // sigma squared!
          double dt = t - gaus.position;
          double x = dt * dt / sigma;
          if (x > 100) continue;
          sigma = sqrt(2 * M_PI * sigma);
          f += peak.nph * gaus.fraction / sigma * exp(-x / 2);
        }
      }
      return f;
    }

  } // namespace TOP
} // namespace Belle2

