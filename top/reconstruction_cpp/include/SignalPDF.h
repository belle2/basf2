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

#include <top/dbobjects/TOPNominalTTS.h>
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
      void append(const PDFPeak& peak) { m_peaks.push_back(peak);}

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
      std::vector<PDFPeak> m_peaks; /**< PDF peaks */
      std::vector<PDFExtra> m_extras; /**< extra information (stored only on request) */

    };

    //--- inline functions ------------------------------------------------------------

    inline void SignalPDF::normalize(double a)
    {
      for (auto& peak : m_peaks) peak.nph /= a;
    }

    inline double SignalPDF::getSum() const
    {
      double s = 0;
      for (const auto& peak : m_peaks) s += peak.nph;
      return s;
    }

  } // namespace TOP
} // namespace Belle2

