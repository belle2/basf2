/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ECLCONFIGURATION_H_
#define ECLCONFIGURATION_H_

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

      static constexpr int        m_nch = 8736;  /**< total number of electronic channels (crystals) in calorimeter */
      static constexpr double    m_rf = 508.887; /**< accelerating RF, http://ptep.oxfordjournals.org/content/2013/3/03A006.full.pdf */
      static constexpr double    m_tick = 24.*12. / m_rf; /**< == 72/127 digitization clock tick (in microseconds) */
      static constexpr double    m_step = 0.5; /**< time between points in internal units t_{asrto}*m_rf/2./24./12. */
      static constexpr double    s_clock = 24.*12.; /**< digitization clock in RF units */
      static constexpr int       m_ntrg = 144; /**< number of trigger counts per ADC clock tick */

      static constexpr int       m_nsmp = 31; /**< number of ADC measurements for signal fitting */
      static constexpr double    m_tmin = -15; /**< lower range of the signal fitting region in ADC clocks */

      static constexpr int         m_nl = 48; /**< length of samples signal in number of ADC clocks */
      static constexpr int         m_ns = 32; /**< number of samples per ADC clock */

      static constexpr int        m_ndt = 96; /**< number of points per ADC tick where signal fit procedure parameters are evaluated */

    private:
      EclConfiguration() {}; /**< constructor */
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

      /** a struct for the fit parameters */
      struct fitparams_t {
        typedef int int_array_192x16_t[2 * m_ndt][16]; /**< Array for DSP coefs in normal fit */
        typedef int int_array_24x16_t[m_ndt / 4][16]; /**< Array for DSP coefs in fast fit */
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

    };
  }
}
#endif
