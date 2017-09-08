/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPInterimFENtuple/TOPInterimFENtupleModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPInterimFEInfo.h>

#include <top/geometry/TOPGeometryPar.h>

#include <iostream>
#include <sstream>

#include <set>
#include <map>
#include <TMath.h>

using namespace std;
using namespace Belle2;

REG_MODULE(TOPInterimFENtuple)


TOPInterimFENtupleModule::TOPInterimFENtupleModule() : HistoModule()
{
  setDescription("TOP data analysis module for Interim FE firmware (since Jan, 2017)");

  addParam("calibrationChannel", m_calibrationChannel, "asic channel # where calibration pulses routed",
           (unsigned)0);
  addParam("saveWaveform", m_saveWaveform, "whether to save waveform data or not",
           (bool)false);
  addParam("useDoublePulse", m_useDoublePulse,
           "set true when you require both of double calibration pulses for reference timing. You need to enable offline feature extraction.",
           (bool)true);
  addParam("averageSamplingRate", m_averageSamplingRate, "sampling rate with assumption of uniform interval in a unit of GHz",
           (float)2.71394);
  addParam("minHeightFirstCalPulse", m_calibrationPulseThreshold1, "pulse height threshold for the first cal. pulse",
           (float)600.);
  addParam("minHeightSecondCalPulse", m_calibrationPulseThreshold1, "pulse height threshold for the second cal. pulse",
           (float)450.);
  addParam("nominalDeltaT", m_calibrationPulseInterval, "nominal DeltaT (time interval of the double calibration pulses) [ns]",
           (float)21.85);
  addParam("nominalDeltaTRange", m_calibrationPulseIntervalRange,
           "acceptable DeltaT shift from the noinal value before calibration [ns]",
           (float)2.);

  //addParam("TimePerWin", m_TimePerWin, "time interval of a single window (=64 samples) [ns]", (Float_t)23.581939);
}

TOPInterimFENtupleModule::~TOPInterimFENtupleModule() {}

void TOPInterimFENtupleModule::initialize()
{
  REG_HISTOGRAM;

  StoreArray<TOPRawDigit> rawDigits;
  rawDigits.isRequired();
  StoreArray<TOPDigit> digits;
  digits.isRequired();
  StoreArray<TOPInterimFEInfo> infos;
  infos.isRequired();
  StoreArray<TOPRawWaveform> waveforms;
  waveforms.isRequired();
}

void TOPInterimFENtupleModule::defineHisto()
{

  m_tree = new TTree("tree", "TTree for generator output");

  m_tree->Branch("nHit", &m_nHit, "nHit/I");
  m_tree->Branch("slotNum", m_slotNum, "slotNum[nHit]/S");
  m_tree->Branch("pixelId", m_pixelId, "pixelId[nHit]/S");
  m_tree->Branch("isCalCh", m_isCalCh, "isCalCh[nHit]/O");
  m_tree->Branch("eventNum", m_eventNum, "eventNum[nHit]/i");
  m_tree->Branch("winNum", m_winNum, "winNum[nHit]/S");
  m_tree->Branch("windowsInOrder", m_windowsInOrder, "windowsInOrder[nHit]/O");
  m_tree->Branch("hitQuality", m_hitQuality, "hitQuality[nHit]/b");
  m_tree->Branch("time", m_time, "time[nHit]/F");
  m_tree->Branch("rawTime", m_rawTime, "rawTime[nHit]/F");
  m_tree->Branch("refTime", m_refTime, "refTime[nHit]/F");
  m_tree->Branch("sample", m_sample, "sample[nHit]/s");
  m_tree->Branch("height", m_height, "height[nHit]/F");
  m_tree->Branch("integral", m_integral, "integral[nHit]/F");
  m_tree->Branch("width", m_width, "width[nHit]/F");
  m_tree->Branch("offlineFlag", m_offlineFlag, "offlineFlag[nHit]/B");
  m_tree->Branch("nHitOfflineFE", m_nHitOfflineFE, "nHitOfflineFE[nHit]/S");
  std::ostringstream brstr[2];
  brstr[0] << "winNumList[nHit][" << c_NWindow << "]/S";
  m_tree->Branch("winNumList", m_winNumList, brstr[0].str().c_str());
  if (m_saveWaveform) {
    brstr[1] << "waveform[nHit][" << c_NWaveformSample << "]/S";
    m_tree->Branch("waveform", m_waveform, brstr[1].str().c_str());
  }

  m_tree->Branch("nFEHeader", &m_nFEHeader, "nFEHeader/S");
  m_tree->Branch("nEmptyFEHeader", &m_nEmptyFEHeader, "nEmptyFEHeader/S");
  m_tree->Branch("nWaveform", &m_nWaveform, "nWaveform/S");
  m_tree->Branch("errorFlag", &m_errorFlag, "errorFlag/i");
  m_tree->Branch("eventErrorFlag", &m_eventErrorFlag, "eventErrorFlag/i");
}

void TOPInterimFENtupleModule::beginRun()
{
}

void TOPInterimFENtupleModule::event()
{

  m_nHit = 0;
  m_nFEHeader = 0;
  m_nEmptyFEHeader = 0;
  m_nWaveform = 0;
  m_errorFlag = 0;

  StoreObjPtr<EventMetaData> EventMetaDataPtr;
  StoreArray<TOPDigit> digits;

  std::map<short, short> nHitOfflineFEMap;
  std::map<short, int> windowNumListMap;
  static std::set<short> noisyChannelBlackListSet;
  UInt_t EventNum = EventMetaDataPtr->getEvent();
  m_eventErrorFlag = EventMetaDataPtr->getErrorFlag();
  for (const auto& digit : digits) {

    if (m_nHit >= c_NMaxHitEvent) {
      B2WARNING("TOPInterimFENtuple : too many hits found (>=" << c_NMaxHitEvent
                << "), EventNo = " << EventNum << ", no more hits are recorded.");
      break;
    }

    m_slotNum[m_nHit] = digit.getModuleID();
    m_pixelId[m_nHit] = (short)digit.getPixelID();
    m_isCalCh[m_nHit] = (digit.getASICChannel() == m_calibrationChannel);
    m_eventNum[m_nHit] = EventNum;
    m_winNum[m_nHit] = (short)digit.getFirstWindow();
    m_hitQuality[m_nHit] = (unsigned char)digit.getHitQuality();
    m_isReallyJunk[m_nHit] = false;
    m_windowsInOrder[m_nHit] = true;
    m_rawTime[m_nHit] = digit.getRawTime();
    m_time[m_nHit] = digit.getTime();
    m_sample[m_nHit] = TMath::FloorNint(m_rawTime[m_nHit] + m_winNum[m_nHit] * c_NSamplePerWindow) % c_NSampleTBC;
    m_height[m_nHit] = digit.getPulseHeight();
    m_integral[m_nHit] = digit.getIntegral();
    m_width[m_nHit] = digit.getPulseWidth();
    for (int iWindow = 0 ; iWindow < c_NWindow ; iWindow++)
      m_winNumList[m_nHit][iWindow] = -32767;
    for (int iSample = 0 ; iSample < c_NWaveformSample ; iSample++)
      m_waveform[m_nHit][iSample] = -32767;

    short globalChannelId = m_pixelId[m_nHit] - 1 + (m_slotNum[m_nHit] - 1) * c_NPixelPerModule;
    if (nHitOfflineFEMap.count(globalChannelId) == 0) nHitOfflineFEMap[globalChannelId] = 0;
    else if (nHitOfflineFEMap[globalChannelId] > c_NMaxHitPerChannel) {
      if (noisyChannelBlackListSet.count(globalChannelId) == 0) {
        noisyChannelBlackListSet.insert(globalChannelId);
        B2WARNING("TOPInterimFENtuple : noisy channel with too many hits (slotNum="
                  << (globalChannelId / c_NPixelPerModule + 1) << ", pixelId = "
                  << (globalChannelId / c_NPixelPerModule + 1) << ") ");
      }
      continue;
    }

    const auto* rawDigit = digit.getRelated<TOPRawDigit>();
    if (rawDigit) {
      m_windowsInOrder[m_nHit] = rawDigit->areWindowsInOrder();
      if (rawDigit->isPedestalJump()) m_isReallyJunk[m_nHit] = true;
      const auto* waveform = rawDigit->getRelated<TOPRawWaveform>();
      if (waveform) {
        if (rawDigit->isMadeOffline()) {

          //fill hit flags ; =1 : first hit from offline FE, =2 : second ... , =0 : online FE, -1 : no waveform
          nHitOfflineFEMap[globalChannelId]++;
          m_offlineFlag[m_nHit] = nHitOfflineFEMap[globalChannelId];
        } else { //hit from online feature extraction
          m_offlineFlag[m_nHit] = 0;

          //store waveform data
          unsigned nSample = TMath::Min((UShort_t)waveform->getSize(), (UShort_t)c_NWaveformSample);
          if (nSample != c_NWaveformSample)
            B2WARNING("TOPInterimFENtuple: unexpected # of samples in TOPRawWaveform : " << nSample);
          for (unsigned iSample = 0 ; iSample < nSample ; iSample++)
            m_waveform[m_nHit][iSample] = waveform->getWaveform()[iSample];

          //store window number
          int iWin = 0;
          windowNumListMap[globalChannelId] = m_nHit;
          for (const auto& window : waveform->getStorageWindows()) {
            if (iWin < c_NWindow)
              m_winNumList[m_nHit][iWin] = window;
            else
              B2WARNING("TOPInterimFENtuple: too many windows were found");

            iWin++;
          }

        }
      }//if( waveform )
      else m_offlineFlag[m_nHit] = -1;
    } else {
      B2WARNING("TOPInterimFENtuple : no TOPRawDigit object found!");
      m_offlineFlag[m_nHit] = -100;
    }//if( rawDigit )


    m_nHit++;
  }//for( const auto& digit : digits )

  for (int iHit = 0 ; iHit < m_nHit ; iHit++) {

    short globalChannelId = m_pixelId[iHit] - 1 + (m_slotNum[iHit] - 1) * c_NPixelPerModule;
    m_nHitOfflineFE[iHit] = nHitOfflineFEMap[globalChannelId];

    //apply correction for discontinuous window number
    short nWindowAdded = m_winNum[iHit];
    if (!m_windowsInOrder[iHit]) {
      if (windowNumListMap.count(globalChannelId) == 0)
        B2WARNING("TOPInterimFENtuple : windoww are not in order, but waveform data is not found!!");
      else {
        short winNumInFE = TMath::FloorNint(m_rawTime[iHit]) / c_NSamplePerWindow;
        int jHit = windowNumListMap[globalChannelId];
        if (winNumInFE < 0 || winNumInFE >= c_NWindow
            || m_winNumList[jHit][winNumInFE] < 0) m_hitQuality[iHit] += 50;

        short nWindowCorrection = (m_winNumList[jHit][winNumInFE] - m_winNumList[jHit][0] - winNumInFE);
        if (nWindowCorrection < 0) nWindowCorrection += c_NWindowRingBuffer;
        nWindowAdded += nWindowCorrection;
      }
    }

    m_time[iHit] += (nWindowAdded * c_NSamplePerWindow / m_averageSamplingRate);
    m_sample[m_nHit] = TMath::FloorNint(m_rawTime[iHit] + nWindowAdded * c_NSamplePerWindow) % c_NSampleTBC;
  }//for(int iHit)

  StoreArray<TOPInterimFEInfo> infos;
  for (const auto& info : infos) {
    m_nFEHeader += info.getFEHeadersCount();
    m_nEmptyFEHeader += info.getEmptyFEHeadersCount();
    m_nWaveform += info.getWaveformsCount();
    m_errorFlag |= info.getErrorFlags();
  }

  //identify cal. pulse timing
  getReferenceTiming();

  m_tree->Fill();
}

void TOPInterimFENtupleModule::endRun()
{
}

void TOPInterimFENtupleModule::terminate()
{
}

void TOPInterimFENtupleModule::getReferenceTiming()
{

  std::map<short, short> iRefHitMap;
  for (int iHit = 0 ; iHit < m_nHit ; iHit++) {
    if (!m_isCalCh[iHit]) continue;
    short reducedPixelId = (m_pixelId[iHit] - 1) / 8 + 100 * m_slotNum[iHit];
    if (iRefHitMap.count(reducedPixelId) > 0) continue;

    if (!m_useDoublePulse) {
      iRefHitMap[reducedPixelId] = iHit;
      continue;
    }

    std::vector<short> iHitVec;
    iHitVec.push_back(iHit);
    for (int jHit = iHit + 1 ; jHit < m_nHit ; jHit++) {
      short jReducedPixelId = (m_pixelId[jHit] - 1) / 8 + 100 * m_slotNum[jHit];
      if (m_isCalCh[jHit] && jReducedPixelId == reducedPixelId) iHitVec.push_back(jHit);
    }

    int nCands = 0;
    for (unsigned int iVec = 0 ; iVec < iHitVec.size() ; iVec++) {
      int jHit = iHitVec[iVec];
      for (unsigned int jVec = iVec + 1 ; jVec < iHitVec.size() ; jVec++) {
        int kHit = iHitVec[jVec];
        float timediff = m_time[kHit] - m_time[jHit];
        if (m_height[jHit] > m_calibrationPulseThreshold1
            && m_height[kHit] > m_calibrationPulseThreshold2
            && TMath::Abs(timediff - m_calibrationPulseInterval) < m_calibrationPulseIntervalRange) {
          if (nCands == 0) {
            iRefHitMap[reducedPixelId] = jHit;
            m_hitQuality[jHit] += 100;
            m_hitQuality[kHit] += 200;
          }
          nCands++;
        }//satisfy selection criteria for calibration signal
      }
    }//iVec (finish selecting a calibration signal candidate)
  }


  //loop all hits again to fill the array "refTime"
  for (int iHit = 0 ; iHit < m_nHit ; iHit++) {
    short reducedPixelId = (m_pixelId[iHit] - 1) / 8 + 100 * m_slotNum[iHit];
    if (iRefHitMap.count(reducedPixelId) > 0) {
      int iRef = iRefHitMap[reducedPixelId];
      m_refTime[iHit] = m_time[iRef];
      if (!m_isReallyJunk[iHit] && m_hitQuality[iRef] >= 100) {
        m_hitQuality[iHit] += 10;
      }
    } else m_refTime[iHit] = -99999;
  }

  return;
}
