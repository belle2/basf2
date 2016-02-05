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
  , m_asic(0)
  , m_asic_ch(0)
  , m_asic_row(0)
  , m_asic_col(0)
  , m_asic_refwin(0)
  , m_time(0)
  , m_amp(0)
  , m_rate(0)
  , m_quality(0)
  , m_time_bin(0)
  , m_flag(0)
{
}

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
  m_asic = in_wp->GetASIC();
  m_asic_ch = in_wp->GetASICChannel();
  m_asic_row = in_wp->GetASICRow();
  m_asic_col = in_wp->GetASICColumn();
  m_rate = in_wp->GetSamplingRate();

  m_flag = 0;

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
