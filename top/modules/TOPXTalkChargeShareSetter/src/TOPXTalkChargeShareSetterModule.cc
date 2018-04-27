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
#include <top/modules/TOPXTalkChargeShareSetter/TOPXTalkChargeShareSetterModule.h>

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
  addParam("sumChargeShare", m_sumChargeShare,
           "sum up charge of PrimaryChargeShare and SecondaryChargeShare", (bool)false);
  addParam("timeCut", m_timeCut,
           "cut range of hittiming for chargeshare flag [ns]", (float) 1);
  addParam("preValleyDepthLoose", m_preValleyDepthLoose,
           "loose threshold for depth of pre valley [ADC counts], for corss talk identification", 20);
  addParam("preValleyDepthTight", m_preValleyDepthTight,
           "tight threshold for depth of pre valley [ADC counts], identified as cross talk with loose threshold for the second peak amplitude",
           50);
  addParam("2ndPeakAmplitudeLoose", m_2ndPeakAmplitudeLoose,
           "loose threshold for amplitude of the second peak [ADC counts] for cross talk identification. Defined as ADC count difference between the valley just after the main peak and the second peak, Used when the \"preValleyDepthTight\" was satisfied",
           30);
  addParam("2ndPeakAmplitudeRatioTight", m_2ndPeakAmplitudeRatioTight,
           "tight threshold for amplitude ratio of the second peak to the main peak height [ADC counts]", 0.2);
  addParam("nSampleBefore", m_nSampleBefore,
           "the number of samples by which the pre-valley should exist from the CFD timing, used for cross talk identification", 5);
  addParam("nSampleAfter", m_nSampleAfter,
           "the number of samples by which the second peak should exist from the CFD timing, used for cross talk identification", 10);

}

TOPXTalkChargeShareSetterModule::~TOPXTalkChargeShareSetterModule() {}

void TOPXTalkChargeShareSetterModule::initialize()
{
  StoreArray<TOPDigit> digits;
  digits.isRequired();

  if (m_preValleyDepthLoose < 0 || m_preValleyDepthTight < 0 || m_nSampleBefore < 0) m_checkPreValleyForXTalkId = false;
  if (m_2ndPeakAmplitudeLoose < 0 || m_2ndPeakAmplitudeRatioTight < 0 || m_nSampleAfter < 0) m_checkPostValleyForXTalkId = false;
}

void TOPXTalkChargeShareSetterModule::beginRun()
{
}

void TOPXTalkChargeShareSetterModule::event()
{
  StoreArray<TOPDigit> digits;

  //Set Cross talk events
  for (auto& digit : digits) {

    if (digit.getHitQuality() == TOPDigit::c_Junk) continue;

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



  //Set Charge Share events
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

    int adjacentPixelIds[] = { pixelId - 1 - c_NPixelsPerRow, pixelId - c_NPixelsPerRow, pixelId + 1 - c_NPixelsPerRow, pixelId + 1,
                               pixelId + 1 + c_NPixelsPerRow, pixelId + c_NPixelsPerRow, pixelId - 1 + c_NPixelsPerRow, pixelId - 1
                             };

    for (const auto& adjacentPixelId : adjacentPixelIds) {
      if (adjacentPixelId > 0 && adjacentPixelId < 512) {
        int globalPixelId = (slotId - 1) * 512 + adjacentPixelId - 1;

        while (hitInfoMap.count(globalPixelId) > 0 && !digit.isSecondaryChargeShare()) {
          float adjacentIntegral = hitInfoMap[globalPixelId]->getIntegral();

          if (pmtId != hitInfoMap[globalPixelId]->getPMTNumber()
              or TMath::Abs(hitTime - hitInfoMap[globalPixelId]->getTime()) > m_timeCut) {globalPixelId += 10000; continue;}

          if (charge > adjacentIntegral) {
            digit.setPrimaryChargeShare();
            if (m_sumChargeShare) {
              digit.setIntegral(digit.getIntegral() + adjacentIntegral);
              digit.setPulseHeight(digit.getPulseHeight() + hitInfoMap[globalPixelId]->getPulseHeight());
            }
          } else if (charge < adjacentIntegral) {
            digit.setSecondaryChargeShare();
            if (m_sumChargeShare) {
              digit.setIntegral(0.0);
              digit.setPulseHeight(0.0);
            }
          } else if (charge == adjacentIntegral && pixelId > hitInfoMap[globalPixelId]->getPixelID()) {
            digit.setPrimaryChargeShare();
            if (m_sumChargeShare) {
              digit.setIntegral(digit.getIntegral() + adjacentIntegral);
              digit.setPulseHeight(digit.getPulseHeight() + hitInfoMap[globalPixelId]->getPulseHeight());
            }
          } else {
            digit.setSecondaryChargeShare();
            if (m_sumChargeShare) {
              digit.setIntegral(0.0);
              digit.setPulseHeight(0.0);
            }
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


bool TOPXTalkChargeShareSetterModule::isCrossTalk(std::vector<short> wfm, int iRawTime, short height)
{

  int nWfmSampling = wfm.size();
  B2INFO("TOPXTalkChargeShareSetter : size = " << nWfmSampling);
  for (int iWin = 0 ; iWin < 16 ; iWin++) {
    //int jRawTime = iRawTime + (iWin - 20) * (TOPRawDigit::c_WindowSize)/4;//scan offset by 16-sample step
    int jRawTime = iRawTime - TMath::FloorNint(iRawTime / (TOPRawDigit::c_WindowSize)) * (TOPRawDigit::c_WindowSize) +
                   (TOPRawDigit::c_WindowSize) / 4 * iWin;
    if (jRawTime > 0 &&  jRawTime < nWfmSampling - 1)
      B2INFO("TOPXTalkChargeShareSetter : wfm[ " << jRawTime << "]= " << wfm[jRawTime] << ", iRawTime = " << iRawTime << ", height = " <<
             height);
    if (jRawTime > 0 && jRawTime < nWfmSampling - 1 && wfm[jRawTime] < height / 2. && wfm[jRawTime + 1] > height / 2.) {
      B2INFO("TOPXTalkChargeShareSetter : offset is found iWin = " << iWin);
      bool preValleyExist = false;
      short preValleyDepth = -1;
      if (!m_checkPreValleyForXTalkId) preValleyExist = true;
      else {
        for (int iSample = jRawTime ; iSample - 1 > 0 ; iSample--) {
          if (jRawTime - iSample > m_nSampleBefore) return false;
          else if (wfm[iSample] - wfm[iSample - 1] >= 0) continue;
          else {
            preValleyDepth = (-1) * wfm[iSample];
            if (preValleyDepth < m_preValleyDepthLoose) return false;
            else {
              if (!m_checkPostValleyForXTalkId) return true;
              preValleyExist = true;
              break;
            }
          }
        }//for( iSample )
      }//if( m_checkPreValleyForXTalkId )
      if (!preValleyExist) return false;

      //check ringing (oscillation pattern) in trailing edge
      short sign = 1;
      short valley_adc = 9999;
      for (int jSample = jRawTime ; jSample < nWfmSampling - 1 ; jSample++) {
        if (jSample - jRawTime > m_nSampleAfter) return false;
        if ((wfm[jSample + 1] - wfm[jSample])*sign > 0) continue;
        else { //when peak or valley is found
          if (sign < 0 && valley_adc > height + 1) //in case a valley is found
            valley_adc = wfm[jSample];
          if (sign > 0 && valley_adc < height) {//in case of second peak
            if (wfm[jSample] - valley_adc > (height * m_2ndPeakAmplitudeRatioTight)
                || (preValleyDepth > m_preValleyDepthTight
                    && (wfm[jSample] - valley_adc) > m_2ndPeakAmplitudeLoose))
              return true;
            else return false;
          }
          sign = (sign > 0 ? -1 : 1);
        }
      }//for( jSample0 )
      return false;
    }//if( jRawTime )
  }//for( iWin )

  return false;
}


