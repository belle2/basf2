#include <topcaf/dataobjects/inc/EventWaveformPacket.h>

using namespace Belle2;

ClassImp(EventWaveformPacket)

EventWaveformPacket::EventWaveformPacket() : Packet() {}

EventWaveformPacket::EventWaveformPacket(const unsigned int* temp_buffer,
                                         int nwords):
  Packet(temp_buffer, nwords)
{
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

  for (unsigned int s = 0; s < (m_nsamples / 2); s++) {
    packet_word_t point = m_packet_payload[(7 + s)];
    v_samples[s * 2] = (point & (0x00000FFF));
    v_samples[s * 2 + 1] = ((point & (0x0FFF0000)) >> 16);
  }
  m_channel_id = m_scrod_id * 1E8
                 + m_asic_row * 1E6
                 + m_asic_col * 1E4
                 + m_asic_ch * 1E2;

  //Set default values
  m_amp = 0;
  m_time = 0;
  m_time_bin = 0;
  m_rate = 2.7135;
  m_quality = 0;
}


void EventWaveformPacket::SetSamples(std::vector< double > samples)
{
  v_samples = samples;
  m_nsamples = samples.size();
}

EventWaveformPacket::EventWaveformPacket(const EventWaveformPacket& in_wp)
{

  //From packet
  m_type = in_wp.m_type;
  m_scrod_rev = in_wp.m_scrod_rev;
  m_scrod_id = in_wp.m_scrod_id;

  //Raw
  m_channel_id = in_wp.m_channel_id;
  m_evt_num = in_wp.m_evt_num;
  m_asic_refwin = in_wp.m_asic_refwin;
  m_nwave_seg = in_wp.m_nwave_seg;
  m_asic_win = in_wp.m_asic_win;
  m_asic_ch = in_wp.m_asic_ch;
  m_asic_row = in_wp.m_asic_row;
  m_asic_col = in_wp.m_asic_col;
  m_nsamples = in_wp.m_nsamples;
  v_samples = in_wp.v_samples;
  //Reco
  m_time = in_wp.m_time;
  m_amp = in_wp.m_amp;
  m_rate = in_wp.m_rate;
  m_quality = in_wp.m_quality;
  m_time_bin = in_wp.m_time_bin;
}

EventWaveformPacket::~EventWaveformPacket() {}
