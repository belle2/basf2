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
    //! ASIC Row
    unsigned short GetASICRow() const {return m_asic_row;}
    //! ASIC Column
    unsigned short GetASICColumn() const {return m_asic_col;}
    //! number of samples
    size_t GetNumSamples() const {return m_nsamples;}
    //! TDC bin
    double GetTDCBin() const {return m_tdc_bin;}
    //! Width
    double GetWidth() const {return m_width;}
    //! ADC height
    double GetADCHeight() const {return m_adc_height;}


    /// Extra stuff not in the raw packet ///
    double GetTime() const {return m_time;}
    //! get time bin
    int GetTimeBin() const {return m_tdc_bin;}
    //! Amplitude
    double GetAmplitude() const {return m_amp;}
    //! sampling rate
    double GetSamplingRate() const {return m_rate;}
    //! Quality
    double GetQuality() const {return m_quality;}
    //! Flag
    int GetFlag() const {return m_flag;}
    //! correcting time
    double GetCorrTime() const {return m_corr_time;}

    ///--- Setters ---///
    void SetTime(double time) {m_time = time;}
    //! set time bin
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    //! TDC bin
    void SetTDCBin(double tdc_bin) {m_tdc_bin = tdc_bin;}
    //! Width
    void SetWidth(int width) {m_width = width;}
    //! amplitude
    void SetAmplitude(double amp) {m_amp = amp;}
    //! sampling rate
    void SetSamplingRate(double rate) {m_rate = rate;}
    //! quality
    void SetQuality(double quality) {m_quality = quality;}
    //! set Hit Values
    void SetHitValues(topcaf_hit_t& mydigit);
    //! set Flag
    void SetFlag(int flag) {m_flag = flag;}
    //! set correcting time
    void SetCorrTime(double time) {m_corr_time = time;}
    //! set Board Stack
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
