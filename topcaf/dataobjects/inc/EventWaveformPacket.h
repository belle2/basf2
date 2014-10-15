#ifndef EventWaveformPacket_H
#define EventWaveformPacket_H

#include <topcaf/dataobjects/inc/Packet.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
// Data class for the Event Packet: Waveform information as defined in:           //
// http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format //
// Author: Malachi Schram (malachi.schram@pnnl.gov)                               //
////////////////////////////////////////////////////////////////////////////////////

namespace Belle2 {

  class EventWaveformPacket: public Packet {

  public:
    EventWaveformPacket();
    EventWaveformPacket(const unsigned int* temp_buffer, int nwords);
    EventWaveformPacket(const EventWaveformPacket& in_wp);

    ~EventWaveformPacket();

    //--- Getters ---//
    unsigned int GetChannelID() const {return m_channel_id;}
    int GetEventNumber() const {return m_evt_num;}
    int GetRefWindow() const {return  m_asic_refwin;}
    int GetNumWaveSegments() const {return  m_nwave_seg;}
    int GetASICWindow()const {return m_asic_win;}
    int GetASICChannel() const {return m_asic_ch;}
    int GetASICRow() const {return m_asic_row;}
    int GetASICColumn() const {return m_asic_col;}
    int GetNumSamples() const {return m_nsamples;}
    std::vector< double > GetSamples() const {return v_samples;}


    // Extra stuff not in the raw packet //
    double GetTime() const {return m_time;}
    int GetTimeBin() const {return m_time_bin;}
    double GetAmplitude() const {return m_amp;}
    double GetSamplingRate() const {return m_rate;}
    double GetQuality() const {return m_quality;}
    //--- Setters ---//
    void SetSamples(std::vector< double > samples);
    void SetTime(double time) {m_time = time;}
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    void SetAmplitude(double amp) {m_amp = amp;}
    void SetSamplingRate(double rate) {m_rate = rate;}
    void SetQuality(double quality) {m_quality = quality;}

  private:
    unsigned int m_channel_id;
    packet_word_t m_evt_num;
    packet_word_t m_asic_win, m_nwave_seg;
    packet_word_t m_nsamples;
    unsigned char m_asic_ch, m_asic_row, m_asic_col;
    unsigned short m_asic_refwin;
    std::vector< double > v_samples;
    double m_time, m_amp, m_rate, m_quality;
    int m_time_bin;

    ClassDef(EventWaveformPacket, 1);
  };
}
#endif
