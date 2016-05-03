#include <topcaf/modules/DoubleCalPulseV5Module/DoubleCalPulseV5Module.h>
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

REG_MODULE(DoubleCalPulseV5);

DoubleCalPulseV5Module::DoubleCalPulseV5Module() : Module()
{
  setDescription("This module adjusts the timing of itop hits by searhing for a double calibration pulse in channel 7 of each asic.");

  addParam("calibrationChannel", m_cal_ch, "Channel in each ASIC that holds the calibration pulse", 7);
  addParam("calibrationTimeMin", m_tmin, "Earliest time to look for a calibration pulse", 0.);
  addParam("calibrationTimeMax", m_tmax, "Latest time to look for a calibration pulse", 2000.);
  addParam("calibrationWidthMin", m_wmin, "Minimum width of a calibration pulse", 1.3);
  addParam("calibrationWidthMax", m_wmax, "Maximum width of a calibration pulse", 3.0);
  addParam("calibrationADCThreshold", m_adcmin, "Min ADC count threshold for calibration pulse", 400.);
  addParam("calibrationADCThreshold_max", m_adcmax, "Max ADC count threshold for calibration pulse", 1000.);

}


DoubleCalPulseV5Module::~DoubleCalPulseV5Module() {}

//void DoubleCalPulseV5Module::defineHisto()
//{
//  m_occupancy = new TH2F("WaveFormOccupancy", "WaveFormOccupancy", 64, 1, 65, 8, 1, 9);
//}

void DoubleCalPulseV5Module::initialize()
{

}

void DoubleCalPulseV5Module::beginRun()
{

}


void DoubleCalPulseV5Module::event()
{

  //Get Waveform from datastore
  StoreArray<TOPCAFDigit> digit_ptr;
  digit_ptr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();

  map<topcaf_channel_id_t, double> asic_ref_time;

  map<topcaf_channel_id_t, double> asic_ref_flag;

  vector<int> h_idx;
  vector<float> h_tdc;

  if (digit_ptr) {
    //Look over all waveforms for calibration times.

    for (int w = 0; w < digit_ptr.getEntries(); w++) {
      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 1000000);
      asicKey *= 1000000;
      int asic = digit_ptr[w]->GetASIC();
      asicKey += asic * 10000;
      int asic_ch = digit_ptr[w]->GetASICChannel();


      //Special case for Tsukuba negative double cal pulse when threshold is set as negative and this module is called
      if ((m_adcmin < 0) && (digit_ptr[w]->GetTDCBin() > m_tmin) && (digit_ptr[w]->GetTDCBin() < m_tmax)
          && (digit_ptr[w]->GetWidth() > m_wmin) && (digit_ptr[w]->GetWidth() < m_wmax) && (digit_ptr[w]->GetADCHeight() < m_adcmin)
          && (digit_ptr[w]->GetADCHeight() > m_adcmax) && (asic_ch == m_cal_ch)) {
        h_idx.push_back(w);
        h_tdc.push_back(digit_ptr[w]->GetTDCBin());
      }

    }

    //We have looped over hits and found the negative ones of interest
    //Now loop over these and: pick the latest one and ensure there is another hit 50-90 tdc bins before it
    float max_tdc(-1); int max_tdc_idx(-1); int cal_1_idx(-1);
    if (h_tdc.size() > 1) {
      for (unsigned int i = 0; i != h_tdc.size(); i++) {
        if (h_tdc[i] > max_tdc) {
          max_tdc = h_tdc[i];
          max_tdc_idx = i;
        }
      }
      for (unsigned int i = 0; i != h_tdc.size(); i++) {
        if ((h_tdc[max_tdc_idx] - h_tdc[i]) > 50 && (h_tdc[max_tdc_idx] - h_tdc[i]) < 90) {
          //Mark the last cal pulse
          topcaf_channel_id_t hardwareID = digit_ptr[h_idx[max_tdc_idx]]->GetChannelID();
          topcaf_channel_id_t asicKey = (hardwareID / 1000000);
          asicKey *= 1000000;
          int asic = digit_ptr[h_idx[max_tdc_idx]]->GetASIC();
          asicKey += asic * 10000;
          asic_ref_time[asicKey] = digit_ptr[h_idx[max_tdc_idx]]->GetTDCBin();
          digit_ptr[h_idx[max_tdc_idx]]->SetFlag(10);

          //Keep this as the possible early cal pulse
          cal_1_idx = i;

        }
      }
      //Mark the first cal pulse
      if (cal_1_idx > -1) {

        hardwareID = digit_ptr[h_idx[cal_1_idx]]->GetChannelID();
        asicKey = (hardwareID / 1000000);
        asicKey *= 1000000;
        asic = digit_ptr[h_idx[cal_1_idx]]->GetASIC();
        asicKey += asic * 10000;
        digit_ptr[h_idx[cal_1_idx]]->SetFlag(11);

      }
    }


    //Loop again to apply calibration times
    for (int w = 0; w < digit_ptr.getEntries(); w++) {
      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 1000000);
      asicKey *= 1000000;
      int asic = digit_ptr[w]->GetASIC();
      asicKey += asic * 10000;
      int asic_ch = digit_ptr[w]->GetASICChannel();

      double corr_time = digit_ptr[w]->GetTDCBin() - asic_ref_time[asicKey];
      digit_ptr[w]->SetTime(corr_time);
      digit_ptr[w]->SetQuality(asic_ref_time[asicKey]);
      if (asic_ref_time[asicKey] > 0) {
        if (digit_ptr[w]->GetFlag() != 10 && digit_ptr[w]->GetFlag() != 11) digit_ptr[w]->SetFlag(1);  // i.e. calibrated to a pulse.
      } else {
        digit_ptr[w]->SetFlag(0);
      }
      //      B2INFO(hardwareID<<"\ttdc: "<<digit_ptr[w]->GetTDCBin()<<"\tasicKey: "<<asicKey<<"\tasic_ref_time: "<<asic_ref_time[asicKey]<<"\tcorr_time: "<<corr_time);

    }

  }  //End loop
}


void DoubleCalPulseV5Module::terminate()
{

}
