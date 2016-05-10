#include <topcaf/modules/DoubleCalPulseModule/DoubleCalPulseModule.h>
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


using namespace Belle2;
using namespace std;

REG_MODULE(DoubleCalPulse);

DoubleCalPulseModule::DoubleCalPulseModule() : Module()
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


DoubleCalPulseModule::~DoubleCalPulseModule() {}

//void DoubleCalPulseModule::defineHisto()
//{
//  m_occupancy = new TH2F("WaveFormOccupancy", "WaveFormOccupancy", 64, 1, 65, 8, 1, 9);
//}

void DoubleCalPulseModule::initialize()
{

}

void DoubleCalPulseModule::beginRun()
{

}


void DoubleCalPulseModule::event()
{

  //Get Waveform from datastore
  StoreArray<TOPCAFDigit> digit_ptr;
  digit_ptr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();

  map<topcaf_channel_id_t, double> asic_ref_time;

  map<topcaf_channel_id_t, double> asic_ref_flag;

  if (digit_ptr) {
    //Look over all waveforms for calibration times.

    for (int w = 0; w < digit_ptr.getEntries(); w++) {
      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 1000000);
      asicKey *= 1000000;
      int asic = digit_ptr[w]->GetASIC();
      asicKey += asic * 10000;
      int asic_ch = digit_ptr[w]->GetASICChannel();

      //Special case for Tsukuba negative cal pulse when threshold is set as negative
      if ((m_adcmin < 0) && (digit_ptr[w]->GetTDCBin() > m_tmin) && (digit_ptr[w]->GetTDCBin() < m_tmax)
          && (digit_ptr[w]->GetWidth() > m_wmin)
          && (digit_ptr[w]->GetWidth() < m_wmax) && (digit_ptr[w]->GetADCHeight() < m_adcmin) && (digit_ptr[w]->GetADCHeight() > m_adcmax)) {
        if ((asic_ch == m_cal_ch) && ((digit_ptr[w]->GetTDCBin() < asic_ref_time[asicKey]) || (asic_ref_time[asicKey] == 0))) {
          asic_ref_time[asicKey] = digit_ptr[w]->GetTDCBin();
          digit_ptr[w]->SetFlag(10);
        }
        //      B2INFO(hardwareID<<"\ttdc: "<<digit_ptr[w]->GetTDCBin()<<"\tasic_ref_time: "<<asic_ref_time[asicKey]);
      }
      //Special case for Tsukuba negative cal pulse when threshold is set as negative
      if ((m_adcmin < 0) && (digit_ptr[w]->GetTDCBin() > m_tmin) && (digit_ptr[w]->GetTDCBin() < m_tmax)
          && (digit_ptr[w]->GetWidth() > m_wmin)
          && (digit_ptr[w]->GetWidth() < m_wmax) && (digit_ptr[w]->GetADCHeight() < m_adcmin) && (digit_ptr[w]->GetADCHeight() > m_adcmax)) {
        if ((asic_ch == m_cal_ch) && ((digit_ptr[w]->GetTDCBin() < asic_ref_time[asicKey]) || (asic_ref_time[asicKey] == 0))) {
          asic_ref_time[asicKey] = digit_ptr[w]->GetTDCBin();
          digit_ptr[w]->SetFlag(10);
        }
        //      B2INFO(hardwareID<<"\ttdc: "<<digit_ptr[w]->GetTDCBin()<<"\tasic_ref_time: "<<asic_ref_time[asicKey]);
      }
      if ((digit_ptr[w]->GetTDCBin() > m_tmin) && (digit_ptr[w]->GetTDCBin() < m_tmax) && (digit_ptr[w]->GetWidth() > m_wmin)
          && (digit_ptr[w]->GetWidth() < m_wmax) && (digit_ptr[w]->GetADCHeight() > m_adcmin) && (digit_ptr[w]->GetADCHeight() < m_adcmax)) {
        if ((asic_ch == m_cal_ch) && ((digit_ptr[w]->GetTDCBin() < asic_ref_time[asicKey]) || (asic_ref_time[asicKey] == 0))) {
          asic_ref_time[asicKey] = digit_ptr[w]->GetTDCBin();
          digit_ptr[w]->SetFlag(10);
        }
        //      B2INFO(hardwareID<<"\ttdc: "<<digit_ptr[w]->GetTDCBin()<<"\tasic_ref_time: "<<asic_ref_time[asicKey]);
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
        if (digit_ptr[w]->GetFlag() != 10) digit_ptr[w]->SetFlag(1); // i.e. calibrated to a pulse.
        if ((corr_time > 45.) && (corr_time < 70.)
            && (asic_ch == m_cal_ch)) { // then mark this as the second calibration pulse. hard coded numbers again...
          digit_ptr[w]->SetFlag(11); // second calibration pulse.
          if (asic_ref_flag[asicKey] > 9) {
            asic_ref_flag[asicKey] = 11;
          } else {
            asic_ref_flag[asicKey] = 1;
          }
        }
        if (((corr_time < -4. && corr_time > -125.) || (corr_time > 4. && corr_time < 15.))
            && (asic_ch == m_cal_ch)) {  // then mark this as a bad cal pulse hit
          digit_ptr[w]->SetFlag(20); // bad cal hit
          if (asic_ref_flag[asicKey] < 10) {
            asic_ref_flag[asicKey] = asic_ref_flag[asicKey] + 10;
          }
        }
      }
      //      B2INFO(hardwareID<<"\ttdc: "<<digit_ptr[w]->GetTDCBin()<<"\tasicKey: "<<asicKey<<"\tasic_ref_time: "<<asic_ref_time[asicKey]<<"\tcorr_time: "<<corr_time);

    }
    //Flag to confirm other pulses found
    for (int w = 0; w < digit_ptr.getEntries(); w++) {
      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 1000000);
      asicKey *= 1000000;
      int asic = digit_ptr[w]->GetASIC();
      asicKey += asic * 10000;


      //int asic_ch = digit_ptr[w]->GetASICChannel();

      if (asic_ref_flag[asicKey] == 1) {
        //double pulse confirmed
        digit_ptr[w]->SetFlag(digit_ptr[w]->GetFlag() + 100);
      }
      if (asic_ref_flag[asicKey] == 10) {
        //bad ch7 digit confirmed
        digit_ptr[w]->SetFlag(digit_ptr[w]->GetFlag() + 1000);
      }
      if (asic_ref_flag[asicKey] == 10) {
        //both
        digit_ptr[w]->SetFlag(digit_ptr[w]->GetFlag() + 1100);
      }
    }
    //Flag to confirm other pulses found
    for (int w = 0; w < digit_ptr.getEntries(); w++) {
      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 1000000);
      asicKey *= 1000000;
      int asic = digit_ptr[w]->GetASIC();
      asicKey += asic * 10000;
      //int asic_ch = digit_ptr[w]->GetASICChannel();

      if (asic_ref_flag[asicKey] == 1) {
        //double pulse confirmed
        digit_ptr[w]->SetFlag(digit_ptr[w]->GetFlag() + 100);
      }
      if (asic_ref_flag[asicKey] == 10) {
        //bad ch7 digit confirmed
        digit_ptr[w]->SetFlag(digit_ptr[w]->GetFlag() + 1000);
      }
      if (asic_ref_flag[asicKey] == 10) {
        //both
        digit_ptr[w]->SetFlag(digit_ptr[w]->GetFlag() + 1100);
      }
    }
  }//End loop
}


void DoubleCalPulseModule::terminate()
{

}
