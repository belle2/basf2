/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPNominalTTS.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Parametrization of signal PDF in a single pixel.
     * PDF is parametrized as a sum of Gaussian distributions.
     */
    class SignalPDF {

    public:

      /**
       * Enumerator for single PDF peak types
       */
      enum EPeakType {
        c_Unknown = 0, /**< unknown */
        c_Direct = 1,   /**< direct photon */
        c_Reflected = 2 /**< reflected photon */
      };

      /**
       * Single PDF peak
       */
      struct PDFPeak {
        double t0 = 0; /**< peak position [ns] */
        double wid = 0; /**< peak width squared [ns^2] */
        double nph = 0; /**< normalized number of photons in a peak */
        double fic = 0; /**< Cerenkov azimuthal angle */
      };

      /**
       * Extra information about single PDF peak
       */
      struct PDFExtra {
        double thc = 0; /**< Cerenkov (polar) angle */
        double e = 0; /**< photon energy */
        double sige = 0; /**< photon energy sigma squared */
        int Nxm = 0; /**< number of reflections in x before mirror */
        int Nxb = 0; /**< number of reflections in x after mirror and before prism */
        int Nxe = 0; /**< number of reflections in x inside prism */
        int Nym = 0; /**< number of reflections in y before mirror */
        int Nyb = 0; /**< number of reflections in y after mirror and before prism */
        int Nye = 0; /**< number of reflections in y inside prism */
        double xD = 0; /**< unfolded detection x coordinate */
        double yD = 0; /**< unfolded detection y coordinate */
        double zD = 0; /**< unfolded detection z coordinate */
        double yB = 0; /**< unfolded coordinate x at prism entrance */
        double kxE = 0; /**< photon direction x at emission */
        double kyE = 0; /**< photon direction y at emission */
        double kzE = 0; /**< photon direction z at emission */
        double kxD = 0; /**< photon direction x at detection */
        double kyD = 0; /**< photon direction y at detection */
        double kzD = 0; /**< photon direction z at detection */
        EPeakType type = c_Unknown; /**< peak type */
      };

      /**
       * Class constructor
       * @param pixelID pixel ID
       * @param tts TTS
       */
      SignalPDF(int pixelID, const TOPNominalTTS& tts);

      /**
       * Appends single PDF peak
       * @param peak single PDF peak
       */
      void append(const PDFPeak& peak)
      {
        if (m_peaks.empty()) {
          m_minT0 = peak.t0;
          m_maxT0 = peak.t0;
        } else {
          m_minT0 = std::min(m_minT0, peak.t0);
          m_maxT0 = std::max(m_maxT0, peak.t0);
        }
        m_peaks.push_back(peak);
        m_sum += peak.nph; // accumulated in the same order as the loop in getSum()
      }

      /**
       * Appends extra information of single PDF peak
       * @param extra extra information
       */
      void append(const PDFExtra& extra) { m_extras.push_back(extra);}

      /**
       * Normalize PDF peaks by dividing nph with given constant
       * @param a normalization constant
       */
      void normalize(double a);

      /**
       * Returns a sum of nph of all peaks
       */
      double getSum() const;

      /**
       * Returns pixel ID
       * @return pixel ID
       */
      int getPixelID() const {return m_pixelID;}

      /**
       * Returns TTS
       * @return TTS
       */
      const TOPNominalTTS* getTTS() const {return m_tts;}

      /**
       * Returns PDF peaks
       * @return PDF peaks
       */
      const std::vector<PDFPeak>& getPDFPeaks() const {return m_peaks;}

      /**
       * Returns PDF extra info
       * Note: available only if stored on demand
       * @return PDF extra info
       */
      const std::vector<PDFExtra>& getPDFExtraInfo() const {return m_extras;}

      /**
       * Returns the position of the earliest peak (meaningless if there are no peaks)
       * @return peak position [ns]
       */
      double getMinT0() const {return m_minT0;}

      /**
       * Returns the position of the latest peak (meaningless if there are no peaks)
       * @return peak position [ns]
       */
      double getMaxT0() const {return m_maxT0;}

      /**
       * Checks if extra info is available
       * @return true if available
       */
      bool isExtraInfoAvailable() const {return m_extras.size() == m_peaks.size();}

      /**
       * Returns PDF value at given time
       * @param time photon hit time
       * @param timeErr uncertainty of hit time (sigma)
       * @param sigt additional time smearing
       * @return PDF value
       */
      double getPDFValue(double time, double timeErr, double sigt = 0) const;

      /**
       * Returns integral of PDF from minTime to maxTime
       * @param minTime integral lower limit
       * @param maxTime integral upper limit
       * @return integral of PDF
       */
      double getIntegral(double minTime, double maxTime) const;

    private:

      int m_pixelID = 0; /**< pixel ID */
      const TOPNominalTTS* m_tts = 0; /**< nominal TTS */
      double m_maxSigmaSq = 0; /**< the largest of the TTS component widths squared [ns^2] */
      double m_maxAbsPos = 0;  /**< the largest of the TTS component offsets in absolute value [ns] */
      double m_sum = 0;    /**< sum of nph of all peaks */
      double m_minT0 = 0;  /**< position of the earliest peak [ns] */
      double m_maxT0 = 0;  /**< position of the latest peak [ns] */
      std::vector<PDFPeak> m_peaks; /**< PDF peaks */
      std::vector<PDFExtra> m_extras; /**< extra information (stored only on request) */

    };

    //--- inline functions ------------------------------------------------------------

    inline void SignalPDF::normalize(double a)
    {
      for (auto& peak : m_peaks) peak.nph /= a;
      m_sum = 0;
      for (const auto& peak : m_peaks) m_sum += peak.nph;
    }

    inline double SignalPDF::getSum() const
    {
      return m_sum;
    }

    inline double SignalPDF::getIntegral(double minTime, double maxTime) const
    {
      if (m_peaks.empty()) return 0;
      if (minTime <= m_minT0 and maxTime >= m_maxT0) return m_sum; // all the peaks are inside
      if (maxTime < m_minT0 or minTime > m_maxT0) return 0;        // none of the peaks is inside

      double s = 0;
      for (const auto& peak : m_peaks) {
        if (peak.t0 < minTime or peak.t0 > maxTime) continue;
        s += peak.nph;
      }
      return s;
    }

  } // namespace TOP
} // namespace Belle2

