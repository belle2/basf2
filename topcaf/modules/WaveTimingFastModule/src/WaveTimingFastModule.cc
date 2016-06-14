#include <topcaf/modules/WaveTimingFastModule/WaveTimingFastModule.h>
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

REG_MODULE(WaveTimingFast);

//const int max_peaks = 320;
//const int max_peaks = 20;

WaveTimingFastModule::WaveTimingFastModule() : Module()
{
  setDescription("This module calculates the timing of itop raw waveform event data");
  addParam("time2TDC", m_time2tdc, "Conversion factor to match iTOPDigit time scale [time unit/ns]", 40.0);
  addParam("sigma", m_sigma, "Sigma of searched peaks (see TSpectrum)", 2.);
  addParam("threshold", m_thresh, "Threshold for peak search", 50.);
  addParam("threshold_n", m_thresh_n, "Negative threshold for Tsukuba calpulse peak search", -1000.);
  addParam("fraction", m_frac, "Fraction of peak height to use for hit timing parameters [0,1]", 0.5);
  addParam("dTcalib", m_dTcalib, "Apply dT calibration.  Data must be in topcaf/data/ directory.", false);
}


WaveTimingFastModule::~WaveTimingFastModule() {}

void WaveTimingFastModule::initialize()
{
  m_topcafdigits_ptr.registerInDataStore();
  m_fraction = 0.4;
  m_time_delay = 15;
  m_rate = 1.0;
  m_crude_time = 0;
  m_cf_time = 0;
}

void WaveTimingFastModule::beginRun()
{
  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  if (topconfig_ptr) {
    m_time2tdc = 1. / (topconfig_ptr->getTDCUnit_ns());
  }
}


void WaveTimingFastModule::event()
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

  //TSpectrum spec(max_peaks);
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
        double amax = -1; // absolute value of the amplitude. For peak finding
        unsigned int s = 1; // skip the first point
        int isAboveThrCount = 0;
        int iMax = 0;

        double xpos[300];
        double ypos[300];
        while (s < v_samples.size() - 1) {
          {

            if (isAboveThrCount >= 3 && (TMath::Abs(v_samples.at(s)) < 50.
                                         || v_samples.at(s)*v_samples.at(s - 1) < 0)) { // end of a segment above thr
              if (iMax < 300) {
                for (int i = 1; i <= isAboveThrCount; i++) { // search for the maximum beckwards
                  if (TMath::Abs(v_samples.at(s - i)) > amax) {
                    xpos[iMax] = s - i;
                    amax = TMath::Abs(v_samples.at(s - i));
                    ypos[iMax] = v_samples.at(s - i);
                  }
                }
                amax = -1;
                iMax++;
              } else
                B2WARNING("Too many segments. Skipping");
            }

            if (TMath::Abs(v_samples.at(s)) > 50. && v_samples.at(s)*v_samples.at(s - 1) > 0)
              isAboveThrCount++;
            else
              isAboveThrCount = 0;
            s++;
          }
        }
        int search_peaks_found = iMax;


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

          //Installed Tsukuba exception - Fast
          //Find 5 negative bins in a row with one below threshold
          //Store all of these as negative hits, then sort it out in the DoubleCalModule

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
              //if we have a peak in mind, store it
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

                neg_count = 0;
                d_cur_peak = 0;
                d_cur_max = 0;
                d_peak = 0;
                d_max = 0;
              }
            }
          }

          //else no peak found

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

void WaveTimingFastModule::terminate()
{

}
