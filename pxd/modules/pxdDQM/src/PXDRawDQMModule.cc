/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDRawDQMModule.h>
#include <vxd/geometry/GeoCache.h>

#include <TDirectory.h>
#include <boost/format.hpp>
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::VXD;

using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawDQM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawDQMModule::PXDRawDQMModule() : HistoModule(), m_storeRawPxdrarray(), m_storeRawHits(), m_storeRawAdcs()
{
  //Set module properties
  setDescription("Monitor raw PXD");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdraw"));

  addParam("RawPXDsName", m_storeRawPxdrarrayName, "The name of the StoreArray of RawPXDs to be processed", string(""));
  addParam("PXDRawHitsName", m_storeRawHitsName, "The name of the StoreArray of PXDRawHits to be processed", string(""));
  addParam("PXDRawAdcsName", m_storeRawAdcsName, "The name of the StoreArray of PXDRawAdcs to be processed", string(""));
}

void PXDRawDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  hrawPxdPackets = new TH1F("hrawPxdPackets", "Pxd Raw Packet Nr;Nr per Event", 16, 0, 16);
  hrawPxdPacketSize = new TH1F("hrawPxdPacketSize", "Pxd Raw Packetsize;Words per packet", 1024, 0, 1024);
  hrawPxdHitMapAll = new TH2F("hrawPxdHitMapAll",
                              "Pxd Raw Hit Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 3700 / 50, 0, 3700, 3500 / 50, 0, 3500);
  // ADC map not supported by DHC anymore ... deactive filling, later remove
  hrawPxdAdcMapAll =
    NULL;// new TH2F("hrawPxdAdcMapAll",                               "Pxd Raw Adc Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 370/*0*/, 0, 3700, 350/*0*/, 0, 3500);

  hrawPxdHitsCount = new TH1F("hrawPxdCount", "Pxd Raw Count ;Nr per Event", 8192, 0, 8192);
  for (auto i = 0; i < 64; i++) {
    auto layer = (((i >> 5) & 0x1) + 1);
    auto ladder = ((i >> 1) & 0xF);
    auto sensor = ((i & 0x1) + 1);

    // Check if sensor exist
    if (Belle2::VXD::GeoCache::getInstance().validSensorID(Belle2::VxdID(layer, ladder, sensor))) {
      string s = str(format("Sensor %d:%d:%d (DHH ID %02Xh)") % layer % ladder % sensor % i);
      string s2 = str(format("_%d.%d.%d") % layer % ladder % sensor);

      hrawPxdHitMap[i] = new TH2F(("hrawPxdHitMap" + s2).c_str(),
                                  ("Pxd Raw Hit Map, " + s + ";column;row").c_str(), 250,
                                  0, 250, 768, 0, 768);
      hrawPxdChargeMap[i] = new TH2F(("hrawPxdChargeMap" + s2).c_str(),
                                     ("Pxd Raw Charge Map, " + s + ";column;row").c_str(), 250, 0, 250, 768, 0, 768);
      hrawPxdHitsCharge[i] = new TH1F(("hrawPxdHitsCharge" + s2).c_str(),
                                      ("Pxd Raw Hit Charge, " + s + ";Charge").c_str(), 256, 0, 256);
      hrawPxdHitTimeWindow[i] = new TH1F(("hrawPxdHitTimeWindow" + s2).c_str(),
                                         ("Pxd Raw Hit Time Window (framenr*192-gate_of_hit), " + s + ";Time [a.u.]").c_str(), 2048, -256, 2048 - 256);
      hrawPxdGateTimeWindow[i] = new TH1F(("hrawPxdGateTimeWindow" + s2).c_str(),
                                          ("Pxd Raw Gate Time Window (framenr*192-triggergate_of_hit), " + s + ";Time [a.u.]").c_str(), 2048, -256, 2048 - 256);
    } else {
      hrawPxdHitMap[i] = NULL;
      hrawPxdChargeMap[i] = NULL;
      hrawPxdHitsCharge[i] =  NULL;
      hrawPxdHitTimeWindow[i] = NULL;
      hrawPxdGateTimeWindow[i] = NULL;
    }
  }

  // cd back to root directory
  oldDir->cd();
}

void PXDRawDQMModule::initialize()
{
  REG_HISTOGRAM
  m_storeRawPxdrarray.isOptional(m_storeRawPxdrarrayName);
  m_storeRawHits.isRequired(m_storeRawHitsName);
  m_storeRawAdcs.isRequired(m_storeRawAdcsName);
  m_storeDAQEvtStats.isRequired();
}

void PXDRawDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  if (hrawPxdPackets) hrawPxdPackets->Reset();
  if (hrawPxdPacketSize) hrawPxdPacketSize->Reset();
  if (hrawPxdHitsCount) hrawPxdHitsCount->Reset();
  if (hrawPxdHitMapAll) hrawPxdHitMapAll->Reset();
  if (hrawPxdAdcMapAll) hrawPxdAdcMapAll->Reset();
  for (int i = 0; i < 64; i++) {
    if (hrawPxdHitMap[i]) hrawPxdHitMap[i]->Reset();
    if (hrawPxdChargeMap[i]) hrawPxdChargeMap[i]->Reset();
    if (hrawPxdHitsCharge[i]) hrawPxdHitsCharge[i]->Reset();
    if (hrawPxdHitTimeWindow[i]) hrawPxdHitTimeWindow[i]->Reset();
    if (hrawPxdGateTimeWindow[i]) hrawPxdGateTimeWindow[i]->Reset();
  }
}

void PXDRawDQMModule::event()
{
  hrawPxdPackets->Fill(m_storeRawPxdrarray.getEntries());

  for (auto& it : m_storeRawPxdrarray) {
    if (hrawPxdPacketSize) hrawPxdPacketSize->Fill(it.size());
  }

  if (hrawPxdHitsCount) hrawPxdHitsCount->Fill(m_storeRawHits.getEntries());

  for (auto& it : m_storeRawHits) {

    VxdID currentVxdId = it.getSensorID();
    auto layer = currentVxdId.getLayerNumber();/// 1 ... 2
    auto ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
    auto sensor = currentVxdId.getSensorNumber();/// 1 ... 2

    // Get startrow and DheID from DAQEvtStats
    const PXDDAQDHEStatus* dhe = (*m_storeDAQEvtStats).findDHE(currentVxdId);
    if (dhe == nullptr) {
      B2ERROR("No DHE found for SensorId: " << currentVxdId);
      continue;
    }

    auto dhh_id = dhe->getDHEID();
    auto startGate = dhe->getTriggerGate();

    if (dhh_id == 0 || dhh_id >= 64) {
      B2ERROR("SensorId (DHH ID) out of range: " << dhh_id);
      continue;
    }
    if (hrawPxdHitMap[dhh_id]) hrawPxdHitMap[dhh_id]->Fill(it.getColumn(), it.getRow());
    if (hrawPxdHitMapAll) hrawPxdHitMapAll->Fill(it.getColumn() + ladder * 300 - 200,
                                                   100 + it.getRow() + 850 * (layer + layer + sensor - 3));
    if (hrawPxdChargeMap[dhh_id]) hrawPxdChargeMap[dhh_id]->Fill(it.getColumn(), it.getRow(), it.getCharge());
    if (hrawPxdHitsCharge[dhh_id]) hrawPxdHitsCharge[dhh_id]->Fill(it.getCharge());
    // Is this histogram necessary? we are folding with occupancy of sensor hits here
    // Think about 1024*framenr-hit_row?
    if (hrawPxdHitTimeWindow[dhh_id]) hrawPxdHitTimeWindow[dhh_id]->Fill(it.getFrameNr() * 192 - it.getRow() / 4);
    if (hrawPxdGateTimeWindow[dhh_id]) hrawPxdGateTimeWindow[dhh_id]->Fill(it.getFrameNr() * 192 - startGate);
  }

  if (hrawPxdAdcMapAll) {
    for (auto& it : m_storeRawAdcs) {
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
      const unsigned char* data = &it.getData()[0];
      for (int row = 0; row < 768; row++) {
        for (int col = 0; col < 64; col++) {
          hrawPxdAdcMapAll->Fill(col + chip_offset + ladder * 300 - 200, 100 + row + 850 * (layer + layer + sensor - 3), *(data++));
        }
      }
    }
  }
}
