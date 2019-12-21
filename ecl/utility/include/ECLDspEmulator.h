/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace ECL {
    typedef struct ECLShapeFit {
      /** Fit amplitude, -128..262015, ADC units (20 ADC units ~= 1 MeV) */
      int amp;
      /** Fit time, -2048..2047 (with reference to trigger time), ADC ticks (1 tick ~= 0.5 ns) */
      int time;
      /** Quality flag, see enum QualityFlag in emulator implementation */
      int quality;
      /** Pedestal */
      int pedestal;
      /** (Peak < HIT_THR) => true */
      bool hit_thr;
      /** (Peak < SKIP_THR) => true */
      bool skip_thr;
      /** (Peak < LA_THR) => true */
      bool low_amp;
      /** Shape of the fitted function. */
      long long fit[31];
      /** Chi^2 from the fit */
      int chi2;
    } ECLShapeFit;

    /**
     * @brief Function that emulates shape fitting algorithm used in ShaperDSP.
     *        f, f1, fg* are coefficients from ECLDspData
     * See ecl/examples/EclShapeFitter.py for usage example.
     *
     * @param[in] f[16][192]    Array with tabulated signal waveform.
     * @param[in] f1[16][192]   Array with tabulated derivative of signal waveform
     * @param[in] fg41[16][24]  Alternative for FG31 for signals with small amplitude
     * @param[in] fg43[16][24]  Alternative for FG33 for signals with small amplitude
     * @param[in] fg31[16][192] Array used to estimate signal amplitude.
     * @param[in] fg32[16][192] Array used to estimate Amplitude * delta_t.
     * @param[in] fg33[16][192] Array used to estimate pedestal height in signal.
     *
     * @param[in] y[31]  Array of signal measurements
     * @param[in] ttrig2 Trigger time (0-23)
     *
     * @param[in] la_thr    Low amplitude threshold
     * @param[in] hit_thr   Hit threshold
     * @param[in] skip_thr  Skip threshold
     *
     * @param[in] k_a Number of bits for FG31, FG41
     * @param[in] k_b Number of bits for FG32
     * @param[in] k_c Number of bits for FG33, FG43
     *
     * @param[in] k_16 Start point for pedestal calculation (aka y0Startr)
     *
     * @param[in] k1_chi    Bit shift for chi2 calculation
     * @param[in] k2_chi    Bit shift for chi2 threshold calculation
     * @param[in] chi_thres Base value for chi2 threshold
     */
    ECLShapeFit lftda_(short int* f, short int* f1, short int* fg41,
                       short int* fg43, short int* fg31, short int* fg32,
                       short int* fg33, int* y, int& ttrig2, int& la_thr,
                       int& hit_thr, int& skip_thr, int& k_a, int& k_b,
                       int& k_c, int& k_16, int& k1_chi, int& k2_chi,
                       int& chi_thres);
  }
}
