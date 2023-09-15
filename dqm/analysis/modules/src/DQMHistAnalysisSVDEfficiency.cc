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
REG_MODULE(DQMHistAnalysisSVDEfficiency);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDEfficiencyModule::DQMHistAnalysisSVDEfficiencyModule()
  : DQMHistAnalysisModule(),
    m_effUstatus(good),
    m_effVstatus(good)
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: Constructor done.");

  setDescription("DQM Analysis Module that computes the average SVD sensor efficiency.");

  addParam("RefHistoFile", m_refFileName, "Reference histogram file name", std::string("SVDrefHisto.root"));
  addParam("effLevel_Error", m_effError, "Efficiency error (%) level (red)", double(0.9));
  addParam("effLevel_Warning", m_effWarning, "Efficiency WARNING (%) level (orange)", double(0.94));
  addParam("statThreshold", m_statThreshold, "minimal number of tracks per sensor to set green/red alert", double(100));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("SVD:"));

}

DQMHistAnalysisSVDEfficiencyModule::~DQMHistAnalysisSVDEfficiencyModule() { }

void DQMHistAnalysisSVDEfficiencyModule::initialize()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: initialize");

  B2DEBUG(10, " black = " << kBlack);
  B2DEBUG(10, " green = " << kGreen);
  B2DEBUG(10, " yellow = " << kYellow);
  B2DEBUG(10, " Red = " << kRed);

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
  m_legWarning->SetFillColor(kYellow);
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

  //find nEvents testing if histograms are present
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents");
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  }
  TString runID = TString((hnEvnts->GetTitle())).Remove(0, 21);
  B2INFO("runID = " << runID);

  gROOT->cd();
  m_cEfficiencyU = new TCanvas("SVDAnalysis/c_SVDEfficiencyU");
  m_cEfficiencyV = new TCanvas("SVDAnalysis/c_SVDEfficiencyV");
  m_cEfficiencyErrU = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrU");
  m_cEfficiencyErrV = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrV");

  m_hEfficiency = new SVDSummaryPlots("SVDEfficiency@view", Form("Summary of SVD efficiencies (%%), @view/@side Side %s",
                                      runID.Data()));
  m_hEfficiencyErr = new SVDSummaryPlots("SVDEfficiencyErr@view", Form("Summary of SVD efficiencies errors (%%), @view/@side Side %s",
                                         runID.Data()));

  m_c3EfficiencyU = new TCanvas("SVDAnalysis/c_3SVDEfficiencyU");
  m_c3EfficiencyV = new TCanvas("SVDAnalysis/c_3SVDEfficiencyV");
  m_c3EfficiencyErrU = new TCanvas("SVDAnalysis/c_3SVDEfficiencyErrU");
  m_c3EfficiencyErrV = new TCanvas("SVDAnalysis/c_3SVDEfficiencyErrV");

  m_h3Efficiency = new SVDSummaryPlots("SVD3Efficiency@view",
                                       Form("Summary of SVD efficiencies (%%), @view/@side Side for 3 samples %s", runID.Data()));
  m_h3EfficiencyErr = new SVDSummaryPlots("SVD3EfficiencyErr@view",
                                          Form("Summary of SVD efficiencies errors (%%), @view/@side Side for 3 samples %s", runID.Data()));


  //register limits for EPICS
  registerEpicsPV(m_pvPrefix + "efficiencyLimits", "effLimits");
}

void DQMHistAnalysisSVDEfficiencyModule::beginRun()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: beginRun called.");
  m_cEfficiencyU->Clear();
  m_cEfficiencyV->Clear();
  m_cEfficiencyErrU->Clear();
  m_cEfficiencyErrV->Clear();

  m_c3EfficiencyU->Clear();
  m_c3EfficiencyV->Clear();
  m_c3EfficiencyErrU->Clear();
  m_c3EfficiencyErrV->Clear();

  //Retrieve limits from EPICS
  requestLimitsFromEpicsPVs("effLimits", m_effError, m_statThreshold, m_effWarning,  m_effError);

  B2INFO(" SVD efficiency thresholds taken from EPICS configuration file:");
  B2INFO("  EFFICIENCY: normal > " << m_effWarning << " > warning > " << m_effError << " > error");
}

void DQMHistAnalysisSVDEfficiencyModule::event()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: event called.");

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents", true);
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  } else {
    B2INFO("SVDExpReco/SVDDQM_nEvents found");
  }

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
    B2DEBUG(10, "U-side Before loop on sensors, size :" << m_SVDModules.size());
    m_effUstatus = good;
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
        m_effUstatus = std::max(lowStat, m_effUstatus);
      } else if (effU > m_effWarning) {
        m_effUstatus = std::max(good, m_effUstatus);
      } else if ((effU <= m_effWarning) && (effU > m_effError)) {
        m_effUstatus = std::max(warning, m_effUstatus);
      } else if ((effU <= m_effError)) {
        m_effUstatus = std::max(error, m_effUstatus);
      }
      B2DEBUG(10, "Status is " << m_effUstatus);
    }
  }

  //Efficiency for the V side
  TH2F* found_tracksV = (TH2F*)findHist("SVDEfficiency/TrackHitsV");
  TH2F* matched_clusV = (TH2F*)findHist("SVDEfficiency/MatchedHitsV");

  if (matched_clusV == NULL || found_tracksV == NULL) {
    B2INFO("Histograms needed for Efficiency computation are not found");
    m_cEfficiencyV->SetFillColor(kRed);
  } else {
    B2DEBUG(10, "V-side Before loop on sensors, size :" << m_SVDModules.size());
    m_effVstatus = good;
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
        m_effVstatus = std::max(lowStat, m_effVstatus);
      } else if (effV > m_effWarning) {
        m_effVstatus = std::max(good, m_effVstatus);
      } else if ((effV <= m_effWarning) && (effV > m_effError)) {
        m_effVstatus = std::max(warning, m_effVstatus);
      } else if ((effV <= m_effError)) {
        m_effVstatus = std::max(error, m_effVstatus);
      }
      B2DEBUG(10, "Status is " << m_effVstatus);
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
      m_cEfficiencyU->SetFillColor(kYellow);
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
//   setEpicsPV("EfficiencyUAlarm", alarm);

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
      m_cEfficiencyV->SetFillColor(kYellow);
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

  /// ------ 3 samples ------

  m_h3Efficiency->getHistogram(0)->Reset();
  m_h3Efficiency->getHistogram(1)->Reset();
  m_h3EfficiencyErr->getHistogram(0)->Reset();
  m_h3EfficiencyErr->getHistogram(1)->Reset();


  effU = -1;
  effV = -1;
  erreffU = -1;
  erreffV = -1;

  // Efficiency for the U side - 3 samples
  TH2F* found3_tracksU = (TH2F*)findHist("SVDEfficiency/TrackHits3U");
  TH2F* matched3_clusU = (TH2F*)findHist("SVDEfficiency/MatchedHits3U");

  if (matched3_clusU == NULL || found3_tracksU == NULL) {
    B2INFO("Histograms needed for Efficiency computation are not found");
    m_c3EfficiencyU->SetFillColor(kRed);
  } else {
    B2DEBUG(10, "U-side Before loop on sensors, size :" << m_SVDModules.size());
    m_effUstatus = good;
    for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
      B2DEBUG(10, "module " << i << "," << m_SVDModules[i]);
      int bin = found3_tracksU->FindBin(m_SVDModules[i].getLadderNumber(), findBinY(m_SVDModules[i].getLayerNumber(),
                                        m_SVDModules[i].getSensorNumber()));
      float numU = matched3_clusU->GetBinContent(bin);
      float denU = found3_tracksU->GetBinContent(bin);
      if (denU > 0)
        effU = numU / denU;
      else
        effU = -1;
      B2DEBUG(10, "effU  = " << numU << "/" << denU << " = " << effU);

      m_h3Efficiency->fill(m_SVDModules[i], 1, effU * 100);
      if (effU == -1)
        erreffU = -1;
      else
        erreffU = std::sqrt(effU * (1 - effU) / denU);
      m_h3EfficiencyErr->fill(m_SVDModules[i], 1, erreffU * 100);

      if (denU < m_statThreshold) {
        m_effUstatus = std::max(lowStat, m_effUstatus);
      } else if (effU > m_effWarning) {
        m_effUstatus = std::max(good, m_effUstatus);
      } else if ((effU <= m_effWarning) && (effU > m_effError)) {
        m_effUstatus = std::max(warning, m_effUstatus);
      } else if ((effU <= m_effError)) {
        m_effUstatus = std::max(error, m_effUstatus);
      }
      B2DEBUG(10, "Status is " << m_effUstatus);
    }
  }

  //Efficiency for the V side - 3 samples
  TH2F* found3_tracksV = (TH2F*)findHist("SVDEfficiency/TrackHits3V");
  TH2F* matched3_clusV = (TH2F*)findHist("SVDEfficiency/MatchedHits3V");

  if (matched3_clusV == NULL || found3_tracksV == NULL) {
    B2INFO("Histograms needed for Efficiency computation are not found");
    m_c3EfficiencyV->SetFillColor(kRed);
  } else {
    B2DEBUG(10, "V-side Before loop on sensors, size :" << m_SVDModules.size());
    m_effVstatus = good;
    for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
      B2DEBUG(10, "module " << i << "," << m_SVDModules[i]);
      int bin = found3_tracksV->FindBin(m_SVDModules[i].getLadderNumber(), findBinY(m_SVDModules[i].getLayerNumber(),
                                        m_SVDModules[i].getSensorNumber()));
      float numV = matched3_clusV->GetBinContent(bin);
      float denV = found3_tracksV->GetBinContent(bin);
      if (denV > 0)
        effV = numV / denV;
      else
        effV = -1;

      B2DEBUG(10, "effV  = " << numV << "/" << denV << " = " << effV);
      m_h3Efficiency->fill(m_SVDModules[i], 0, effV * 100);
      if (effV == -1)
        erreffV = -1;
      else
        erreffV = std::sqrt(effV * (1 - effV) / denV);

      m_h3EfficiencyErr->fill(m_SVDModules[i], 0, erreffV * 100);

      if (denV < m_statThreshold) {
        m_effVstatus = std::max(lowStat, m_effVstatus);
      } else if (effV > m_effWarning) {
        m_effVstatus = std::max(good, m_effVstatus);
      } else if ((effV <= m_effWarning) && (effV > m_effError)) {
        m_effVstatus = std::max(warning, m_effVstatus);
      } else if ((effV <= m_effError)) {
        m_effVstatus = std::max(error, m_effVstatus);
      }
      B2DEBUG(10, "Status is " << m_effVstatus);
    }
  }

  // update summary for U side
  m_c3EfficiencyU->cd();
  m_h3Efficiency->getHistogram(1)->Draw("text");

  switch (m_effUstatus) {
    case good: {
      m_c3EfficiencyU->SetFillColor(kGreen);
      m_c3EfficiencyU->SetFrameFillColor(10);
      m_legNormal->Draw("same");
      break;
    }
    case error: {
      m_c3EfficiencyU->SetFillColor(kRed);
      m_c3EfficiencyU->SetFrameFillColor(10);
      m_legProblem->Draw("same");
      break;
    }
    case warning: {
      m_c3EfficiencyU->SetFillColor(kYellow);
      m_c3EfficiencyU->SetFrameFillColor(10);
      m_legWarning->Draw("same");
      break;
    }
    case lowStat: {
      m_c3EfficiencyU->SetFillColor(kGray);
      m_c3EfficiencyU->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
      break;
    }
    default: {
      B2INFO("effUstatus not set properly: " << m_effUstatus);
      break;
    }
  }

  m_c3EfficiencyU->Draw("text");
  m_c3EfficiencyU->Update();
  m_c3EfficiencyU->Modified();
  m_c3EfficiencyU->Update();

  // update summary for V side
  m_c3EfficiencyV->cd();
  m_h3Efficiency->getHistogram(0)->Draw("text");

  switch (m_effVstatus) {
    case good: {
      m_c3EfficiencyV->SetFillColor(kGreen);
      m_c3EfficiencyV->SetFrameFillColor(10);
      m_legNormal->Draw("same");
      break;
    }
    case error: {
      m_c3EfficiencyV->SetFillColor(kRed);
      m_c3EfficiencyV->SetFrameFillColor(10);
      m_legProblem->Draw("same");
      break;
    }
    case warning: {
      m_c3EfficiencyV->SetFillColor(kYellow);
      m_c3EfficiencyV->SetFrameFillColor(10);
      m_legWarning->Draw("same");
      break;
    }
    case lowStat: {
      m_c3EfficiencyV->SetFillColor(kGray);
      m_c3EfficiencyV->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
      break;
    }
    default: {
      B2INFO("effVstatus not set properly: " << m_effVstatus);
      break;
    }
  }

  m_c3EfficiencyV->Draw();
  m_c3EfficiencyV->Update();
  m_c3EfficiencyV->Modified();
  m_c3EfficiencyV->Update();

  m_c3EfficiencyErrU->cd();
  m_h3EfficiencyErr->getHistogram(1)->Draw("colztext");
  m_c3EfficiencyErrU->Draw();
  m_c3EfficiencyErrU->Update();
  m_c3EfficiencyErrU->Modified();
  m_c3EfficiencyErrU->Update();

  m_c3EfficiencyErrV->cd();
  m_h3EfficiencyErr->getHistogram(0)->Draw("colztext");
  m_c3EfficiencyErrV->Draw();
  m_c3EfficiencyErrV->Update();
  m_c3EfficiencyErrV->Modified();
  m_c3EfficiencyErrV->Update();
}

void DQMHistAnalysisSVDEfficiencyModule::endRun()
{
  B2INFO("DQMHistAnalysisSVDEfficiency:  endRun called");
}

void DQMHistAnalysisSVDEfficiencyModule::terminate()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: terminate called");

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

