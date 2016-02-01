#ifndef TOPCAFDigit_H
#define TOPCAFDigit_H

#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
// Data class for the Event Packet: Waveform information as defined in:           //
// http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format //
// Author: Malachi Schram (malachi.schram@pnnl.gov)                               //
////////////////////////////////////////////////////////////////////////////////////

namespace Belle2 {

  class TOPCAFDigit: public Packet {

  public:
    TOPCAFDigit();
    TOPCAFDigit(const EventWaveformPacket* wp);

    ~TOPCAFDigit();

    //--- Getters ---//
    topcaf_channel_id_t GetChannelID() const {return m_channel_id;}
    int GetEventNumber() const {return m_evt_num;}
    int GetRefWindow() const {return  m_asic_refwin;}
    int GetNumWaveSegments() const {return  m_nwave_seg;}
    int GetASICWindow()const {return m_asic_win;}
    int GetASICChannel() const {return m_asic_ch;}
    int GetASICRow() const {return m_asic_row;}
    int GetASICColumn() const {return m_asic_col;}
    int GetNumSamples() const {return m_nsamples;}
    double GetTDCBin() const {return m_tdc_bin;}
    double GetWidth() const {return m_width;}

    // Extra stuff not in the raw packet //
    double GetTime() const {return m_time;}
    int GetTimeBin() const {return m_time_bin;}
    double GetAmplitude() const {return m_amp;}
    double GetSamplingRate() const {return m_rate;}
    double GetQuality() const {return m_quality;}
    //--- Setters ---//
    void SetTime(double time) {m_time = time;}
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    void SetAmplitude(double amp) {m_amp = amp;}
    void SetSamplingRate(double rate) {m_rate = rate;}
    void SetQuality(double quality) {m_quality = quality;}
    void SetHitValues(topcaf_hit_t& mydigit);

  private:
    topcaf_channel_id_t m_channel_id;
    packet_word_t m_evt_num;
    packet_word_t m_asic_win;
    packet_word_t m_nwave_seg;
    packet_word_t m_nsamples;
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

    ClassDef(TOPCAFDigit, 1);
  };
}
#endif
