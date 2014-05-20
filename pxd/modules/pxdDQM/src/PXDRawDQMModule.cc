/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDRawDQMModule.h>

#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawDQMModule::PXDRawDQMModule() : HistoModule() , m_storeRawPxdrarray() , m_storeRawHits()
{
  //Set module properties
  setDescription("Monitor raw PXD");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void PXDRawDQMModule::defineHisto()
{
  hrawPxdPackets = new TH1F("hrawPxdPackets", "Pxd Raw Packet Nr;Nr per Event", 16, 0, 16);
  hrawPxdPacketSize = new TH1F("hrawPxdPacketSize", "Pxd Raw Packetsize;Words per packet", 1024, 0, 1024);

  hrawPxdHitsCount = new TH1F("hrawPxdCount", "Pxd Raw Count ;Nr per Event", 256, 0, 256);
  for (auto i = 0; i < 10; i++) {
    std::string s = std::to_string(i);
    hrawPxdHits[i]  = new TH2F(("hrawPxdHits" + s).c_str(), ("Pxd Raw Hit Map, Sensor " + s + ";column;row").c_str(), 256, 0, 256, 786, 0, 786);
    hrawPxdHitsCharge[i]  = new TH1F(("hrawPxdHitsCharge" + s).c_str(), ("Pxd Raw Hit Charge, Sensor " + s + ";Charge").c_str(), 256, 0, 256);
    hrawPxdHitsCommonMode[i]  = new TH1F(("hrawPxdHitsCommonMode" + s).c_str(), ("Pxd Raw Hit Common Mode, Sensor " + s + ";Value").c_str(), 256, 0, 256);
    hrawPxdHitsTimeWindow[i]  = new TH1F(("hrawPxdHitsTimeWindow" + s).c_str(), ("Pxd Raw Hit Time Window (framenr*1024-startrow), Sensor " + s + ";Time [a.u.]").c_str(), 8192, -1024, 8192 - 1024);
  }
}

void PXDRawDQMModule::initialize()
{
  REG_HISTOGRAM
}

void PXDRawDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  hrawPxdPackets->Reset();
  hrawPxdPacketSize->Reset();
  hrawPxdHitsCount->Reset();
  for (int i = 0; i < 10; i++) {
    hrawPxdHits[i]->Reset();
    hrawPxdHitsCharge[i]->Reset();
    hrawPxdHitsCommonMode[i]->Reset();
    hrawPxdHitsTimeWindow[i]->Reset();
  }
}

void PXDRawDQMModule::event()
{
  hrawPxdPackets->Fill(m_storeRawPxdrarray.getEntries());

  for (auto & it : m_storeRawPxdrarray) {
    hrawPxdPacketSize->Fill(it.size());
  }

  hrawPxdHitsCount->Fill(m_storeRawHits.getEntries());

  for (auto & it : m_storeRawHits) {
    int i;
    i = it.getSensorID() % 10;
    hrawPxdHits[i]->Fill(it.getColumn(), it.getRow());
    hrawPxdHitsCharge[i]->Fill(it.getCharge());
    hrawPxdHitsCommonMode[i]->Fill(it.getCommonMode());
    hrawPxdHitsTimeWindow[i]->Fill(it.getFrameNr() * 1024 - it.getStartRow());
  }
}
