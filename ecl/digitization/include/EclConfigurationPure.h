/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ECLCONFIGURATIONPURE_H
#define ECLCONFIGURATIONPURE_H

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
        double m_ft1[m_nlPure * m_ns];

        void InitSample(const TH1F*, const TH1F*);
        /**
         * @param[in]  a  Signal amplitude
         * @param[in]  t  Signal offset
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

      /** a struct for the fit parameters for the pure CsI calorimeter */
      struct fitparamspure_t {
        typedef double double_matrix[16][2 * m_ndtPure];
        typedef double fine_array[2 * m_ndtPure];
        double invC[16][16];
        double_matrix f, f1;
        double_matrix c100, c010;
        fine_array c110, c200, c020, c101, c011;
        double c002;
        double c001[16];
      };

    };

  }
}

#endif
