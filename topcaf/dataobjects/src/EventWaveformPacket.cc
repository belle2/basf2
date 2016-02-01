#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <framework/logging/Logger.h>
#include <iomanip>

using namespace Belle2;
using namespace std;

// default constructor
// needs to be here to put this class into the object store
// also needs to be here so all members can be explicitly initialized, otherwise Cppcheck complains
EventWaveformPacket::EventWaveformPacket()
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


EventWaveformPacket::EventWaveformPacket(const unsigned int* temp_buffer,
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

  /*
  cout << "Waveform Header" << endl;
  for (unsigned int c = 0 ; c < 7 ; c++) {
    packet_word_t myword = m_packet_payload[c];
    cout << c << "\t...\t0x" << setfill('0') << setw(8) << hex << myword << dec << endl;
  }
  */

  for (size_t s = 0; s < (m_nsamples / 2); s++) {
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


void EventWaveformPacket::SetSamples(const vector< double >& samples)
{
  v_samples = samples;
  m_nsamples = samples.size();
}

void EventWaveformPacket::SetHits(const vector< topcaf_hit_t >& hits)
{
  v_hits = hits;
  m_nhits = hits.size();
  for (size_t c = 0; c < m_nhits; c++)
    B2DEBUG(1, m_channel_id << "setting peak found adc:" << hits[c].adc_height << "\ttdc:" << hits[c].tdc_bin << "\twidth: " <<
            hits[c].width << "\tchi2: " << hits[c].chi2 << endl);
}

EventWaveformPacket::~EventWaveformPacket() {}
