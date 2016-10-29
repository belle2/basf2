#include <topcaf/modules/DoubleCalPulseSkimV2Module/DoubleCalPulseSkimV2Module.h>
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

REG_MODULE(DoubleCalPulseSkimV2);

DoubleCalPulseSkimV2Module::DoubleCalPulseSkimV2Module() : Module()
{
  setDescription("This module adjusts the timing of itop hits by searhing for a double calibration pulse in channel 7 of each asic.");

  addParam("outputFileName", m_out_filename, "Output filename for saving skim results in a text file",
           std::string("/tmp/temp_skim.root"));
  addParam("calibrationChannel", m_cal_ch, "Channel in each ASIC that holds the calibration pulse", 7);
  addParam("calibrationTimeMin", m_tmin, "Earliest time to look for a calibration pulse", 0.);
  addParam("calibrationTimeMax", m_tmax, "Latest time to look for a calibration pulse", 2000.);
  addParam("calibrationWidthMin", m_wmin, "Minimum width of a calibration pulse", 1.3);
  addParam("calibrationWidthMax", m_wmax, "Maximum width of a calibration pulse", 3.0);
  addParam("calibrationADCThreshold", m_adcmin, "Min ADC count threshold for calibration pulse", 400.);
  addParam("calibrationADCThreshold_max", m_adcmax, "Max ADC count threshold for calibration pulse", 1000.);
  addParam("skim", m_skim, "0: no skim; 1: skim calpulse hits by printf ; 2: skim all hits", 1);
  addParam("runno", m_runno, "0: no run number set; others, should be from input", 0);

  m_out_file = nullptr;

}

DoubleCalPulseSkimV2Module::~DoubleCalPulseSkimV2Module() {}

//void DoubleCalPulseSkimV2Module::defineHisto()
//{
//  m_occupancy = new TH2F("WaveFormOccupancy", "WaveFormOccupancy", 64, 1, 65, 8, 1, 9);
//}

void DoubleCalPulseSkimV2Module::initialize()
{
  m_out_file = TFile::Open(m_out_filename.c_str(), "recreate");

//Define the branches for TTrees
  if (m_skim == 1) {
    ttree1 = new TTree("skim1", "data sample skim mode 1");
    ttree1->Branch("runno",        &runno,      "runno/I");
    ttree1->Branch("evtnum",       &evtnum,     "evtnum/I");
    ttree1->Branch("h_asic_id",    &h_asic_id,  "h_asic_id/I");
    ttree1->Branch("h_asic_chan",  &h_asic_chan, "h_asic_chan/I");
    ttree1->Branch("h_win",        &h_win,      "h_win/I");
    ttree1->Branch("h_tdc1",       &h_tdc1,     "h_tdc1/F");
    ttree1->Branch("h_adc1",       &h_adc1,     "h_adc1/F");
    ttree1->Branch("h_tdc2",       &h_tdc2,     "h_tdc2/F");
    ttree1->Branch("h_adc2",       &h_adc2,     "h_adc2/F");
  } else if (m_skim == 2) {
    ttree2 = new TTree("skim2", "data sample skim mode 2");
    ttree2->Branch("runno",  &runno,  "runno/I");
    ttree2->Branch("calch",  &calch,  "calch/I");
    ttree2->Branch("trig",   &trig,   "trig/I");
    ttree2->Branch("bs2",    &bs2,    "bs2/I");
    ttree2->Branch("ca2",    &ca2,    "ca2/I");
    ttree2->Branch("as2",    &as2,    "as2/I");
    ttree2->Branch("ch2",    &ch2,    "ch2/I");
    ttree2->Branch("win2",   &win2,   "win2/I");
    ttree2->Branch("flag",   &flag,   "flag/I");
    ttree2->Branch("tdc2",   &tdc2,   "tdc2/F");
    ttree2->Branch("adc2",   &adc2,   "adc2/F");
    ttree2->Branch("width2", &width2, "width2/F");
    ttree2->Branch("chg_int", &chg_int, "chg_int/F");

  }

}

void DoubleCalPulseSkimV2Module::beginRun()
{
  h_trigger = -1;
}

void DoubleCalPulseSkimV2Module::event()
{
  //Get Waveform from datastore
  StoreArray<TOPCAFDigit> digit_ptr;

  digit_ptr.isRequired();

  map<topcaf_channel_id_t, double> asic_ref_time;
  map<topcaf_channel_id_t, double> asic_ref_flag;

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
//  int   h_hits;
  //
  if (digit_ptr) {
    h_events = digit_ptr[0]->GetEventNumber();

    if (h_events % 16 == 0) {
      h_trigger++;// event instead of frames
//      h_hits=0;  //reset # hits =0 per event
    }
    int  asic_name[4];
    for (int i = 0; i < 4; i++) {
      asic_name[i] = -1;
      for (int j = 0; j < 2; j++) {
        asic_cal[i][j] = -1;
      }
    }
    int prev_asic = -1;
    int idx = -1;
    int check_evtnum[4];

    // Store all calpulse candidates into vectors
    //only one set of [a,b,c,d] data (4 asics)
    for (int w = 0; w < digit_ptr.getEntries(); w++) {

      int asic_ch = digit_ptr[w]->GetASICChannel();
      if ((asic_ch == m_cal_ch) &&
          ((m_adcmin > 0 && digit_ptr[w]->GetADCHeight() < m_adcmax && digit_ptr[w]->GetADCHeight() > m_adcmin)
           || (m_adcmin < 0 && digit_ptr[w]->GetADCHeight() > m_adcmax && digit_ptr[w]->GetADCHeight() < m_adcmin))
          && (digit_ptr[w]->GetTDCBin() > m_tmin) && (digit_ptr[w]->GetTDCBin() < m_tmax)
          && (digit_ptr[w]->GetWidth() > m_wmin) && (digit_ptr[w]->GetWidth() < m_wmax)
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
        if (asic_idx == prev_asic)
        { asic_name[idx] = asic_idx; asic_cal[idx][0] = ht - 1; asic_cal[idx][1] = ht;}
        else   { prev_asic = asic_idx; idx++;}
        ht++;

      }
    }

    for (int k = 0; k < 4; k++)check_evtnum[k] = -1;

    for (int w = 0; w < digit_ptr.getEntries(); w++) {
      for (int j = 0; j < 4; j++) {
        if (asic_name[j] < 0) continue;
        // confirm time interval between 1st and 2nd calpulses.
        // 40-80 tdc bins.
        if ((h_tdc[asic_cal[j][1]] - h_tdc[asic_cal[j][0]]) > 40
            && (h_tdc[asic_cal[j][1]] - h_tdc[asic_cal[j][0]]) < 80) {
          int w = h_idx[asic_cal[j][0]];
          int asicKey = 100 * digit_ptr[w]->GetBoardstack() + 10 * digit_ptr[w]->GetCarrier() + digit_ptr[w]->GetASIC();

          asic_ref_time[asicKey] = digit_ptr[h_idx[asic_cal[j][0]]]->GetTDCBin();
          digit_ptr[h_idx[asic_cal[j][0]]]->SetFlag(10);//flag  1st calpulse

          asic_ref_time[asicKey] = digit_ptr[h_idx[asic_cal[j][1]]]->GetTDCBin();
          digit_ptr[h_idx[asic_cal[j][1]]]->SetFlag(11);//flag  2nd calpulse
          if (m_skim == 1) {
            if (h_events == check_evtnum[j])continue;
            check_evtnum[j] = h_events;
            runno = m_runno;
            evtnum = h_events;
            h_asic_id = asic_name[j];
            h_asic_chan = h_asic_ch[asic_cal[j][0]];
            h_win = h_1st_win[asic_cal[j][0]];
            h_tdc1 = h_tdc[asic_cal[j][0]];
            h_adc1 = h_adc[asic_cal[j][0]];
            h_tdc2 = h_tdc[asic_cal[j][1]];
            h_adc2 = h_adc[asic_cal[j][1]];
            //
            ttree1->Fill();
          }
        }
      }
    }
    for (int w = 0; w < digit_ptr.getEntries(); w++) {

      if (m_skim == 2) {
        flag = 0;
        runno = m_runno;
        calch = m_cal_ch;
        bs2 = digit_ptr[w]->GetBoardstack();
        ca2 = digit_ptr[w]->GetASICRow();
        as2 = digit_ptr[w]->GetASIC();
        ch2 = digit_ptr[w]->GetASICChannel();
        win2 = digit_ptr[w]->GetASICWindow();
        tdc2 = digit_ptr[w]->GetTDCBin();
        adc2 = digit_ptr[w]->GetADCHeight();
        flag = digit_ptr[w]->GetFlag();
        width2 = digit_ptr[w]->GetWidth();
        chg_int = digit_ptr[w]->GetCharge();
        trig = h_trigger;
        ttree2->Fill();

//  h_hits++;//number of hits in this event
      }
    }
  }
}

void DoubleCalPulseSkimV2Module::terminate()
{
//ofs.close();
  if (m_out_file) {
    m_out_file->Write();
    m_out_file->Close();
    delete m_out_file;
  }

}
