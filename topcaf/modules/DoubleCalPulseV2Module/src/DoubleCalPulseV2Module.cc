#include <topcaf/modules/DoubleCalPulseV2Module/DoubleCalPulseV2Module.h>
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

REG_MODULE(DoubleCalPulseV2);

DoubleCalPulseV2Module::DoubleCalPulseV2Module() : Module()
{
  setDescription("This module adjusts the timing of itop hits by searhing for a double calibration pulse in channel 7 of each asic.");

  addParam("calibrationChannel", m_cal_ch, "Channel in each ASIC that holds the calibration pulse", 7);

}


DoubleCalPulseV2Module::~DoubleCalPulseV2Module()
{
}


void DoubleCalPulseV2Module::initialize()
{

}

void DoubleCalPulseV2Module::beginRun()
{

}


void DoubleCalPulseV2Module::event()
{

  //Get Waveform from datastore
  StoreArray<TOPCAFDigit> digit_ptr;
  digit_ptr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();

  map<topcaf_channel_id_t, double> asic_ref_time;

  if (not digit_ptr) {
    return;
  }
  //Look over all waveforms for calibration times.

  for (int w = 0; w < digit_ptr.getEntries(); w++) {
    int asic_ch = digit_ptr[w]->GetASICChannel();
    if (asic_ch != m_cal_ch) {
      continue;
    }
    topcaf_channel_id_t hardwareID = digit_ptr[w]->GetChannelID();
    topcaf_channel_id_t asicKey = (hardwareID / 1000000);
    asicKey *= 1000000;
    int asic = digit_ptr[w]->GetASIC();
    asicKey += asic * 10000;

    if ((digit_ptr[w]->GetTDCBin() > 150) && (digit_ptr[w]->GetTDCBin() < 400)
        && ((digit_ptr[w]->GetTDCBin() < asic_ref_time[asicKey]) || (asic_ref_time[asicKey] == 0))) {
      asic_ref_time[asicKey] = digit_ptr[w]->GetTDCBin();
      digit_ptr[w]->SetFlag(10);
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
      if (digit_ptr[w]->GetFlag() != 10) {
        digit_ptr[w]->SetFlag(1); // i.e. calibrated to a pulse.
      }
      if (corr_time < 65. && asic_ch == m_cal_ch) {
        digit_ptr[w]->SetFlag(11); // second calibration pulse.
      }
    }
  }//End loop
}


void DoubleCalPulseV2Module::terminate()
{

}
