/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDEfficiency.cc
// Description : module for DQM histogram analysis of SVD sensors efficiencies
//
// Author : Giulia Casarosa (PI), Gaetano De Marino (PI)
// Date : 20190428
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDEfficiency.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDEfficiency)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDEfficiencyModule::DQMHistAnalysisSVDEfficiencyModule()
  : DQMHistAnalysisModule(),
    m_effUstatus(lowStat),
    m_effVstatus(lowStat)
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: Constructor done.");

  setDescription("DQM Analysis Module that computes the average SVD sensor efficiency.");

  addParam("RefHistoFile", m_refFileName, "Reference histogram file name", std::string("SVDrefHisto.root"));
  addParam("effLevel_Error", m_effError, "Efficiency error (%) level (red)", float(0.9));
  addParam("effLevel_Warning", m_effWarning, "Efficiency WARNING (%) level (orange)", float(0.94));
  addParam("statThreshold", m_statThreshold, "minimal number of tracks per sensor to set green/red alert", float(100));
}

DQMHistAnalysisSVDEfficiencyModule::~DQMHistAnalysisSVDEfficiencyModule() { }

void DQMHistAnalysisSVDEfficiencyModule::initialize()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: initialized.");
  B2DEBUG(10, " black = " << kBlack);
  B2DEBUG(10, " green = " << kGreen);
  B2DEBUG(10, " orange = " << kOrange);
  B2DEBUG(10, " Red = " << kRed);

  m_refFile = NULL;
  if (m_refFileName != "") {
    m_refFile = new TFile(m_refFileName.data(), "READ");
  }

  //search for reference
  if (m_refFile && m_refFile->IsOpen()) {
    B2INFO("SVD DQMHistAnalysis: reference root file (" << m_refFileName << ") FOUND, reading ref histograms");

    TH1F* ref_eff = (TH1F*)m_refFile->Get("refEfficiency");
    if (!ref_eff)
      B2WARNING("SVD DQMHistAnalysis: Efficiency Level Reference not found! using module parameters");
    else {
      m_effWarning = ref_eff->GetBinContent(2);
      m_effError = ref_eff->GetBinContent(3);
    }

  } else
    B2WARNING("SVD DQMHistAnalysis: reference root file (" << m_refFileName << ") not found, or closed, using module parameters");

  B2INFO(" SVD efficiency thresholds:");
  B2INFO(" EFFICIENCY: normal < " << m_effWarning << " < warning < " << m_effError << " < error");


  //build the legend
  m_legProblem = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legProblem->AddText("ERROR!");
  m_legProblem->AddText("at least one sensor with:");
  m_legProblem->AddText(Form("efficiency < %1.0f%%", m_effError * 100));
  m_legProblem->SetFillColor(kRed);
  m_legWarning = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legWarning->AddText("WARNING!");
  m_legWarning->AddText("at least one sensor with:");
  m_legWarning->AddText(Form("%1.0f%% < efficiency < %1.0f%%", m_effError * 100, m_effWarning * 100));
  m_legWarning->SetFillColor(kOrange);
  m_legNormal = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legNormal->AddText("EFFICIENCY WITHIN LIMITS");
  m_legNormal->AddText(Form("efficiency > %1.0f%%", m_effWarning * 100));
  m_legNormal->SetFillColor(kGreen);
  m_legNormal->SetBorderSize(0.);
  m_legNormal->SetLineColor(kBlack);
  m_legEmpty = new TPaveText(11, findBinY(4, 3) - 2, 16, findBinY(4, 3));
  m_legEmpty->AddText("Not enough statistics,");
  m_legEmpty->AddText("check again in a few minutes");
  m_legEmpty->SetFillColor(kBlack);
  m_legEmpty->SetTextColor(kWhite);
  m_legEmpty->SetBorderSize(0.);
  m_legEmpty->SetLineColor(kBlack);


  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all SVD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2INFO("VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    m_SVDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_SVDModules.begin(), m_SVDModules.end());  // back to natural order

  gROOT->cd();
  m_cEfficiencyU = new TCanvas("SVDAnalysis/c_SVDEfficiencyU");
  m_cEfficiencyV = new TCanvas("SVDAnalysis/c_SVDEfficiencyV");
  m_cEfficiencyErrU = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrU");
  m_cEfficiencyErrV = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrV");

  m_hEfficiency = new SVDSummaryPlots("SVDEfficiency@view", "Summary of SVD efficiencies (%), @view/@side Side");
  m_hEfficiencyErr = new SVDSummaryPlots("SVDEfficiencyErr@view", "Summary of SVD efficiencies errors (%), @view/@side Side");
}

void DQMHistAnalysisSVDEfficiencyModule::beginRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: beginRun called.");
  m_cEfficiencyU->Clear();
  m_cEfficiencyV->Clear();
  m_cEfficiencyErrU->Clear();
  m_cEfficiencyErrV->Clear();
}

void DQMHistAnalysisSVDEfficiencyModule::event()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: event called.");

  //SETUP gSTYLE - all plots
  //  gStyle->SetOptStat(0);
  //  gStyle->SetTitleY(.97);

  //set dedicate gStyle
  //  const Int_t colNum = 4;
  //  Int_t palette[colNum] {kBlack,  kGreen, kOrange, kRed};
  //  gStyle->SetPalette(colNum, palette);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.1f");

  m_hEfficiency->getHistogram(0)->Reset();
  m_hEfficiency->getHistogram(1)->Reset();
  m_hEfficiencyErr->getHistogram(0)->Reset();
  m_hEfficiencyErr->getHistogram(1)->Reset();

  Float_t effU = -1;
  Float_t effV = -1;
  Float_t erreffU = -1;
  Float_t erreffV = -1;

  // Efficiency for the U side
  TH2F* found_tracksU = (TH2F*)findHist("SVDEfficiency/TrackHitsU");
  TH2F* matched_clusU = (TH2F*)findHist("SVDEfficiency/MatchedHitsU");

  if (matched_clusU == NULL || found_tracksU == NULL) {
    B2INFO("Histograms needed for Efficiency computation are not found");
    m_cEfficiencyU->SetFillColor(kRed);
  } else {
    B2INFO("U-side Before loop on sensors, size :" << m_SVDModules.size());
    for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
      B2DEBUG(10, "module " << i << "," << m_SVDModules[i]);
      int bin = found_tracksU->FindBin(m_SVDModules[i].getLadderNumber(), findBinY(m_SVDModules[i].getLayerNumber(),
                                       m_SVDModules[i].getSensorNumber()));
      float numU = matched_clusU->GetBinContent(bin);
      float denU = found_tracksU->GetBinContent(bin);
      if (denU > 0)
        effU = numU / denU;
      else
        effU = -1;
      B2DEBUG(10, "effU  = " << numU << "/" << denU << " = " << effU);
      m_hEfficiency->fill(m_SVDModules[i], 1, effU * 100);
      if (effU == -1)
        erreffU = -1;
      else
        erreffU = std::sqrt(effU * (1 - effU) / denU);
      m_hEfficiencyErr->fill(m_SVDModules[i], 1, erreffU * 100);

      if (denU < m_statThreshold) {
        m_effUstatus = lowStat;
        break; // break loop if one of sensor collected less then m_statThreshold
      } else {
        if ((effU - erreffU <= m_effWarning) && (effU - erreffU > m_effError)) {
          m_effUstatus = warning;
        } else if ((effU - erreffU <= m_effError)) {
          m_effUstatus = error;
        } else if (effU - erreffU > m_effWarning) {
          m_effUstatus = good;
        }
      }
    }
  }

  //Efficiency for the V side
  TH2F* found_tracksV = (TH2F*)findHist("SVDEfficiency/TrackHitsV");
  TH2F* matched_clusV = (TH2F*)findHist("SVDEfficiency/MatchedHitsV");

  if (matched_clusV == NULL || found_tracksV == NULL) {
    B2INFO("Histograms needed for Efficiency computation are not found");
    m_cEfficiencyV->SetFillColor(kRed);
  } else {
    B2INFO("V-side Before loop on sensors, size :" << m_SVDModules.size());
    for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
      B2DEBUG(10, "module " << i << "," << m_SVDModules[i]);
      int bin = found_tracksV->FindBin(m_SVDModules[i].getLadderNumber(), findBinY(m_SVDModules[i].getLayerNumber(),
                                       m_SVDModules[i].getSensorNumber()));
      float numV = matched_clusV->GetBinContent(bin);
      float denV = found_tracksV->GetBinContent(bin);
      if (denV > 0)
        effV = numV / denV;
      else
        effV = -1;

      B2DEBUG(10, "effV  = " << numV << "/" << denV << " = " << effV);
      m_hEfficiency->fill(m_SVDModules[i], 0, effV * 100);
      if (effV == -1)
        erreffV = -1;
      else
        erreffV = std::sqrt(effV * (1 - effV) / denV);

      m_hEfficiencyErr->fill(m_SVDModules[i], 0, erreffV * 100);

      if (denV < m_statThreshold) {
        m_effVstatus = lowStat;
        break; // break loop if one of sensor collected less then m_statThreshold
      } else {
        if ((effV - erreffV <= m_effWarning) && (effV - erreffV > m_effError)) {
          m_effVstatus = warning;
        } else if ((effV - erreffV <= m_effError)) {
          m_effVstatus = error;
        } else if (effV - erreffV > m_effWarning) {
          m_effVstatus = good;
        }
      }
    }
  }

  // update summary for U side
  m_cEfficiencyU->cd();
  m_hEfficiency->getHistogram(1)->Draw("text");

  switch (m_effUstatus) {
    case good: {
      m_cEfficiencyU->SetFillColor(kGreen);
      m_cEfficiencyU->SetFrameFillColor(10);
      m_legNormal->Draw("same");
      break;
    }
    case error: {
      m_cEfficiencyU->SetFillColor(kRed);
      m_cEfficiencyU->SetFrameFillColor(10);
      m_legProblem->Draw("same");
      break;
    }
    case warning: {
      m_cEfficiencyU->SetFillColor(kOrange);
      m_cEfficiencyU->SetFrameFillColor(10);
      m_legWarning->Draw("same");
      break;
    }
    case lowStat: {
      m_cEfficiencyU->SetFillColor(kGray);
      m_cEfficiencyU->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
      break;
    }
    default: {
      B2INFO("effUstatus not set properly: " << m_effUstatus);
      break;
    }
  }

  m_cEfficiencyU->Draw("text");
  m_cEfficiencyU->Update();
  m_cEfficiencyU->Modified();
  m_cEfficiencyU->Update();

  // update summary for V side
  m_cEfficiencyV->cd();
  m_hEfficiency->getHistogram(0)->Draw("text");

  switch (m_effVstatus) {
    case good: {
      m_cEfficiencyV->SetFillColor(kGreen);
      m_cEfficiencyV->SetFrameFillColor(10);
      m_legNormal->Draw("same");
      break;
    }
    case error: {
      m_cEfficiencyV->SetFillColor(kRed);
      m_cEfficiencyV->SetFrameFillColor(10);
      m_legProblem->Draw("same");
      break;
    }
    case warning: {
      m_cEfficiencyV->SetFillColor(kOrange);
      m_cEfficiencyV->SetFrameFillColor(10);
      m_legWarning->Draw("same");
      break;
    }
    case lowStat: {
      m_cEfficiencyV->SetFillColor(kGray);
      m_cEfficiencyV->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
      break;
    }
    default: {
      B2INFO("effVstatus not set properly: " << m_effVstatus);
      break;
    }
  }

  m_cEfficiencyV->Draw();
  m_cEfficiencyV->Update();
  m_cEfficiencyV->Modified();
  m_cEfficiencyV->Update();

  m_cEfficiencyErrU->cd();
  m_hEfficiencyErr->getHistogram(1)->Draw("colztext");
  m_cEfficiencyErrU->Draw();
  m_cEfficiencyErrU->Update();
  m_cEfficiencyErrU->Modified();
  m_cEfficiencyErrU->Update();

  m_cEfficiencyErrV->cd();
  m_hEfficiencyErr->getHistogram(0)->Draw("colztext");
  m_cEfficiencyErrV->Draw();
  m_cEfficiencyErrV->Update();
  m_cEfficiencyErrV->Modified();
  m_cEfficiencyErrV->Update();
}

void DQMHistAnalysisSVDEfficiencyModule::endRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency:  endRun called");
}

void DQMHistAnalysisSVDEfficiencyModule::terminate()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: terminate called");

  delete m_refFile;
  delete m_legProblem;
  delete m_legWarning;
  delete m_legNormal;
  delete m_legEmpty;
  delete m_hEfficiency;
  delete m_cEfficiencyU;
  delete m_cEfficiencyV;
  delete m_hEfficiencyErr;
  delete m_cEfficiencyErrU;
  delete m_cEfficiencyErrV;
}

// return y coordinate in TH2F histogram for specified sensor
Int_t DQMHistAnalysisSVDEfficiencyModule::findBinY(Int_t layer, Int_t sensor)
{
  if (layer == 3)
    return sensor; //2 -> 1,2
  if (layer == 4)
    return 2 + 1 + sensor; //6 -> 4,5,6
  if (layer == 5)
    return 6 + 1 + sensor; // 11 -> 8, 9, 10, 11
  if (layer == 6)
    return 11 + 1 + sensor; // 17 -> 13, 14, 15, 16, 17
  else
    return -1;
}

