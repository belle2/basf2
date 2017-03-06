#include <topcaf/dataobjects/TOPCAFDigitCalib.h>
#include <iostream>
#include <iomanip>

using namespace Belle2;

// default constructor
// needs to be here to put this class into the object store
// also needs to be here so all members can be explicitly initialized, otherwise Cppcheck complains
// Values for initialization since m_cross_A1 are arbitrary, X.L. Wang
TOPCAFDigitCalib::TOPCAFDigitCalib()
  : Packet()
  , m_channel_id(-1)
  , m_evt_num(-1)
  , m_asic_win(-1)
  , m_nwave_seg(-1)
  , m_nsamples(-1)
  , m_asic(-1)
  , m_asic_ch(-1)
  , m_asic_row(-1)
  , m_asic_col(-1)
  , m_asic_refwin(-1)
  , m_time(0)
  , m_amp(0)
  , m_rate(0)
  , m_quality(0)
  , m_time_bin(0)
  , m_pixel_id(0)
  , m_pmt_id(0)
  , m_pmtch_id(0)
  , m_tdc_bin(0)
  , m_adc_height(0)
  , m_width(0)
  , m_chi2(0)
  , m_corr_time(0)
  , m_boardstack(-1)
  , m_flag(0)
  , m_q(0)
  , m_id(-1)
  , m_int_before(-1)
  , m_int_after(-1)
  , m_risetime(0)
  , m_falltime(0)
  , m_ped_rms(0)
  , m_ref_time(0)
  , m_ref_amp(0)
  , m_ref_max_bin(0)
  , m_ref_T1(0)
  , m_ref_A1(0)
  , m_ref_A2(0)
  , m_cross_T1(0)
  , m_cross_A1(0)
  , m_cross_A2(0)
  , m_max_bin(0)
{
  for (int i = 0; i < 3000; ++i) {
    m_adc_values[i] = 0;
    m_tdc_values[i] = 0;
  }
}

TOPCAFDigitCalib::TOPCAFDigitCalib(const EventWaveformPacket* in_wp)
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

// default values
  m_quality = -1;
  m_amp = 0;
  m_q = 0;
  m_time = 0;
  m_nsamples = -1;
  m_time_bin = -1;
  m_pixel_id = -1;
  m_pmt_id = -1;
  m_pmtch_id = -1;
  m_tdc_bin = -1;
  m_adc_height = -1;
  m_width = -1;
  m_chi2 = -1;
  m_corr_time = -1;
  m_boardstack = -1; // has to be set when the top config object is available.
  m_flag = 0;
  for (int i = 0; i < 3000; ++i) {
    m_adc_values[i] = 0;
    m_tdc_values[i] = 0;
  }
  m_cross_A1 = 0;
  m_cross_A2 = 0;
  m_cross_T1 = 0;
  m_falltime = 0;
  m_id = -1;
  m_int_after = 0;
  m_int_before = 0;
  m_max_bin = 0;
  m_ped_rms = 0;
  m_ref_amp = 0;
  m_ref_max_bin = 0;
  m_ref_time = 0;
  m_risetime = 0;
  m_ref_T1 = 0;
  m_ref_A1 = 0;
  m_ref_A2 = 0;
}

void TOPCAFDigitCalib::SetHitValues(topcaf_hit_t& mydigit)
{
  m_pixel_id = mydigit.channel_id;
  m_pmt_id = mydigit.pmt_id;
  m_pmtch_id = mydigit.pmtch_id;
  m_tdc_bin = mydigit.tdc_bin;
  m_adc_height = mydigit.adc_height;
  m_width = mydigit.width;
  m_chi2 = mydigit.chi2;
  m_q = mydigit.q;

  return;

}

TOPCAFDigitCalib::~TOPCAFDigitCalib() {}
