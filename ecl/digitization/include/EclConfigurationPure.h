/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCONFIGURATIONPURE_H
#define ECLCONFIGURATIONPURE_H

#include <ecl/digitization/EclConfiguration.h>

class TH1F;

namespace Belle2 {
  namespace ECL {

    /** class to hole the ECL configuration for pure CsI calorimeter */
    class EclConfigurationPure {
    public:
      static constexpr int    m_nch      = 72 * 16; /**< total number of electronic channels (crystals) in fwd endcap calorimeter */
      static constexpr int    m_nsmp     = EclConfiguration::m_nsmp; /**< number of ADC measurements for signal fitting */
      static constexpr double m_tmin     = -15; /**<  lower range of the signal fitting region in ADC clocks */
      static constexpr int    m_ntrg     = EclConfiguration::m_ntrg; /**< number of trigger counts per ADC clock tick */
      static double           m_tickPure;
      static constexpr int    m_nlPure   = EclConfiguration::m_nl * 15; /**< length of samples signal in number of ADC clocks */
      static constexpr int    m_ns       = EclConfiguration::m_ns; /**< number of samples per ADC clock */

      static constexpr int    m_ndtPure  = m_ns; /**< number of points per ADC tick where signal fit procedure parameters are evaluated */


      /** a struct for a signal sample for the pure CsI calorimeter */
      struct signalsamplepure_t {
        double m_sumscale; /**< energy deposit in fitting window scale factor */
        double m_ft[m_nlPure * m_ns];
        double m_ft1[m_nlPure * m_ns];

        void InitSample(const TH1F*, const TH1F*);
        double Accumulate(const double, const double, double*) const;
      };

      /** a struct for the fit parameters for the pure CsI calorimeter */
      struct adccountspure_t {
        double total; /**< total deposition (sum of m_s array) */
        double c[m_nsmp]; /**< flash ADC measurements */
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
