/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLDIGITIZERPURECSIMODULE_H_
#define ECLDIGITIZERPURECSIMODULE_H_

#include <framework/core/Module.h>
#include <vector>

class TH1F;

namespace Belle2 {
  namespace ECL {
    /** The ECLDigitizerPureCsI module.
     *
     * This module is responsible to digitize all hits found in the ECL from ECLHit
     * First, we simualte the sampling array by waveform and amplitude of hit, and
     * smear this sampling array by corresponding error matrix.
     * We then fit the array as hardware of shaper DSP board to obtain the fit result
     * of amplitude, time and quality.
     * The initial parameters of fit and algorithm are same as hardware.
     * This module outputs two array which are ECLDsp and ECLDigit.

     */
    class ECLDigitizerPureCsIModule : public Module {
    public:

      /** Constructor.
       */
      ECLDigitizerPureCsIModule();

      /** Destructor.
       */
      ~ECLDigitizerPureCsIModule();


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
      bool isPureCsI(int cellId)
      {
        if (cellId <= 8736) return true;
        return false;
      }

      static constexpr int        m_nch = 8736; // total number of electronic channels (crystals) in calorimeter
      static constexpr double    m_tick_Tl = 24.*12. / 508.; // == 72/127 digitization clock tick (in microseconds ???)
      static constexpr int       m_ntrg = 144; // number of trigger counts per ADC clock tick
      static constexpr double    m_tick = 8 * (m_tick_Tl / m_ntrg);   // o(16 ns)
      static constexpr int       m_nsmp = 31; // number of ADC measurements for signal fitting
      static constexpr double    m_tmin = -15; // lower range of the signal fitting region in ADC clocks

      static constexpr int         m_nl_Tl = 48; // length of samples signal in number of ADC clocks
      static constexpr int         m_nl = m_nl_Tl * 15; // length of samples signal in number of ADC clocks

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
        void InitSample(const TH1F*);
        double Accumulate(const double, const double, double*) const;
      };

      struct crystallinks_t { // offsets for storages of ECL channels
        short unsigned int idn;
        short unsigned int inoise;
        short unsigned int ifunc;
        short unsigned int iss;
      };

      std::vector<crystallinks_t> m_tbl;
      // std::vector<ECLNoiseData> m_noise;
      std::vector<signalsample_t> m_ss;

      // Storage for adc hits from entire calorimeter (8736 crystals)
      std::vector<adccounts_t> m_adc;

      void shapeFitterWrapperPureCsI(const int j, const int* FitA, const int m_ttrig,
                                     int& m_lar, int& m_ltr, int& m_lq) const;

      /** read Shaper-DSP data from root file */
      void readDSPDB();

      /** Event number */
      int    m_nEvent;

      /** Module parameters */
      bool m_background;
      bool m_calibration;
      static constexpr const char* eclDigitArrayName() { return "ECLDigitsPureCsI"; }
      static constexpr const char* eclDspArrayName() { return "ECLDspsPureCsI"; }
    };
  }//ECL
}//Belle2

#endif /* ECLDIGITIZERPURECSIMODULE_H_ */
