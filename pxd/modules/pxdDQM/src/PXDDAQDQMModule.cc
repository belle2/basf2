/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDDAQDQMModule.h>
#include <vxd/geometry/GeoCache.h>

#include "TDirectory.h"
#include <TAxis.h>
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;
using namespace Belle2::VXD;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDAQDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDAQDQMModule::PXDDAQDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("Monitor DAQ errors");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdDAQ"));
}

void PXDDAQDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());

  hDAQErrorEvent = new TH1F("PXDDAQError", "PXDDAQError/Event;Error Bit;Count", 64, 0, 64);
  hDAQErrorDHC = new TH2F("PXDDAQDHCError", "PXDDAQError/DHC;DHC ID;Error Bit", 6, 0, 6, 64, 0, 64);
  hDAQErrorDHE = new TH2F("PXDDAQDHEError", "PXDDAQError/DHE;DHE ID;Error Bit", 64, 0, 64, 64, 0, 64);

  for (auto i = 0; i < 64; i++) {
    auto num1 = (((i >> 5) & 0x1) + 1);
    auto num2 = ((i >> 1) & 0xF);
    auto num3 = ((i & 0x1) + 1);

    // Check if sensor exist and Fill Label on Axis
    if (Belle2::VXD::GeoCache::getInstance().validSensorID(Belle2::VxdID(num1, num2, num3))) {
      //cppcheck-suppress zerodiv
      string s = str(format("DHE %d:%d:%d (DHH ID %02Xh)") % num1 % num2 % num3 % i);
      //cppcheck-suppress zerodiv
      string s2 = str(format("_%d.%d.%d") % num1 % num2 % num3);
    }
  }
//   hDAQErrorEvent->LabelsDeflate("X");
//   hDAQErrorEvent->LabelsOption("v");
//   hDAQErrorEvent->SetStats(0);

  // cd back to root directory
  oldDir->cd();
}

void PXDDAQDQMModule::initialize()
{
  REG_HISTOGRAM
  m_storeDAQEvtStats.isRequired();
}

void PXDDAQDQMModule::beginRun()
{
  hDAQErrorEvent->Reset();
  hDAQErrorDHC->Reset();
  hDAQErrorDHE->Reset();
}

void PXDDAQDQMModule::event()
{
  B2DEBUG(20, "Iterate PXD DAQ Status");
  for (auto& evt : m_storeDAQEvtStats) {
    PXDErrorFlags evt_emask = evt.getErrorMask();
    for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
      PXDErrorFlags mask = (1ull << i);
      if ((evt_emask & mask) == mask) hDAQErrorEvent->Fill(i);
    }
    B2DEBUG(20, "Iterate PXD Packets, Err " << evt_emask);
    for (auto && pkt = evt.pkt_begin(); pkt < evt.pkt_end(); pkt++) {
      B2DEBUG(20, "Iterate PXD DHC in Pkt " << pkt->getPktIndex());
      for (auto && dhc = pkt->dhc_begin(); dhc < pkt->dhc_end(); dhc++) {
        PXDErrorFlags dhc_emask = dhc->getErrorMask();
        for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
          PXDErrorFlags mask = (1ull << i);
          if ((dhc_emask & mask) == mask) hDAQErrorDHC->Fill(dhc->getDHCID(), i);
        }
        B2DEBUG(20, "Iterate PXD DHE in DHC " << dhc->getDHCID() << " , Err " << dhc_emask);
        for (auto && dhe = dhc->dhe_begin(); dhe < dhc->dhe_end(); dhe++) {
          PXDErrorFlags dhe_emask = dhe->getErrorMask();
          B2DEBUG(20, "DHE " << dhe->getDHEID() << " , Err " << dhe_emask);
          for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
            PXDErrorFlags mask = (1ull << i);
            if ((dhe_emask & mask) == mask) hDAQErrorDHE->Fill(dhe->getDHEID(), i);
          }
        }
      }
    }
  }
}
