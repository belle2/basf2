/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
namespace Belle2 {
  namespace ECL {
    /**
     * @brief Function that emulates shape fitting algorithm used in ShaperDSP.
     *        f, f1, fg* are coefficients from ECLDspData
     *
     * @param[in] f[16][192]    Array with tabulated signal waveform.
     * @param[in] f1[16][192]   Array with tabulated derivative of signal waveform
     * @param[in] fg41[16][192] Alternative for FG31 for signals with small amplitude
     * @param[in] fg43[16][192] Alternative for FG33 for signals with small amplitude
     * @param[in] fg31[16][192] Array used to estimate signal amplitude.
     * @param[in] fg32[16][192] Array used to estimate Amplitude * delta_t.
     * @param[in] fg33[16][192] Array used to estimate pedestal height in signal.
     *
     * @param[in] y[31]  Array of signal measurements
     * @param[in] ttrig2 Trigger time (0-23)
     *
     * @param[in] A0    Low amplitude threshold
     * @param[in] Ahard Hit threshold
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
     *
     * @param[out] m_AmpFit     Amplitude from the fit
     * @param[out] m_TimeFit    Time from the fit
     * @param[out] m_QualityFit Quality from the fit
     */
    void lftda_(short int* f, short int* f1, short int* fg41, short int* fg43, short int* fg31, short int* fg32, short int* fg33,
                int* y, int& ttrig2, int& A0, int& Ahard, int& k_a, int& k_b, int& k_c, int& k_16, int& k1_chi, int& k2_chi, int& chi_thres,
                int& m_AmpFit, int& m_TimeFit, int& m_QualityFit);
  }
}
