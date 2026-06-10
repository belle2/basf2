/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>

namespace Belle2 {
  namespace ECL {
    /** Singleton class to hold the ECL configuration */
    class  EclConfigurationUp {
    public:
      /** return this instance */
      static EclConfigurationUp& get()
      {
        static EclConfigurationUp instance;
        return instance;
      }
      /** return the background flag */
      bool background() const { return m_background; }
      /** set the background flag */
      void setBackground(bool val) { m_background = val; }

      static constexpr int        m_nch =
        ECLElementNumbers::c_NCrystals;  /**< total number of electronic channels (crystals) in calorimeter */
      static constexpr double    m_step = 1. / 9.; /**< time between points in internal units t_{asrto}*m_rf/2./64. */
      static constexpr double    s_clock = 64.; /**< digitization clock in RF units */
      static constexpr int       m_ntrg = 256; /**< number of trigger counts per ADC clock tick */

      static constexpr int       m_nsmp = 64; /**< number of ADC measurements for signal fitting */
      static constexpr double    m_tmin = -32; /**< lower range of the signal fitting region in ADC clocks */

      static constexpr int       m_nl = 96; /**< length of samples signal in number of ADC clocks */
      static constexpr int       m_ns = 32; /**< number of samples per ADC clock */
      static constexpr int       m_ndt = 128; /**< number of points per ADC tick where signal fit procedure parameters are evaluated */
      static constexpr int       m_ntf = 4; /**< number of time shifts for signal fit procedure parameters */
      static constexpr int       m_nta = 32; /**< number of time shifts for signal fit procedure parameters in low amplitude case */

    private:
      static double m_rf; /**< RF clock, www-linac.kek.jp/linac-com/report/skb-tdr/, ch. 6 */
      static double m_tick; /**< == 72/127 digitization clock tick (in microseconds) */

    public:
      /** See m_rf. This will try to use the value from the database */
      static double getRF();
      /** See m_tick */
      static double getTick();

    private:
      EclConfigurationUp() {}; /**< constructor */
      bool m_background{false}; /**< background configuration */
    public:

      /** a struct for a signal sample */
      struct signalsample_t {
        /** @param MP        Float array of waveform parameters
         *  @param unitscale Normalization of template waveform
         */
        void InitSample(const float* MP, double unitscale = -1);
        /** @param MPd       Double array of waveform parameters
         *  @param unitscale Normalization of template waveform
         */
        void InitSample(const double* MPd, double unitscale = -1);
        /**
         * @param[in]  a  Signal amplitude
         * @param[in]  t0 Signal offset
         * @param[out] s  Output array with added signal
         *
         * @return Energy deposition in ADC units
         */
        double Accumulate(const double a, const double t0, double* s) const;

        double m_sumscale; /**< energy deposit in fitting window scale factor */
        double m_ft[m_nl * m_ns]; /**< Simulated signal shape */
      };

      /** a struct for the ADC count */
      struct adccounts_t {
        /** add hit method */
        void AddHit(const double a, const double t0, const signalsample_t& q);
        double total; /**< total deposition (sum of m_s array) */
        double c[m_nsmp]; /**< flash ADC measurements */
        double totalHadronDep; /**< total true hadron energy deposition*/
        double totalDep; /**< total true energy deposition*/
        double energyConversion; /**< energy conversion factor*/
        double flighttime; /**< simulated time weighted by true deposited energy*/
        double timeshift; /**< simulated time shift*/
        double timetosensor; /**< simulated time to sensor*/
      };

      static const int m_npre = m_nsmp; /**< number of samples in the fitting window */

      /** a struct for the fit parameters */
      struct fitparams_t {
        typedef int int_array_gridxadc_t[m_ntf * m_ndt][m_npre]; /**< Array for DSP coefs in normal fit */
        typedef int int_array_subxadc_t[m_nta][m_npre]; /**< Array for DSP coefs in fast fit */
        int_array_gridxadc_t f; /**< Array with tabulated signal waveform */
        int_array_gridxadc_t f1; /**< Array with tabulated derivative of signal waveform */
        int_array_gridxadc_t fg31; /**< Array to estimate signal amplitude */
        int_array_gridxadc_t fg32; /**< Array to estimate amp * time */
        int_array_gridxadc_t fg33; /**< Array to estimate pedestal height */
        int_array_subxadc_t fg41; /**< Alternative for FG31 for low amplitude fits */
        int_array_subxadc_t fg43; /**< Alternative for FG33 for low amplitude fits */
      };

      /** a struct for the parameters of the algorithm */
      struct algoparams_t {
        union {
          short int     id[16]; /**< */
          unsigned char ic[32]; /**< */
        };
      };

    };
  }
}
