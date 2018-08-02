#pragma once
#include "TObject.h"
#include <vector>

namespace Belle2 {

  /*
   * For detailed overview of ECLDSP file structure see Confluence,
   * https://confluence.desy.de/display/BI/ECL+Technical+Notes
   * Attached file ECL-TN-2013-02 (digi.pdf)
   *
   * Note: Some things in the PDF are outdated, please verify with
   * Alexander Kuzmin or Vladimir Zhulanov
   */
  class ECLDspData: public TObject {
  private:
    /** ShaperDSP board number, 0..11 */
    int m_boardNumber;
    /** */
    short int m_ka;
    /** */
    short int m_kb;
    /** */
    short int m_kc;
    /** */
    short int m_y0Startr;
    /** */
    short int m_chiThresh;
    /** */
    short int m_k1Chi;
    /** */
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
    /**
     * Array with tabulated signal waveform.
     *
     */
    std::vector<short int> getF() const;
    /**
     * Array with tabulated derivative of signal waveform
     */
    std::vector<short int> getF1() const;
    /**
     * Array FG31, used to estimate signal amplitude.
     * Calculated from F and covariance matrix.
     */
    std::vector<short int> getF31() const;
    /**
     * Array FG32, used to estimate A * delta_t.
     * (A -- amplitude, delta_t -- time shift for linearization)
     *
     * Calculated from F and covariance matrix.
     */
    std::vector<short int> getF32() const;
    /**
     * Array FG33, used to estimate pedestal height in signal.
     *
     * Calculated from F and covariance matrix.
     */
    std::vector<short int> getF33() const;
    /**
     * Alternative for FG31 for signals with small amplitude.
     */
    std::vector<short int> getF41() const;
    /**
     * Alternative for FG33 for signals with small amplitude.
     */
    std::vector<short int> getF43() const;
    /**
     * @return Low amp threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    short int getlAT() const;
    /**
     * @return Skip threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    short int getsT() const;
    /**
     * @return Hit threshold (https://confluence.desy.de/display/BI/Electronics+Thresholds)
     */
    short int getthT() const;
    /** */
    short int getk1() const;
    /** */
    short int getk2() const;
    /** */
    short int getka() const;
    /** */
    short int getkb() const;
    /** */
    short int getkc() const;
    /** */
    short int getky0s() const;
    /** Return ShaperDSP board number, 0..11 */
    int getBoardNumber() const;

    /**
     * Save DSP coefficients back to binary file.
     */
    void write(const char* filename);

    ClassDef(ECLDspData, 1);
  };
}

