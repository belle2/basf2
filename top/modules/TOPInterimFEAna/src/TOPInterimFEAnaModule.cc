/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <top/modules/TOPInterimFEAna/TOPInterimFEAnaModule.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>


#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <iostream>
#include <sstream>

#include <set>
#include <map>
#include <TMath.h>

using namespace std;
using namespace Belle2;

REG_MODULE(TOPInterimFEAna)


TOPInterimFEAnaModule::TOPInterimFEAnaModule() : HistoModule()
{
  setDescription("TOP data analysis module for Interim FE firmware (since Jan, 2017)");

  addParam("CalCh", m_CalCh, "asic channel # where calibration pulses routed", (Short_t)0);
  //addParam("TimePerWin", m_TimePerWin, "time interval of a single window (=64 samples) [ns]", (Float_t)23.581939);
}

TOPInterimFEAnaModule::~TOPInterimFEAnaModule() {}

void TOPInterimFEAnaModule::initialize()
{
  REG_HISTOGRAM;
}

void TOPInterimFEAnaModule::defineHisto()
{

  m_tree = new TTree("tree", "TTree for generator output");

  m_tree->Branch("nHit", &m_nHit, "nHit/I");
  m_tree->Branch("SlotNum", m_SlotNum, "SlotNum[nHit]/S");
  m_tree->Branch("PixelId", m_PixelId, "PixelId[nHit]/S");
  m_tree->Branch("IsCalCh", m_IsCalCh, "IsCalCh[nHit]/O");
  m_tree->Branch("EventNum", m_EventNum, "EventNum[nHit]/i");
  m_tree->Branch("WinNum", m_WinNum, "WinNum[nHit]/S");
  m_tree->Branch("time", m_time, "time[nHit]/F");
  m_tree->Branch("TdcRaw", m_TdcRaw, "TdcRaw[nHit]/F");
  m_tree->Branch("RefTdc", m_RefTdc, "RefTdc[nHit]/F");
  m_tree->Branch("height", m_height, "height[nHit]/F");
  m_tree->Branch("q", m_q, "q[nHit]/F");
  m_tree->Branch("width", m_width, "width[nHit]/F");
  m_tree->Branch("nPixelProcessed", &m_nPixelProcessed, "nPixelProcessed/S");
}

void TOPInterimFEAnaModule::beginRun() {}

void TOPInterimFEAnaModule::event()
{

  m_nHit = 0;
  m_nPixelProcessed = 0;

  // Get List of MCParticles
  StoreObjPtr<EventMetaData> EventMetaDataPtr;
  StoreArray<TOPRawDigit> TOPRawDigitsPtr;
  StoreArray<TOPDigit> TOPDigitsPtr;

  std::map<Short_t, Float_t> RefTimeMap;
  std::map<Int_t, Short_t> ChannelIdMap;
  UInt_t nTOPRawDigits = TOPRawDigitsPtr.getEntries();
  UInt_t nTOPDigits = TOPDigitsPtr.getEntries();
  UInt_t EventNum = EventMetaDataPtr->getEvent();
  for (UInt_t iDigit = 0 ; iDigit < nTOPDigits ; iDigit++) {

    TOPDigit* digi = TOPDigitsPtr[iDigit];
    //"digi->getASICChannel()" does not seem to be working properly
    m_SlotNum[m_nHit] = digi->getModuleID();
    m_PixelId[m_nHit] = (Short_t)digi->getPixelID();
    m_IsCalCh[m_nHit] = IsCalCh(m_PixelId[m_nHit]);  //(digi->getASICChannel()==m_CalCh);
    m_EventNum[m_nHit] = EventNum;
    m_WinNum[m_nHit] = (Short_t)digi->getFirstWindow();
    m_time[m_nHit] = digi->getTime();// + m_WinNum[m_nHit] * m_TimePerWin;
    m_TdcRaw[m_nHit] = digi->getTDC() / 16.;
    m_height[m_nHit] = digi->getADC();
    m_q[m_nHit] = digi->getIntegral();
    m_width[m_nHit] = digi->getPulseWidth();

    if (m_IsCalCh[m_nHit]) {
      Short_t ReducedPixelId = (m_PixelId[m_nHit] - 1) / 8;
      if (RefTimeMap.count(ReducedPixelId) == 0)
        RefTimeMap[ReducedPixelId] = m_TdcRaw[m_nHit];
    }
    m_nHit++;
  }

  for (Int_t iHit = 0 ; iHit < m_nHit ; iHit++) {
    Short_t ReducedPixelId = (m_PixelId[iHit] - 1) / 8;
    if (RefTimeMap.count(ReducedPixelId) > 0)
      m_RefTdc[iHit] = RefTimeMap[ReducedPixelId];
    else m_RefTdc[iHit] = -99999;
  }


  for (UInt_t iDigit = 0 ; iDigit < nTOPRawDigits ; iDigit++) {
    TOPRawDigit* rawdigi = TOPRawDigitsPtr[iDigit];
    Int_t ChannelNum = rawdigi->getScrodID() * 1000 + rawdigi->getCarrierNumber() * 32
                       + rawdigi->getASICNumber() * 8 + rawdigi->getASICChannel();
    if (ChannelIdMap.count(ChannelNum) == 0) ChannelIdMap[ChannelNum] = 0;
    else ChannelIdMap[ChannelNum]++;
  }
  m_nPixelProcessed = ChannelIdMap.size();

  m_tree->Fill();
}

void TOPInterimFEAnaModule::endRun() {}

void TOPInterimFEAnaModule::terminate() {}

Bool_t TOPInterimFEAnaModule::IsCalCh(Short_t PixelId)
{

  if ((((PixelId - 1) / 64) % 2) == 0) {
    if (((PixelId - 1) % 8) == m_CalCh) return true;
  } else if (((PixelId - 1) % 8) == (7 - m_CalCh)) return true;

  return false;
}
