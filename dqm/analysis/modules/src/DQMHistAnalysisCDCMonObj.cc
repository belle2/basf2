/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki, Luka Santelj                               *
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
  addParam("Filename", m_filename, "Output root filename (if not set mon_e{exp}r{run}.root is used", std::string(""));
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

  m_cMain = new TCanvas("main", "main", 1500, 1000);
  m_monObj->addCanvas(m_cMain);
  m_cBadWire = new TCanvas("badwire", "Bad wires", 1000, 1000);
  m_monObj->addCanvas(m_cBadWire);
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

  //    const double offset = m_offSet[L];
  //...Offset modification to be aligned to axial at z=0...
  //  const double phiSize = 2 * M_PI / double(m_nWires[L]);

  //  const double phiF = phiSize * (double(C) + offset)
  //                      + phiSize * 0.5 * double(m_nShifts[L]) + m_globalPhiRotation;


  /*
   * calculate cell geometry
   */

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
    const int nbins = m_hHit[il]->GetNbinsX();
    for (int iw = 0; iw < nbins; ++iw) {
      const int y = m_hHit[il]->GetBinContent(iw + 1);
      if (y == 0) {
        //B2INFO("l " << layer << " w " << wire);
        m_badChannels.push_back(std::make_pair(il, iw));
      }
    }
  }
  B2INFO("num bad wires " << m_badChannels.size());
}

float DQMHistAnalysisCDCMonObjModule::getHistMean(TH1F* h)
{
  TH1F* hist = (TH1F*)h->Clone();
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

  m_hNEvent = (TH1F*)findHist("CDC/hNEvents");
  TF1* fitFunc[300] = {};
  for (int i = 0; i < 300; ++i) {
    m_hADC[i] = (TH1F*)findHist(Form("CDC/hADC%d", i));
    m_hTDC[i] = (TH1F*)findHist(Form("CDC/hTDC%d", i));
    fitFunc[i] = new TF1(Form("f%d", i), "[0]+[6]*x+[1]*(exp([2]*(x-[3]))/(1+exp(-([4]-x)/[5])))",
                         4921 - 100, 4921 + 100);
    fitFunc[i]->SetParLimits(0, 0, 100);
    fitFunc[i]->SetParLimits(6, 0, 0.1);
    fitFunc[i]->SetParLimits(4, 4850., 5000.0);
    fitFunc[i]->SetParLimits(5, 0, 50.0);
  }

  for (int i = 0; i < 56; ++i) {
    m_hHit[i]  = (TH1F*)findHist(Form("CDC/hHitL%d", i));
  }

  const int neve = m_hNEvent->GetBinContent(1);

  // ADC related
  int nDeadADC = -1; // bid 0 always empty
  int nBadADC = 0;
  TH1F* hADCMean = new TH1F("hADCMean", "ADC mean;board;adc mean", 300, 0, 300);
  std::vector<float> means = {};
  for (int i = 0; i < 300; ++i) {
    if (m_hADC[i]->GetEntries() == 0) {
      nDeadADC += 1;
    } else {
      float n = static_cast<float>(m_hADC[i]->GetEntries());
      float n0 = static_cast<float>(m_hADC[i]->GetBinContent(1));
      if (n0 / n > 0.9) {
        B2DEBUG(99, "bad adc bid " << i << " " << n0 << " " << n);
        nBadADC += 1;
      }
      float m = getHistMean(m_hADC[i]);
      //B2INFO(i << " " << m );
      means.push_back(m);
      hADCMean->SetBinContent(i + 1, m);
      hADCMean->SetBinError(i + 1, 0);
    }
  }

  // TDC related
  int nDeadTDC = 1; // bid 0 always empty
  TH1F* hTDCEdge = new TH1F("hTDCEdge", "TDC edge;board;tdc edge [nsec]", 300, 0, 300);
  TH1F* hTDCSlope = new TH1F("hTDCSlope", "TDC slope;board;tdc slope [nsec]", 300, 0, 300);
  std::vector<float> tdcEdges = {};
  std::vector<float> tdcSlopes = {};
  for (int i = 0; i < 300; ++i) {
    if (m_hTDC[i]->GetEntries() == 0 || m_hTDC[i] == nullptr) {
      nDeadTDC += 1;
    } else {
      fitFunc[i]->SetParameters(0, 100, 0.01, 4700, 4900, 2, 0.01);
      fitFunc[i]->SetParameter(0, 10);
      fitFunc[i]->SetParameter(6, 0.02);
      int xxx = -1;
      if (i < 28) {
        xxx = m_hTDC[i]->Fit(fitFunc[i], "qM0", "", 4850, 5000);
      } else {
        xxx = m_hTDC[i]->Fit(fitFunc[i], "qM0", "", 4800, 5000);
      }
      float p4 = fitFunc[i]->GetParameter(4);
      float p5 = fitFunc[i]->GetParameter(5);
      if (xxx != -1 && 4850 < p4 && p4 < 5000) {
        hTDCEdge->SetBinContent(i + 1, p4);
        hTDCEdge->SetBinError(i + 1, 0);
        hTDCSlope->SetBinContent(i + 1, p5);
        hTDCSlope->SetBinError(i + 1, 0);
        tdcEdges.push_back(p4);
        tdcSlopes.push_back(p5);
      }
    }
  }

  // Bad wires relasted
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
    B2DEBUG(99, "l " << l << " w " << w);
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


  // Hit related
  TH1F* hHitPerLayer = new TH1F("hHitPerLayer", "hit/Layer;layer", 56, 0, 56);

  int nHits = 0;
  for (int i = 0; i < 56; ++i) {
    int nhitSumL = 0;
    const int nBins = m_hHit[i]->GetNbinsX();
    for (int j = 0; j < nBins; ++j) {
      nhitSumL += m_hHit[i]->GetBinContent(j + 1);
    }
    hHitPerLayer->SetBinContent(i + 1, static_cast<float>(nhitSumL / neve));
    hHitPerLayer->SetBinError(i + 1, 0);
    nHits += nhitSumL;
  }



  m_cMain->Divide(3, 2);

  m_cMain->cd(1);
  hADCMean->SetMinimum(0);
  hADCMean->SetMaximum(200);
  hADCMean->Draw();

  m_cMain->cd(2);
  hTDCEdge->SetMinimum(4800);
  hTDCEdge->SetMaximum(5000);
  hTDCEdge->Draw();

  m_cMain->cd(3);
  hTDCSlope->SetMinimum(0);
  hTDCSlope->SetMaximum(50);
  hTDCSlope->Draw();

  m_cMain->cd(4);
  hBadChannel->Draw("col");

  m_cMain->cd(5);
  hBadChannelBC->Draw("col");
  m_cMain->cd(6);
  hHitPerLayer->Draw();

  m_cBadWire->cd();
  h2p->Draw("col");
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

  std::string comment = "";
  m_monObj->setVariable("comment", comment); // tentative no comments
  m_monObj->setVariable("adcMean", std::accumulate(means.begin(), means.end(), 0) / means.size());
  m_monObj->setVariable("tdcEdge", std::accumulate(tdcEdges.begin(), tdcEdges.end(), 0) / tdcEdges.size());
  m_monObj->setVariable("tdcSlope", std::accumulate(tdcSlopes.begin(), tdcSlopes.end(), 0) / tdcSlopes.size());
  m_monObj->setVariable("nEvents", neve);
  m_monObj->setVariable("nDeadADC", nDeadADC);
  m_monObj->setVariable("nDeadTDC", nDeadTDC);
  m_monObj->setVariable("nHits", nHits / neve);
  m_monObj->setVariable("nADC(n_0/n_tot>0.9)", nBadADC);
  m_monObj->setVariable("nBadWires", m_badChannels.size());

  TString fname;
  if (m_filename.length()) fname = m_filename;
  else fname = "cdc_mon_output.root";

  TFile f(fname, "NEW");

  if (f.IsZombie()) {
    B2WARNING("File " << fname << "already exists and it will not be rewritten. If desired please delete file and re-run.");
    return;
  }
  // get list of existing monitoring objects
  const MonObjList& objts =  getMonObjList();
  // write them to the output file
  for (const auto& obj : objts)(obj.second)->Write();

  f.Close();

}

void DQMHistAnalysisCDCMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

