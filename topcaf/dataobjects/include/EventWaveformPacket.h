/**********************************************************************************
 * Data class for the Event Packet: Waveform information as defined in:           *
 * http://www.phys.hawaii.edu/~kurtisn*doku.php?id=itop:documentation:data_format *
 * Author: Malachi Schram (malachi.schram@pnnl.gov)                               *
 **********************************************************************************/

#ifndef EventWaveformPacket_H
#define EventWaveformPacket_H

#include <topcaf/dataobjects/Packet.h>
#include <vector>

class topcaf_hit_t : public TObject {
public:
  //! pixel number 0-512..
  int channel_id;
  //! PMT ID
  int pmt_id;
  //! PMT Channel ID
  int pmtch_id;
  //! hit time (bin number)
  double tdc_bin;
  //! hit height in adc counts
  double adc_height;
  //! bin width (1 sigma)
  double width;
  //! chi2 of gaus fit to hit
  double chi2;
  //! integral over waveform ROI
  double q;
  //! Rise Time
  double risetime;
  //! Fall Time
  double falltime;
  double int_before;
  double int_after;
  //! RMS of pedestal
  double ped_rms;
  //! Reference time
  double ref_time;
  int cross_T1;
  //! max bin
  int max_bin;
  double cross_A1;
  double cross_A2;

  ClassDef(topcaf_hit_t, 1)
} ;


namespace Belle2 {

  typedef unsigned long long topcaf_channel_id_t;

  class EventWaveformPacket: public Packet {

  public:
    EventWaveformPacket();
    EventWaveformPacket(const packet_word_t* temp_buffer, int nwords);

    ~EventWaveformPacket();

    ///--- Getters ---///
    topcaf_channel_id_t GetChannelID() const {return m_channel_id;}
    //! get event number
    packet_word_t GetEventNumber() const {return m_evt_num;}
    //! get reference window
    unsigned short GetRefWindow() const {return  m_asic_refwin;}
    //! get number of wave segments
    size_t GetNumWaveSegments() const {return  m_nwave_seg;}
    //! get ASIC window
    packet_word_t GetASICWindow()const {return m_asic_win;}
    //! get ASIC channel
    unsigned short GetASICChannel() const {return m_asic_ch;}
    //! ASIC row
    unsigned short GetASICRow() const {return m_asic_row;}
    //! get ASIC
    unsigned short GetASIC() const {return m_asic;}
    //! ASIC column
    unsigned short GetASICColumn() const {return m_asic_col;}
    //! Number of Samples
    size_t GetNumSamples() const {return m_nsamples;}
    //! vector of Samples
    std::vector< double > GetSamples() const {return v_samples;}
    //! vector of Hits
    std::vector< topcaf_hit_t > GetHits() const {return v_hits;}

    /// Extra stuff not in the raw packet ///
    double GetTime() const {return m_time;}
    //! time bins
    int GetTimeBin() const {return m_time_bin;}
    //! Amplitude
    double GetAmplitude() const {return m_amp;}
    //! Sampling Rate
    double GetSamplingRate() const {return m_rate;}
    //! Quality
    double GetQuality() const {return m_quality;}

    ///--- Setters ---///
    void SetSamples(const std::vector< double >& samples);
    //! set Hits
    void SetHits(const std::vector< topcaf_hit_t >& hits);
    //! Set time
    void SetTime(double time) {m_time = time;}
    //! Set time bins
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    //! set ASIC
    void SetASIC(unsigned short asic) {m_asic = asic;}
    //! Set Amplitude
    void SetAmplitude(double amp) {m_amp = amp;}
    //! Set Sampling Rate
    void SetSamplingRate(double rate) {m_rate = rate;}
    //! Set Quality
    void SetQuality(double quality) {m_quality = quality;}

  private:
    //! channel ID
    topcaf_channel_id_t m_channel_id;
    //! number of events
    packet_word_t m_evt_num;
    //! ASIC window
    packet_word_t m_asic_win;
    //! number of wave segments
    size_t m_nwave_seg;
    //! number of samples
    size_t m_nsamples;
    //! number of Hits
    size_t m_nhits;
    //! ASIC
    unsigned short m_asic;
    //! ASIC channel
    unsigned short m_asic_ch;
    //! ASIC row
    unsigned short m_asic_row;
    //! ASIC column
    unsigned short m_asic_col;
    //! ASIC reference window
    unsigned short m_asic_refwin;
    //! vector of samples
    std::vector< double > v_samples;
    //! vector of hits
    std::vector< topcaf_hit_t > v_hits;
    //! time
    double m_time;
    //! amplitude
    double m_amp;
    //! rate
    double m_rate;
    //! rate
    double m_quality;
    //! time bin
    int m_time_bin;
    ClassDef(EventWaveformPacket, 1);
  };
}

#endif
