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

    explicit TOPCAFDigit(const EventWaveformPacket* wp);


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
    //! get Carrier (not carrier is stored as asic_row internally due to change in naming scheme).
    unsigned short GetCarrier() const {return m_asic_row;}
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
    //! Boardstack
    double GetBoardstack() const {return m_boardstack;}


    /// Extra stuff not in the raw packet ///
    double GetTime() const {return m_time;}
    //! get time bin
    int GetTimeBin() const {return m_time_bin;}
    //! Amplitude
    double GetAmplitude() const {return m_amp;}
    //! sampling rate
    double GetSamplingRate() const {return m_rate;}
    //! Quality
    double GetQuality() const {return m_quality;}
    //! Flag
    int GetFlag() const {return m_flag;}
    //! corrected time
    double GetCorrTime() const {return m_corr_time;}
    //! Get pixel
    int GetPixel() const {return m_pixel_id;}
    //! Get PMT
    int GetPMT() const {return m_pmt_id;}
    //! Get PMT pixel
    int GetPMTPixel() const {return m_pmtch_id;}
    //! Get chi squared
    double GetChi2() const {return m_chi2;}
    //! Get charge
    double GetCharge() const {return m_q;}




    ///--- Setters ---///
    void SetTime(double time) {m_time = time;}
    //! set time bin
    void SetTimeBin(int time_bin) {m_time_bin = time_bin;}
    //! TDC bin
    void SetTDCBin(double tdc_bin) {m_tdc_bin = tdc_bin;}
    //! Width
    void SetWidth(double width) {m_width = width;}
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
    //! set asic number
    void SetASIC(unsigned short asic) {m_asic = asic;}
    //! Set carrier
    void SetCarrier(unsigned short carrier) {m_asic_row = carrier;}
    //! ASIC channel
    void SetASICChannel(unsigned short asic_ch) {m_asic_ch = asic_ch;}
    //! topcaf channel ID
    void SetChannelID(topcaf_channel_id_t channel_id) {m_channel_id = channel_id;}
    //! set event number
    void SetEventNumber(packet_word_t evt_num) {m_evt_num = evt_num;}
    //! set asic window
    void SetASICWindow(packet_word_t asic_win) {m_asic_win = asic_win;}
    //! set NumWaveSegments
    void SetNumWaveSegments(size_t nwave_seg) {m_nwave_seg = nwave_seg;}
    //! set number of samples
    void SetNumSamples(size_t nsamples) {m_nsamples = nsamples;}
    //! set reference window
    void SetRefWindow(unsigned short asic_refwin) {m_asic_refwin = asic_refwin;}
    //! set pixel number (1-512)
    void SetPixel(int pixel) {m_pixel_id = pixel;}
    //! set PMT number (1-32)
    void SetPMT(int pmt) {m_pmt_id = pmt;}
    //! set pmt pixel (1-16)
    void SetPMTPixel(int pmtpixel) {m_pmtch_id = pmtpixel;}
    //! set adc height
    void SetADCHeight(double adc_height) {m_adc_height = adc_height;}
    //! set chi2
    void SetChi2(double chi2) {m_chi2 = chi2;}
    //! set q (charge)
    void SetCharge(double q) {m_q = q;}




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
