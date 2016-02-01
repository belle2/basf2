#include <topcaf/modules/WaveTimingV2Module/WaveTimingV2Module.h>
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


using namespace Belle2;

REG_MODULE(WaveTimingV2);

const int max_peaks = 320;

WaveTimingV2Module::WaveTimingV2Module() : Module()
{
  setDescription("This module calculates the timing of itop raw waveform event data");
  addParam("time2TDC", m_time2tdc, "Conversion factor to match iTOPDigit time scale [time unit/ns]", 40.0);
  addParam("sigma", m_sigma, "Sigma of searched peaks (see TSpectrum)", 2.);
  addParam("threshold", m_thresh, "Threshold for peak search", 50.);
}


WaveTimingV2Module::~WaveTimingV2Module() {}

//void WaveTimingV2Module::defineHisto()
//{
//  m_occupancy = new TH2F("WaveFormOccupancy", "WaveFormOccupancy", 64, 1, 65, 8, 1, 9);
//}

void WaveTimingV2Module::initialize()
{
  m_topcafdigits_ptr.registerInDataStore();
  m_fraction = 0.4;
  m_time_delay = 15;
  m_rate = 1.0;
  m_crude_time = 0;
  m_cf_time = 0;
  //  m_tmp_h = new TH1D("wave_h", "wave_h", channel_samples, 0, channel_samples);
  //  REG_HISTOGRAM
}

void WaveTimingV2Module::beginRun()
{

  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  if (topconfig_ptr) {
    m_time2tdc = 1. / (topconfig_ptr->getTDCUnit_ns());
  }
}


void WaveTimingV2Module::event()
{

  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();

  //Output TOPCAFDigit
  m_topcafdigits_ptr.clear();

  double ftsw = 0;
  if (evtheader_ptr) {
    ftsw = evtheader_ptr->GetFTSW();
  }


  TSpectrum* spec = new TSpectrum(max_peaks);
  if (evtwaves_ptr) {

    //Look over all waveforms
    for (int w = 0; w < evtwaves_ptr.getEntries(); w++) {

      topcaf_channel_id_t hardwareID = evtwaves_ptr[w]->GetChannelID();
      //double win_time_shift = evtwaves_ptr[w]->GetTime();
      std::vector< double > v_samples = evtwaves_ptr[w]->GetSamples();
      if (v_samples.size() > 0) {
        int refwin = evtwaves_ptr[w]->GetRefWindow();
        int win = evtwaves_ptr[w]->GetASICWindow();
        float window_dt = (1. / 0.0212) / 2.0; // need to check this.  I don't like hard coded numbers.
        float sample_dt = window_dt / 64.0;

        //coarse time setup
        int coarse_int = refwin > win ? refwin - win : 64 - (win - refwin);
        float coarse_t = float(coarse_int) * window_dt;
        //Find rough peak locations
        if (m_tmp_h) delete m_tmp_h;
        m_tmp_h = new TH1D("m_tmp_h", "m_tmp_h", (int)v_samples.size(), 0., (double)v_samples.size());
        for (unsigned int s = 0; s < v_samples.size(); s++) {
          m_tmp_h->SetBinContent(s, v_samples.at(s));
        }
        //      m_tmp_h->Smooth(2);
        int search_peaks_found = spec->Search(m_tmp_h, m_sigma, "nodraw", 0.05);
        double* xpos = spec->GetPositionX();
        double* ypos = spec->GetPositionY();

        //Create TOPDigit
        int channelID = -9;
        int pmtID = -9;
        int pmtchID = -9;
        StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
        if (topconfig_ptr) {
          TopPixel p = topconfig_ptr->hardwareID_to_pixel(hardwareID);
          channelID = p.second;
          pmtID = topconfig_ptr->pixel_to_PMTNumber(p);
          pmtchID = topconfig_ptr->pixel_to_channelNumber(p);
        } else {
          B2WARNING("ITOP channel mapping not found, TOPDigit channel IDs will be incorrect.");
        }

        std::vector < topcaf_hit_t > hits;
        topcaf_hit_t hit;
        hit.channel_id = channelID;
        hit.pmt_id = pmtID;
        hit.pmtch_id = pmtchID;
        int peaks_found = 0;
        TF1* hitfit;
        for (int c = 0; c < search_peaks_found; c++) {
          if (ypos[c] > m_thresh) {
            peaks_found++;
            hitfit = new TF1("hitfit", "gaus", xpos[c] - 10, xpos[c] + 10);
            m_tmp_h->Fit("hitfit", "RQ");
            //    hit.adc_height = ypos[c];
            hit.adc_height = hitfit->GetParameter(0);
            //    hit.tdc_bin = xpos[c];
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

        for (int c = 0; c < hits.size(); c++) {
          TOPCAFDigit* this_topcafdigit = m_topcafdigits_ptr.appendNew(evtwaves_ptr[w]);

          this_topcafdigit->SetHitValues(hits[c]);
        }

      }
    }//End loop

  }


}

void WaveTimingV2Module::terminate()
{

}
