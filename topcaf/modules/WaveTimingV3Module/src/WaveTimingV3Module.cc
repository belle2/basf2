#include <topcaf/modules/WaveTimingV3Module/WaveTimingV3Module.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/TopConfigurations.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include "TH1D.h"
#include "TF1.h"
#include "TSpectrum.h"
#include <cmath>

using namespace Belle2;
using namespace std;

REG_MODULE(WaveTimingV3);

const int max_peaks = 20;

WaveTimingV3Module::WaveTimingV3Module() : Module()
{
  setDescription("This module calculates the timing of itop raw waveform event data");
  addParam("time2TDC", m_time2tdc, "Conversion factor to match iTOPDigit time scale [time unit/ns]", 40.0);
  addParam("sigma", m_sigma, "Sigma of searched peaks (see TSpectrum)", 2.);
  addParam("threshold", m_thresh, "Threshold for peak search", 100.);
  addParam("calibrationChannel", m_calChannel, "Calibration channel", 7);
}


WaveTimingV3Module::~WaveTimingV3Module()
{
}

void WaveTimingV3Module::initialize()
{
  m_topcafdigits_ptr.registerInDataStore();
  m_fraction = 0.4;
  m_time_delay = 15;
  m_rate = 1.0;
  m_crude_time = 0;
  m_cf_time = 0;
}

void WaveTimingV3Module::beginRun()
{

  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  if (topconfig_ptr) {
    m_time2tdc = 1. / (topconfig_ptr->getTDCUnit_ns());
  }
}


void WaveTimingV3Module::event()
{

  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();

  //Output TOPCAFDigit
  m_topcafdigits_ptr.clear();

  // double ftsw = 0;
  // if (evtheader_ptr) {
  //   ftsw = evtheader_ptr->GetFTSW();
  // }


  TSpectrum* spec = new TSpectrum(max_peaks);
  if (not evtwaves_ptr) {
    return;
  }

  //Look over all waveforms
  for (int w = 0; w < evtwaves_ptr.getEntries(); w++) {

    topcaf_channel_id_t hardwareID = evtwaves_ptr[w]->GetChannelID();
    unsigned int scrod = hardwareID / 100000000;
    int row = evtwaves_ptr[w]->GetASICRow();
    int asic = evtwaves_ptr[w]->GetASIC();
    int asicch = evtwaves_ptr[w]->GetASICChannel();
    //double win_time_shift = evtwaves_ptr[w]->GetTime();
    std::vector< double > v_samples = evtwaves_ptr[w]->GetSamples();
    if (v_samples.empty()) {
      return;
    }
    // int refwin = evtwaves_ptr[w]->GetRefWindow();
    // int win = evtwaves_ptr[w]->GetASICWindow();
    // float window_dt = (1. / 0.0212) / 2.0; // need to check this.  I don't like hard coded numbers.
    // float sample_dt = window_dt / 64.0;

    //coarse time setup
    // int coarse_int = refwin > win ? refwin - win : 64 - (win - refwin);
    // float coarse_t = float(coarse_int) * window_dt;
    //Find rough peak locations
    const size_t CALPULSEOFFSET = 150;
    TH1D m_tmp_h("m_tmp_h", "m_tmp_h", v_samples.size() - CALPULSEOFFSET - 100, CALPULSEOFFSET, v_samples.size() - 100);
    for (size_t s = CALPULSEOFFSET; s < v_samples.size() - 100; s++) {
      m_tmp_h.SetBinContent(s - CALPULSEOFFSET + 1, v_samples.at(s));
    }

    //      m_tmp_h->Smooth(2);
    int search_peaks_found = spec->Search(&m_tmp_h, m_sigma, "nodraw", 0.1);
    double* xpos = spec->GetPositionX();
    double* ypos = spec->GetPositionY();

    //Create TOPDigit
    int channelID = -9;
    int pmtID = -9;
    int pmtchID = -9;
    int electronics = -9;
    StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
    if (topconfig_ptr) {
      electronics = topconfig_ptr->scrod_to_electronicsModuleNumber(scrod);
      TopPixel p = topconfig_ptr->hardwareID_to_pixel(hardwareID);
      p.second = topconfig_ptr->electronics_to_pixel(electronics, row, asic, asicch);
      channelID = p.second;
      pmtID = topconfig_ptr->pixel_to_PMTNumber(p);
      pmtchID = topconfig_ptr->pixel_to_channelNumber(p);
      //    if(scrod==25) B2INFO("scrod: 25\tEMN: "<<topconfig_ptr->scrod_to_electronicsModuleNumber(scrod)<<"\tpix: "<<p.second);
    } else {
      B2WARNING("ITOP channel mapping not found, TOPDigit channel IDs will be incorrect.");
    }

    vector<topcaf_hit_t> hits;
    topcaf_hit_t hit;
    hit.channel_id = channelID;
    hit.pmt_id = pmtID;
    hit.pmtch_id = pmtchID;

    // find double peak structure of calibration pulse
    if (channelID == m_calChannel) {
      TF1 calpulse("doublePeak", "gaus(0)+gaus(3)", 0, 300);
      // Start with some good guesses (obtained offline)
      calpulse.SetParameter(0, -500);
      calpulse.SetParameter(1, 295 - CALPULSEOFFSET);
      calpulse.SetParameter(2, 3.5);
      calpulse.SetParameter(3, 700);
      calpulse.SetParameter(4, 301 - CALPULSEOFFSET);
      calpulse.SetParameter(5, 2.1);
      m_tmp_h.Fit("doublePeak", "RQ");
      // negativePeak
      if (calpulse.GetParameter(0) < -m_thresh) {
        hit.adc_height = calpulse.GetParameter(0);
        hit.tdc_bin = calpulse.GetParameter(1) + CALPULSEOFFSET;
        hit.width = calpulse.GetParameter(2);
        hit.chi2 = calpulse.GetChisquare();
        hits.push_back(hit);
        // positivePeak
        hit.adc_height = calpulse.GetParameter(3);
        hit.tdc_bin = calpulse.GetParameter(4) + CALPULSEOFFSET;
        hit.width = calpulse.GetParameter(5);
        hit.chi2 = calpulse.GetChisquare();
        hits.push_back(hit);
      }
    }

    int peaks_found = 0;
    TF1* hitfit;
    for (int c = 0; c < search_peaks_found; c++) {
      if (ypos[c] > m_thresh) {
        peaks_found++;
        hitfit = new TF1("hitfit", "gaus", xpos[c] - 10, xpos[c] + 10);
        m_tmp_h.Fit("hitfit", "RQ");
        hit.adc_height = hitfit->GetParameter(0);
        hit.tdc_bin = hitfit->GetParameter(1);
        hit.width = hitfit->GetParameter(2);
        hit.chi2 = hitfit->GetChisquare();
        hits.push_back(hit);

        B2DEBUG(1, hardwareID << " peak found (" << xpos[c] << "," << ypos[c] << ")\twidth: " << hit.width << "\tchi2: " << hit.chi2);
        delete hitfit;
      }
    }
    //      B2DEBUG(1,peaks_found<<" peaks found.");
    evtwaves_ptr[w]->SetHits(hits);
    B2DEBUG(1, hits.size() << " peaks found, " << evtwaves_ptr[w]->GetHits().size() << " made it");

    //Create topcafDIGITs
    for (size_t c = 0; c < hits.size(); c++) {
      TOPCAFDigit* this_topcafdigit = m_topcafdigits_ptr.appendNew(evtwaves_ptr[w]);
      this_topcafdigit->SetHitValues(hits[c]);
      this_topcafdigit->SetBoardstack(electronics);
    }
  }//End loop
}

void WaveTimingV3Module::terminate()
{

}
