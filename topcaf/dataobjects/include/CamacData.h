/**********************************************************************************
 * Data class for the Event Packet: Header information as defined in:             *
 * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format *
 * Author: Malachi Schram (malachi.schram@pnnl.gov)                               *
 **********************************************************************************/

#ifndef CamacData_H
#define CamacData_H

#include <TObject.h>
#include <vector>
#include <map>

namespace Belle2 {

///const double sampling_rate=0.04508;//[ns] -- Determine using decay FTSW data (jun2013leps/20130606/datatopcrt-t0cal-e000001r000194-f000.cmc)

//! all these values are in ns from Itoh-san's TDC testing as described in iTOP CRT ELOG entry 551
  const double slot2_tdc_timing[8] = { 0.025963, 0.025756, 0.025663, 0.025668, 0.025512, 0.0, 0.0, 0.0 };
//!
  const double slot7_tdc_timing[8] = { 0.0, 0.045056, 0.045088, 0.045347, 0.045728, 0.045183, 0.0, 0.0 };

  class CamacData : public TObject {

  public:
    //! Empty constructor
    CamacData();
    virtual ~CamacData();

    ///--- Getters ---///
    unsigned int GetEventNumber() const {return m_evt_num;}
    //! Get Raw TDC
    unsigned short GetRawTDC() const {return m_raw_tdc;}
    //! Get Raw RF0
    unsigned short GetRawRF0() const {return m_raw_rf0;}
    //! Get Raw RF1
    unsigned short GetRawRF1() const {return m_raw_rf1;}
    //! Get Raw RF2
    unsigned short GetRawRF2() const {return m_raw_rf2;}
    //! Get Raw RF3
    unsigned short GetRawRF3() const {return m_raw_rf3;}

    ///--- Getters ---///
    double GetTDC() const {return float(m_raw_tdc) * slot7_tdc_timing[1];}
    //! Get RF0
    double GetRF0() const {return float(m_raw_rf0) * slot7_tdc_timing[2];}
    //! Get RF1
    double GetRF1() const {return float(m_raw_rf1) * slot7_tdc_timing[3];}
    //! Get RF2
    double GetRF2() const {return float(m_raw_rf2) * slot7_tdc_timing[4];}
    //! Get RF3
    double GetRF3() const {return float(m_raw_rf3) * slot7_tdc_timing[5];}

    ///--- Getters ---///
    unsigned short GetRawTrigS0_TDC()     const {return m_raw_trigS_tdc0;}
    //! Get Raw Trig M0 TDC
    unsigned short GetRawTrigM0_TDC()     const {return m_raw_trigM_tdc0;}
    //! Get Raw Trig S1 TDC
    unsigned short GetRawTrigS1_TDC()     const {return m_raw_trigS_tdc1;}
    //! Get Raw Trig M1 TDC
    unsigned short GetRawTrigM1_TDC()     const {return m_raw_trigM_tdc1;}
    //! Get Raw Trig timing
    unsigned short GetRawTrigTiming_TDC() const {return m_raw_timing_tdc;}
    //! Get Raw rate
    unsigned short GetRawRateMon()        const {return m_raw_ratemon;}

    ///--- Getters ---///
    double GetTrigS0_TDC()     const {return float(m_raw_trigS_tdc0) * slot2_tdc_timing[0];}
    //! Get Trig TDC M0
    double GetTrigM0_TDC()     const {return float(m_raw_trigM_tdc0) * slot2_tdc_timing[1];}
    //! Get Trig TDC S1
    double GetTrigS1_TDC()     const {return float(m_raw_trigS_tdc1) * slot2_tdc_timing[2];}
    //! Get Trig TDC M1
    double GetTrigM1_TDC()     const {return float(m_raw_trigM_tdc1) * slot2_tdc_timing[3];}
    //! Get Trig timing
    double GetTrigTiming_TDC() const {return float(m_raw_timing_tdc) * slot2_tdc_timing[4];}

    ///--- Getters ---///
    unsigned short GetRawTrigS_ADC0()     const {return m_raw_trigS_adc0;}
    //! Get Raw Trig ADC0
    unsigned short GetRawTrigM_ADC0()     const {return m_raw_trigM_adc0;}
    //! Get Raw Trig ADC1
    unsigned short GetRawTrigS_ADC1()     const {return m_raw_trigS_adc1;}
    unsigned short GetRawTrigM_ADC1()     const {return m_raw_trigM_adc1;}
    //! Trigger timing
    unsigned short GetRawTrigTiming_ADC() const {return m_raw_timing_adc;}
    //! Raw Veto
    unsigned short GetRawVeto0_ADC()      const {return m_raw_veto0;}
    unsigned short GetRawVeto1_ADC()      const {return m_raw_veto1;}

    unsigned short GetRawLEPSMarkerWord() const {return m_raw_leps_marker_word;}

    ///--- Setters ---///
    void SetEventNumber(const unsigned int evt) {m_evt_num = evt;}
    void SetRawTDC(const unsigned short tdc)    {m_raw_tdc = tdc;}
    void SetRawRF0TDC(const unsigned short tdc) {m_raw_rf0 = tdc;}
    void SetRawRF1TDC(const unsigned short tdc) {m_raw_rf1 = tdc;}
    void SetRawRF2TDC(const unsigned short tdc) {m_raw_rf2 = tdc;}
    void SetRawRF3TDC(const unsigned short tdc) {m_raw_rf3 = tdc;}

    ///--- Setters ---///
    void SetRawTrigS_TDC0(const unsigned short x) {m_raw_trigS_tdc0 = x;}
    void SetRawTrigM_TDC0(const unsigned short x) {m_raw_trigM_tdc0 = x;}
    void SetRawTrigS_TDC1(const unsigned short x) {m_raw_trigS_tdc1 = x;}
    void SetRawTrigM_TDC1(const unsigned short x) {m_raw_trigM_tdc1 = x;}
    void SetRawTiming_TDC(const unsigned short x) {m_raw_timing_tdc = x;}
    void SetRawRateMon(const unsigned short x)    {m_raw_ratemon = x;}

    ///--- Setters ---///
    void SetRawTrigS_ADC0(const unsigned short x) {m_raw_trigS_adc0 = x;}
    void SetRawTrigM_ADC0(const unsigned short x) {m_raw_trigM_adc0 = x;}
    void SetRawTrigS_ADC1(const unsigned short x) {m_raw_trigS_adc1 = x;}
    void SetRawTrigM_ADC1(const unsigned short x) {m_raw_trigM_adc1 = x;}
    void SetRawTiming_ADC(const unsigned short x) {m_raw_timing_adc = x;}
    void SetRawVeto0(const unsigned short x)      {m_raw_veto0 = x;}
    void SetRawVeto1(const unsigned short x)      {m_raw_veto1 = x;}

    //!
    void SetRawLEPSMarkerWord(const unsigned short x) {m_raw_leps_marker_word = x;}

  private:
    unsigned int m_evt_num;

    /// timing data
    unsigned short m_raw_tdc, m_raw_rf0, m_raw_rf1, m_raw_rf2, m_raw_rf3;

    /// trigger paddle data
    unsigned short m_raw_trigS_tdc0, m_raw_trigM_tdc0, m_raw_trigS_tdc1, m_raw_trigM_tdc1;
    unsigned short m_raw_timing_tdc, m_raw_ratemon;
    unsigned short m_raw_trigS_adc0, m_raw_trigM_adc0, m_raw_trigS_adc1, m_raw_trigM_adc1;
    unsigned short m_raw_timing_adc, m_raw_veto0, m_raw_veto1;

    // LEPS marker word
    unsigned short m_raw_leps_marker_word;

    ClassDef(CamacData, 1);
  };


}
#endif
