#include <topcaf/modules/DoubleCalPulseSkimModule/DoubleCalPulseSkimModule.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/TopConfigurations.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include "TH1D.h"
#include "TF1.h"
#include "TSpline.h"
#include "TSpectrum.h"
#include <cmath>
#include <map>
#include <vector>

using namespace Belle2;
using namespace std;

REG_MODULE(DoubleCalPulseSkim);

DoubleCalPulseSkimModule::DoubleCalPulseSkimModule() : Module()
{
  setDescription("This module adjusts the timing of itop hits by searhing for a double calibration pulse in channel 7 of each asic.");

  addParam("calibrationChannel", m_cal_ch, "Channel in each ASIC that holds the calibration pulse", 7);
  addParam("calibrationTimeMin", m_tmin, "Earliest time to look for a calibration pulse", 0.);
  addParam("calibrationTimeMax", m_tmax, "Latest time to look for a calibration pulse", 2000.);
  addParam("calibrationWidthMin", m_wmin, "Minimum width of a calibration pulse", 1.3);
  addParam("calibrationWidthMax", m_wmax, "Maximum width of a calibration pulse", 3.0);
  addParam("calibrationADCThreshold", m_adcmin, "Min ADC count threshold for calibration pulse", 400.);
  addParam("calibrationADCThreshold_max", m_adcmax, "Max ADC count threshold for calibration pulse", 1000.);
  addParam("skim", m_skim, "0: no skim; 1: skim calpulse hits by printf ; 2: skim all hits", 1);
}

DoubleCalPulseSkimModule::~DoubleCalPulseSkimModule() {}

//void DoubleCalPulseSkimModule::defineHisto()
//{
//  m_occupancy = new TH2F("WaveFormOccupancy", "WaveFormOccupancy", 64, 1, 65, 8, 1, 9);
//}

void DoubleCalPulseSkimModule::initialize()
{
}

void DoubleCalPulseSkimModule::beginRun()
{
  h_trigger = -1;
}

void DoubleCalPulseSkimModule::event()
{
  //Get Waveform from datastore
  StoreArray<TOPCAFDigit> digit_ptr;
  digit_ptr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;

  evtheader_ptr.isRequired();

  map<topcaf_channel_id_t, double> asic_ref_time;

  map<topcaf_channel_id_t, double> asic_ref_flag;

  B2DEBUG(1, "DCPV5 " <<  "EvtNo = " <<  evtheader_ptr->GetEventNumber());

  vector<int>   h_idx;
  vector<float> h_tdc;
  vector<float> h_adc;
  vector<int>   h_1st_win;
  vector<int>   h_bs;
  vector<int>   h_carrier;//asic_col
  vector<int>   h_asic;//asic,asic_raw
  vector<int>   h_asic_ch;
  //
  int   ht = 0;
  int   asic_cal[4][2];// hit_id for 1st and 2nd calpulses
  int   asic_idx;
  int   h_hits;
  //
  int h_events = digit_ptr[0]->GetEventNumber();
  //here, we define the first frame in an event
  //this is for m_skim=2
  if (h_events % 16 == 0) {
    h_trigger++;// event instead of frames
    //printf("1: h_events=%d h_trigger=%d\n",h_events,h_trigger);
    h_hits = 0; //reset # hits =0 per event
  }
  //
  if (digit_ptr) {
    //Look over all waveforms for calibration times.
    //B2DEBUG(1,"DCPV5 " <<  "nhits = " <<  digit_ptr.getEntries() );
    //check 4 asics in one frame data
    //only one set of [a,b,c,d] data (4 asics)
    //
    int  asic_name[4];
    for (int i = 0; i < 4; i++) {
      asic_name[i] = -1;
      for (int j = 0; j < 2; j++) {
        asic_cal[i][j] = -1;
      }
    }
    //
    int prev_asic = -1;
    int idx = -1;
    // Store all calpulse candidates into vectors
    // with cuts of time bin and peak ADC and width of peak and cal_ch.
    //
    // if ((m_adcmin < 0) && (digit_ptr[w]->GetTDCBin() > m_tmin) && (digit_ptr[w]->GetTDCBin() < m_tmax)
    //    && (digit_ptr[w]->GetWidth() > m_wmin) && (digit_ptr[w]->GetWidth() < m_wmax) && (digit_ptr[w]->GetADCHeight() < m_adcmin)
    //    && (digit_ptr[w]->GetADCHeight() > m_adcmax) && (asic_ch == m_cal_ch))
    // printf TOPCAFDigits info right before calpulse selection.
    //only one set of [a,b,c,d] data (4 asics)
    for (int w = 0; w < digit_ptr.getEntries(); w++) {
      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 1000000);
      asicKey *= 1000000;
      int asic = digit_ptr[w]->GetASIC();
      asicKey += asic * 10000;
      int asic_ch = digit_ptr[w]->GetASICChannel();
      //
      if (
        ((m_adcmin > 0 && digit_ptr[w]->GetADCHeight() < m_adcmax && digit_ptr[w]->GetADCHeight() > m_adcmin)
         || (m_adcmin < 0 && digit_ptr[w]->GetADCHeight() > m_adcmax && digit_ptr[w]->GetADCHeight() < m_adcmin))
        && (digit_ptr[w]->GetTDCBin() > m_tmin) && (digit_ptr[w]->GetTDCBin() < m_tmax)
        && (digit_ptr[w]->GetWidth() > m_wmin) && (digit_ptr[w]->GetWidth() < m_wmax)
        && (asic_ch == m_cal_ch)
      ) {
        h_idx.push_back(w);
        h_tdc.push_back(digit_ptr[w]->GetTDCBin());
        h_adc.push_back(digit_ptr[w]->GetADCHeight());
        h_1st_win.push_back(digit_ptr[w]->GetASICWindow());
        h_bs.push_back(digit_ptr[w]->GetBoardstack());
        h_carrier.push_back(digit_ptr[w]->GetASICRow());
        h_asic.push_back(digit_ptr[w]->GetASIC());
        h_asic_ch.push_back(digit_ptr[w]->GetASICChannel());
        asic_idx = 16 * h_bs[ht] + 4 * h_carrier[ht] + h_asic[ht];
        //printf("ht=%2d idx %2d asic=%2d tdc %6.2f adc %7.2f 1st_w %2d bs %1d ca %1d as %1d ch %1d\n",
        //   ht, h_idx[ht],asic_idx,h_tdc[ht],h_adc[ht],h_1st_win[ht],h_bs[ht],h_carrier[ht],h_asic[ht],h_asic_ch[ht]);
        if (asic_idx == prev_asic) {
          asic_name[idx] = asic_idx; asic_cal[idx][0] = ht - 1; asic_cal[idx][1] = ht;
        } else   { prev_asic = asic_idx; idx++;}
        ht++;
      }
    }
    //
    // assume that calpulse candidates have only two hits of 1st and second pulses.
    //
    // Find a pair of calpulses for each asic module hk20160601
    // We have to loop over asics.
    for (int j = 0; j < 4; j++) {
      if (asic_name[j] < 0) continue;
      // confirm time interval between 1st and 2nd calpulses.
      // 40-80 tdc bins.
      if ((h_tdc[asic_cal[j][1]] - h_tdc[asic_cal[j][0]]) > 40
          && (h_tdc[asic_cal[j][1]] - h_tdc[asic_cal[j][0]]) < 80) {
        //flag calpulse hits
        topcaf_channel_id_t hardwareID = digit_ptr[h_idx[asic_cal[j][0]]]->GetChannelID();
        topcaf_channel_id_t asicKey = (hardwareID / 1000000);
        asicKey *= 1000000;
        int asic = digit_ptr[h_idx[asic_cal[j][0]]]->GetASIC();
        asicKey += asic * 10000;
        asic_ref_time[asicKey] = digit_ptr[h_idx[asic_cal[j][0]]]->GetTDCBin();
        digit_ptr[h_idx[asic_cal[j][0]]]->SetFlag(10);//flag  1st calpulse

        asic_ref_time[asicKey] = digit_ptr[h_idx[asic_cal[j][1]]]->GetTDCBin();
        digit_ptr[h_idx[asic_cal[j][1]]]->SetFlag(11);//flag  2nd calpulse
        int   h_asic_id = asic_name[j];
        int   h_asic_chan = h_asic_ch[asic_cal[j][0]];
        float h_tdc1 = h_tdc[asic_cal[j][0]];
        float h_adc1 = h_adc[asic_cal[j][0]];
        float h_tdc2 = h_tdc[asic_cal[j][1]];
        float h_adc2 = h_adc[asic_cal[j][1]];
        int   h_win = h_1st_win[asic_cal[j][0]];
        //
        if (m_skim == 1)   printf("%8d %3d %3d %3d %8.3f %7.2f %8.3f %7.2f\n",
                                    h_events, h_asic_id, h_asic_chan, h_win, h_tdc1, h_adc1, h_tdc2, h_adc2);
        //
      }
    }
    if (m_skim == 2) {
      // save all hits on ascii file with the calpulse flags
      for (int w = 0; w < digit_ptr.getEntries(); w++) {
        int   bs2, ca2, as2, ch2, win2, flag;
        float tdc2, adc2;
        //
        flag = 0;
        bs2 = digit_ptr[w]->GetBoardstack();
        ca2 = digit_ptr[w]->GetASICRow();
        as2 = digit_ptr[w]->GetASIC();
        ch2 = digit_ptr[w]->GetASICChannel();
        win2 = digit_ptr[w]->GetASICWindow();
        tdc2 = digit_ptr[w]->GetTDCBin();
        adc2 = digit_ptr[w]->GetADCHeight();
        // get calpulse flags
        flag = digit_ptr[w]->GetFlag();
        printf("%8d %2d %2d %2d %2d %4d %2d %8.3f %7.2f\n", h_trigger, bs2, ca2, as2, ch2, win2, flag, tdc2, adc2);
        h_hits++;//number of hits in this event
      }
    }
  }
  //need calibration after this.
} //End loop

void DoubleCalPulseSkimModule::terminate()
{
}
