/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi  (tamponi@to.infn.it)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



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

  addParam("calibrationChannel", m_calCh, "Channel in each ASIC that holds the calibration pulse", 7);
  addParam("calibrationWidthMin", m_wMin, "Minimum width of a calibration pulse", 12.);
  addParam("calibrationWidthMax", m_wMax, "Maximum width of a calibration pulse", 30.);
  addParam("calibrationADCThresholdMin", m_adcMin, "Min ADC count threshold for a positive calibration pulse", 400.);
  addParam("calibrationADCThresholdMax", m_adcMax, "Max ADC count threshold for a positive calibration pulse", 1000.);
  addParam("calibrationADCThresholdMinNeg", m_adcMinNeg, "Min ADC count threshold for a negative calibration pulse", -1000.);
  addParam("calibrationADCThresholdMaxNeg", m_adcMaxNeg, "Max ADC count threshold for a negative calibration pulse", -400.);

}


DoubleCalPulseModule::~DoubleCalPulseModule() {}

void DoubleCalPulseModule::initialize()
{

}

void DoubleCalPulseModule::beginRun()
{

}


void  DoubleCalPulseModule::event()
{
  //Get hits from datastore
  StoreArray<TOPCAFDigit> digit_ptr;
  digit_ptr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();

  double asic_ref_time[1000];
  int asic_ref_calpulsenum[1000];

  for (int k = 0; k < 1000; k++)
    asic_ref_calpulsenum[k] = 0;


  if (digit_ptr) {
    cout << "new event" << endl;
    // first loop to look for the pulse candidates
    for (int w = 0; w < digit_ptr.getEntries(); w++) {

      int asicKey = 100 * digit_ptr[w]->GetBoardstack() + 10 * digit_ptr[w]->GetCarrier() + digit_ptr[w]->GetASIC();
      int asic_ch = digit_ptr[w]->GetASICChannel();
      digit_ptr[w]->SetFlag(0); // unprocessed hit has flag = 0
      //First applies the width selection and the channel ID requirement
      if (digit_ptr[w]->GetWidth() > m_wMin && digit_ptr[w]->GetWidth() < m_wMax && asic_ch == m_calCh) {
        // we don't distinguish bewteen positive and negative candidates, but we use different thresholds
        if ((digit_ptr[w]->GetADCHeight() > m_adcMin && digit_ptr[w]->GetADCHeight() < m_adcMax)
            || (digit_ptr[w]->GetADCHeight() > m_adcMinNeg && digit_ptr[w]->GetADCHeight() < m_adcMaxNeg)) {
          // if this is the first calpulse candidate, then use it as reference
          if (asic_ref_calpulsenum[asicKey] == 0)
            asic_ref_time[asicKey] = digit_ptr[w]->GetTDCBin();

          asic_ref_calpulsenum[asicKey]++;

          digit_ptr[w]->SetFlag(100 + asic_ref_calpulsenum[asicKey]);
        }
      }
    }


    //Loop again to apply calibration times
    for (int w = 0; w < digit_ptr.getEntries(); w++) {

      int asicKey = 100 * digit_ptr[w]->GetBoardstack() + 10 * digit_ptr[w]->GetCarrier() + digit_ptr[w]->GetASIC();

      // no calibration pulse has been found!
      if (asic_ref_calpulsenum[asicKey] == 0) {
        digit_ptr[w]->SetTime(0);
        digit_ptr[w]->SetQuality(0);
        digit_ptr[w]->SetFlag(-1000 - digit_ptr[w]->GetFlag()); // not calibrated hit
      }
      // only one calpulse candidate found
      if (asic_ref_calpulsenum[asicKey] == 1) {
        digit_ptr[w]->SetTime(digit_ptr[w]->GetTDCBin() - asic_ref_time[asicKey]);
        digit_ptr[w]->SetQuality(asic_ref_time[asicKey]);
        digit_ptr[w]->SetFlag(1000 + digit_ptr[w]->GetFlag()); // possibly problematic hit
      }
      // two calpulse found. Should be the correct situation
      if (asic_ref_calpulsenum[asicKey] == 2) {
        digit_ptr[w]->SetTime(digit_ptr[w]->GetTDCBin() - asic_ref_time[asicKey]);
        digit_ptr[w]->SetQuality(asic_ref_time[asicKey]);
        digit_ptr[w]->SetFlag(2000 + digit_ptr[w]->GetFlag()); // good hit
      }
      // more than two calpulse candidates found...
      if (asic_ref_calpulsenum[asicKey] > 2) {
        digit_ptr[w]->SetTime(digit_ptr[w]->GetTDCBin() - asic_ref_time[asicKey]);
        digit_ptr[w]->SetQuality(asic_ref_time[asicKey]);
        digit_ptr[w]->SetFlag(3000 + digit_ptr[w]->GetFlag()); // possibly problematic hit
      }
    }
  }
}



void DoubleCalPulseModule::terminate()
{

}
