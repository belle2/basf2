#include <topcaf/dataobjects/TOPCAFDigit.h>
#include <iostream>
#include <iomanip>

using namespace Belle2;

// default constructor
// needs to be here to put this class into the object store
// also needs to be here so all members can be explicitly initialized, otherwise Cppcheck complains
TOPCAFDigit::TOPCAFDigit()
  : Packet()
  , m_channel_id(0)
  , m_evt_num(0)
  , m_asic_win(0)
  , m_nwave_seg(0)
  , m_nsamples(0)
  , m_asic_ch(0)
  , m_asic_row(0)
  , m_asic_col(0)
  , m_asic_refwin(0)
  , m_time(0)
  , m_amp(0)
  , m_rate(0)
  , m_quality(0)
  , m_time_bin(0)
{
}

/*
TOPCAFDigit::TOPCAFDigit(const unsigned int* temp_buffer,
                                         int nwords)
  : Packet(temp_buffer, nwords)
{
  m_scrod_id = m_packet_payload[1];
  m_asic_refwin = m_packet_payload[2];
  m_evt_num = m_packet_payload[3];
  m_nwave_seg = ((0x0000FFFF)&m_packet_payload[4]);
  packet_word_t seg_asic_win = m_packet_payload[5];
  m_asic_win = ((0x000001FF)&seg_asic_win);
  m_asic_ch = ((0x00000E00)&seg_asic_win)  >>  9;
  m_asic_row = ((0x00003000)&seg_asic_win) >> 12;
  m_asic_col = ((0x0000C000)&seg_asic_win) >> 14;
  m_nsamples = m_packet_payload[6];
  v_samples.resize(m_nsamples, 0);


  std::cout << "Waveform Header" << std::endl;
  for (unsigned int c = 0 ; c < 7 ; c++) {
    packet_word_t myword = m_packet_payload[c];
    std::cout << c << "\t...\t0x" << std::setfill('0') << std::setw(8) << std::hex << myword << std::dec << std::endl;
  }


  for (unsigned int s = 0; s < (m_nsamples / 2); s++) {
    packet_word_t point = m_packet_payload[(7 + s)];
    v_samples[s * 2] = (point & (0x00000FFF));
    v_samples[s * 2 + 1] = ((point & (0x0FFF0000)) >> 16);
  }
  m_channel_id = (m_scrod_id * 1E8
                  + m_asic_row * 1E6
                  + m_asic_col * 1E4
                  + m_asic_ch * 1E2);

  //Set default values
  m_amp = 0;
  m_time = 0;
  m_time_bin = 0;
  m_rate = 2.7135;
  m_quality = 0;
}
*/

TOPCAFDigit::TOPCAFDigit(const EventWaveformPacket* in_wp)
{

  //From packet
  m_type = in_wp->GetPacketType();
  m_scrod_rev = in_wp->GetScrodRev();
  m_scrod_id = in_wp->GetScrodID();

  //Raw
  m_channel_id = in_wp->GetChannelID();
  m_evt_num = in_wp->GetEventNumber();
  m_asic_refwin = in_wp->GetRefWindow();
  m_nwave_seg = in_wp->GetNumWaveSegments();
  m_asic_win = in_wp->GetASICWindow();
  m_asic_ch = in_wp->GetASICChannel();
  m_asic_row = in_wp->GetASICRow();
  m_asic_col = in_wp->GetASICColumn();
  //  m_nsamples = in_wp.;

}

void TOPCAFDigit::SetHitValues(topcaf_hit_t& mydigit)
{
  m_pixel_id = mydigit.channel_id;
  m_pmt_id = mydigit.pmt_id;
  m_pmtch_id = mydigit.pmtch_id;
  m_tdc_bin = mydigit.tdc_bin;
  m_adc_height = mydigit.adc_height;
  m_width = mydigit.width;
  m_chi2 = mydigit.chi2;

  return;

}

TOPCAFDigit::~TOPCAFDigit() {}
