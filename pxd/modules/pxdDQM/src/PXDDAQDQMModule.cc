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
//#include <vxd/geometry/SensorInfoBase.h>

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

PXDDAQDQMModule::PXDDAQDQMModule() : HistoModule() , m_vxdGeometry(VXD::GeoCache::getInstance())
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

  hDAQErrorEvent = new TH1F("PXDDAQError", "PXDDAQError/Event;;Count", ONSEN_USED_TYPE_ERR, 0, ONSEN_USED_TYPE_ERR);
  hDAQErrorDHC = new TH2F("PXDDAQDHCError", "PXDDAQError/DHC;DHC ID;", 16, 0, 16, ONSEN_USED_TYPE_ERR, 0, ONSEN_USED_TYPE_ERR);
  hDAQErrorDHE = new TH2F("PXDDAQDHEError", "PXDDAQError/DHE;DHE ID;", 64, 0, 64, ONSEN_USED_TYPE_ERR, 0, ONSEN_USED_TYPE_ERR);

  // histograms might get unreadable, but, if necessary, you can zoom in anyways.
  // we could use full alphanumeric histograms, but then, the labels would change (in the worst case) depending on observed errors
  // and ... the histogram would contain NO labels if there is NO error ... confusing.
  // ... an we would have to use alphanumeric X axis (DHE ID, DHC ID), too)
  for (int i = 0; i < ONSEN_USED_TYPE_ERR; i++) {
    const char* label = getPXDBitErrorName(i).c_str();
    hDAQErrorEvent->GetXaxis()->SetBinLabel(i + 1, label);
    hDAQErrorDHE->GetYaxis()->SetBinLabel(i + 1, label);
    hDAQErrorDHC->GetYaxis()->SetBinLabel(i + 1, label);
  }

  hDAQErrorEvent->LabelsOption("v"); // rotate the labels.

  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    //Only interested in PXD sensors

    TString buff = (std::string)avxdid;
    TString bufful = buff;
    buff.ReplaceAll(".", "_");

//     string s = str(format("DHE %d:%d:%d (DHH ID %02Xh)") % num1 % num2 % num3 % i);
//     string s2 = str(format("_%d.%d.%d") % num1 % num2 % num3);

    hDAQDHETriggerGate[avxdid] = new TH1F("PXDDAQDHETriggerRowOffset_" + bufful,
                                          "TriggerRowOffset DHE " + buff + "; Trigger Gate; Counts", 192, 0, 192);
    hDAQDHEReduction[avxdid] = new TH1F("PXDDAQDHEDataReduction_" + bufful, "Data Reduction DHE " + buff + "; Raw/Red; Counts", 200, 0,
                                        40);// If max changed, check overflow copy below
    hDAQCM[avxdid] = new TH2F("PXDDAQCM_" + bufful, "Common Mode on DHE " + buff + "; Gate+Chip*192; Common Mode", 192 * 4, 0, 192 * 4,
                              64, 0, 64);
  }
  for (int i = 0; i < 16; i++) {
    //cppcheck-suppress zerodiv
    hDAQDHCReduction[i] = new TH1F(("PXDDAQDHCDataReduction_" + str(format("%d") % i)).c_str(),
                                   ("Data Reduction DHC " + str(format(" %d") % i) + "; Raw/Red; Counts").c_str(), 200, 0,
                                   40); // If max changed, check overflow copy below
    //cppcheck-suppress zerodiv
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
  auto evt = *m_storeDAQEvtStats;
  PXDErrorFlags evt_emask = evt.getErrorMask();
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
    PXDErrorFlags mask = (1ull << i);
    if ((evt_emask & mask) == mask) hDAQErrorEvent->Fill(getPXDBitErrorName(i).c_str(), 1);
  }
  B2DEBUG(20, "Iterate PXD Packets, Err " << evt_emask);
  for (auto& pkt : evt) {
    B2DEBUG(20, "Iterate PXD DHC in Pkt " << pkt.getPktIndex());
    for (auto& dhc : pkt) {
      PXDErrorFlags dhc_emask = dhc.getErrorMask();
      for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
        PXDErrorFlags mask = (1ull << i);
        if ((dhc_emask & mask) == mask) hDAQErrorDHC->Fill(dhc.getDHCID(), i);
      }
      if (hDAQDHCReduction[dhc.getDHCID()]) {
        float red = dhc.getRedCnt() ? float(dhc.getRawCnt()) / dhc.getRedCnt() : 0.;
        B2DEBUG(98, "==DHC " << dhc.getDHCID() << "(Raw)" << dhc.getRawCnt() << " / (Red)" << dhc.getRedCnt() << " = " << red);
        if (red >= 40.) red = 39.999999999; // Bad, bad workaround. but we want to see the overflows
        hDAQDHCReduction[dhc.getDHCID()]->Fill(red);
      }
      B2DEBUG(20, "Iterate PXD DHE in DHC " << dhc.getDHCID() << " , Err " << dhc_emask);
      for (auto& dhe : dhc) {
        PXDErrorFlags dhe_emask = dhe.getErrorMask();
        B2DEBUG(20, "DHE " << dhe.getDHEID() << " , Err " << dhe_emask);
        for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
          PXDErrorFlags mask = (1ull << i);
          if ((dhe_emask & mask) == mask) hDAQErrorDHE->Fill(dhe.getDHEID(), i);
        }

        if (hDAQDHETriggerGate[dhe.getSensorID()]) hDAQDHETriggerGate[dhe.getSensorID()]->Fill(dhe.getTriggerGate());
        if (hDAQDHEReduction[dhe.getSensorID()]) {
          float red = dhe.getRedCnt() ? float(dhe.getRawCnt()) / dhe.getRedCnt() : 0.;
          B2DEBUG(98, "==DHE " << dhe.getSensorID() << "(Raw)" << dhe.getRawCnt() << " / (Red)" << dhe.getRedCnt() << " = " << red);
          if (red >= 40.) red = 39.999999999; // Bad, bad workaround. but we want to see the overflows
          hDAQDHEReduction[dhe.getSensorID()]->Fill(red);
        }
        for (auto cm = dhe.cm_begin(); cm < dhe.cm_end(); ++cm) {
          // uint8_t, uint16_t, uint8_t ; tuple of Chip ID (2 bit), Row (10 bit), Common Mode (6 bit)
          if (hDAQCM[dhe.getSensorID()]) hDAQCM[dhe.getSensorID()]->Fill(std::get<0>(*cm) * 192 + std::get<1>(*cm) / 4, std::get<2>(*cm));
        }
      }
    }
  }
}
