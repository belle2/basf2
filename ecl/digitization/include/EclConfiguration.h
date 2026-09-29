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

/* C++ headers. */
#include <array>

namespace Belle2 {
  namespace ECL {
    /** Singleton class to hold the ECL configuration */
    class  EclConfiguration {
    public:
      /** return this instance */
      static EclConfiguration& get()
      {
        static EclConfiguration instance;
        return instance;
      }
      /** return the background flag */
      bool background() const { return m_background; }
      /** set the background flag */
      void setBackground(bool val) { m_background = val; }

      static constexpr int        m_nch =
        ECLElementNumbers::c_NCrystals;  /**< total number of electronic channels (crystals) in calorimeter */
      static constexpr double    m_step = 0.5; /**< time between points in internal units t_{asrto}*m_rf/2./24./12. */
      static constexpr double    s_clock = 24.*12.; /**< digitization clock in RF units */
      static constexpr int       m_ntrg = 144; /**< number of trigger counts per ADC clock tick */

      static constexpr int       m_nsmp = 31; /**< number of ADC measurements for signal fitting */
      static constexpr double    m_tmin = -15; /**< lower range of the signal fitting region in ADC clocks */

      static constexpr int         m_nl = 48; /**< length of samples signal in number of ADC clocks */
      static constexpr int         m_ns = 32; /**< number of samples per ADC clock */
      static constexpr int        m_ndt = 96; /**< number of points per ADC tick where signal fit procedure parameters are evaluated */

      // Common configuration interface shared with EclConfigurationUp.
      static constexpr int       c_nChannels = m_nch; /**< total number of electronic channels (crystals) in calorimeter */
      static constexpr double    c_step = m_step;  /**< time between points in internal units t_{asrto}*m_rf/2./24./12. */
      static constexpr double    c_clock = s_clock;  /**< digitization clock in RF units */
      static constexpr int       c_nTriggerCounts = m_ntrg; /**< number of trigger counts per ADC clock tick */

      static constexpr int       c_nAdcSamples = m_nsmp;  /**< number of ADC measurements for signal fitting */
      static constexpr double    c_fitTimeMin = m_tmin;  /**< lower range of the signal fitting region in ADC clocks */
      static constexpr int       c_nPedestalSamples = c_nAdcSamples + c_fitTimeMin; /**< number of samples used for pedestal fit */

      static constexpr int       c_nSignalAdcClocks =
        m_nl;   /**< length of samples signal in number of ADC clocks (used for waveform generation) */
      static constexpr int       c_nSamplesPerAdcClock = m_ns;  /**< number of samples per ADC clock (used for waveform generation) */

      static constexpr int       c_nFitSamples = 16;  /**< number of samples in the fitting window */
      static constexpr int       c_nFitPointsPerAdcTick =
        m_ndt;  /**< number of points per ADC tick where signal fit procedure parameters are evaluated */
      static constexpr int       c_nSubTimeCorr = 12;  /**< number of sub-divisions for time correction for signal fit procedure */
      static constexpr int       c_nFitTimeShifts = 2;  /**< number of time shifts for signal fit procedure parameters */
      static constexpr int       c_nLowAmpFitPoints = c_nFitPointsPerAdcTick /
                                                      4;  /**< number of points for low amplitude fit procedure */

      static constexpr int       c_nWaveformParameters = 10; /**< number of parameters defining the waveform shape */

      static constexpr double    c_signalUnitScale = 27.7221; /**< Normalization of the default signal waveform. */
      static constexpr double    c_diodeUnitScale = 0.9569100 * 9.98822; /**< Normalization of the default diode waveform. */

    private:
      static double m_rf; /**< RF clock, www-linac.kek.jp/linac-com/report/skb-tdr/, ch. 6 */
      static double m_tick; /**< == 72/127 digitization clock tick (in microseconds) */

    public:
      /** See m_rf. This will try to use the value from the database */
      static double getRF();
      /** See m_tick */
      static double getTick();
      /** Return the default diode waveform parameters in shaper internal units. */
      static std::array<double, c_nWaveformParameters>
      getDefaultDiodeParameters();

    private:
      EclConfiguration() {}; /**< constructor */
      bool m_background{false}; /**< background configuration */
    public:

      /** a struct for a signal sample */
      struct SignalSample {
        /** @param MP        Float array of waveform parameters
         *  @param unitscale Normalization of template waveform
         */
        void initSample(const float* MP, double unitscale = -1);
        /** @param MPd       Double array of waveform parameters
         *  @param unitscale Normalization of template waveform
         */
        void initSample(const double* MPd, double unitscale = -1);
        /**
         * @param[in]  a  Signal amplitude
         * @param[in]  t0 Signal offset
         * @param[out] s  Output array with added signal
         *
         * @return Energy deposition in ADC units
         */
        double accumulate(const double a, const double t0, double* s) const;

        double m_sumscale; /**< energy deposit in fitting window scale factor */
        double m_ft[m_nl * m_ns]; /**< Simulated signal shape */
      };

      /** a struct for the ADC count */
      struct AdcCounts {
        /** add hit method */
        void addHit(const double amplitude, const double timeOffset, const SignalSample& signalSample);
        double m_total; /**< total deposition (sum of m_s array) */
        double m_adcCounts[c_nAdcSamples]; /**< flash ADC measurements */
        double m_totalHadronDep; /**< total true hadron energy deposition*/
        double m_totalDep; /**< total true energy deposition*/
        double m_energyConversion; /**< energy conversion factor*/
        double m_flightTime; /**< simulated time weighted by true deposited energy*/
        double m_timeShift; /**< simulated time shift*/
        double m_timeToSensor; /**< simulated time to sensor*/
      };

      /** a struct for the fit parameters */
      struct fitparams_t {
        typedef int int_array_192x16_t[c_nFitTimeShifts * c_nFitPointsPerAdcTick][c_nFitSamples]; /**< Array for DSP coefs in normal fit */
        typedef int int_array_24x16_t[c_nLowAmpFitPoints][c_nFitSamples]; /**< Array for DSP coefs in fast fit */

        using IntArrayGridXAdc = int_array_192x16_t; /**< Array for DSP coefficients in normal fit. */
        using IntArraySubXAdc = int_array_24x16_t; /**< Array for DSP coefficients in low-amplitude fit. */

        int_array_192x16_t f; /**< Array with tabulated signal waveform */
        int_array_192x16_t f1; /**< Array with tabulated derivative of signal waveform */
        int_array_192x16_t fg31; /**< Array to estimate signal amplitude */
        int_array_192x16_t fg32; /**< Array to estimate amp * time */
        int_array_192x16_t fg33; /**< Array to estimate pedestal height */
        int_array_24x16_t fg41; /**< Alternative for FG31 for low amplitude fits */
        int_array_24x16_t fg43; /**< Alternative for FG33 for low amplitude fits */
      };

      /** a struct for the parameters of the algorithm */
      struct algoparams_t {
        union {
          short int     id[16]; /**< */
          unsigned char ic[32]; /**< */
        };
      };

      using FitParams = fitparams_t; /**< Fit parameters type. */
      using AlgoParams = algoparams_t; /**< Algorithm parameters type. */

    };
  }
}
