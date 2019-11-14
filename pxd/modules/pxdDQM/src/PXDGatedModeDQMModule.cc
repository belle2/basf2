/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDGatedModeDQMModule.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::VXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDGatedModeDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDGatedModeDQMModule::PXDGatedModeDQMModule() : HistoModule() , m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Set module properties
  setDescription("Monitor GatedMode");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDINJ"));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "Name of PXD raw hits", std::string(""));
  addParam("perGate", m_perGate, "Make plots per GM Start Gate", true);
}

void PXDGatedModeDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory

  hBunchInjHER = new TH1F("hBunchInjHER", "Last Inj Bunch HER", 4096, 0, 4095);
  hBunchInjLER = new TH1F("hBunchInjLER", "Last Inj Bunch LER", 4096, 0, 4095);
  hBunchTrg = new TH1F("hBunchTrg", "Triggered Bunch", 4096, 0, 4095);


  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    // Only interested in PXD sensors

    TString buff = (std::string)avxdid;
    TString bufful = buff;
    bufful.ReplaceAll(".", "_");

    for (int rgate = m_perGate ? 0 : 96; rgate <= 96; rgate++) { // 96 is no gating
      hGatedModeMapLER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapLER_%d_", rgate) + bufful,
                                                                 Form("PXDGatedModeMapLER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);
      hGatedModeMapHER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapHER_%d_", rgate) + bufful,
                                                                 Form("PXDGatedModeMapHER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);
      hGatedModeMapCutLER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapCutLER_%d_", rgate) + bufful,
          Form("PXDGatedModeMapCutLER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);
      hGatedModeMapCutHER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapCutHER_%d_", rgate) + bufful,
          Form("PXDGatedModeMapCutHER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);

      hGatedModeMapADCLER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapADCLER_%d_", rgate) + bufful,
          Form("PXDGatedModeMapADCLER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);
      hGatedModeMapADCHER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapADCHER_%d_", rgate) + bufful,
          Form("PXDGatedModeMapADCHER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);
      hGatedModeMapCutADCLER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapCutADCLER_%d_", rgate) + bufful,
          Form("PXDGatedModeMapCutADCLER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);
      hGatedModeMapCutADCHER[std::make_pair(avxdid, rgate)] = new TH2F(Form("PXDGatedModeMapCutADCHER_%d_", rgate) + bufful,
          Form("PXDGatedModeMapCutADCHER %d ", rgate) + buff + ";U;V", 25, 0, 250, 192, 0, 768);
    }
    hGatedModeProjLER[avxdid] = new TH2F("PXDGatedModeProjLER_" + bufful,
                                         "PXDGatedModeProjLER " + buff + ";Gate;V", 96, 0, 96, 192, 0, 768);
    hGatedModeProjHER[avxdid] = new TH2F("PXDGatedModeProjHER_" + bufful,
                                         "PXDGatedModeProjHER " + buff + ";Gate;V", 96, 0, 96, 192, 0, 768);

    hGatedModeMapSubLER[avxdid] = new TH2F("PXDGatedModeMapSubLER_" + bufful,
                                           "PXDGatedModeMapSubLER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);
    hGatedModeMapSubHER[avxdid] = new TH2F("PXDGatedModeMapSubHER_" + bufful,
                                           "PXDGatedModeMapSubHER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);
    hGatedModeMapAddLER[avxdid] = new TH2F("PXDGatedModeMapAddLER_" + bufful,
                                           "PXDGatedModeMapAddLER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);
    hGatedModeMapAddHER[avxdid] = new TH2F("PXDGatedModeMapAddHER_" + bufful,
                                           "PXDGatedModeMapAddHER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);

    hGatedModeProjADCLER[avxdid] = new TH2F("PXDGatedModeProjADCLER_" + bufful,
                                            "PXDGatedModeProjADCLER " + buff + ";Gate;V", 96, 0, 96, 192, 0, 768);
    hGatedModeProjADCHER[avxdid] = new TH2F("PXDGatedModeProjADCHER_" + bufful,
                                            "PXDGatedModeProjADCHER " + buff + ";Gate;V", 96, 0, 96, 192, 0, 768);

    hGatedModeMapSubADCLER[avxdid] = new TH2F("PXDGatedModeMapSubADCLER_" + bufful,
                                              "PXDGatedModeMapSubADCLER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);
    hGatedModeMapSubADCHER[avxdid] = new TH2F("PXDGatedModeMapSubADCHER_" + bufful,
                                              "PXDGatedModeMapSubADCHER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);
    hGatedModeMapAddADCLER[avxdid] = new TH2F("PXDGatedModeMapAddADCLER_" + bufful,
                                              "PXDGatedModeMapAddADCLER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);
    hGatedModeMapAddADCHER[avxdid] = new TH2F("PXDGatedModeMapAddADCHER_" + bufful,
                                              "PXDGatedModeMapAddADCHER " + buff + ";U;V", 25, 0, 250, 192, 0, 768);

  }
  // cd back to root directory
  oldDir->cd();
}

void PXDGatedModeDQMModule::initialize()
{
  REG_HISTOGRAM
  m_rawTTD.isOptional(); /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failing
  m_storeRawHits.isRequired(m_PXDRawHitsName);
}

void PXDGatedModeDQMModule::beginRun()
{
  // reset all histograms
  hBunchInjHER->Reset();
  hBunchInjLER->Reset();
  hBunchTrg->Reset();

  for (auto& it : hGatedModeMapLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapCutLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapCutHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapADCLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapADCHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapCutADCLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapCutADCHER) if (it.second) it.second->Reset();

  for (auto& it : hGatedModeProjLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeProjHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapSubLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapSubHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapAddLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapAddHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeProjADCLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeProjADCHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapSubADCLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapSubADCHER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapAddADCLER) if (it.second) it.second->Reset();
  for (auto& it : hGatedModeMapAddADCHER) if (it.second) it.second->Reset();
}

void PXDGatedModeDQMModule::event()
{

  for (auto& it : m_rawTTD) {
    B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
            it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

    // get last injection time
    auto difference = it.GetTimeSinceLastInjection(0);
    // check time overflow, too long ago
    if (difference != 0x7FFFFFFF) {
      auto isher = it.GetIsHER(0);
      float diff2 = difference / (508.877 / 4.); //  127MHz clock ticks to us, inexact rounding
      int bunch_trg = it.GetBunchNumber(0);
      int time_inj  = it.GetTimeSinceLastInjection(0);
      int bunch_inj = (bunch_trg - time_inj) % 1280;
      if (bunch_inj < 0) bunch_inj += 1280;
      int rgate = bunch_inj / (1280. / 96.); // 0-96 ?
      if (diff2 > 100 && diff2 < 10000) { // 10ms  ... variable wie lange gegated wird
        hBunchTrg->Fill(it.GetBunchNumber(0) & 0x7FF);
        if (isher) hBunchInjHER->Fill(bunch_inj);
        else hBunchInjLER->Fill(bunch_inj);
        for (auto& p : m_storeRawHits) {
          auto charge = p.getCharge();
          if (charge > 20) {
            int v = int(p.getVCellID()) - rgate * 4;
            if (v < 0) v += 768;
            int v2 = int(p.getVCellID()) + rgate * 4;
            if (v2 >= 768) v2 -= 768;
            if (isher) {
              auto h = hGatedModeMapHER[std::make_pair(p.getSensorID(), rgate)];
              if (h) {
                h->Fill(p.getUCellID(), p.getVCellID());
              }
              auto h2 = hGatedModeProjHER[p.getSensorID()];
              if (h2) {
                h2->Fill(rgate, p.getVCellID());
              }
              auto h3 = hGatedModeMapSubHER[p.getSensorID()];
              if (h3) {
                h3->Fill(p.getUCellID(), v);
              }
              auto h4 = hGatedModeMapAddHER[p.getSensorID()];
              if (h4) {
                h4->Fill(p.getUCellID(), v2);
              }
              auto h5 = hGatedModeMapADCHER[std::make_pair(p.getSensorID(), rgate)];
              if (h5) {
                h5->Fill(p.getUCellID(), p.getVCellID(), p.getCharge());
              }
              auto h6 = hGatedModeProjADCHER[p.getSensorID()];
              if (h6) {
                h6->Fill(rgate, p.getVCellID(), p.getCharge());
              }
              auto h7 = hGatedModeMapSubADCHER[p.getSensorID()];
              if (h7) {
                h7->Fill(p.getUCellID(), v, p.getCharge());
              }
              auto h8 = hGatedModeMapAddADCHER[p.getSensorID()];
              if (h8) {
                h8->Fill(p.getUCellID(), v2, p.getCharge());
              }
            } else {
              auto h = hGatedModeMapLER[std::make_pair(p.getSensorID(), rgate)];
              if (h) {
                h->Fill(p.getUCellID(), p.getVCellID());
              }
              auto h2 = hGatedModeProjLER[p.getSensorID()];
              if (h2) {
                h2->Fill(rgate, p.getVCellID());
              }
              auto h3 = hGatedModeMapSubLER[p.getSensorID()];
              if (h3) {
                h3->Fill(p.getUCellID(), v);
              }
              auto h4 = hGatedModeMapAddLER[p.getSensorID()];
              if (h4) {
                h4->Fill(p.getUCellID(), v2);
              }
              auto h5 = hGatedModeMapADCLER[std::make_pair(p.getSensorID(), rgate)];
              if (h5) {
                h5->Fill(p.getUCellID(), p.getVCellID(), p.getCharge());
              }
              auto h6 = hGatedModeProjADCLER[p.getSensorID()];
              if (h6) {
                h6->Fill(rgate, p.getVCellID(), p.getCharge());
              }
              auto h7 = hGatedModeMapSubADCLER[p.getSensorID()];
              if (h7) {
                h7->Fill(p.getUCellID(), v, p.getCharge());
              }
              auto h8 = hGatedModeMapAddADCLER[p.getSensorID()];
              if (h8) {
                h8->Fill(p.getUCellID(), v2, p.getCharge());
              }
            }
          }
          if (charge > 30) {

            if (isher) {
              auto h = hGatedModeMapCutHER[std::make_pair(p.getSensorID(), rgate)];
              if (h) {
                h->Fill(p.getUCellID(), p.getVCellID());
              }
              auto h2 = hGatedModeMapCutADCHER[std::make_pair(p.getSensorID(), rgate)];
              if (h2) {
                h2->Fill(p.getUCellID(), p.getVCellID(), p.getCharge());
              }
            } else {
              auto h = hGatedModeMapCutLER[std::make_pair(p.getSensorID(), rgate)];
              if (h) {
                h->Fill(p.getUCellID(), p.getVCellID());
              }
              auto h2 = hGatedModeMapCutADCLER[std::make_pair(p.getSensorID(), rgate)];
              if (h2) {
                h2->Fill(p.getUCellID(), p.getVCellID(), p.getCharge());
              }
            }
          }
        }
      } else if (diff2 > 20000) {
        rgate = 96;
        for (auto& p : m_storeRawHits) {
          if (isher) {
            auto h = hGatedModeMapHER[std::make_pair(p.getSensorID(), rgate)];
            if (h) {
              h->Fill(p.getUCellID(), p.getVCellID());
            }
            auto h2 = hGatedModeMapADCHER[std::make_pair(p.getSensorID(), rgate)];
            if (h2) {
              h2->Fill(p.getUCellID(), p.getVCellID(), p.getCharge());
            }
          } else {
            auto h = hGatedModeMapLER[std::make_pair(p.getSensorID(), rgate)];
            if (h) {
              h->Fill(p.getUCellID(), p.getVCellID());
            }
            auto h2 = hGatedModeMapADCLER[std::make_pair(p.getSensorID(), rgate)];
            if (h2) {
              h2->Fill(p.getUCellID(), p.getVCellID(), p.getCharge());
            }
          }
        }
      }
    }
    break;
  }
}
