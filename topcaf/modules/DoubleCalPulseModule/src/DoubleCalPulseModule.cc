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
  addParam("forcePolarityConsistency", m_usePolarity, "Requires the two calibration pulses to have the same polarity", true);

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
  StoreArray<TOPCAFDigit> digitPtr;
  digitPtr.isRequired();

  StoreObjPtr<EventHeaderPacket> evtheaderPtr;
  evtheaderPtr.isRequired();

  double asicRefTime[1000];
  int asicRefCalpulseNum[1000];
  int firstCalpulsePolarity = 1.;

  // sets the number of calpulses to 0 at the beginning
  for (int k = 0; k < 1000; k++)
    asicRefCalpulseNum[k] = 0;


  if (digitPtr) {
    cout << "new event" << endl;
    // first loop to look for the pulse candidates
    for (int w = 0; w < digitPtr.getEntries(); w++) {

      int asicKey = 100 * digitPtr[w]->GetBoardstack() + 10 * digitPtr[w]->GetCarrier() + digitPtr[w]->GetASIC();
      int asic_ch = digitPtr[w]->GetASICChannel();
      digitPtr[w]->SetFlag(0); // unprocessed hit has flag = 0
      //First applies the width selection and the channel ID requirement
      if (digitPtr[w]->GetWidth() > m_wMin && digitPtr[w]->GetWidth() < m_wMax && asic_ch == m_calCh) {
        // we don't distinguish bewteen positive and negative candidates, but we use different thresholds
        if ((digitPtr[w]->GetADCHeight() > m_adcMin && digitPtr[w]->GetADCHeight() < m_adcMax)
            || (digitPtr[w]->GetADCHeight() > m_adcMinNeg && digitPtr[w]->GetADCHeight() < m_adcMaxNeg)) {
          // if this is the first calpulse candidate, then use it as reference
          if (asicRefCalpulseNum[asicKey] == 0) {
            asicRefTime[asicKey] = digitPtr[w]->GetTDCBin();
            firstCalpulsePolarity = digitPtr[w]->GetADCHeight();
          }
          // requires all the calpulses to have the same polarity
          if (m_usePolarity) {
            if (digitPtr[w]->GetADCHeight()*firstCalpulsePolarity > 0) {
              asicRefCalpulseNum[asicKey]++;
              digitPtr[w]->SetFlag(100 + asicRefCalpulseNum[asicKey]);
            }
          } else {
            asicRefCalpulseNum[asicKey]++;
            digitPtr[w]->SetFlag(100 + asicRefCalpulseNum[asicKey]);
          }
        }
      }
    }


    //Loop again to apply calibration times
    for (int w = 0; w < digitPtr.getEntries(); w++) {

      int asicKey = 100 * digitPtr[w]->GetBoardstack() + 10 * digitPtr[w]->GetCarrier() + digitPtr[w]->GetASIC();

      // no calibration pulse has been found!
      if (asicRefCalpulseNum[asicKey] == 0) {
        digitPtr[w]->SetTime(0);
        digitPtr[w]->SetQuality(0);
        digitPtr[w]->SetFlag(-1000 - digitPtr[w]->GetFlag()); // not calibrated hit
      }
      // only one calpulse candidate found
      if (asicRefCalpulseNum[asicKey] == 1) {
        digitPtr[w]->SetTime(digitPtr[w]->GetTDCBin() - asicRefTime[asicKey]);
        digitPtr[w]->SetQuality(asicRefTime[asicKey]);
        digitPtr[w]->SetFlag(1000 + digitPtr[w]->GetFlag()); // possibly problematic hit
      }
      // two calpulse found. Should be the correct situation
      if (asicRefCalpulseNum[asicKey] == 2) {
        digitPtr[w]->SetTime(digitPtr[w]->GetTDCBin() - asicRefTime[asicKey]);
        digitPtr[w]->SetQuality(asicRefTime[asicKey]);
        digitPtr[w]->SetFlag(2000 + digitPtr[w]->GetFlag()); // good hit
      }
      // more than two calpulse candidates found...
      if (asicRefCalpulseNum[asicKey] > 2) {
        digitPtr[w]->SetTime(digitPtr[w]->GetTDCBin() - asicRefTime[asicKey]);
        digitPtr[w]->SetQuality(asicRefTime[asicKey]);
        digitPtr[w]->SetFlag(3000 + digitPtr[w]->GetFlag()); // possibly problematic hit
      }
    }
  }
}



void DoubleCalPulseModule::terminate()
{

}
