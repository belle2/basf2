/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/digitization/EclConfiguration.h>

class TH1F;

namespace Belle2 {
  namespace ECL {

    /** Singleton class to hold the ECL configuration for pure CsI calorimeter */
    class EclConfigurationPure {
    public:
      static constexpr int    m_nch      = 72 * 16; /**< total number of electronic channels (crystals) in fwd endcap calorimeter */
      static constexpr int    m_nsmp     = EclConfiguration::m_nsmp; /**< number of ADC measurements for signal fitting */
      static constexpr double m_tmin     = -15; /**<  lower range of the signal fitting region in ADC clocks */
      static constexpr int    m_ntrg     = EclConfiguration::m_ntrg; /**< number of trigger counts per ADC clock tick */
      static constexpr int    m_nlPure   = EclConfiguration::m_nl * 15; /**< length of samples signal in number of ADC clocks */
      static constexpr int    m_ns       = EclConfiguration::m_ns; /**< number of samples per ADC clock */

      static constexpr int    m_ndtPure  = m_ns; /**< number of points per ADC tick where signal fit procedure parameters are evaluated */

    private:
      /** Digitization tick for pure CsI calorimeter (microseconds) */
      static double m_tickPure;

    public:
      /** Getter for m_tickPure */
      static double getTickPure()
      {
        if (m_tickPure < 0) {
          m_tickPure = EclConfiguration::getTick() / EclConfiguration::m_ntrg * 8;
        }
        return m_tickPure;
      }
      /** Setter for m_tickPure */
      static void setTickPure(double newval)
      {
        m_tickPure = newval;
      }

      /** a struct for a signal sample for the pure CsI calorimeter */
      struct signalsamplepure_t {
        double m_sumscale; /**< energy deposit in fitting window scale factor */
        double m_ft[m_nlPure * m_ns]; /**< Simulated signal shape */
        double m_ft1[m_nlPure * m_ns]; /**< Simulated signal shape */

        /** initialisation of signal sample */
        void InitSample(const TH1F*, const TH1F*);
        /**
         * @param[in]  a  Signal amplitude
         * @param[in]  t0 Signal offset
         * @param[out] s  Output array with added signal
         *
         * @return Energy deposition in ADC units
         */
        double Accumulate(const double a, const double t0, double* s) const;
      };

      /** a struct for the fit parameters for the pure CsI calorimeter */
      struct adccountspure_t {
        double total; /**< total deposition (sum of m_s array) */
        double c[m_nsmp]; /**< flash ADC measurements */
        /** add hit method */
        void AddHit(const double a, const double t0, const signalsamplepure_t& q);
      };

      /**
       * A struct for the fit parameters for a single channel of the pure CsI
       * calorimeter (in the simulation, all calorimeter channels normally use
       * the same set of fit parameters).
       *
       * For detailed description of the fit algorithm, see ECL-TN-2013-02
       * (latest version at https://gitlab.desy.de/belle2/documents/ecl-tn-2013-02/-/blob/master/digi.pdf)
       */
      struct fitparamspure_t {
        /** Matrix used in shape fit algorithm.
         *  1st index (i): signal sample ID (waveform always contains 15 signal samples)
         *   0     -> special point that represents average pedestal value
         *   1..15 -> signal samples
         *  2nd index (j): tabulated point ID
         *   0                            -> point at the ADC sample #i
         *   1..(m_ndtPure-1)             -> points to the right of ADC sample #i
         *   m_ndtPure..(2*m_ndtPure - 1) -> points to the left of ADC sample #i
         */
        typedef double double_matrix[16][2 * m_ndtPure];
        /** Array used in shape fit algorithm.
         *  1st index: tabulated point ID
         *   0                            -> point at the signal start
         *   1..(m_ndtPure-1)             -> points to the right of the signal start
         *   m_ndtPure..(2*m_ndtPure - 1) -> points to the left of the signal start
         */
        typedef double fine_array[2 * m_ndtPure]; /**< sub-array to tabulate signal fit parameters */

        double invC[16][16]; /**< inverse noise covariance matrix */
        double_matrix f;     /**< signal response function */
        double_matrix f1;    /**< first derivative of a signal response function */
        /**
         * Intermediate coefficients for the left side of the system of linear
         * equations to reconstruct amplitude, time and pedestal
         */
        fine_array c110;
        fine_array c200; /**< \see c110 */
        fine_array c020; /**< \see c110 */
        fine_array c101; /**< \see c110 */
        fine_array c011; /**< \see c110 */
        double c002;     /**< \see c110 */
        /**
         * Intermediate coefficients for the right side of the system of linear
         * equations to reconstruct amplitude, time and pedestal
         */
        double_matrix c100;
        double_matrix c010; /**< \see c100 */
        double c001[16];    /**< \see c100 */
      };

    };

  }
}
