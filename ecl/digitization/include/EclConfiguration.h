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
    /** singleton class to hold the ECL configuration */
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
      static constexpr double    m_tick = 24.*12. / m_rf; /**< == 72/127 digitization clock tick (in microseconds ???) */
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
        void InitSample(const float*, double);
        void InitSample(const double*, double);
        double Accumulate(const double, const double, double*) const;

        double m_sumscale; /**< energy deposit in fitting window scale factor */
        double m_ft[m_nl * m_ns];
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
        typedef int int_array_192x16_t[2 * m_ndt][16];
        typedef int int_array_24x16_t[m_ndt / 4][16];
        int_array_192x16_t f, f1, fg31, fg32, fg33;
        int_array_24x16_t fg41, fg43;
      };

      /** a struct for the parameters of the algorithm */
      struct algoparams_t {
        typedef short int shortint_array_16_t[16];
        typedef unsigned char uchar_array_32_t[32];
        union {
          shortint_array_16_t id;
          uchar_array_32_t    ic;
        };
      };

    };
  }
}
#endif
