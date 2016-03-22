/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen, Guglielmo De Nardo                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITIZERMODULE_H_
#define ECLDIGITIZERMODULE_H_

#include <framework/core/Module.h>
#include <ecl/dataobjects/ECLWaveformData.h>
#include <vector>
#include <boost/multi_array.hpp>

namespace Belle2 {
  namespace ECL {
    /** The ECLDigitizer module.
     *
     * This module is responsible to digitize all hits found in the ECL from ECLHit
     * First, we simualte the sampling array by waveform and amplitude of hit, and
     * smear this sampling array by corresponding error matrix.
     * We then fit the array as hardware of shaper DSP board to obtain the fit result
     * of amplitude, time and quality.
     * The initial parameters of fit and algorithm are same as hardware.
     * This module outputs two array which are ECLDsp and ECLDigit.

       \correlationdiagram
       ECLHit = graph.data('ECLHit')
       ECLDigit   = graph.data('ECLDigit')
       ECLDsp = graph.data('ECLDsp')

       graph.module('ECLDigitizer', [ECLHit], [ECLDigit,ECLDsp])
       graph.relation(ECLDigitizer, ECLHit)
       graph.relation(ECLDigitizer, ECLDigit)
       graph.relation(ECLDigitizer, ECLDsp)
       \endcorrelationdiagram

     */
    class ECLDigitizerModule : public Module {
    public:

      /** Constructor.
       */
      ECLDigitizerModule();

      /** Destructor.
       */
      ~ECLDigitizerModule();


      /** Initialize variables  */
      virtual void initialize();

      /** Nothing so far.*/
      virtual void beginRun();

      /** Actual digitization of all hits in the ECL.
       *
       *  The digitized hits are written into the DataStore.
       */
      virtual void event();

      /** Nothing so far. */
      virtual void endRun();

      /** Free memory */
      virtual void terminate();

    private:
      static constexpr int        m_nch = 8736; // total number of electronic channels (crystals) in calorimeter
      static constexpr double m_rf = 508.887; // accelerating RF, http://ptep.oxfordjournals.org/content/2013/3/03A006.full.pdf
      static constexpr double    m_tick = 24.*12. / 508.; // == 72/127 digitization clock tick (in microseconds ???)
      static constexpr double    m_step =
        0.5; // time between points in internal units t_{asrto}*m_rf/2./24./12.
      static constexpr double    s_clock = 24.*12.; // digitization clock in RF units
      static constexpr int       m_ntrg = 144; // number of trigger counts per ADC clock tick

      static constexpr int       m_nsmp = 31; // number of ADC measurements for signal fitting
      static constexpr double    m_tmin = -15; // lower range of the signal fitting region in ADC clocks

      static constexpr int         m_nl = 48; // length of samples signal in number of ADC clocks
      static constexpr int         m_ns = 32; // number of samples per ADC clock

      static constexpr int        m_ndt = 96; // number of points per ADC tick where signal fit procedure parameters are evaluated

      struct signalsample_t; // forward declaration

      struct adccounts_t {
        double total; // total deposition (sum of m_s array)
        double c[m_nsmp]; // flash ADC measurements
        void AddHit(const double a, const double t0, const signalsample_t& q);
      };

      struct signalsample_t {
        double m_sumscale; // energy deposit in fitting window scale factor
        double m_ft[m_nl * m_ns];

        void InitSample(const float*);
        void InitSample(const double*);
        double Accumulate(const double, const double, double*) const;
      };

      typedef int int_array_192x16_t[2 * m_ndt][16];
      typedef int int_array_24x16_t[m_ndt / 4][16];
      typedef short int shortint_array_16_t[16];
      typedef unsigned char uchar_array_32_t[32];
      typedef std::pair<unsigned int, unsigned int> uint_pair_t;

      struct fitparams_t {
        int_array_192x16_t f, f1, fg31, fg32, fg33;
        int_array_24x16_t fg41, fg43;
      };

      struct algoparams_t {
        union {
          shortint_array_16_t id;
          uchar_array_32_t    ic;
        };
      };

      struct crystallinks_t { // offsets for storages of ECL channels
        short unsigned int idn;
        short unsigned int inoise;
        short unsigned int ifunc;
        short unsigned int iss;
      };

      std::vector<crystallinks_t> m_tbl;

      // Fit algorihtm parameters shared by group of crystals
      std::vector<algoparams_t> m_idn;
      std::vector<fitparams_t> m_fitparams;
      std::vector<ECLNoiseData> m_noise;
      std::vector<signalsample_t> m_ss;

      // Storage for adc hits from entire calorimeter (8736 crystals)
      std::vector<adccounts_t> m_adc;

      void shapeFitterWrapper(const int j, const int* FitA, const int m_ttrig,
                              int& m_lar, int& m_ltr, int& m_lq, int& m_chi) const ;

      /** read Shaper-DSP data from root file */
      void readDSPDB();

      void repack(const ECLWFAlgoParams&, algoparams_t&);
      void getfitparams(const ECLWaveformData&, const ECLWFAlgoParams&, fitparams_t&);

      /** Event number */
      int    m_nEvent;

      /** Module parameters */
      bool m_background;
      bool m_calibration;
    };
  }//ECL
}//Belle2

#endif /* ECLDIGITIZERMODULE_H_ */
