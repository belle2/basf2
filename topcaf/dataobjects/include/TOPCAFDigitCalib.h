#ifndef TOPCAFDigitCalib_H
#define TOPCAFDigitCalib_H

#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
// Data class for the Event Packet: Waveform information as defined in:           //
// http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format //
// Author: Malachi Schram (malachi.schram@pnnl.gov)                               //
////////////////////////////////////////////////////////////////////////////////////

namespace Belle2 {

  class TOPCAFDigitCalib: public Packet {

  public:
    TOPCAFDigitCalib();
    TOPCAFDigitCalib(const EventWaveformPacket* wp);

    ~TOPCAFDigitCalib();

    //--- Getters ---//
    topcaf_channel_id_t GetChannelID() const {return m_channel_id;}
    packet_word_t GetEventNumber() const {return m_evt_num;}
    unsigned short GetRefWindow() const {return  m_asic_refwin;}
    size_t GetNumWaveSegments() const {return  m_nwave_seg;}
    packet_word_t GetASICWindow()const {return m_asic_win;}
    unsigned short GetASIC() const {return m_asic;}
    unsigned short GetASICChannel() const {return m_asic_ch;}
    unsigned short GetASICRow() const {return m_asic_row;}
    unsigned short GetASICColumn() const {return m_asic_col;}
    size_t GetNumSamples() const {return m_nsamples;}
    double GetTDCBin() const {return m_tdc_bin;}
    double GetWidth() const {return m_width;}
    double GetADCHeight() const {return m_adc_height;}

    double GetRefTime() const {return m_ref_time;}
    int GetRefT1() const {return m_ref_T1;}
    int GetRefMaxBin() const {return m_ref_max_bin;}
    double GetRefA1() const {return m_ref_A1;}
    double GetRefA2() const {return m_ref_A2;}
    double GetRefAmplitude() const {return m_ref_amp;}


    int GetCrossT1() const {return m_cross_T1;}
    int GetMaxBin() const {return m_max_bin;}
    double GetCrossA1() const {return m_cross_A1;}
    double GetCrossA2() const {return m_cross_A2;}

    // Extra stuff not in the raw packet //
    double GetTime() const {return m_time;}
    int GetTimeBin() const {return m_tdc_bin;}
    double GetAmplitude() const {return m_amp;}
    double GetSamplingRate() const {return m_rate;}
    double GetQuality() const {return m_quality;}
    int GetFlag() const {return m_flag;}
    double GetCorrTime() const {return m_corr_time;}
    //--- Setters ---//
    void SetTime(double time) {m_time = time;}
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    void SetTDCBin(double tdc_bin) {m_tdc_bin = tdc_bin;}
    void SetWidth(int width) {m_width = width;}
    void SetAmplitude(double amp) {m_amp = amp;}
    void SetSamplingRate(double rate) {m_rate = rate;}
    void SetQuality(double quality) {m_quality = quality;}
    void SetHitValues(topcaf_hit_t& mydigit);
    void SetFlag(int flag) {m_flag = flag;}
    void SetCorrTime(double time) {m_corr_time = time;}
    void SetBoardstack(double bs) {m_boardstack = bs;}
    //void SetAdcVal(int ibin, int val) {m_adc_values[ibin] = val;}
    //void SetTdcVal(int ibin, int val) {m_tdc_values[ibin] = val;}
    void SetID(int id) {m_id = id;}
    void SetIntBefore(double int_before) {m_int_before = int_before;}
    void SetIntAfter(double int_after) {m_int_after = int_after;}
    void SetRisetime(double risetime) {m_risetime = risetime;}
    void SetFalltime(double falltime) {m_falltime = falltime;}
    void SetPedRMS(double ped_rms) {m_ped_rms = ped_rms;}
    void SetRefTime(double ref_time) {m_ref_time = ref_time;}
    void SetRefT1(int ref_t1) {m_ref_T1 = ref_t1;}
    //void SetRefT2(double ref_t2) {m_ref_T2 = ref_t2;}
    void SetRefA1(double ref_a1) {m_ref_A1 = ref_a1;}
    void SetRefA2(double ref_a2) {m_ref_A2 = ref_a2;}

    void SetCrossT1(int cross_t1) {m_cross_T1 = cross_t1;}
    //    void SetCrossT2(double cross_t2) {m_cross_T2 = cross_t2;}
    void SetCrossA1(double cross_a1) {m_cross_A1 = cross_a1;}
    void SetCrossA2(double cross_a2) {m_cross_A2 = cross_a2;}
    void SetRefAmplitude(double ref_amp) {m_ref_amp = ref_amp;}
    void SetMaxBin(int max_bin) {m_max_bin = max_bin;}
    void SetRefMaxBin(int ref_max_bin) {m_ref_max_bin = ref_max_bin;}


  private:
    topcaf_channel_id_t m_channel_id;
    packet_word_t m_evt_num;
    packet_word_t m_asic_win;
    size_t m_nwave_seg;
    size_t m_nsamples;
    unsigned short m_asic;
    unsigned short m_asic_ch;
    unsigned short m_asic_row;
    unsigned short m_asic_col;
    unsigned short m_asic_refwin;
    double m_time;
    double m_amp;
    double m_rate;
    double m_quality;
    int m_time_bin;
    int m_pixel_id;
    int m_pmt_id;
    int m_pmtch_id;
    double m_tdc_bin;
    double m_adc_height;
    double m_width;
    double m_chi2;
    double m_corr_time;
    double m_boardstack;
    int m_flag;
    double m_q;
    //int m_adc_values[128];
    //int m_tdc_values[128];
    int m_id;
    double m_int_before;
    double m_int_after;
    double m_risetime;
    double m_falltime;
    double m_ped_rms;
    double m_ref_time;
    double m_ref_amp;
    int m_ref_max_bin;
    int m_ref_T1;
    double m_ref_A1;
    double m_ref_A2;
    int m_cross_T1;
    double m_cross_A1;
    double m_cross_A2;
    int m_max_bin;



    ClassDef(TOPCAFDigitCalib, 1);
  };
}
#endif

/*  LocalWords:  SetRefT
 */
