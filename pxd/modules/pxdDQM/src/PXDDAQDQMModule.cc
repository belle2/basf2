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

#include <TDirectory.h>
#include <TAxis.h>
#include <boost/format.hpp>
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
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  hDAQErrorEvent = new TH1D("PXDDAQError", "PXDDAQError/Event;;Count", ONSEN_USED_TYPE_ERR, 0, ONSEN_USED_TYPE_ERR);
  hDAQErrorDHC = new TH2D("PXDDAQDHCError", "PXDDAQError/DHC;DHC ID;", 16, 0, 16, ONSEN_USED_TYPE_ERR, 0, ONSEN_USED_TYPE_ERR);
  hDAQErrorDHE = new TH2D("PXDDAQDHEError", "PXDDAQError/DHE;DHE ID;", 64, 0, 64, ONSEN_USED_TYPE_ERR, 0, ONSEN_USED_TYPE_ERR);
  hDAQUseableModule = new TH1D("PXDDAQUseableModule", "PXDDAQUseableModule/DHE;DHE ID;", 64, 0, 64);
  hDAQNotUseableModule = new TH1D("PXDDAQNotUseableModule", "PXDDAQNotUseableModule/DHE;DHE ID;", 64, 0, 64);
  hDAQDHPDataMissing = new TH1D("PXDDAQDHPDataMissing", "PXDDAQDHPDataMissing/DHE*DHP;DHE ID;", 64 * 4, 0, 64);
  hDAQEndErrorDHC = new TH2D("PXDDAQDHCEndError", "PXDDAQEndError/DHC;DHC ID;", 16, 0, 16, 32, 0, 32);
  hDAQEndErrorDHE = new TH2D("PXDDAQDHEEndError", "PXDDAQEndError/DHE;DHE ID;", 64, 0, 64, 4 * 2 * 8, 0, 4 * 2 * 8);

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

    hDAQDHETriggerGate[avxdid] = new TH1D("PXDDAQDHETriggerGate_" + bufful,
                                          "TriggerGate DHE " + buff + "; Trigger Gate; Counts", 192, 0, 192);
    hDAQDHEReduction[avxdid] = new TH1D("PXDDAQDHEDataReduction_" + bufful, "Data Reduction DHE " + buff + "; Raw/Red; Counts", 200, 0,
                                        40);// If max changed, check overflow copy below
    hDAQCM[avxdid] = new TH2D("PXDDAQCM_" + bufful, "Common Mode on DHE " + buff + "; Gate+Chip*192; Common Mode", 192 * 4, 0, 192 * 4,
                              64, 0, 64);
    hDAQCM2[avxdid] = new TH1D("PXDDAQCM2_" + bufful, "Common Mode on DHE " + buff + "; Common Mode", 64, 0, 64);
  }
  for (int i = 0; i < 16; i++) {
    hDAQDHCReduction[i] = new TH1D(("PXDDAQDHCDataReduction_" + str(format("%d") % i)).c_str(),
                                   ("Data Reduction DHC " + str(format(" %d") % i) + "; Raw/Red; Counts").c_str(), 200, 0,
                                   40); // If max changed, check overflow copy below
  }
//   hDAQErrorEvent->LabelsDeflate("X");
//   hDAQErrorEvent->LabelsOption("v");
//   hDAQErrorEvent->SetStats(0);
  hCM63AfterInjLER  = new TH1I("PXDCM63InjLER", "PXDCM63InjLER/Time;Time in #mus;Events/Time (5 #mus bins)", 4000, 0, 20000);
  hCM63AfterInjHER  = new TH1I("PXDCM63InjHER", "PXDCM63InjHER/Time;Time in #mus;Events/Time (5 #mus bins)", 4000, 0, 20000);
  hTruncAfterInjLER  = new TH1I("PXDTruncInjLER", "PXDTruncInjLER/Time;Time in #mus;Events/Time (5 #mus bins)", 4000, 0, 20000);
  hTruncAfterInjHER  = new TH1I("PXDTruncInjHER", "PXDTruncInjHER/Time;Time in #mus;Events/Time (5 #mus bins)", 4000, 0, 20000);

  // cd back to root directory
  oldDir->cd();
}

void PXDDAQDQMModule::initialize()
{
  REG_HISTOGRAM
  m_storeDAQEvtStats.isRequired();
  m_rawTTD.isOptional(); /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failing
}

void PXDDAQDQMModule::beginRun()
{
  hDAQErrorEvent->Reset();
  hDAQErrorDHC->Reset();
  hDAQErrorDHE->Reset();
  hDAQUseableModule->Reset();
  hDAQNotUseableModule->Reset();
  hDAQDHPDataMissing->Reset();
  hDAQEndErrorDHC->Reset();
  hDAQEndErrorDHE->Reset();
  for (auto& it : hDAQDHETriggerGate) if (it.second) it.second->Reset();
  for (auto& it : hDAQDHCReduction) if (it.second) it.second->Reset();
  for (auto& it : hDAQDHEReduction) if (it.second) it.second->Reset();
  for (auto& it : hDAQCM) if (it.second) it.second->Reset();
  for (auto& it : hDAQCM2) if (it.second) it.second->Reset();
  if (hCM63AfterInjLER) hCM63AfterInjLER->Reset();
  if (hCM63AfterInjHER) hCM63AfterInjHER->Reset();
  if (hTruncAfterInjLER) hTruncAfterInjLER->Reset();
  if (hTruncAfterInjHER) hTruncAfterInjHER->Reset();
}

void PXDDAQDQMModule::event()
{
  hDAQDHPDataMissing->Fill(-1); // to normalize to the number of events
  hDAQErrorDHC->Fill(-1, -1); // to normalize to the number of events
  hDAQErrorDHE->Fill(-1, -1); // to normalize to the number of events
  for (auto& it : hDAQCM2) it.second->Fill(-1); // to normalize to the number of events
  /// An Error Flag can only be set, if the object actually exists,
  /// thus we have to check for a difference to the number of events, too
  /// Remark: for HLT event selection and/or events rejected by the event-
  /// of-doom-buster, we might count anyhow broken events as broken from PXD

  bool truncFlag = false; // flag events which are DHE truncated
  bool cm63Flag = false; // flag event which are CM63 truncated

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
      hDAQErrorDHC->Fill(dhc.getDHCID(), -1);// normalize
      PXDErrorFlags dhc_emask = dhc.getErrorMask();
      for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
        PXDErrorFlags mask = (1ull << i);
        if ((dhc_emask & mask) == mask) hDAQErrorDHC->Fill(dhc.getDHCID(), i);
      }
      unsigned int cmask = dhc.getEndErrorInfo();
      for (int i = 0; i < 32; i++) {
        unsigned int mask = (1 << i);
        if ((cmask & mask) == mask) hDAQEndErrorDHC->Fill(dhc.getDHCID(), i);
      }
      if (hDAQDHCReduction[dhc.getDHCID()]) {
        float red = dhc.getRedCnt() ? float(dhc.getRawCnt()) / dhc.getRedCnt() : 0.;
        B2DEBUG(98, "==DHC " << dhc.getDHCID() << "(Raw)" << dhc.getRawCnt() << " / (Red)" << dhc.getRedCnt() << " = " << red);
        if (red >= 40.) red = 39.999999999; // Bad, bad workaround. but we want to see the overflows
        hDAQDHCReduction[dhc.getDHCID()]->Fill(red);
      }
      B2DEBUG(20, "Iterate PXD DHE in DHC " << dhc.getDHCID() << " , Err " << dhc_emask);
      for (auto& dhe : dhc) {
        hDAQErrorDHE->Fill(dhe.getDHEID(), -1);// normalize
        PXDErrorFlags dhe_emask = dhe.getErrorMask();
        B2DEBUG(20, "DHE " << dhe.getDHEID() << " , Err " << dhe_emask);
        for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
          PXDErrorFlags mask = (1ull << i);
          if ((dhe_emask & mask) == mask) hDAQErrorDHE->Fill(dhe.getDHEID(), i);
        }
        if (dhe.isUsable()) {
          hDAQUseableModule->Fill(dhe.getDHEID());
        } else {
          hDAQNotUseableModule->Fill(dhe.getDHEID());
        }
        for (int i = 0; i < 4; i++) {
          if ((dhe.getDHPFoundMask() & (1 << i)) == 0) hDAQDHPDataMissing->Fill(dhe.getDHEID() + i * 0.25);
        }
        unsigned int emask = dhe.getEndErrorInfo();
        truncFlag |= emask != 0; // lets make it simple, catch any DHE SM error
        for (int i = 0; i < 4 * 2; i++) {
          auto sm = (emask >> i * 4) & 0xF;
          if (sm >= 8) sm = 7; // clip unknow to 7, as value >6 undefined for now
          if (sm > 0) hDAQEndErrorDHE->Fill(dhe.getDHEID(), i * 8 + sm); // we dont want to fill noerror=0
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
          if (hDAQCM2[dhe.getSensorID()]) hDAQCM2[dhe.getSensorID()]->Fill(std::get<2>(*cm));
          cm63Flag |= 63 == std::get<2>(*cm);
        }
      }
    }
  }
  // Now fill the histograms which need flags set above
  // the code is unluckily a copy of whats in PXDInjection Module, but therewe dont have the DAQ flags :-/
  for (auto& it : m_rawTTD) {
//     B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
//             (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
//             it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

    // get last injection time
    auto difference = it.GetTimeSinceLastInjection(0);
    // check time overflow, too long ago
    if (difference != 0x7FFFFFFF) {
      float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
      if (it.GetIsHER(0)) {
        if (hCM63AfterInjHER && cm63Flag) hCM63AfterInjHER->Fill(diff2);
        if (hTruncAfterInjHER && truncFlag) hTruncAfterInjHER->Fill(diff2);
      } else {
        if (hCM63AfterInjLER && cm63Flag) hCM63AfterInjLER->Fill(diff2);
        if (hTruncAfterInjLER && truncFlag) hTruncAfterInjLER->Fill(diff2);
      }
    }
    break;
  }
}
