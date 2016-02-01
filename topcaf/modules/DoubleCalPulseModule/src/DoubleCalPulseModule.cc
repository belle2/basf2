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

REG_MODULE(DoubleCalPulse);

const int max_peaks = 320;

DoubleCalPulseModule::DoubleCalPulseModule() : Module()
{
  setDescription("This module adjusts the timing of itop hits by searhing for a double calibration pulse in channel 7 of each asic.");

  addParam("calibrationChannel", m_cal_ch, "Channel in each ASIC that holds the calibration pulse", 7);

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

  std::map<topcaf_channel_id_t, double> asic_ref_time;


  if (digit_ptr) {

    //Look over all waveforms for calibration times.
    for (int w = 0; w < digit_ptr.getEntries(); w++) {

      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 10000);
      int asic_ch = digit_ptr[w]->GetASICChannel();
      asicKey *= 10000;
      if ((digit_ptr[w]->GetTDCBin() > 600) && (digit_ptr[w]->GetTDCBin() < 1100))
        if ((asic_ch == m_cal_ch) && ((digit_ptr[w]->GetTDCBin() < asic_ref_time[asicKey]) || (asic_ref_time[asicKey] == 0))) {
          asic_ref_time[asicKey] = digit_ptr[w]->GetTDCBin();

        }
      //      B2INFO(hardwareID<<"\ttdc: "<<digit_ptr[w]->GetTDCBin()<<"\tasic_ref_time: "<<asic_ref_time[asicKey]);
    }

    //Loop again to apply calibration times
    for (int w = 0; w < digit_ptr.getEntries(); w++) {

      topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
      topcaf_channel_id_t asicKey = (hardwareID / 10000);
      asicKey *= 10000;
      double corr_time = digit_ptr[w]->GetTDCBin() - asic_ref_time[asicKey];
      digit_ptr[w]->SetTime(corr_time);
      digit_ptr[w]->SetQuality(asic_ref_time[asicKey]);
      //      B2INFO(hardwareID<<"\ttdc: "<<digit_ptr[w]->GetTDCBin()<<"\tasicKey: "<<asicKey<<"\tasic_ref_time: "<<asic_ref_time[asicKey]<<"\tcorr_time: "<<corr_time);

    }

  }//End loop

}




void DoubleCalPulseModule::terminate()
{

}
