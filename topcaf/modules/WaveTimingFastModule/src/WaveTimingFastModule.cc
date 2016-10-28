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
  addParam("threshold", m_thresh, "Threshold for peak search", 50.);
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



double WaveTimingFastModule::calcCrossTimeRise(std::vector<double> v_samples, unsigned int tmax, double amp, double frac)
{
  // The peak search is done in absolute value to cope with both positive and negative pulses.
  amp = TMath::Abs(amp);
  while ((TMath::Abs(v_samples.at(tmax)) > amp * frac) && (tmax > 0)) {
    tmax--;
  }

  double first = TMath::Abs(v_samples.at(tmax));
  double second =  TMath::Abs(v_samples.at(tmax + 1));
  double dV = second - first;
  double crosstime = (tmax + ((amp * frac) - first) / dV);
  if (tmax == 0) {
    crosstime = 0.;
  }

  return crosstime;
}


double WaveTimingFastModule::calcCrossTimeFall(std::vector<double> v_samples, unsigned int tmax, double amp, double frac)
{
// The peak search is done in absolute value to cope with both positive and negative pulses.
  amp = TMath::Abs(amp);
  while ((tmax + 1 < v_samples.size()) && (TMath::Abs(v_samples.at(tmax)) > amp * frac)) {
    tmax++;
  }

  double first = TMath::Abs(v_samples.at(tmax));
  double second =  TMath::Abs(v_samples.at(tmax - 1));
  double dV = second - first;
  double crosstime = (tmax - ((amp * frac) - first) / dV);
  if (tmax + 1 == v_samples.size()) {
    crosstime = 0.;
  }
  return crosstime;
}


double WaveTimingFastModule::calcIntegral(std::vector<double> v_samples, unsigned int tmax, double width)
{
  if (width < 0) {
    B2WARNING("Negative width in calcIntegral. Skipping the hit.");
    return 0.;
  }

  // first checks the boundaries for the integral calculation
  int min = (int)(tmax - 2 * width);
  int max = (int)(tmax + 2 * width);

  if (min < 0) min = 0;
  if (max > (int)v_samples.size()) max = v_samples.size();

  double integral = 0;

  while (min < max) {
    integral += v_samples.at(min);
    min++;
  }

  return integral;
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

  if (evtwaves_ptr) {
    //Look over all waveforms
    for (int w = 0; w < evtwaves_ptr.getEntries(); w++) {

      // electronics numbering
      topcaf_channel_id_t hardwareID = evtwaves_ptr[w]->GetChannelID();
      unsigned int scrod = hardwareID / 100000000;
      int row = evtwaves_ptr[w]->GetASICRow();
      int asic = evtwaves_ptr[w]->GetASIC();
      int asicch = evtwaves_ptr[w]->GetASICChannel();
      int pmtID = -9;
      int pmtchID = -9;
      int boardstack = -9;
      int pixelID = -1;
      StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
      if (topconfig_ptr) {
        boardstack = (int)topconfig_ptr->scrod_to_electronicsModuleNumber(scrod);
        if (scrod == 102 || scrod == 78 || scrod == 94 || scrod == 103) boardstack = 3;
        if (scrod == 44 || scrod == 52 || scrod == 82 || scrod == 41) boardstack = 2;
        if (scrod == 23 || scrod == 47 || scrod == 80 || scrod == 88) boardstack = 1;
        if (scrod == 35 || scrod == 40 || scrod == 79 || scrod == 74) boardstack = 0;
        TopPixel p = topconfig_ptr->hardwareID_to_pixel(hardwareID);
        pixelID = topconfig_ptr->electronics_to_pixel(boardstack, row, asic, asicch);
        pmtID = topconfig_ptr->pixel_to_PMTNumber(p);
        pmtchID = topconfig_ptr->pixel_to_channelNumber(p);
      } else {
        B2WARNING("ITOP channel mapping not found, TOPDigit channel IDs will be incorrect.");
      }

      vector<double> v_samples = evtwaves_ptr[w]->GetSamples();

      // loops ove the traces and looks for hits in each one of them
      if (v_samples.size() > 0) {
        double amax = -1; // auxiliary variable ofr peak finding (absolute value of the amplitude)
        unsigned int s = 1; // counter of the sample insde the trace
        int isAboveThrCount = 0; // counter of the number of consecutive samples above the threshold
        int iHit = 0; // counter of the number fo potential hits


        // loops over the samples of the trace
        while (s < v_samples.size() - 1) {
          // hit candidate found: the amplitude was above threshold for at least 3 samples and now it is ether belov threshold or
          // has a different sign
          if (isAboveThrCount >= 3 && (TMath::Abs(v_samples.at(s)) < m_thresh || v_samples.at(s)*v_samples.at(s - 1) < 0)) {
            if (iHit < 300) {
              // time and amplitude of the hit will be stored here during the search procedure
              unsigned int hitTime = 0; // in time bins
              double hitAmp = 0.; // in acd counts. double because of the pedestal subtraction

              //this is a good segment. Look backward to find the hit maximum
              for (int i = 1; i <= isAboveThrCount; i++) {
                if (TMath::Abs(v_samples.at(s - i)) > amax) {
                  hitTime = s - i;
                  amax = TMath::Abs(v_samples.at(s - i));
                  hitAmp = v_samples.at(s - i);
                }
              }

              //now hitTime and hitAmp contains the basic hit information. We can construct the TOPCAFDigit
              TOPCAFDigit* this_topcafdigit = m_topcafdigits_ptr.appendNew(evtwaves_ptr[w]);

              double frontTime = calcCrossTimeRise(v_samples, hitTime, hitAmp, m_frac);
              double backTime = calcCrossTimeFall(v_samples, hitTime, hitAmp, m_frac);
              double integral = calcIntegral(v_samples, hitTime, (backTime - frontTime) / 2.);

              this_topcafdigit->SetPMT(pmtID);
              this_topcafdigit->SetPMTPixel(pmtchID);
              this_topcafdigit->SetPixel(pixelID);
              this_topcafdigit->SetBoardstack(boardstack);

              this_topcafdigit->SetTDCBin(frontTime);
              this_topcafdigit->SetTimeBin(hitTime);
              this_topcafdigit->SetADCHeight(hitAmp);
              this_topcafdigit->SetWidth(backTime - frontTime);
              this_topcafdigit->SetCharge(integral);

              B2DEBUG(1, hardwareID << " peak found (" << hitTime << "," << hitAmp << ")\twidth: " << backTime - frontTime <<
                      "\tfrontTime: " << frontTime << "\tbackTime: " << backTime);

              amax = -1;
              iHit++;
            } else
              B2WARNING("Too many segments. Skipping");
          }

          if (TMath::Abs(v_samples.at(s)) > m_thresh && v_samples.at(s)*v_samples.at(s - 1) > 0)
            isAboveThrCount++;
          else
            isAboveThrCount = 0;
          s++;
        }
      }
    }//End loop
  }
}

void WaveTimingFastModule::terminate()
{

}
