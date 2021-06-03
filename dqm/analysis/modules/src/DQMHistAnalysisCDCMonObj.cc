/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisCDCMonObj.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

// CDC geometry
#include <cdc/geometry/CDCGeometryPar.h>

#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH2Poly.h>
#include <TEllipse.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TClass.h>
#include <TROOT.h>
#include <TString.h>
#include <TFile.h>
#include <TStyle.h>

#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisCDCMonObj);

DQMHistAnalysisCDCMonObjModule::DQMHistAnalysisCDCMonObjModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Modify and analyze the data quality histograms of CDCMonObj");
  setPropertyFlags(c_ParallelProcessingCertified);
  for (int i = 0; i < 300; i++) {
    m_hADCs[i] = nullptr;
    m_hADCTOTCuts[i] = nullptr;
    m_hTDCs[i] = nullptr;
  }
  for (int i = 0; i < 56; i++) m_hHits[i] = nullptr;
}

DQMHistAnalysisCDCMonObjModule::~DQMHistAnalysisCDCMonObjModule()
{

}

void DQMHistAnalysisCDCMonObjModule::initialize()
{

  m_channelMapFromDB = new DBArray<CDCChannelMap>;
  if (!(*m_channelMapFromDB).isValid()) {
    B2FATAL("Channel map is not valid");
  }

  m_cdcGeo = new DBObjPtr<CDCGeometry>();
  if (m_cdcGeo == nullptr) {
    B2FATAL("CDCGeometryp is not valid");
  }



  m_monObj = getMonitoringObject("cdc");

  gStyle->SetOptStat(0);
  gStyle->SetPalette(kViridis);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadBottomMargin(0.1);
  gStyle->SetPadLeftMargin(0.15);

  m_cMain = new TCanvas("cdc_main", "cdc_main", 1500, 1000);
  m_monObj->addCanvas(m_cMain);

  m_cADC = new TCanvas("cdc_adc", "cdc_adc", 2000, 10000);
  m_monObj->addCanvas(m_cADC);
  m_cTDC = new TCanvas("cdc_tdc", "cdc_tdc", 2000, 10000);
  m_monObj->addCanvas(m_cTDC);
  m_cHit = new TCanvas("cdc_hit", "cdc_hit", 1500, 6000);
  m_monObj->addCanvas(m_cHit);

  B2DEBUG(20, "DQMHistAnalysisCDCMonObj: initialized.");

}

void DQMHistAnalysisCDCMonObjModule::beginRun()
{
  for (const auto& cm : (*m_channelMapFromDB)) {
    const int isl = cm.getISuperLayer();
    const int il = cm.getILayer();
    const int iw = cm.getIWire();
    const int iBoard = cm.getBoardID();
    const int iCh = cm.getBoardChannel();
    const WireID  wireId(isl, il, iw);
    m_chMap.insert(std::make_pair(wireId, std::make_pair(iBoard, iCh)));
  }


  const CDCGeometry& geom = **m_cdcGeo;

  for (const auto& sense : geom.getSenseLayers()) {
    int i = sense.getId();
    if (i < 0 || i > 55) {
      B2FATAL("no such sense layer");
    }
    m_senseR[i]  = sense.getR();
    m_nSenseWires[i] = sense.getNWires();
    m_offset[i] = sense.getOffset();
  }

  for (const auto& field : geom.getFieldLayers()) {
    int i = field.getId();
    if (i < 0 || i > 54) {
      B2FATAL("no such sense layer");
    }
    m_fieldR[i + 1] = field.getR();
  }
  m_fieldR[56] = geom.getOuterWall(0).getRmin();
  m_fieldR[0] = geom.getInnerWall(0).getRmax();

}

void DQMHistAnalysisCDCMonObjModule::event()
{
}

void DQMHistAnalysisCDCMonObjModule::configureBins(TH2Poly* h)
{
  for (int ilayer = 0; ilayer < 56; ++ilayer) {
    float dPhi = float(2 * M_PI / m_nSenseWires[ilayer]);
    float r1 = m_fieldR[ilayer];
    float r2 = m_fieldR[ilayer + 1];
    for (int iwire = 0; iwire < m_nSenseWires[ilayer]; ++iwire) {
      float phi = dPhi * (iwire + m_offset[ilayer]);
      float phi1 = phi - dPhi * 0.5;
      float phi2 = phi + dPhi * 0.5;
      Double_t x_pos[] = {r1* (sin(phi)*tan(phi - phi1) + cos(phi)),
                          r2 * cos(phi1),
                          r2 * cos(phi2),
                          r1* (sin(phi)*tan(phi - phi2) + cos(phi))
                         };
      Double_t y_pos[]  = {r1* (-cos(phi)*tan(phi - phi1) + sin(phi)),
                           r2 * sin(phi1),
                           r2 * sin(phi2),
                           r1* (-cos(phi)*tan(phi - phi2) + sin(phi))
                          };
      h->AddBin(4, x_pos, y_pos);
    }
  }
}


void DQMHistAnalysisCDCMonObjModule::makeBadChannelList()
{
  m_badChannels.clear();
  for (int il = 0; il < 56; ++il) {
    for (int iw = 0; iw < m_nSenseWires[il]; ++iw) {
      const int y = m_hHits[il]->GetBinContent(iw + 1);
      if (y == 0) {
        m_badChannels.push_back(std::make_pair(il, iw));
      }
    }
  }
  B2DEBUG(20, "num bad wires " << m_badChannels.size());
}

float DQMHistAnalysisCDCMonObjModule::getHistMean(TH1D* h)
{
  TH1D* hist = (TH1D*)h->Clone();
  hist->SetBinContent(1, 0.0);
  float m = hist->GetMean();
  return m;
}


std::pair<int, int> DQMHistAnalysisCDCMonObjModule::getBoardChannel(unsigned short layer, unsigned short wire)
{
  const WireID w(layer, wire);
  decltype(m_chMap)::iterator it = m_chMap.find(w);
  if (it != m_chMap.end()) {
    return it->second;
  } else {
    B2ERROR("no corresponding board/channel found layer "  << layer << " wire " << wire);
    return std::make_pair(-1, -1);
  }
}


void DQMHistAnalysisCDCMonObjModule::endRun()
{
  B2DEBUG(20, "end run");
  m_hfastTDC = (TH1F*)findHist("CDC/fast_tdc");
  m_hADC = (TH2F*)findHist("CDC/hADC");
  m_hADCTOTCut = (TH2F*)findHist("CDC/hADCTOTCut");
  m_hTDC = (TH2F*)findHist("CDC/hTDC");
  m_hHit = (TH2F*)findHist("CDC/hHit");
  TF1* fitFunc[300] = {};
  for (int i = 0; i < 300; ++i) {
    fitFunc[i] = new TF1(Form("f%d", i), "[0]+[6]*x+[1]*(exp([2]*(x-[3]))/(1+exp(-([4]-x)/[5])))",
                         4921 - 100, 4921 + 100);
    // fitFunc[i]->SetParLimits(0, 0, 100);
    fitFunc[i]->SetParLimits(6, 0, 0.1);
    fitFunc[i]->SetParLimits(4, 4850., 5000.0);
    fitFunc[i]->SetParLimits(5, 0, 50.0);
  }

  int neve = m_hfastTDC->GetEntries();
  if (neve == 0)neve = 1;

  B2DEBUG(20, "adc related");
  int nDeadADC = -1; // bid 0 always empty
  int nBadADC = 0;
  TH1F* hADCMean = new TH1F("hADCMean", "ADC mean;board;adc mean", 300, 0, 300);
  TH1F* hADC1000 = new TH1F("ADC1000", "ADC1000", 300, 0, 300);
  TH1F* hADC0 = new TH1F("ADC0", "ADC0", 300, 0, 300);

  std::vector<float> means = {};
  for (int i = 0; i < 300; ++i) {
    m_hADCTOTCuts[i] = m_hADCTOTCut->ProjectionY(Form("hADCTOTCut%d", i), i + 1, i + 1, "");
    m_hADCTOTCuts[i]->SetTitle(Form("hADCTOTCut%d", i));
    m_hADCs[i] = m_hADC->ProjectionY(Form("hADC%d", i), i + 1, i + 1, "");
    m_hADCs[i]->SetTitle(Form("hADC%d", i));
    float n = static_cast<float>(m_hADCs[i]->GetEntries());
    if (m_hADCs[i]->GetEntries() == 0) {
      nDeadADC += 1;
      hADC0->SetBinContent(i + 1, -0.1);
    } else {
      float n0 = static_cast<float>(m_hADCs[i]->GetBinContent(1));
      if (n0 / n > 0.9) {
        B2DEBUG(21, "bad adc bid " << i << " " << n0 << " " << n);
        nBadADC += 1;
      }
      float bin1 = m_hADCs[i]->GetBinContent(1);
      float m = getHistMean(m_hADCs[i]);
      means.push_back(m);
      hADCMean->SetBinContent(i + 1, m);
      hADCMean->SetBinError(i + 1, 0);
      double overflow = m_hADCs[i]->GetBinContent(1001);
      hADC1000->SetBinContent(i + 1, overflow / (overflow + n));
      hADC0->SetBinContent(i + 1, bin1 / (overflow + n));
      /// for some reason, GetEntries() does not include overflow???
    }
  }
  // TDC related
  B2DEBUG(20, "tdc related");
  int nDeadTDC = 1; // bid 0 always empty
  TH1F* hTDCEdge = new TH1F("hTDCEdge", "TDC edge;board;tdc edge [nsec]", 300, 0, 300);
  TH1F* hTDCSlope = new TH1F("hTDCSlope", "TDC slope;board;tdc slope [nsec]", 300, 0, 300);
  std::vector<float> tdcEdges = {};
  std::vector<float> tdcSlopes = {};
  for (int i = 0; i < 300; ++i) {
    m_hTDCs[i] = m_hTDC->ProjectionY(Form("hTDC%d", i), i + 1, i + 1);
    m_hTDCs[i]->SetTitle(Form("hTDC%d", i));
    if (m_hTDCs[i]->GetEntries() == 0 || m_hTDCs[i] == nullptr) {
      nDeadTDC += 1;
      tdcEdges.push_back(0);
      tdcSlopes.push_back(0);
    } else {
      double init_p0 = m_hTDCs[i]->GetBinContent(700 + 60);
      fitFunc[i]->SetParameters(init_p0, 100, 0.01, 4700, 4900, 2, 0.01);
      //fitFunc[i]->SetParameter(0, 10);
      fitFunc[i]->SetParameter(6, 0.02);
      fitFunc[i]->SetParLimits(0, init_p0 - 200, init_p0 + 200);
      int xxx = -1;
      if (i < 28) {
        xxx = m_hTDCs[i]->Fit(fitFunc[i], "qM0", "", 4850, 5000);
      } else {
        xxx = m_hTDCs[i]->Fit(fitFunc[i], "qM0", "", 4800, 5000);
      }
      float p4 = fitFunc[i]->GetParameter(4);
      float p5 = fitFunc[i]->GetParameter(5);

      if (xxx != -1 && 4850 < p4 && p4 < 5000) {
        hTDCEdge->SetBinContent(i + 1, p4);
        hTDCEdge->SetBinError(i + 1, 0);
        hTDCSlope->SetBinContent(i + 1, p5);
        hTDCSlope->SetBinError(i + 1, 0);
      }
      tdcEdges.push_back(p4);
      tdcSlopes.push_back(p5);
    }

  }

  // Hit related
  B2DEBUG(20, "hit related");
  TH1F* hHitPerLayer = new TH1F("hHitPerLayer", "hit/Layer;layer", 56, 0, 56);
  int nHits = 0;
  for (int i = 0; i < 56; ++i) {
    m_hHits[i] = m_hHit->ProjectionY(Form("hHit%d", i), i + 1, i + 1);
    m_hHits[i]->SetTitle(Form("hHit%d", i));
    if (m_hHits[i]->GetEntries() > 0 && m_hHits[i] != nullptr) {
      int nhitSumL = 0;
      int nBins = m_nSenseWires[i];
      for (int j = 0; j < nBins; ++j) {
        nhitSumL += m_hHits[i]->GetBinContent(j + 1);
      }
      if (neve > 0) {
        hHitPerLayer->SetBinContent(i + 1, static_cast<float>(nhitSumL / neve));
      } else hHitPerLayer->SetBinContent(i + 1, static_cast<float>(nhitSumL));
      hHitPerLayer->SetBinError(i + 1, 0);
      nHits += nhitSumL;
    }
  }

  // Bad wires related
  B2DEBUG(20, "bad wire related");
  TH2F* hBadChannel = new TH2F("hbadch", "bad channel map;wire;layer", 400, 0, 400, 56, 0, 56);
  for (int i = 0; i < 400; ++i) {
    for (int j = 0; j < 56; ++j) {
      hBadChannel->Fill(i, j, -1);
    }
  }

  TH2F* hBadChannelBC = new TH2F("hbadchBC", "bad channel map per board/channel;board;channel", 300, 0, 300, 48, 0, 48);
  for (int i = 0; i < 300; ++i) {
    for (int j = 0; j < 48; ++j) {
      hBadChannelBC->Fill(i, j, -1);
    }
  }

  TH2Poly* h2p = new TH2Poly();
  configureBins(h2p);
  h2p->SetTitle("bad wires in xy view");
  h2p->GetXaxis()->SetTitle("X [cm]");
  h2p->GetYaxis()->SetTitle("Y [cm]");
  makeBadChannelList();
  for (const auto& lw : m_badChannels) {
    const int l = lw.first;
    const int w = lw.second;
    B2DEBUG(21, "l " << l << " w " << w);
    hBadChannel->Fill(w, l);
    std::pair<int, int> bc = getBoardChannel(l, w);
    hBadChannelBC->Fill(bc.first, bc.second);
    float r = m_senseR[l];
    float dPhi = static_cast<float>(2.0 * M_PI / m_nSenseWires[l]);
    float phi = dPhi * (w + m_offset[l]);
    float x = r * cos(phi);
    float y = r * sin(phi);
    h2p->Fill(x, y, 1.1);
  }

  B2DEBUG(20, "writing");
  m_cMain->Divide(3, 3);

  m_cMain->cd(1);
  hADCMean->SetMinimum(0);
  hADCMean->SetMaximum(300);
  hADCMean->DrawCopy();

  m_cMain->cd(2);
  hTDCEdge->SetMinimum(4800);
  hTDCEdge->SetMaximum(5000);
  hTDCEdge->DrawCopy();

  m_cMain->cd(3);
  hTDCSlope->SetMinimum(0);
  hTDCSlope->SetMaximum(50);
  hTDCSlope->DrawCopy();
  m_cMain->cd(4);
  hBadChannel->DrawCopy("col");

  m_cMain->cd(5);
  hBadChannelBC->DrawCopy("col");
  m_cMain->cd(9);
  hHitPerLayer->DrawCopy();
  m_cMain->cd(7);
  hADC1000->DrawCopy();
  m_cMain->cd(8);
  hADC0->DrawCopy();

  m_cHit->Divide(4, 14);
  for (int i = 0; i < 56; i++) {
    m_cHit->cd(i + 1);
    m_hHits[i]->GetXaxis()->SetRangeUser(0, m_nSenseWires[i]);
    m_hHits[i]->Draw("hist");
  }

  m_cADC->Divide(6, 50, 0.0002, 0.0002);
  m_cTDC->Divide(6, 50, 0.0002, 0.0002);

  for (int i = 0; i < 300; i++) {
    m_cADC->cd(i + 1);
    m_hADCTOTCuts[i]->SetFillColor(42);
    Double_t max = m_hADCTOTCuts[i]->GetMaximum();
    m_hADCs[i]->GetYaxis()->SetRangeUser(0, 3 * max);
    m_hADCs[i]->Draw("hist");
    m_hADCTOTCuts[i]->Draw("hist same");

    m_cTDC->cd(i + 1);
    m_hTDCs[i]->Draw("hist");
    fitFunc[i]->SetLineColor(kRed);
    fitFunc[i]->Draw("same");
    max = m_hTDCs[i]->GetMaximum();
    TLine* l1 = new TLine(tdcEdges[i], 0, tdcEdges[i], max * 1.05);
    l1->SetLineColor(kRed);
    TLine* l0 = new TLine(4910, 0, 4910, max * 1.05);
    l0->Draw();
    l1->Draw();
  }

  m_cMain->cd(6);
  h2p->DrawCopy("col");
  float superLayerR[10] = {16.3, 24.3, 35.66, 46.63, 57.55, 68.47,
                           79.39, 90.31, 101.23, 112.05
                          };

  TEllipse* circs[10];
  for (int i = 0; i < 10; ++i) {
    circs[i] = new TEllipse(0, 0, superLayerR[i], superLayerR[i]);
    circs[i]->SetFillStyle(4000);
    circs[i]->SetLineStyle(kDashed);
    circs[i]->SetLineColor(0);
    circs[i]->Draw("same");
  }

  m_monObj->setVariable("nEvents", neve);
  m_monObj->setVariable("nHits", nHits / neve);
  m_monObj->setVariable("nBadWires", m_badChannels.size());
  m_monObj->setVariable("adcMean", std::accumulate(means.begin(), means.end(), 0.0) / means.size());
  m_monObj->setVariable("nDeadADC", nDeadADC);
  m_monObj->setVariable("nBadADC", nBadADC); //???? n_0/n_tot>0.9
  m_monObj->setVariable("tdcEdge", std::accumulate(tdcEdges.begin(), tdcEdges.end(), 0.0) / tdcEdges.size());
  m_monObj->setVariable("nDeadTDC", nDeadTDC);
  m_monObj->setVariable("tdcSlope", std::accumulate(tdcSlopes.begin(), tdcSlopes.end(), 0.0) / tdcSlopes.size());

  delete hADCMean;
  delete hADC1000;
  delete hADC0;
  delete hTDCEdge;
  delete hTDCSlope;
  delete hHitPerLayer;
  delete hBadChannel;
  delete hBadChannelBC;
  delete h2p;

}

void DQMHistAnalysisCDCMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

