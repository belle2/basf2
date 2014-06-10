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
  hrawPxdHitMapAll  = new TH2F("hrawPxdHitMapAll", "Pxd Raw Hit Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 370, 0, 3700, 350, 0, 3500);
  hrawPxdAdcMapAll  = new TH2F("hrawPxdAdcMapAll", "Pxd Raw Adc Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 3700, 0, 3700, 3500, 0, 3500);
  hrawPxdPedestalMapAll  = new TH2F("hrawPxdPedestalMapAll", "Pxd Raw Pedestal Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 3700, 0, 3700, 3500, 0, 3500);

  hrawPxdHitsCount = new TH1F("hrawPxdCount", "Pxd Raw Count ;Nr per Event", 8192, 0, 8192);
  for (auto i = 0; i < 64; i++) {
    std::string s = boost::str(boost::format("Sensor %d:%d:%d (DHH ID $%02X)") % (((i >> 5) & 0x1) + 1) % ((i >> 1) & 0xF) % ((i & 0x1) + 1) % i);

    hrawPxdHitMap[i]  = new TH2F(("hrawPxdHitMap" + s).c_str(), ("Pxd Raw Hit Map, " + s + ";column;row").c_str(), 256, 0, 256, 786, 0, 786);
    hrawPxdChargeMap[i]  = new TH2F(("hrawPxdChargeMap" + s).c_str(), ("Pxd Raw Charge Map, " + s + ";column;row").c_str(), 256, 0, 256, 786, 0, 786);
    hrawPxdHitsCharge[i]  = new TH1F(("hrawPxdHitsCharge" + s).c_str(), ("Pxd Raw Hit Charge, " + s + ";Charge").c_str(), 256, 0, 256);
    hrawPxdHitsCommonMode[i]  = new TH1F(("hrawPxdHitsCommonMode" + s).c_str(), ("Pxd Raw Hit Common Mode, " + s + ";Value").c_str(), 256, 0, 256);
    hrawPxdHitsTimeWindow[i]  = new TH1F(("hrawPxdHitsTimeWindow" + s).c_str(), ("Pxd Raw Hit Time Window (framenr*1024-startrow), " + s + ";Time [a.u.]").c_str(), 8192, -1024, 8192 - 1024);
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
  hrawPxdHitMapAll->Reset();
  hrawPxdAdcMapAll->Reset();
  hrawPxdPedestalMapAll->Reset();
  for (int i = 0; i < 64; i++) {
    hrawPxdHitMap[i]->Reset();
    hrawPxdChargeMap[i]->Reset();
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
    int dhh_id;
    // calculate DHH id from Vxd Id
    unsigned int layer, ladder, sensor;//, segment;
    VxdID currentVxdId;
    currentVxdId = it.getSensorID();
    layer = currentVxdId.getLayerNumber();/// 1 ... 2
    ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
    sensor = currentVxdId.getSensorNumber();/// 1 ... 2
    // segment = currentVxdId.getSegmentNumber();// Frame nr? ... ignore
    dhh_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
    if (dhh_id <= 0 || dhh_id >= 64) {
      B2ERROR("SensorId (DHH ID) out of range: " << dhh_id);
      continue;
    }
    hrawPxdHitMap[dhh_id]->Fill(it.getColumn(), it.getRow());
    hrawPxdHitMapAll->Fill(it.getColumn() + ladder * 300 - 200, 100 + it.getRow() + 850 * (layer + layer + sensor - 3));
    hrawPxdChargeMap[dhh_id]->Fill(it.getColumn(), it.getRow(), it.getCharge());
    hrawPxdHitsCharge[dhh_id]->Fill(it.getCharge());
    hrawPxdHitsCommonMode[dhh_id]->Fill(it.getCommonMode());
    hrawPxdHitsTimeWindow[dhh_id]->Fill(it.getFrameNr() * 1024 - it.getStartRow());
  }

  for (auto & it : m_storeRawAdcs) {
    int dhh_id;
    // calculate DHH id from Vxd Id
    unsigned int layer, ladder, sensor;//, segment;
    VxdID currentVxdId;
    currentVxdId = it.getSensorID();
    layer = currentVxdId.getLayerNumber();/// 1 ... 2
    ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
    sensor = currentVxdId.getSensorNumber();/// 1 ... 2
    // segment = currentVxdId.getSegmentNumber();// Frame nr? ... ignore
    dhh_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
    if (dhh_id <= 0 || dhh_id >= 64) {
      B2ERROR("SensorId (DHH ID) out of range: " << dhh_id);
      continue;
    }

    unsigned int chip_offset;
    chip_offset = it.getChip() * 64;
    const unsigned char* data = it.getData();
    for (int row = 0; row < 786; row++) {
      for (int col = 0; col < 64; col++) {
        hrawPxdAdcMapAll->Fill(col + chip_offset + ladder * 300 - 200, 100 + row + 850 * (layer + layer + sensor - 3), *(data++));
      }
    }
  }
  for (auto & it : m_storeRawPedestals) {
    int dhh_id;
    // calculate DHH id from Vxd Id
    unsigned int layer, ladder, sensor;//, segment;
    VxdID currentVxdId;
    currentVxdId = it.getSensorID();
    layer = currentVxdId.getLayerNumber();/// 1 ... 2
    ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
    sensor = currentVxdId.getSensorNumber();/// 1 ... 2
    // segment = currentVxdId.getSegmentNumber();// Frame nr? ... ignore
    dhh_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
    if (dhh_id <= 0 || dhh_id >= 64) {
      B2ERROR("SensorId (DHH ID) out of range: " << dhh_id);
      continue;
    }

    const unsigned char* data = it.getData();
    unsigned int chip_offset;
    chip_offset = it.getChip() * 64;
    for (int row = 0; row < 786; row++) {
      for (int col = 0; col < 64; col++) {
        hrawPxdPedestalMapAll->Fill(col + chip_offset + ladder * 300 - 200, 100 + row + 850 * (layer + layer + sensor - 3), *(data++));
      }
    }
  }
}
