/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rikuya Okuto                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//own include
#include <top/modules/TOPXTalkChargeShareSetter/include/TOPXTalkChargeShareSetterModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/dataobjects/EventMetaData.h>

#include <top/geometry/TOPGeometryPar.h>

using namespace std;

using namespace Belle2;

using namespace TOP;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TOPXTalkChargeShareSetter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TOPXTalkChargeShareSetterModule::TOPXTalkChargeShareSetterModule() : Module()
{
  // Set module properties
  setDescription("Crosstalk & chargeshare flag setter");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameter
  addParam("outputDigitsName", m_outputDigitsName,
           "name of TOPDigit store array", string(""));

  addParam("timeCut", m_timeCut,
           "cut range of hittiming for chargeshare flag [ns]", (float) 1);
  addParam("preValleyDepthLow", m_preValleyDepthLow,
           "loose threshold for depth of pre valley [ADC counts], for corss talk identification", 20);
  addParam("preValleyDepthHigh", m_preValleyDepthHigh,
           "tight threshold for depth of pre valley [ADC counts], identified as cross talk with loose threshold for the second peak amplitude",
           50);
  addParam("2ndPeakAmplitudeLow", m_2ndPeakAmplitudeLow,
           "loose threshold for amplitude of the second peak [ADC counts] for cross talk identification. Defined as ADC count difference between the valley just after the main peak and the second peak, Used when the \"preValleyDepthHigh\" was satisfied",
           30);
  addParam("2ndPeakAmplitudeRatioHigh", m_2ndPeakAmplitudeRatioHigh,
           "tight threshold for amplitude ratio of the second peak to the main peak height [ADC counts]", 0.2);
  addParam("nSampleBefore", m_nSampleBefore,
           "the number of samples by which the pre-valley should exist from the CFD timing, used for cross talk identification", 5);
  addParam("nSampleAfter", m_nSampleAfter,
           "the number of samples by which the second peak should exist from the CFD timing, used for cross talk identification", 10);

}

TOPXTalkChargeShareSetterModule::~TOPXTalkChargeShareSetterModule()
{
}

void TOPXTalkChargeShareSetterModule::initialize()
{
}

void TOPXTalkChargeShareSetterModule::beginRun()
{
}

void TOPXTalkChargeShareSetterModule::event()
{
  m_digits.registerInDataStore(m_outputDigitsName);

  for (auto& digit : m_digits) {

    if (digit.getHitQuality() != TOPDigit::c_Good) continue;

    const auto* rawDigit = digit.getRelated<TOPRawDigit>();
    if (!rawDigit) continue;

    const auto* waveform = rawDigit->getRelated<TOPRawWaveform>();
    if (waveform and isCrossTalk(waveform->getWaveform(), TMath::FloorNint(digit.getRawTime()), digit.getPulseHeight()))
      digit.setHitQuality(TOPDigit::c_CrossTalk);
    else {
      //identify a ringing which follows a cross talk hit in the same channle as cross talk
      int slotId = digit.getModuleID();
      unsigned int channelId = digit.getChannel();
      double rawTime = digit.getRawTime();
      for (auto& digit2 : digits) {
        if (digit2.getChannel() != channelId || digit2.getModuleID() != slotId) continue;
        if (digit2.getHitQuality() == TOPDigit::c_CrossTalk
            and TMath::Abs(rawTime - digit2.getRawTime() - m_nCrossTalkRingingSamples / 2.) < m_nCrossTalkRingingSamples / 2.)
          digit.setHitQuality(TOPDigit::c_CrossTalk);
      }//for(digit2)
    }//if(waveform and isCrossTalk) else
  }//for(digit)

  StoreArray<TOPDigit> digits(m_outputDigitsName);

  std::map<int, TOPDigit*> hitInfoMap;

  for (auto& digit : digits) {

    if (digit.getHitQuality() != TOPDigit::c_Good) continue;

    int globalPixelId = (digit.getModuleID() - 1) * 512 + digit.getPixelID() - 1;
    while (hitInfoMap.count(globalPixelId) > 0)globalPixelId += 10000;
    hitInfoMap[globalPixelId] = &digit;
  }

  for (auto& digit : digits) {

    if (digit.getHitQuality() != TOPDigit::c_Good) continue;

    short pixelId = digit.getPixelID();
    short slotId = digit.getModuleID();
    short pmtId = digit.getPMTNumber();
    double hitTime = digit.getTime();
    double charge = digit.getIntegral();

    int adjacentPixelIds[] = { pixelId - 1 - TOPRawDigit::c_NPixelsPerRow, pixelId - TOPRawDigit::c_NPixelsPerRow, pixelId + 1 - TOPRawDigit::c_NPixelsPerRow, pixelId + 1,
                               pixelId + 1 + TOPRawDigit::c_NPixelsPerRow, pixelId + TOPRawDigit::c_NPixelsPerRow, pixelId - 1 + TOPRawDigit::c_NPixelsPerRow, pixelId - 1
                             };

    for (const auto& adjacentPixelId : adjacentPixelIds) {
      if (adjacentPixelId > 0 && adjacentPixelId < 512) {
        int globalPixelId = (slotId - 1) * 512 + adjacentPixelId - 1;

        while (hitInfoMap.count(globalPixelId) > 0 && digit.getHitTypeFlags() ^ TOPDigit::c_SecondaryChargeShare) {

          if (pmtId != hitInfoMap[globalPixelId]->getPMTNumber()
              or TMath::Abs(hitTime - hitInfoMap[globalPixelId]->getTime()) > m_timeCut) {globalPixelId += 10000; continue;}

          digit.setPrimaryChargeShare();
          if (charge < hitInfoMap[globalPixelId]->getIntegral()) {
            digit.setSecondaryChargeShare();
          }

          globalPixelId += 10000;
        }
      }//for pair adjacent pixels
    }//for pair
  }//for digit pair
}

void TOPXTalkChargeShareSetterModule::endRun()
{
}

void TOPXTalkChargeShareSetterModule::terminate()
{
}


