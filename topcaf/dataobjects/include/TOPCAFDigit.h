/**********************************************************************************
 * Data class for the Event Packet: Waveform information as defined in:           *
 * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format *
 * Author: Malachi Schram (malachi.schram@pnnl.gov)                               *
 **********************************************************************************/

#ifndef TOPCAFDigit_H
#define TOPCAFDigit_H

#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <vector>

namespace Belle2 {

  class TOPCAFDigit: public Packet {

  public:
    TOPCAFDigit();
    ~TOPCAFDigit();

    TOPCAFDigit(const EventWaveformPacket* wp);


    ///--- Getters ---///
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


    /// Extra stuff not in the raw packet ///
    double GetTime() const {return m_time;}
    int GetTimeBin() const {return m_tdc_bin;}
    double GetAmplitude() const {return m_amp;}
    double GetSamplingRate() const {return m_rate;}
    double GetQuality() const {return m_quality;}
    int GetFlag() const {return m_flag;}
    double GetCorrTime() const {return m_corr_time;}
    ///--- Setters ---///
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



    ClassDef(TOPCAFDigit, 1);
  };
}
#endif

/*  LocalWords:  SetRefT
 */
