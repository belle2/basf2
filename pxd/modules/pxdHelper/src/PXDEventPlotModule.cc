/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdHelper/PXDEventPlotModule.h>
#include <TBox.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TH2.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDEventPlot)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDEventPlotModule::PXDEventPlotModule() : Module(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Set module properties
  setDescription("Plot Events on PXD Hit/Charge Maps and write pictures");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("PXDRawHitsName", m_storeRawHitsName, "The name of the StoreArray of PXDRawHits to be processed", string(""));
  addParam("GatedMode", m_gateModeFlag, "Extra histograms for gated mode", false);
}

void PXDEventPlotModule::initialize()
{
  m_eventMetaData.isRequired();
  m_storeRawHits.isRequired(m_storeRawHitsName);
  m_rawTTD.isRequired();
  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& avxdid : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;

    m_histos[avxdid] = new TH2F("Chargemap for " + TString((std::string)avxdid),
                                "PXD Module Chargemap for " + TString((std::string)avxdid) + " ;VCell;UCell", 768, 0, 768, 250, 0, 256);
    m_histos[avxdid]->SetContour(100);
    if (m_gateModeFlag) {
      m_histos_gm[avxdid] = new TH2F("Chargemap for " + TString((std::string)avxdid),
                                     "PXD Module Chargemap for GM " + TString((std::string)avxdid) + " ;VCell-GM;UCell", 768, 0, 768, 250, 0, 256);
      m_histos_gm[avxdid]->SetContour(100);
      m_histos_gm2[avxdid] = new TH2F("Chargemap for " + TString((std::string)avxdid),
                                      "PXD Module Chargemap for GM2 " + TString((std::string)avxdid) + " ;VCell+GM;UCell", 768, 0, 768, 250, 0, 256);
      m_histos_gm2[avxdid]->SetContour(100);
    }
  }
  m_c = new TCanvas("c1", "c1", 4000, 3000);
  m_l1 = new TLine(0, 62, 768, 62);
  m_l2 = new TLine(0, 125, 768, 125);
  m_l3 = new TLine(0, 187, 768, 187);
  m_l1->SetLineColor(kMagenta);
  m_l2->SetLineColor(kMagenta);
  m_l3->SetLineColor(kMagenta);
}

void PXDEventPlotModule::event()
{
  unsigned int evtNr = m_eventMetaData->getEvent();
  unsigned int evtRun = m_eventMetaData->getRun();

  for (auto h : m_histos) if (h.second) h.second->Reset();
  for (auto h : m_histos_gm) if (h.second) h.second->Reset();
  for (auto h : m_histos_gm2) if (h.second) h.second->Reset();

  std::map <VxdID, bool> cm_map;
  bool cm_flag = false;
  for (auto& pkt : *m_storeDAQEvtStats) {
    for (auto& dhc : pkt) {
      for (auto& dhe : dhc) {
        for (auto cm = dhe.cm_begin(); cm < dhe.cm_end(); ++cm) {
          // uint8_t, uint16_t, uint8_t ; tuple of Chip ID (2 bit), Row (10 bit), Common Mode (6 bit)
          if (std::get<2>(*cm) == 63) {
            m_histos[dhe.getSensorID()]->Fill(std::get<1>(*cm), 252 + std::get<0>(*cm), std::get<2>(*cm));
            cm_flag = true;
            cm_map[dhe.getSensorID()] = true;
          }
        }
      }
    }
  }

//  if( !cm_flag ) return;

  int tinj = -1, rgate = -1;
  for (auto& it : m_rawTTD) {
    // get last injection time
    auto difference = it.GetTimeSinceLastInjection(0);
    // check time overflow, too long ago
    if (difference != 0x7FFFFFFF) {
      /// auto isher = it.GetIsHER(0);// TODO, differentiate HER and LER GM
      float diff2 = difference / (508.877 / 4.); //  127MHz clock ticks to us, inexact rounding
      int bunch_trg = it.GetBunchNumber(0);
      int time_inj  = it.GetTimeSinceLastInjection(0);
      int bunch_inj = (bunch_trg - time_inj) % 1280;
      if (bunch_inj < 0) bunch_inj += 1280;
      rgate = bunch_inj / (1280. / 96.); // 0-96 ?
      tinj = diff2;
    }
    break;
  }

  for (auto& pix : m_storeRawHits) {
    if (m_histos[pix.getSensorID()]) {
      m_histos[pix.getSensorID()]->Fill(pix.getRow(), pix.getColumn(), pix.getCharge());
    }
    if (m_gateModeFlag) {
      if (m_histos_gm[pix.getSensorID()]) {
        int v = int(pix.getVCellID())  - rgate * 4;
        if (v < 0) v += 768; // work only for inner modules
        m_histos_gm[pix.getSensorID()]->Fill(v, pix.getColumn(), pix.getCharge());
      }
      if (m_histos_gm2[pix.getSensorID()]) {
        int v = int(pix.getVCellID()) + rgate * 4;
        if (v >= 768) v -= 768; // work only for outer modules
        m_histos_gm2[pix.getSensorID()]->Fill(v, pix.getColumn(), pix.getCharge());
      }
    }
  }

  string canvasname;
  canvasname = std::string(Form("Run_%d_Evt_%d", evtRun, evtNr));

  gStyle->SetPalette(55);
  gStyle->SetOptStat(0);

  {
    m_c->Clear();
    m_c->Divide(4, 5);
    m_c->cd(0);
    if (cm_flag) {
      m_c->Pad()->SetFrameLineColor(kRed);
      m_c->Pad()->SetFrameBorderSize(4);
      m_c->Pad()->SetFrameBorderMode(4);
      m_c->SetFrameLineColor(kRed);
      m_c->SetFrameBorderSize(4);
      m_c->SetFrameBorderMode(4);
    }
    int i = 1;

    for (auto h : m_histos) {
      m_c->cd(i++);
      if (h.second) {
        string abc = string(h.first);
        auto dhe = (*m_storeDAQEvtStats).findDHE(h.first);
        if (dhe) {

          auto tg = dhe->getTriggerGate();
          auto fn = dhe->getFrameNr();
          auto err = dhe->getEndErrorInfo();

          abc += Form(" ERR: $%X TG: %d FN: %d IN: %d GA: %d", err, tg, fn, tinj, rgate);
          for (auto itdhp = dhe->cbegin(); itdhp != dhe->cend(); ++itdhp) {
            abc += Form("(%d) %d ", itdhp->getChipID(), itdhp->getFrameNr());
          }
          if (err != 0) {
            m_c->Pad()->SetFillColor(kYellow);
          }
          if (cm_map[h.first]) {
            m_c->Pad()->SetFillColor(kRed);
            m_c->Pad()->SetFrameFillColor(kWhite);
          }
        }
        h.second->SetTitle(abc.data());
        h.second->Draw("colz");
        m_l1->Draw();
        m_l2->Draw();
        m_l3->Draw();
      }
    }
    m_c->cd(0);
    m_c->Print((canvasname + ".png").data());
    m_c->Print((canvasname + ".pdf").data());
    m_c->Print((canvasname + ".root").data());
  }
  if (m_gateModeFlag) {
    m_c->Clear();
    m_c->Divide(4, 5);
    m_c->cd(0);
    if (cm_flag) {
      m_c->Pad()->SetFrameLineColor(kRed);
      m_c->Pad()->SetFrameBorderSize(4);
      m_c->Pad()->SetFrameBorderMode(4);
      m_c->SetFrameLineColor(kRed);
      m_c->SetFrameBorderSize(4);
      m_c->SetFrameBorderMode(4);
    }
    int i = 1;

    for (auto h : m_histos_gm) {
      m_c->cd(i++);
      if (h.second) {
        string abc = string(h.first);
        auto dhe = (*m_storeDAQEvtStats).findDHE(h.first);
        if (dhe) {

          auto tg = dhe->getTriggerGate();
          auto fn = dhe->getFrameNr();
          auto err = dhe->getEndErrorInfo();

          abc += Form(" ERR: $%X TG: %d FN: %d IN: %d GA: %d", err, tg, fn, tinj, rgate);
          for (auto itdhp = dhe->cbegin(); itdhp != dhe->cend(); ++itdhp) {
            abc += Form("(%d) %d ", itdhp->getChipID(), itdhp->getFrameNr());
          }
          if (err != 0) {
            m_c->Pad()->SetFillColor(kYellow);
          }
          if (cm_map[h.first]) {
            m_c->Pad()->SetFillColor(kRed);
            m_c->Pad()->SetFrameFillColor(kWhite);
          }
        }
        h.second->SetTitle(abc.data());
        h.second->Draw("colz");
        m_l1->Draw();
        m_l2->Draw();
        m_l3->Draw();
      }
    }
    m_c->cd(0);
    m_c->Print((canvasname + "_gm.png").data());
    m_c->Print((canvasname + "_gm.pdf").data());
    m_c->Print((canvasname + "_gm.root").data());

    m_c->Clear();
    m_c->Divide(4, 5);
    m_c->cd(0);
    if (cm_flag) {
      m_c->Pad()->SetFrameLineColor(kRed);
      m_c->Pad()->SetFrameBorderSize(4);
      m_c->Pad()->SetFrameBorderMode(4);
      m_c->SetFrameLineColor(kRed);
      m_c->SetFrameBorderSize(4);
      m_c->SetFrameBorderMode(4);
    }
    i = 1;

    for (auto h : m_histos_gm2) {
      m_c->cd(i++);
      if (h.second) {
        string abc = string(h.first);
        auto dhe = (*m_storeDAQEvtStats).findDHE(h.first);
        if (dhe) {

          auto tg = dhe->getTriggerGate();
          auto fn = dhe->getFrameNr();
          auto err = dhe->getEndErrorInfo();

          abc += Form(" ERR: $%X TG: %d FN: %d IN: %d GA: %d", err, tg, fn, tinj, rgate);
          for (auto itdhp = dhe->cbegin(); itdhp != dhe->cend(); ++itdhp) {
            abc += Form("(%d) %d ", itdhp->getChipID(), itdhp->getFrameNr());
          }
          if (err != 0) {
            m_c->Pad()->SetFillColor(kYellow);
          }
          if (cm_map[h.first]) {
            m_c->Pad()->SetFillColor(kRed);
            m_c->Pad()->SetFrameFillColor(kWhite);
          }
        }
        h.second->SetTitle(abc.data());
        h.second->Draw("colz");
        m_l1->Draw();
        m_l2->Draw();
        m_l3->Draw();
      }
    }
    m_c->cd(0);
    m_c->Print((canvasname + "_gm2.png").data());
    m_c->Print((canvasname + "_gm2.pdf").data());
    m_c->Print((canvasname + "_gm2.root").data());
  }
}
