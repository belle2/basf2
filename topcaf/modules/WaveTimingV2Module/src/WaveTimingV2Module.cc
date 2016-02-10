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
using namespace std;

REG_MODULE(WaveTimingV2);

const int max_peaks = 320;

WaveTimingV2Module::WaveTimingV2Module() : Module()
{
  setDescription("This module calculates the timing of itop raw waveform event data");
  addParam("time2TDC", m_time2tdc, "Conversion factor to match iTOPDigit time scale [time unit/ns]", 40.0);
  addParam("sigma", m_sigma, "Sigma of searched peaks (see TSpectrum)", 2.);
  addParam("threshold", m_thresh, "Threshold for peak search", 50.);
  addParam("fraction", m_frac, "Fraction of peak height to use for hit timing parameters [0,1]", 0.5);
  addParam("dTcalib", m_dTcalib, "Apply dT calibration.  Data must be in topcaf/data/ directory.", false);
}


WaveTimingV2Module::~WaveTimingV2Module() {}

void WaveTimingV2Module::initialize()
{
  m_topcafdigits_ptr.registerInDataStore();
  m_fraction = 0.4;
  m_time_delay = 15;
  m_rate = 1.0;
  m_crude_time = 0;
  m_cf_time = 0;
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

  // double ftsw = 0;
  // if (evtheader_ptr) {
  //   ftsw = evtheader_ptr->GetFTSW();
  // }

  TSpectrum spec(max_peaks);
  if (evtwaves_ptr) {

    //Look over all waveforms
    for (int w = 0; w < evtwaves_ptr.getEntries(); w++) {

      topcaf_channel_id_t hardwareID = evtwaves_ptr[w]->GetChannelID();
      unsigned int scrod = hardwareID / 100000000;
      int row = evtwaves_ptr[w]->GetASICRow();
      int asic = evtwaves_ptr[w]->GetASIC();
      int asicch = evtwaves_ptr[w]->GetASICChannel();
      //double win_time_shift = evtwaves_ptr[w]->GetTime();
      vector<double> v_samples = evtwaves_ptr[w]->GetSamples();
      if (v_samples.size() > 0) {

        //Find rough peak locations
        TH1D m_tmp_h("m_tmp_h", "m_tmp_h", (int)v_samples.size(), 0., (double)v_samples.size());

        for (unsigned int s = 0; s < v_samples.size(); s++) {
          m_tmp_h.SetBinContent(s, v_samples.at(s));
        }
        //      m_tmp_h->Smooth(2);
        int search_peaks_found = spec.Search(&m_tmp_h, m_sigma, "nodraw", 0.05);
        double* xpos = spec.GetPositionX();
        double* ypos = spec.GetPositionY();

        //Create TOPDigit
        int channelID = -9;
        int pmtID = -9;
        int pmtchID = -9;
        int electronics = -9;
        StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
        if (topconfig_ptr) {
          electronics = (int)topconfig_ptr->scrod_to_electronicsModuleNumber(scrod);
          TopPixel p = topconfig_ptr->hardwareID_to_pixel(hardwareID);
          p.second = topconfig_ptr->electronics_to_pixel(electronics, row, asic, asicch);
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
        for (int c = 0; c < search_peaks_found; c++) {
          if (ypos[c] > m_thresh) {
            peaks_found++;
            hit.adc_height = ypos[c];
            unsigned int d = (xpos[c] + 0.5); // First measure front edge.
            while ((v_samples.at(d) > ypos[c]*m_frac) && (d > 0)) {
              d--;
            }

            double first = v_samples.at(d);
            double second = v_samples.at(d + 1);
            double dV = second - first;
            double frontTime = (d + ((ypos[c] * m_frac) - first) / dV);
            if (d == 0) {
              frontTime = 0.;
            }
            hit.tdc_bin = frontTime;

            d = (xpos[c] + 0.5); // Now measure back edge.
            while ((v_samples.at(d) > (ypos[c]*m_frac)) && ((d + 1) < v_samples.size())) {
              d++;
            }
            first = v_samples.at(d);
            second = v_samples.at(d - 1);
            dV = second - first;
            double backTime = (d - ((ypos[c] * m_frac) - first) / dV);
            if ((d + 1) == v_samples.size()) {
              backTime = d;
            }

            hit.width = backTime - frontTime;
            hit.chi2 = 0.;

            hits.push_back(hit);

            B2DEBUG(1, hardwareID << " peak found (" << xpos[c] << "," << ypos[c] << ")\twidth: " << hit.width << "\ttdc: " << hit.tdc_bin <<
                    "\tfrontTime: " << frontTime << "\tbackTime: " << backTime);
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

      }
    }//End loop

  }


}

void WaveTimingV2Module::terminate()
{

}
