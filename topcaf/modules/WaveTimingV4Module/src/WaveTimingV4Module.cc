#include <topcaf/modules/WaveTimingV4Module/WaveTimingV4Module.h>
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
#include <iostream>

using namespace Belle2;
using namespace std;

REG_MODULE(WaveTimingV4);

//const int max_peaks = 320;
const int max_peaks = 20;

WaveTimingV4Module::WaveTimingV4Module() : Module()
{
  setDescription("This module calculates the timing of itop raw waveform event data");
  addParam("time2TDC", m_time2tdc, "Conversion factor to match iTOPDigit time scale [time unit/ns]", 40.0);
  addParam("sigma", m_sigma, "Sigma of searched peaks (see TSpectrum)", 2.);
  addParam("threshold", m_thresh, "Threshold for peak search", 50.);
  addParam("threshold_n", m_thresh_n, "Negative threshold for Tsukuba calpulse peak search", -1000.);
  addParam("fraction", m_frac, "Fraction of peak height to use for hit timing parameters [0,1]", 0.5);
  addParam("dTcalib", m_dTcalib, "Apply dT calibration.  Data must be in topcaf/data/ directory.", false);
}


WaveTimingV4Module::~WaveTimingV4Module() {}

void WaveTimingV4Module::initialize()
{
  m_topcafdigits_ptr.registerInDataStore();
  m_fraction = 0.4;
  m_time_delay = 15;
  m_rate = 1.0;
  m_crude_time = 0;
  m_cf_time = 0;
}

void WaveTimingV4Module::beginRun()
{
  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  if (topconfig_ptr) {
    m_time2tdc = 1. / (topconfig_ptr->getTDCUnit_ns());
  }
}


void WaveTimingV4Module::event()
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
  //TSpectrum spec_n(max_peaks);
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
        //TH1D m_tmp_h_n("m_tmp_h_n", "m_tmp_h_n", (int)v_samples.size(), 0., (double)v_samples.size());

        for (unsigned int s = 0; s < v_samples.size(); s++) {
          m_tmp_h.SetBinContent(s, v_samples.at(s));
          //m_tmp_h_n.SetBinContent(s, -1.*v_samples.at(s));
        }
        //      m_tmp_h->Smooth(2);
        int search_peaks_found = spec.Search(&m_tmp_h, m_sigma, "nodraw", 0.05);
        double* xpos = spec.GetPositionX();
        double* ypos = spec.GetPositionY();
        //int search_peaks_found_n = spec_n.Search(&m_tmp_h_n, m_sigma, "nodraw", 0.05);
        //double* xpos_n = spec_n.GetPositionX();
        //double* ypos_n = spec_n.GetPositionY();


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

            unsigned int x1 = xpos[c] - 2 * (xpos[c] - d);
            if (x1 <= 1) {
              x1 = 1;
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
            while (((d + 1) < v_samples.size()) && (v_samples.at(d) > (ypos[c]*m_frac))) {
              d++;
            }
            first = v_samples.at(d);
            second = v_samples.at(d - 1);
            dV = second - first;
            double backTime = (d - ((ypos[c] * m_frac) - first) / dV);
            if ((d + 1) == v_samples.size()) {
              backTime = d;
            }
            unsigned int x2 = xpos[c] + 2 * (d - xpos[c]);
            if (x2 >= (v_samples.size() - 1)) {
              x2 = v_samples.size() - 1;
            }

            hit.width = backTime - frontTime;
            hit.chi2 = 0.;
            hit.q = 0.;

            while (x1 < x2) {
              hit.q +=  v_samples.at(x1);
              x1++;
            }

            hits.push_back(hit);

            B2DEBUG(1, hardwareID << " peak found (" << xpos[c] << "," << ypos[c] << ")\twidth: " << hit.width << "\ttdc: " << hit.tdc_bin <<
                    "\tfrontTime: " << frontTime << "\tbackTime: " << backTime);
          }

        }
        if (m_thresh_n < 0.) {

          //Installed Tsukuba exception
          //Find 5 negative bins in a row with one below threshold
          //But pick the last one in x
          //Then lookback a bit and see if there is a hint of a double negative peak
          int neg_count(0);
          unsigned int d_peak(0), d_cur_peak(0);
          double d_max(0.), d_cur_max(0.);
          for (unsigned int d = 0; d < v_samples.size(); d++) {
            //cout<<d<<": "<<v_samples.at(d)<<endl;
            if (v_samples.at(d) < 0) {
              neg_count++;
              //cout<<neg_count<<" "<<d_peak<<" "<<d_max<<" "<<endl;
              if (v_samples.at(d) < m_thresh_n) {
                if (v_samples.at(d) < d_cur_max) {
                  d_cur_peak = d;
                  d_cur_max = v_samples.at(d);
                }
              }
              if (neg_count > 4) {
                if (d_cur_peak > d_peak) {
                  //if(d_cur_max<d_max){
                  d_peak = d_cur_peak;
                  d_max = d_cur_max;
                }
              }
            } else {
              neg_count = 0;
              d_cur_peak = 0;
              d_cur_max = 0;
            }
          }

          //Count backwards 80 from the peak and look for at least one bin below threshold
          double d_min = d_peak - 80;
          int dbl_count(0);
          if (d_min < 1) d_min = 1; //Handle exception
          for (unsigned int d = d_peak; d > d_min; d--) {
            if (v_samples.at(d) < m_thresh_n) dbl_count++;
          }
          if (dbl_count < 1) d_max = 0;


          if (d_max != 0) {
            peaks_found++;
            hit.adc_height = d_max;
            unsigned int d = (d_peak + 0.5); // First measure front edge.
            while ((v_samples.at(d) < d_max * m_frac) && (d > 0)) {
              d--;
            }

            double first = v_samples.at(d);
            double second = v_samples.at(d + 1);
            double dV = second - first;
            double frontTime = d + (d_max * m_frac - first) / dV;
            if (d == 0) {
              frontTime = 0.;
            }
            //hit.tdc_bin = frontTime;

            d = (d_peak + 0.5); // Now measure back edge.
            while (((d + 1) < v_samples.size()) && (v_samples.at(d) < (d_max * m_frac))) {
              d++;
            }
            first = v_samples.at(d);
            second = v_samples.at(d - 1);
            dV = second - first;
            double backTime = d - (d_max * m_frac - first) / dV;
            if ((d + 1) >= v_samples.size()) {
              backTime = d;
            }

            hit.tdc_bin = frontTime;

            hit.width = backTime - frontTime;
            hit.chi2 = 0.;
            hit.q = 0.;

            hits.push_back(hit);

          }
          //else no peak found

          /*
          for (int c = 0; c < search_peaks_found_n; c++) {
            if (ypos_n[c] > m_thresh_n) {
              peaks_found++;
              hit.adc_height = -1*ypos_n[c];
              unsigned int d = (xpos_n[c] + 0.5); // First measure front edge.
              while ( (-1.*(v_samples.at(d)) > ypos_n[c]*m_frac) && (d > 0)) {
          d--;
              }

              double first = -1.*v_samples.at(d);
              double second = -1.*v_samples.at(d + 1);
              double dV = second - first;
              double frontTime = (d + ((ypos_n[c] * m_frac) - first) / dV);
              if (d == 0) {
          frontTime = 0.;
              }
              //hit.tdc_bin = frontTime;

              d = (xpos[c] + 0.5); // Now measure back edge.
              while (((d + 1) < v_samples.size()) && (-1.*(v_samples.at(d)) > (ypos_n[c]*m_frac)) ) {
          d++;
              }
              first = -1.*v_samples.at(d);
              second = -1.*v_samples.at(d - 1);
              dV = second - first;
              double backTime = (d - ((ypos_n[c] * m_frac) - first) / dV);
              if ((d + 1) == v_samples.size()) {
          backTime = d;
              }

              //For negative pulse, use the back edge for our timing point
              hit.tdc_bin = backTime;

              hit.width = backTime - frontTime;
              hit.chi2 = 0.;
              hit.q=0.;

              hits.push_back(hit);

              B2DEBUG(1, hardwareID << " negative peak found (" << xpos_n[c] << "," << ypos_n[c] << ")\twidth: " << hit.width << "\ttdc: " << hit.tdc_bin <<
                "\tfrontTime: " << frontTime << "\tbackTime: " << backTime);
            }

          }
          */
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

void WaveTimingV4Module::terminate()
{

}
