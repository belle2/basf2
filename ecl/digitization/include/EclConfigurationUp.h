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
#include <array>

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

      static constexpr int        c_nChannels =
        ECLElementNumbers::c_NCrystals;  /**< total number of electronic channels (crystals) in calorimeter */
      static constexpr double    c_step = 1. / 9.; /**< time between points in internal units t_{asrto}*m_rf/2./64. */
      static constexpr double    c_clock = 64.; /**< digitization clock in RF units */
      static constexpr int       c_nTriggerCounts = 256; /**< number of trigger counts per ADC clock tick */

      static constexpr int       c_nAdcSamples = 64; /**< number of ADC measurements for signal fitting */
      static constexpr double    c_fitTimeMin = -32; /**< lower range of the signal fitting region in ADC clocks */

      static constexpr int       c_nSignalAdcClocks = 96; /**< length of samples signal in number of ADC clocks */
      static constexpr int       c_nSamplesPerAdcClock = 32; /**< number of samples per ADC clock */
      static constexpr int       c_nFitPointsPerAdcTick =
        128; /**< number of points per ADC tick where signal fit procedure parameters are evaluated */
      static constexpr int       c_nFitTimeShifts = 4; /**< number of time shifts for signal fit procedure parameters */
      static constexpr int       c_nLowAmplitudeFitTimeShifts =
        32; /**< number of time shifts for signal fit procedure parameters in low amplitude case */

      static constexpr int       c_nWaveformParameters = 14; /**< number of parameters defining the waveform shape */

      static constexpr double c_tauD = 0.242;          /**< Default upgraded shaper parameter tauD. */
      static constexpr double c_tau0 = 0.2111;         /**< Default upgraded shaper parameter tau0. */
      static constexpr double c_omega0 = 0.1767;       /**< Default upgraded shaper parameter omega0. */
      static constexpr double c_tau1 = 0.2441;         /**< Default upgraded shaper parameter tau1. */
      static constexpr double c_omega1 = 0.0929;       /**< Default upgraded shaper parameter omega1. */
      static constexpr double c_tauI = 11.134;         /**< Default upgraded shaper parameter tauI. */
      static constexpr double c_fractionI = 0.0872;    /**< Default upgraded shaper parameter fractionI. */
      static constexpr double c_tauS = 0.966;          /**< Default upgraded shaper parameter tauS. */
      static constexpr double c_tauS1 = 6.975;         /**< Default upgraded shaper parameter tauS1. */
      static constexpr double c_alpha = 0.603;         /**< Default upgraded shaper parameter alpha. */
      static constexpr double c_timeShift = 0.378;     /**< Default upgraded shaper time-shift parameter. */
      static constexpr double c_eta = 0.720;           /**< Default upgraded shaper parameter eta. */
      static constexpr double c_amplitudeNorm = 0.70235; /**< Default upgraded shaper amplitude normalization. */
      static constexpr double c_timeOffset = 0.0;      /**< Default upgraded shaper time offset. */

      /** Return the default waveform parameters in shaper internal units. */
      static std::array<double, c_nWaveformParameters>
      getDefaultWaveformParameters();

      /** See s_rf. This will try to use the value from the database */
      static double getRF();

      /** a struct for a signal sample */
      struct SignalSample {
        /** @param waveformParameters Float array of waveform parameters
         *  @param unitScale          Normalization of template waveform
         */
        void initSample(const float* waveformParameters, double unitScale = -1);
        /** @param waveformParameters Double array of waveform parameters
         *  @param unitScale          Normalization of template waveform
         */
        void initSample(const double* waveformParameters, double unitScale = -1);
        /**
         * @param[in]  amplitude  Signal amplitude
         * @param[in]  timeOffset Signal offset
         * @param[out] signal     Output array with added signal
         *
         * @return Energy deposition in ADC units
         */
        double accumulate(double amplitude, double timeOffset, double* signal) const;

        double m_sumScale; /**< energy deposit in fitting window scale factor */
        double m_signalShape[c_nSignalAdcClocks * c_nSamplesPerAdcClock]; /**< Simulated signal shape */
      };

      /** a struct for the ADC count */
      struct AdcCounts {
        /** add hit method */
        void addHit(double amplitude, double timeOffset,
                    const SignalSample& signalSample);
        double m_total; /**< total deposition (sum of m_s array) */
        double m_adcCounts[c_nAdcSamples]; /**< flash ADC measurements */
        double m_totalHadronDep; /**< total true hadron energy deposition*/
        double m_totalDep; /**< total true energy deposition*/
        double m_energyConversion; /**< energy conversion factor*/
        double m_flightTime; /**< simulated time weighted by true deposited energy*/
        double m_timeShift; /**< simulated time shift*/
        double m_timeToSensor; /**< simulated time to sensor*/
      };

      static constexpr int c_nFitSamples = c_nAdcSamples; /**< number of samples in the fitting window */

      /** a struct for the fit parameters */
      struct FitParams {
        typedef int IntArrayGridXAdc[c_nFitTimeShifts * c_nFitPointsPerAdcTick][c_nFitSamples]; /**< Array for DSP coefs in normal fit */
        typedef int IntArraySubXAdc[c_nLowAmplitudeFitTimeShifts][c_nFitSamples]; /**< Array for DSP coefs in fast fit */
        IntArrayGridXAdc f; /**< Array with tabulated signal waveform */
        IntArrayGridXAdc f1; /**< Array with tabulated derivative of signal waveform */
        IntArrayGridXAdc fg31; /**< Array to estimate signal amplitude */
        IntArrayGridXAdc fg32; /**< Array to estimate amp * time */
        IntArrayGridXAdc fg33; /**< Array to estimate pedestal height */
        IntArraySubXAdc fg41; /**< Alternative for FG31 for low amplitude fits */
        IntArraySubXAdc fg43; /**< Alternative for FG33 for low amplitude fits */
      };

      /** a struct for the parameters of the algorithm */
      struct AlgoParams {
        union {
          short int     id[16]; /**< */
          unsigned char ic[32]; /**< */
        };
      };

    private:
      EclConfigurationUp() = default; /**< constructor */

      bool m_background{false}; /**< background configuration */

      static double s_rf; /**< RF clock, www-linac.kek.jp/linac-com/report/skb-tdr/, ch. 6 */
    };
  }
}
