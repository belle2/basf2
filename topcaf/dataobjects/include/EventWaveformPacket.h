#ifndef EventWaveformPacket_H
#define EventWaveformPacket_H

#include <topcaf/dataobjects/Packet.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
// Data class for the Event Packet: Waveform information as defined in:           //
// http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format //
// Author: Malachi Schram (malachi.schram@pnnl.gov)                               //
////////////////////////////////////////////////////////////////////////////////////


class topcaf_hit_t : public TObject {
public:
  int channel_id; // pixel number 0-512..
  int pmt_id;
  int pmtch_id;
  double tdc_bin; // hit time (bin number)
  double adc_height; // hit height in adc counts
  double width; // bin width (1 sigma)
  double chi2; // chi2 of gaus fit to hit
  double q; //integral over waveform ROI
  ClassDef(topcaf_hit_t, 1)
} ;


namespace Belle2 {

  typedef unsigned long long topcaf_channel_id_t;

  class EventWaveformPacket: public Packet {

  public:
    EventWaveformPacket();
    EventWaveformPacket(const packet_word_t* temp_buffer, int nwords);

    ~EventWaveformPacket();

    //--- Getters ---//
    topcaf_channel_id_t GetChannelID() const {return m_channel_id;}
    packet_word_t GetEventNumber() const {return m_evt_num;}
    unsigned short GetRefWindow() const {return  m_asic_refwin;}
    size_t GetNumWaveSegments() const {return  m_nwave_seg;}
    packet_word_t GetASICWindow()const {return m_asic_win;}
    unsigned short GetASICChannel() const {return m_asic_ch;}
    unsigned short GetASICRow() const {return m_asic_row;}
    unsigned short GetASIC() const {return m_asic;}
    unsigned short GetASICColumn() const {return m_asic_col;}
    size_t GetNumSamples() const {return m_nsamples;}
    std::vector< double > GetSamples() const {return v_samples;}
    std::vector< topcaf_hit_t > GetHits() const {return v_hits;}


    // Extra stuff not in the raw packet //
    double GetTime() const {return m_time;}
    int GetTimeBin() const {return m_time_bin;}
    double GetAmplitude() const {return m_amp;}
    double GetSamplingRate() const {return m_rate;}
    double GetQuality() const {return m_quality;}
    //--- Setters ---//
    void SetSamples(const std::vector< double >& samples);
    void SetHits(const std::vector< topcaf_hit_t >& hits);
    void SetTime(double time) {m_time = time;}
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    void SetASIC(unsigned short asic) {m_asic = asic;}
    void SetAmplitude(double amp) {m_amp = amp;}
    void SetSamplingRate(double rate) {m_rate = rate;}
    void SetQuality(double quality) {m_quality = quality;}

  private:
    topcaf_channel_id_t m_channel_id;
    packet_word_t m_evt_num;
    packet_word_t m_asic_win;
    size_t m_nwave_seg;
    size_t m_nsamples;
    size_t m_nhits;
    unsigned short m_asic;
    unsigned short m_asic_ch;
    unsigned short m_asic_row;
    unsigned short m_asic_col;
    unsigned short m_asic_refwin;
    std::vector< double > v_samples;
    std::vector< topcaf_hit_t > v_hits;
    double m_time;
    double m_amp;
    double m_rate;
    double m_quality;
    int m_time_bin;
    ClassDef(EventWaveformPacket, 1);
  };
}


#ifdef __MAKECINT__
#pragma link C++ class topcaf_hit_t+;
#endif

#endif
