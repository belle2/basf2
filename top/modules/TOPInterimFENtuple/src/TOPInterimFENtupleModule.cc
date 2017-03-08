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
  m_tree->Branch("time", m_time, "time[nHit]/F");
  m_tree->Branch("rawTdc", m_rawTdc, "rawTdc[nHit]/F");
  m_tree->Branch("refTdc", m_refTdc, "refTdc[nHit]/F");
  m_tree->Branch("height", m_height, "height[nHit]/F");
  m_tree->Branch("q", m_q, "q[nHit]/F");
  m_tree->Branch("width", m_width, "width[nHit]/F");
  m_tree->Branch("nPixelRawInRawDigit", &m_nPixelInRawDigit, "nPixelInRawDigit/S");

  m_tree->Branch("nFEHeader", &m_nFEHeader, "nFEHeader/S");
  m_tree->Branch("nEmptyFEHeader", &m_nEmptyFEHeader, "nEmptyFEHeader/S");
  m_tree->Branch("nWaveform", &m_nWaveform, "nWaveform/S");
  m_tree->Branch("errorFlag", &m_errorFlag, "errorFlag/i");
}

void TOPInterimFENtupleModule::beginRun() {}

void TOPInterimFENtupleModule::event()
{

  m_nHit = 0;
  m_nPixelInRawDigit = 0;
  m_nFEHeader = 0;
  m_nEmptyFEHeader = 0;
  m_nWaveform = 0;
  m_errorFlag = 0;

  StoreObjPtr<EventMetaData> EventMetaDataPtr;
  StoreArray<TOPRawDigit> rawDigits;
  StoreArray<TOPDigit> digits;

  std::map<short, float> refTdcMap;
  std::map<int, short> channelIdMap;
  UInt_t EventNum = EventMetaDataPtr->getEvent();
  for (const auto& digit : digits) {
    m_slotNum[m_nHit] = digit.getModuleID();
    m_pixelId[m_nHit] = (short)digit.getPixelID();
    m_isCalCh[m_nHit] = (digit.getASICChannel() == m_calibrationChannel);
    m_eventNum[m_nHit] = EventNum;
    m_winNum[m_nHit] = (short)digit.getFirstWindow();
    m_time[m_nHit] = digit.getTime();//Not yet available
    m_rawTdc[m_nHit] = digit.getTDC() / 16.;
    m_height[m_nHit] = digit.getADC();
    m_q[m_nHit] = digit.getIntegral();
    m_width[m_nHit] = digit.getPulseWidth();

    //    const auto* rawDigit = digit.getRelated<TOPRawDigit>();
    //    if( rawDigit ){
    //    }

    if (m_isCalCh[m_nHit]) {
      short reducedPixelId = (m_pixelId[m_nHit] - 1) / 8;
      if (refTdcMap.count(reducedPixelId) == 0)
        refTdcMap[reducedPixelId] = m_rawTdc[m_nHit];
    }
    m_nHit++;
  }

  for (int iHit = 0 ; iHit < m_nHit ; iHit++) {
    short reducedPixelId = (m_pixelId[iHit] - 1) / 8;
    if (refTdcMap.count(reducedPixelId) > 0)
      m_refTdc[iHit] = refTdcMap[reducedPixelId];
    else m_refTdc[iHit] = -99999;
  }

  for (const auto& rawDigit : rawDigits) {
    auto scrodID = rawDigit.getScrodID();
    //int moduleID = 0;
    int boardstack = 4;
    const auto* feemap = TOP::TOPGeometryPar::Instance()->getFrontEndMapper().getMap(scrodID);
    if (feemap) {
      //moduleID = feemap->getModuleID();
      boardstack = feemap->getBoardstackNumber();
    } else {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
    }
    const auto& chMapper = TOP::TOPGeometryPar::Instance()->getChannelMapper();
    Int_t channelNum = chMapper.getChannel(boardstack, rawDigit.getCarrierNumber(), rawDigit.getASICNumber(),
                                           rawDigit.getASICChannel());
    if (channelIdMap.count(channelNum) == 0) channelIdMap[channelNum] = 0;
    else channelIdMap[channelNum]++;
  }
  m_nPixelInRawDigit = channelIdMap.size();

  StoreArray<TOPInterimFEInfo> infos;
  for (const auto& info : infos) {
    m_nFEHeader += info.getFEHeadersCount();
    m_nEmptyFEHeader += info.getEmptyFEHeadersCount();
    m_nWaveform += info.getWaveformsCount();
    m_errorFlag |= info.getErrorFlags();
  }

  m_tree->Fill();
}

void TOPInterimFENtupleModule::endRun() {}

void TOPInterimFENtupleModule::terminate() {}
