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

#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
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
  m_monObj = getMonitoringObject("cdc");
  //  m_cMain = new TCanvas("main", "main", 1300, 1000);
  m_cMain = new TCanvas("main", "main", 650, 500);
  m_monObj->addCanvas(m_cMain);
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
}

void DQMHistAnalysisCDCMonObjModule::event()
{
}



void DQMHistAnalysisCDCMonObjModule::makeBadChannelList()
{
  m_badChannels.clear();
  for (int i = 0; i < 56; ++i) {
    const int layer = i;
    const auto h = m_hHit[layer];
    const int nbins = h->GetNbinsX();
    for (int j = 0; j < nbins; ++j) {
      const int wire = i;
      const int y = h->GetBinContent(j + 1);
      if (y == 0) {
        m_badChannels.push_back(std::make_pair(layer, wire));
      }
    }
  }
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
  if (it != m_chMap.end()) { // 見つかった
    return it->second;
  } else {
    B2ERROR("no corresponding board/channel found layer "  << layer << " wire " << wire);
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
  }

  for (int i = 0; i < 56; ++i) {
    m_hHit[i]  = (TH1F*)findHist(Form("CDC/hHitL%d", i));
  }

  const int neve = m_hNEvent->GetBinContent(1);

  // ADC related
  int nDeadADC = -1; // bid 0 always empty
  int nBadADC = 0;
  TH1F* hADCMean = new TH1F("hADCMean", "ADC mean", 300, 0, 300);
  for (int i = 0; i < 300; ++i) {
    if (m_hADC[i]->GetEntries() == 0) {
      nDeadADC += 1;
    } else {
      float n = static_cast<float>(m_hADC[i]->GetEntries());
      float n0 = static_cast<float>(m_hADC[i]->GetBinContent(1));
      if (n0 / n > 0.1) {
        B2INFO("bad adc bid " << i << " " << n0 << " " << n);
        nBadADC += 1;
      }
      float m = getHistMean(m_hADC[i]);
      //B2INFO(i << " " << m );
      hADCMean->SetBinContent(i + 1, m);
      hADCMean->SetBinError(i + 1, 0);
    }
  }

  // TDC related
  int nDeadTDC = 1; // bid 0 always empty
  TH1F* hTDCEdge = new TH1F("hTDCEdge", "TDC edge", 300, 0, 300);
  TH1F* hTDCSlope = new TH1F("hTDCSlope", "TDC slope", 300, 0, 300);
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
      }
    }
  }

  // Bad wires relasted
  TH2F* hBadChannel = new TH2F("hbadch", "bad channel map;wire;layer", 400, 0, 400, 56, 0, 55);
  TH2F* hBadChannelBC = new TH2F("hbadchBC", "bad channel map per board/channel;board;channel", 300, 0, 300, 48, 0, 48);
  makeBadChannelList();
  for (const auto& lw : m_badChannels) {
    const int l = lw.first;
    const int w = lw.second;
    hBadChannel->SetBinContent(w + 1, l + 1, 8);
    std::pair<int, int> bc = getBoardChannel(l, w);
    hBadChannelBC->SetBinContent(bc.first + 1, bc.second + 1, 8);
  }



  // Hit related
  TH1F* hHitPerLayer = new TH1F("hHitPerLayer", "hit/Layer", 56, 0, 56);

  for (int i = 0; i < 56; ++i) {
    int nhitSumL = 0;
    const int nBins = m_hHit[i]->GetNbinsX();
    for (int j = 0; j < nBins; ++j) {
      nhitSumL += m_hHit[i]->GetBinContent(j + 1);
    }
    hHitPerLayer->SetBinContent(i + 1, static_cast<float>(nhitSumL / neve));
    hHitPerLayer->SetBinError(i + 1, 0);
  }


  gStyle->SetOptStat(0);
  gStyle->SetPalette(kViridis);

  m_cMain->SetLeftMargin(0.);
  m_cMain->Divide(3, 2);
  m_cMain->cd(1);
  hADCMean->Draw();
  m_cMain->cd(2);
  hTDCEdge->Draw();
  m_cMain->cd(3);
  hTDCSlope->Draw();
  m_cMain->cd(4);
  //hBadChannel->SetMaximum(2);
  //hBadChannel->SetMinimum(-1);
  hBadChannel->Draw("col");
  m_cMain->cd(5);
  hBadChannelBC->Draw("col");
  m_cMain->cd(6);
  hHitPerLayer->Draw();

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

