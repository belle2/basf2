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

  /**
   * This object contains ECL DSP coefs -- electromagnetic calorimeter
   * digital signal processing coefficients.
   * Payloads with these objects are used in two use cases.
   *
   * 1. They are uploaded to ShaperDSP boards and then used in waveform fitting
   *    (amplitude and time reconstruction).
   * 2. ECL DQM uses these payloads with ShaperDSP emulator to quickly detect
   *    ShaperDSP boards that started sending corrupted data.
   *    (ECL expert then resets these modules)
   *
   * For detailed overview of ECLDSP file structure see Confluence,
   * https://confluence.desy.de/display/BI/ECL+Technical+Notes
   * Attached file ECL-TN-2013-02 (digi.pdf)
   */
  class ECLDspData: public TObject {
  private:
    /** ShaperDSP board number, 0..11 */
    unsigned char m_boardNumber;
    /** Major version of DSP coefficients */
    unsigned char m_verMaj;
    /** Minor version of DSP coefficients */
    unsigned char m_verMin;
    /** Number of bits for FG31, FG41 */
    unsigned char m_ka;
    /** Number of bits for FG32 */
    unsigned char m_kb;
    /** Number of bits for FG33, FG43 */
    unsigned char m_kc;
    /** start point for pedestal calculation */
    unsigned char m_y0Startr;
    /**
     * chi2 threshold for quality bit
     *
     * We use condition chi^2 < (Amp^2 / (2^{k2Chi}) + chiThresh)
     */
    short int m_chiThresh;
    /** multipliers power of 2 for f, f1 */
    unsigned char m_k1Chi;
    /** multipliers power of 2 for chi2 calculation */
    unsigned char m_k2Chi;
    /** See https://confluence.desy.de/display/BI/Electronics+Thresholds */
    short int m_hitThresh;
    /** See https://confluence.desy.de/display/BI/Electronics+Thresholds */
    short int m_lowAmpThresh;
    /** See https://confluence.desy.de/display/BI/Electronics+Thresholds */
    short int m_skipThresh;
    /** See https://confluence.desy.de/display/BI/Electronics+Thresholds */
    short int m_adcAlwThresh;
    /** See documentation for method 'getF31' */
    std::vector<short int> m_fg31 = {};
    /** See documentation for method 'getF32' */
    std::vector<short int> m_fg32 = {};
    /** See documentation for method 'getF33' */
    std::vector<short int> m_fg33 = {};
    /** See documentation for method 'getF41' */
    std::vector<short int> m_fg41 = {};
    /** See documentation for method 'getF43' */
    std::vector<short int> m_fg43 = {};
    /** See documentation for method 'getF' */
    std::vector<short int> m_f = {};
    /** See documentation for method 'getF1' */
    std::vector<short int> m_f1 = {};
    /**
     * This vector contains all parameters that didn't exist in the initial
     * version of ECL DSP file format.
     *
     * This vector allows loose coupling between ECLDspData class and
     * ECL DSP file format.
     *
     * m_extraData[0] -- FORMAT_VERSION
     * m_extraData[1] -- PACKER_VERSION
     */
    std::vector<short int> m_extraData;

    /**
     * Version of packer used in packCoefVector by default
     * Versioon of unpacker is dynamically determined from the data.
     */
    static const short int c_packerVersion = 2;

  public:
    /**
     * @brief Initialize DSP coefficients class from binary file.
     * @param board_number Id of specific shaperDSP, 0..11
     */
    explicit ECLDspData(unsigned char board_number = 0) :
      m_boardNumber(board_number),
      m_verMaj(0),
      m_verMin(0),
      m_ka(0),
      m_kb(0),
      m_kc(0),
      m_y0Startr(0),
      m_chiThresh(0),
      m_k1Chi(0),
      m_k2Chi(0),
      m_hitThresh(0),
      m_lowAmpThresh(0),
      m_skipThresh(0),
      m_adcAlwThresh(0)
    {}
    /** */
    ~ECLDspData() {};

    /*************/
    /** GETTERS **/
    /*************/

    /** Return version of packer algorithm used in [un]packCoefVector */
    static constexpr short int getPackerVersion() { return c_packerVersion; }

    /**
     * @return Vector with all parameters that haven't been
     * included in basic ECL DSP file format
     */
    std::vector<short int> getExtraData() const { return m_extraData; }

    /**
     * Array with tabulated signal waveform.
     */
    void getF(std::vector<short int>& dst) const { unpackCoefVector(m_f, dst); }
    /**
     * Array with tabulated derivative of signal waveform
     */
    void getF1(std::vector<short int>& dst) const { unpackCoefVector(m_f1, dst); }
    /**
     * Array FG31, used to estimate signal amplitude.
     * Calculated from F and covariance matrix.
     */
    void getF31(std::vector<short int>& dst) const { unpackCoefVector(m_fg31, dst); }
    /**
     * Array FG32, used to estimate A * delta_t.
     * (A -- amplitude, delta_t -- time shift for linearization)
     *
     * Calculated from F and covariance matrix.
     */
    void getF32(std::vector<short int>& dst) const { unpackCoefVector(m_fg32, dst); }
    /**
     * Array FG33, used to estimate pedestal height in signal.
     *
     * Calculated from F and covariance matrix.
     */
    void getF33(std::vector<short int>& dst) const { unpackCoefVector(m_fg33, dst); }
    /**
     * Alternative for FG31 for signals with small amplitude.
     */
    void getF41(std::vector<short int>& dst) const { unpackCoefVector(m_fg41, dst); }
    /**
     * Alternative for FG33 for signals with small amplitude.
     */
    void getF43(std::vector<short int>& dst) const { unpackCoefVector(m_fg43, dst); }
    /** @return Major version of DSP coefficients */
    unsigned char getverMaj() const { return m_verMaj; }
    /** @return Minor version of DSP coefficients */
    unsigned char getverMin() const { return m_verMin; }
    /**
     * @return ADC always threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    short int getaAT() const { return m_adcAlwThresh; }
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
    short int gethT() const { return m_hitThresh; }
    /** chi2 threshold for fit quality flag */
    short int getchiThresh() const { return m_chiThresh; }
    /** multipliers power of 2 for f, f1 */
    unsigned char getk1() const { return m_k1Chi; }
    /** multipliers power of 2 for chi2 calculation */
    unsigned char getk2() const { return m_k2Chi; }
    /** Number of bits for FG31, FG41 */
    unsigned char getka() const { return m_ka; }
    /** Number of bits for FG32 */
    unsigned char getkb() const { return m_kb; }
    /** Number of bits for FG33, FG43 */
    unsigned char getkc() const { return m_kc; }
    /** start point for pedestal calculation */
    unsigned char gety0Startr() const { return m_y0Startr; }

    /** Return ShaperDSP board number, 0..11 */
    unsigned char getBoardNumber() const { return m_boardNumber; }

    /*************/
    /** SETTERS **/
    /*************/

    /**
     * Sets vector with all parameters that haven't been
     * included in basic ECL DSP file format
     */
    void setExtraData(const std::vector<short int>& newval) { m_extraData = newval; }

    /**
     * Array with tabulated signal waveform.
     */
    void setF(const std::vector<short int>& src) { packCoefVector(src, m_f); }
    /**
     * Array with tabulated derivative of signal waveform
     */
    void setF1(const std::vector<short int>& src) { packCoefVector(src, m_f1); }
    /**
     * Array FG31, used to estimate signal amplitude.
     * Calculated from F and covariance matrix.
     */
    void setF31(const std::vector<short int>& src) { packCoefVector(src, m_fg31); }
    /**
     * Array FG32, used to estimate A * delta_t.
     * (A -- amplitude, delta_t -- time shift for linearization)
     *
     * Calculated from F and covariance matrix.
     */
    void setF32(const std::vector<short int>& src) { packCoefVector(src, m_fg32); }
    /**
     * Array FG33, used to estimate pedestal height in signal.
     *
     * Calculated from F and covariance matrix.
     */
    void setF33(const std::vector<short int>& src) { packCoefVector(src, m_fg33); }
    /**
     * Alternative for FG31 for signals with small amplitude.
     */
    void setF41(const std::vector<short int>& src) { packCoefVector(src, m_fg41); }
    /**
     * Alternative for FG33 for signals with small amplitude.
     */
    void setF43(const std::vector<short int>& src) { packCoefVector(src, m_fg43); }

    /** Sets major version of DSP coefficients */
    void setverMaj(unsigned char val) { m_verMaj = val; }
    /** Sets minor version of DSP coefficients */
    void setverMin(unsigned char val) { m_verMin = val; }
    /**
     * Set Low amp threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    void setaAT(short int val) { m_adcAlwThresh = val; }
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
    void sethT(short int val) { m_hitThresh = val; }
    /** Set chi2 threshold for fit quality flag */
    void setchiThresh(short int val) { m_chiThresh = val; }
    /** Set multipliers power of 2 for f, f1 */
    void setk1(unsigned char val) { m_k1Chi = val; }
    /** Set multipliers power of 2 for chi2 calculation */
    void setk2(unsigned char val) { m_k2Chi = val; }
    /** Set number of bits for FG31, FG41 */
    void setka(unsigned char val) { m_ka = val; }
    /** Set number of bits for FG32 */
    void setkb(unsigned char val) { m_kb = val; }
    /** Set number of bits for FG33, FG43 */
    void setkc(unsigned char val) { m_kc = val; }
    /** Set start point for pedestal calculation */
    void sety0Startr(unsigned char val) { m_y0Startr = val; }

  private:
    /**
     * @brief Convert vector of DSP coefficients (src) to ECLDspData
     *        internal format (dst).
     * The internal format of coefficient vectors stored in this object is
     * different from their real values: this improves data compression by ~200%.
     * Thus, any accessor methods to m_f* vectors utilize packCoefVector and
     * unpackCoefVector.
     */
    void packCoefVector(const std::vector<short int>& src, std::vector<short int>& dst);
    /**
     * @brief Convert vector of DSP coefficients (src) to ECLDspData
     *        internal format (dst).
     * See documentation for packCoefVector on why this was necessary.
     */
    void unpackCoefVector(const std::vector<short int>& src, std::vector<short int>& dst) const;

    ClassDef(ECLDspData, 1); /**< ClassDef */
  };
}

