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

#include <top/reconstruction_cpp/BackgroundPDF.h>
#include <top/reconstruction_cpp/PixelPositions.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <vector>
#include <cmath>

namespace Belle2 {
  namespace TOP {

    /**
     * Parametrization of delta-ray PDF in pixels of single module
     */
    class DeltaRayPDF {

    public:

      /**
       * Class constructor
       * @param moduleID slot ID
       */
      explicit DeltaRayPDF(int moduleID);

      /**
       * Prepare the object
       * @param track track at TOP
       * @param hypothesis particle hypothesis
       */
      void prepare(const TOPTrack& track, const Const::ChargedStable& hypothesis);

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns number of photons
       * @return number of photons
       */
      double getNumPhotons() const {return m_numPhotons;}

      /**
       * Returns fraction of delta-ray photons in the time window
       * @return fraction of delta-ray photons in the time window
       */
      double getFraction() const {return m_fraction;}

      /**
       * Returns PDF value at given time and pixel
       * @param pixelID pixel ID
       * @param time photon hit time
       * @return PDF value
       */
      double getPDFValue(int pixelID, double time) const;

      /**
       * Returns PDF value at given time and integrated over all pixels
       * @param time photon hit time
       * @param dt0 direct peak position correction
       * @param acc acceptance correction factor for direct peak
       * @return PDF value (projection to time axis)
       */
      double getPDFValue(double time, double dt0 = 0, double acc = 1) const;

      /**
       * Returns integral of PDF from minTime to maxTime
       * @param minTime integral lower limit
       * @param maxTime integral upper limit
       * @return integral of PDF
       */
      double getIntegral(double minTime, double maxTime) const;

    private:

      /**
       * Normal (Gaussian) distribution: an entry for the table
       */
      struct GausXY {
        double x = 0; /**< abscissa */
        double y = 0; /**< function value */

        /**
         * Constructor
         * @param X abscissa
         */
        explicit GausXY(double X): x(X), y(exp(-0.5 * x * x))
        {}
      };

      /**
       * Angular distribution of photons from delta rays w/ total reflection requirement.
       * Distribution is not normalized.
       * @param kz z-component of photon direction
       * @return distribution value
       */
      double angularDistr(double kz) const;

      /**
       * Time distribution of photons from delta rays (normalized).
       * @param t photon propagation time
       * @param t0 minimal possible propagation time
       * @return normalized time distribution value at given propagaton time
       */
      double timeDistr(double t, double t0) const;

      /**
       * Smeared time distribution of photons from delta rays (normalized).
       * @param t photon propagation time
       * @param t0 minimal possible propagation time
       * @return normalized time distribution value at given propagaton time
       */
      double smearedTimeDistr(double t, double t0) const;

      /**
       * Fraction of delta-ray photons within given propagation time interval for single peak at t0
       * @param tmin time interval lower edge
       * @param tmax time interval upper edge
       * @param t0 minimal possible propagation time
       * @return fraction of photons within time interval for single peak
       */
      double peakFraction(double tmin, double tmax, double t0) const;

      /**
       * Total fraction of delta-ray photons within given propagation time interval
       * @param tmin time interval lower edge
       * @param tmax time interval upper edge
       * @return fraction of photons within time interval including direct and reflected peaks
       */
      double totalFraction(double tmin, double tmax) const;

      /**
       * Fraction of direct photons from delta-rays, e.g direct/(direct+reflected)
       * @param z local z position of track at TOP
       * @return fraction of direct photons
       */
      double directFraction(double z) const;

      /**
       * Photon yield from delta-rays per track length in quartz for nominal photon detection efficiency
       * @param beta particle velocity
       * @param PDGCode PDG code
       * @return photon yield per cm
       */
      double photonYield(double beta, int PDGCode) const;

      // variables set in constructor (slot dependent)
      int m_moduleID; /**< slot ID */
      const BackgroundPDF* m_background = 0; /**< background PDF */
      const PixelPositions* m_pixelPositions = 0; /**< pixel positions */
      double m_zD = 0; /**< detector (photo-cathode) position in z */
      double m_zM = 0; /**< spherical mirror position in z */
      double m_phaseIndex = 0; /**< phase refractive index */
      double m_groupIndex = 0; /**< group refractive index */
      double m_dispersion = 0; /**< dispersion coefficient */
      double m_angularNorm = 0; /**< angular distribution normalization constant */
      std::vector<double> m_norms; /**< relative angular distribution normalization constants (cumulative) */
      std::vector<GausXY> m_tableGaus; /**< table of normal (Gaussian) distribution */

      // variables set in prepare method (track/hypothesis dependent)
      double m_xE = 0;  /**< average photon emission position in x */
      double m_yE = 0;  /**< average photon emission position in y */
      double m_zE = 0;  /**< average photon emission position in z */
      double m_dirFrac = 0; /**< fraction of direct photons */
      double m_dirT0 = 0;   /**< minimal propagation time of direct photons */
      double m_reflT0 = 0;  /**< minimal propagation time of reflected photons */
      double m_TOF = 0;     /**< time-of-flight of particle */
      double m_fraction = 0; /**< fraction of delta-ray photons within time window */
      double m_numPhotons = 0; /**< number of photons */
      std::vector<double> m_pixelAcceptances; /** pixel angular acceptances for direct peak (index = pixelID - 1) */

    };


    //--- inline functions ------------------------------------------------------------

    inline double DeltaRayPDF::angularDistr(double kz) const
    {
      if (abs(kz) < 1 / m_phaseIndex) {
        double x = sqrt((1 - 1 / pow(m_phaseIndex, 2)) / (1 - pow(kz, 2)));
        return 1 - acos(x) * 4 / M_PI;
      }
      return 1;
    }

    inline double DeltaRayPDF::timeDistr(double t, double t0) const
    {
      if (t < t0) return 0;
      return angularDistr(t0 / t) / m_angularNorm * t0 / pow(t, 2);
    }

    inline double DeltaRayPDF::getPDFValue(double time, double dt0, double acc) const
    {
      double t = time - m_TOF;
      double pdfDirect = smearedTimeDistr(t, m_dirT0 + dt0) * acc;
      double pdfReflec = smearedTimeDistr(t, m_reflT0);
      return m_dirFrac * pdfDirect + (1 - m_dirFrac) * pdfReflec;
    }

    inline double DeltaRayPDF::getIntegral(double minTime, double maxTime) const
    {
      return totalFraction(minTime, maxTime) / m_fraction;
    }

  } // namespace TOP
} // namespace Belle2

