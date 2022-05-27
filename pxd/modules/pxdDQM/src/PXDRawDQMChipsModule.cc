/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDRawDQMChipsModule.h>
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
REG_MODULE(PXDRawDQMChips);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawDQMChipsModule::PXDRawDQMChipsModule() : HistoModule(), m_storeRawHits()
{
  //Set module properties
  setDescription("Monitor raw PXD");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdraw"));

  addParam("PXDRawHitsName", m_storeRawHitsName, "The name of the StoreArray of PXDRawHits to be processed", string(""));
}

void PXDRawDQMChipsModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }


  for (auto i = 0; i < 64; i++) {
    auto layer = (((i >> 5) & 0x1) + 1);
    auto ladder = ((i >> 1) & 0xF);
    auto sensor = ((i & 0x1) + 1);

    // Check if sensor exist
    if (Belle2::VXD::GeoCache::getInstance().validSensorID(Belle2::VxdID(layer, ladder, sensor))) {
      for (auto j = 0; j < eNumSwitcher; j++) {
        for (auto k = 0; k < eNumDCD; k++) {
          string s = str(format("Sensor %d:%d:%d (DHH ID %02Xh) Switcher %d DCD %d") % layer % ladder % sensor % i % j % k);
          string s2 = str(format("_%d.%d.%d_%d_%d") % layer % ladder % sensor % j % k);

          hrawPxdHitsCount[i][j][k] = new TH1F(("hrawPxdCount" + s2).c_str(), ("Pxd Raw Count " + s + ";Nr per Event").c_str(), 8192, 0,
                                               8192);
          hrawPxdHitsCharge[i][j][k] = new TH1F(("hrawPxdHitsCharge" + s2).c_str(),
                                                ("Pxd Raw Hit Charge, " + s + ";Charge").c_str(), 256, 0, 256);
        }
      }
    } else {
      for (auto j = 0; j < eNumSwitcher; j++) {
        for (auto k = 0; k < eNumDCD; k++) {
          hrawPxdHitsCount[i][j][k] =  NULL;
          hrawPxdHitsCharge[i][j][k] =  NULL;
        }
      }
    }
  }

  // cd back to root directory
  oldDir->cd();
}

void PXDRawDQMChipsModule::initialize()
{
  REG_HISTOGRAM
  m_storeRawHits.isRequired(m_storeRawHitsName);
}

void PXDRawDQMChipsModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  for (auto i = 0; i < eNumSensors; i++) {
    for (auto j = 0; j < eNumSwitcher; j++) {
      for (auto k = 0; k < eNumDCD; k++) {
        if (hrawPxdHitsCount[i][j][k]) hrawPxdHitsCount[i][j][k]->Reset();
        if (hrawPxdHitsCharge[i][j][k]) hrawPxdHitsCharge[i][j][k]->Reset();
      }
    }
  }
}

void PXDRawDQMChipsModule::event()
{
  uint nhits[eNumSensors][eNumSwitcher][eNumDCD] = {};

  for (auto& it : m_storeRawHits) {
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
    int switcher = it.getRow() / 128; // TODO here must be: function for REAL inverse mapping
    int dcd = it.getColumn() / 64; // TODO here must be: function for REAL inverse mapping
    // TODO check switcher 0-6? DCD 0-4?
    nhits[dhh_id][switcher][dcd]++;
    if (hrawPxdHitsCharge[dhh_id][switcher][dcd]) hrawPxdHitsCharge[dhh_id][switcher][dcd]->Fill(it.getCharge());
  }
  for (auto i = 0; i < eNumSensors; i++) {
    for (auto j = 0; j < eNumSwitcher; j++) {
      for (auto k = 0; k < eNumDCD; k++) {
        if (hrawPxdHitsCount[i][j][k]) hrawPxdHitsCount[i][j][k]->Fill(nhits[i][j][k]);
      }
    }
  }
}
