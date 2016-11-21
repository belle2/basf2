/**********************************************************************************
 * Data class for the Event Packet: Waveform information as defined in:           *
 * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format *
 * Author: Malachi Schram (malachi.schram@pnnl.gov)                               *
 **********************************************************************************/

#ifndef TOPCAFDigitCalib_H
#define TOPCAFDigitCalib_H

#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <vector>


namespace Belle2 {

  class TOPCAFDigitCalib: public Packet {

  public:
    TOPCAFDigitCalib();
    ~TOPCAFDigitCalib();

    explicit TOPCAFDigitCalib(const EventWaveformPacket* wp);

    ///--- Getters ---///
    //! ge Channel ID
    topcaf_channel_id_t GetChannelID() const {return m_channel_id;}
    //! event number
    packet_word_t GetEventNumber() const {return m_evt_num;}
    //! reference window
    unsigned short GetRefWindow() const {return  m_asic_refwin;}
    //! number of wave segments
    size_t GetNumWaveSegments() const {return  m_nwave_seg;}
    //! ASIC window
    packet_word_t GetASICWindow()const {return m_asic_win;}
    //! get ASIC
    unsigned short GetASIC() const {return m_asic;}
    //! ASIC channel
    unsigned short GetASICChannel() const {return m_asic_ch;}
    //! ASIC row
    unsigned short GetASICRow() const {return m_asic_row;}
    //! ASIC column
    unsigned short GetASICColumn() const {return m_asic_col;}
    //! number of samples
    size_t GetNumSamples() const {return m_nsamples;}
    //! TDC bin
    double GetTDCBin() const {return m_tdc_bin;}
    //! get width
    double GetWidth() const {return m_width;}
    //! ADC height
    double GetADCHeight() const {return m_adc_height;}

    ///--- Getters ---///
    //! get reference time
    double GetRefTime() const {return m_ref_time;}
    //! reference T1
    int GetRefT1() const {return m_ref_T1;}
    int GetRefMaxBin() const {return m_ref_max_bin;}
    double GetRefA1() const {return m_ref_A1;}
    double GetRefA2() const {return m_ref_A2;}
    //! reference amplitude
    double GetRefAmplitude() const {return m_ref_amp;}

    ///--- Getters ---///
    int GetCrossT1() const {return m_cross_T1;}
    int GetMaxBin() const {return m_max_bin;}
    //! get cross A1
    double GetCrossA1() const {return m_cross_A1;}
    //! get cross A2
    double GetCrossA2() const {return m_cross_A2;}

    /// Extra stuff not in the raw packet ///
    //! get time
    double GetTime() const {return m_time;}
    //! get time bin
    int GetTimeBin() const {return m_tdc_bin;}
    //! get amplitude
    double GetAmplitude() const {return m_amp;}
    //! get sampling rate
    double GetSamplingRate() const {return m_rate;}
    //! quality
    double GetQuality() const {return m_quality;}
    //! get Flag
    int GetFlag() const {return m_flag;}
    //! get correcting time
    double GetCorrTime() const {return m_corr_time;}

    ///--- Setters ---///
    //! set time
    void SetTime(double time) {m_time = time;}
    //! set time bin
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    //!set TDC bin
    void SetTDCBin(double tdc_bin) {m_tdc_bin = tdc_bin;}
    //! set width
    void SetWidth(int width) {m_width = width;}
    //! set Amplitude
    void SetAmplitude(double amp) {m_amp = amp;}
    //! sampling rate
    void SetSamplingRate(double rate) {m_rate = rate;}
    //! quality
    void SetQuality(double quality) {m_quality = quality;}
    //! hit values
    void SetHitValues(topcaf_hit_t& mydigit);
    //! set Flag
    void SetFlag(int flag) {m_flag = flag;}
    //! set correcting time
    void SetCorrTime(double time) {m_corr_time = time;}
    //! set Board Stack
    void SetBoardstack(double bs) {m_boardstack = bs;}
    void SetAdcVal(int ibin, int val) {m_adc_values[ibin] = val;}
    void SetTdcVal(int ibin, int val) {m_tdc_values[ibin] = val;}

    ///--- Setters ---///
    //! set ID
    void SetID(int id) {m_id = id;}
    void SetIntBefore(double int_before) {m_int_before = int_before;}
    void SetIntAfter(double int_after) {m_int_after = int_after;}
    //! set rise time
    void SetRisetime(double risetime) {m_risetime = risetime;}
    //! set fall time
    void SetFalltime(double falltime) {m_falltime = falltime;}
    //! set pedestal RMS
    void SetPedRMS(double ped_rms) {m_ped_rms = ped_rms;}
    //! set reference time
    void SetRefTime(double ref_time) {m_ref_time = ref_time;}
    void SetRefT1(int ref_t1) {m_ref_T1 = ref_t1;}
    //void SetRefT2(double ref_t2) {m_ref_T2 = ref_t2;}
    void SetRefA1(double ref_a1) {m_ref_A1 = ref_a1;}
    void SetRefA2(double ref_a2) {m_ref_A2 = ref_a2;}
    ///---Setters ---///
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
    int m_adc_values[3000];
    int m_tdc_values[3000];
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
