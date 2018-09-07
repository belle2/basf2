/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <vector>

namespace Belle2 {

  /*
   * For detailed overview of ECLDSP file structure see Confluence,
   * https://confluence.desy.de/display/BI/ECL+Technical+Notes
   * Attached file ECL-TN-2013-02 (digi.pdf)
   *
   * Note: Some things in that PDF are outdated, please verify with
   * Alexander Kuzmin or Vladimir Zhulanov
   */
  class ECLDspData: public TObject {
  private:
    /** ShaperDSP board number, 0..11 */
    int m_boardNumber;
    /** Number of bits for FG31, FG41 */
    short int m_ka;
    /** Number of bits for FG32 */
    short int m_kb;
    /** Number of bits for FG33, FG43 */
    short int m_kc;
    /** start point for pedestal calculation */
    short int m_y0Startr;
    /**
     * chi2 threshold for quality bit
     *
     * We use condition chi^2 < (Amp^2 / (2^{k2Chi}) + chiThresh)
     */
    short int m_chiThresh;
    /** multipliers power of 2 for f, f1 */
    short int m_k1Chi;
    /** multipliers power of 2 for chi2 calculation */
    short int m_k2Chi;
    /** See https://confluence.desy.de/display/BI/Electronics+Thresholds */
    short int m_hitThresh;
    /** See https://confluence.desy.de/display/BI/Electronics+Thresholds */
    short int m_lowAmpThresh;
    /** See https://confluence.desy.de/display/BI/Electronics+Thresholds */
    short int m_skipThresh;
    /** See documentation for method 'getF31' */
    std::vector<short int> m_fg31;
    /** See documentation for method 'getF32' */
    std::vector<short int> m_fg32;
    /** See documentation for method 'getF33' */
    std::vector<short int> m_fg33;
    /** See documentation for method 'getF41' */
    std::vector<short int> m_fg41;
    /** See documentation for method 'getF43' */
    std::vector<short int> m_fg43;
    /** See documentation for method 'getF' */
    std::vector<short int> m_f;
    /** See documentation for method 'getF1' */
    std::vector<short int> m_f1;

  public:
    /**
     * @brief Initialize DSP coefficients class from binary file.
     * @param board_number Id of specific shaperDSP, 0..11
     * @param filename Name of binary file with coefficients (Usually "dsp*.dat")
     */
    ECLDspData(int board_number, const char* filename);
    /** */
    ~ECLDspData();

    void write(const char* filename);

    /*************/
    /** GETTERS **/
    /*************/

    /**
     * Array with tabulated signal waveform.
     */
    std::vector<short int>& getF() { return m_f; }
    /**
     * Array with tabulated derivative of signal waveform
     */
    std::vector<short int>& getF1() { return m_f1; }
    /**
     * Array FG31, used to estimate signal amplitude.
     * Calculated from F and covariance matrix.
     */
    std::vector<short int>& getF31() { return m_fg31; }
    /**
     * Array FG32, used to estimate A * delta_t.
     * (A -- amplitude, delta_t -- time shift for linearization)
     *
     * Calculated from F and covariance matrix.
     */
    std::vector<short int>& getF32() { return m_fg32; }
    /**
     * Array FG33, used to estimate pedestal height in signal.
     *
     * Calculated from F and covariance matrix.
     */
    std::vector<short int>& getF33() { return m_fg33; }
    /**
     * Alternative for FG31 for signals with small amplitude.
     */
    std::vector<short int>& getF41() { return m_fg41; }
    /**
     * Alternative for FG33 for signals with small amplitude.
     */
    std::vector<short int>& getF43() { return m_fg43; }
    /**
     * @return Low amp threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    short int getlAT() const { return m_lowAmpThresh; }
    /**
     * @return Skip threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    short int getsT() const { return m_skipThresh; }
    /**
     * @return Hit threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    short int getthT() const { return m_hitThresh; }
    /** chi2 threshold for fit quality flag */
    short int getchiThresh() const { return m_chiThresh; }
    /** multipliers power of 2 for f, f1 */
    short int getk1() const { return m_k1Chi; }
    /** multipliers power of 2 for chi2 calculation */
    short int getk2() const { return m_k2Chi; }
    /** Number of bits for FG31, FG41 */
    short int getka() const { return m_ka; }
    /** Number of bits for FG32 */
    short int getkb() const { return m_kb; }
    /** Number of bits for FG33, FG43 */
    short int getkc() const { return m_kc; }
    /** start point for pedestal calculation */
    short int gety0Startr() const { return m_y0Startr; }

    /** Return ShaperDSP board number, 0..11 */
    int getBoardNumber() const { return m_boardNumber; }

    /*************/
    /** SETTERS **/
    /*************/

    /**
     * Set Low amp threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    void setlAT(short int val) { m_lowAmpThresh = val; }
    /**
     * Set Skip threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    void setsT(short int val) { m_skipThresh = val; }
    /**
     * Set Hit threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    void setthT(short int val) { m_hitThresh = val; }
    /** Set chi2 threshold for fit quality flag */
    void setchiThresh(short int val) { m_chiThresh = val; }
    /** Set multipliers power of 2 for f, f1 */
    void setk1(short int val) { m_k1Chi = val; }
    /** Set multipliers power of 2 for chi2 calculation */
    void setk2(short int val) { m_k2Chi = val; }
    /** Set number of bits for FG31, FG41 */
    void setka(short int val) { m_ka = val; }
    /** Set number of bits for FG32 */
    void setkb(short int val) { m_kb = val; }
    /** Set number of bits for FG33, FG43 */
    void setkc(short int val) { m_kc = val; }
    /** Set start point for pedestal calculation */
    void sety0Startr(short int val) { m_y0Startr = val; }

    ClassDef(ECLDspData, 1);
  };
}

