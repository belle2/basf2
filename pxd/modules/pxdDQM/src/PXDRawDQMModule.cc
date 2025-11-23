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
  // ADC map not supported by DHC anymore ... deactivate filling, later remove
  hrawPxdAdcMapAll =
    NULL;// new TH2F("hrawPxdAdcMapAll",                               "Pxd Raw Adc Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 370/*0*/, 0, 3700, 350/*0*/, 0, 3500);

  hrawPxdHitsCount = new TH1F("hrawPxdCount", "Pxd Raw Count ;Nr per Event", 8192, 0, 8192);

  VXD::GeoCache& vxdGeometry(GeoCache::getInstance());
  std::vector<VxdID> sensors = vxdGeometry.getListOfSensors();
  for (const VxdID& avxdid : sensors) {
    const auto&  info = vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    //Only interested in PXD sensors
    TString buff = (std::string)avxdid;
    TString buffus = buff;
    buffus.ReplaceAll(".", "_");

    m_hrawPxdHitMap[avxdid] = new TH2F(("hrawPxdHitMap_" + buffus),
                                       ("Pxd Raw Hit Map, " + buff + ";column;row"), 250,
                                       0, 250, 768, 0, 768);
    m_hrawPxdChargeMap[avxdid] = new TH2F(("hrawPxdChargeMap_" + buffus),
                                          ("Pxd Raw Charge Map, " + buff + ";column;row"), 250, 0, 250, 768, 0, 768);
    m_hrawPxdHitsCharge[avxdid] = new TH1F(("hrawPxdHitsCharge_" + buffus),
                                           ("Pxd Raw Hit Charge, " + buff + ";Charge"), 256, 0, 256);
    m_hrawPxdHitTimeWindow[avxdid] = new TH1F(("hrawPxdHitTimeWindow_" + buffus),
                                              ("Pxd Raw Hit Time Window (framenr*192-gate_of_hit), " + buff + ";Time [a.u.]"), 2048, -256, 2048 - 256);
    m_hrawPxdGateTimeWindow[avxdid] = new TH1F(("hrawPxdGateTimeWindow_" + buffus),
                                               ("Pxd Raw Gate Time Window (framenr*192-triggergate_of_hit), " + buff + ";Time [a.u.]"), 2048, -256, 2048 - 256);
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

  for (auto& h : m_hrawPxdHitMap) if (h.second) h.second->Reset();
  for (auto& h : m_hrawPxdChargeMap) if (h.second) h.second->Reset();
  for (auto& h : m_hrawPxdHitsCharge) if (h.second) h.second->Reset();
  for (auto& h : m_hrawPxdHitTimeWindow) if (h.second) h.second->Reset();
  for (auto& h : m_hrawPxdGateTimeWindow) if (h.second) h.second->Reset();
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
    auto startGate = dhe->getTriggerGate();

    if (hrawPxdHitMapAll) hrawPxdHitMapAll->Fill(it.getColumn() + ladder * 300 - 200,
                                                   100 + it.getRow() + 850 * (layer + layer + sensor - 3));

    if (m_hrawPxdHitMap[currentVxdId]) m_hrawPxdHitMap[currentVxdId]->Fill(it.getColumn(), it.getRow());
    if (m_hrawPxdChargeMap[currentVxdId]) m_hrawPxdChargeMap[currentVxdId]->Fill(it.getColumn(), it.getRow(), it.getCharge());
    if (m_hrawPxdHitsCharge[currentVxdId]) m_hrawPxdHitsCharge[currentVxdId]->Fill(it.getCharge());
    // Is this histogram necessary? we are folding with occupancy of sensor hits here
    // Think about 1024*framenr-hit_row?
    if (m_hrawPxdHitTimeWindow[currentVxdId]) m_hrawPxdHitTimeWindow[currentVxdId]->Fill(it.getFrameNr() * 192 - it.getRow() / 4);
    if (m_hrawPxdGateTimeWindow[currentVxdId]) m_hrawPxdGateTimeWindow[currentVxdId]->Fill(it.getFrameNr() * 192 - startGate);
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
