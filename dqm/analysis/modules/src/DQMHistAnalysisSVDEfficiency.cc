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
#include <TMath.h>

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
  : DQMHistAnalysisSVDModule(),
    m_effUstatus(good),
    m_effVstatus(good)
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: Constructor done.");

  setDescription("DQM Analysis Module that computes the average SVD sensor efficiency.");

  addParam("effLevel_Error", m_effError, "Efficiency error (%) level (red)", double(0.9));
  addParam("effLevel_Warning", m_effWarning, "Efficiency WARNING (%) level (orange)", double(0.94));
  addParam("statThreshold", m_statThreshold, "minimal number of tracks per sensor to set green/red alert", double(100));
  addParam("samples3", m_3Samples, "if True 3 samples histograms analysis is performed", bool(false));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("SVD:"));
  addParam("setEfficiencyRange", m_setEfficiencyRange,
           "If true you can set the range of the efficiency histogram with 'efficiencyMax' and 'efficiencyMin' parameters.",
           bool(false));
  addParam("efficiencyMin", m_efficiencyMin, "Minimum of efficiency histogram", int(0));
  addParam("efficiencyMax", m_efficiencyMax, "Maximum of efficiency histogram",
           int(-1111)); //-1111 set the maximum depending on the content
}

DQMHistAnalysisSVDEfficiencyModule::~DQMHistAnalysisSVDEfficiencyModule()
{

}

void DQMHistAnalysisSVDEfficiencyModule::initialize()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: initialize");


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

  m_cEfficiencyRPhiViewU = new TCanvas("SVDAnalysis/c_SVDEfficiencyRPhiViewU", "", 800, 800);
  m_cEfficiencyRPhiViewV = new TCanvas("SVDAnalysis/c_SVDEfficiencyRPhiViewV", "", 800, 800);
  m_cEfficiencyErrRPhiViewU = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrRPhiViewU", "", 800, 800);
  m_cEfficiencyErrRPhiViewV = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrRPhiViewV", "", 800, 800);

  m_hEfficiency = new SVDSummaryPlots("SVDEfficiency@view", "Summary of SVD efficiencies (%), @view/@side Side");
  m_hEfficiency->setStats(0);
  if (m_setEfficiencyRange) {
    m_hEfficiency->setMaximum(m_efficiencyMax);
    m_hEfficiency->setMinimum(m_efficiencyMin);
  }
  m_hEfficiencyErr = new SVDSummaryPlots("SVDEfficiencyErr@view", "Summary of SVD efficiencies errors (%), @view/@side Side");
  m_hEfficiencyErr->setStats(0);

  if (m_3Samples) {
    m_cEfficiencyU3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyU3Samples");
    m_cEfficiencyV3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyV3Samples");
    m_cEfficiencyErrU3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrU3Samples");
    m_cEfficiencyErrV3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrV3Samples");

    m_cEfficiencyRPhiViewU3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyRPhiViewU3Samples", "", 800, 800);
    m_cEfficiencyRPhiViewV3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyRPhiViewV3Samples", "", 800, 800);
    m_cEfficiencyErrRPhiViewU3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrRPhiViewU3Samples", "", 800, 800);
    m_cEfficiencyErrRPhiViewV3Samples = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrRPhiViewV3Samples", "", 800, 800);

    m_hEfficiency3Samples = new SVDSummaryPlots("SVD3Efficiency@view",
                                                "Summary of SVD efficiencies (%), @view/@side Side for 3 samples");
    m_hEfficiency3Samples->setStats(0);
    if (m_setEfficiencyRange) {
      m_hEfficiency3Samples->setMaximum(m_efficiencyMax);
      m_hEfficiency3Samples->setMinimum(m_efficiencyMin);
    }
    m_hEfficiencyErr3Samples = new SVDSummaryPlots("SVD3EfficiencyErr@view",
                                                   "Summary of SVD efficiencies errors (%), @view/@side Side for 3 samples");
    m_hEfficiencyErr3Samples->setStats(0);
  }

  //register limits for EPICS
  registerEpicsPV(m_pvPrefix + "efficiencyLimits", "effLimits");

  //find nEvents testing if histograms are present
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents");
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  }
}

void DQMHistAnalysisSVDEfficiencyModule::beginRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: beginRun called.");

  if (m_cEfficiencyU)
    m_cEfficiencyU->Clear();
  if (m_cEfficiencyV)
    m_cEfficiencyV->Clear();
  if (m_cEfficiencyErrU)
    m_cEfficiencyErrU->Clear();
  if (m_cEfficiencyErrV)
    m_cEfficiencyErrV->Clear();

  if (m_cEfficiencyRPhiViewU)
    m_cEfficiencyRPhiViewU->Clear();
  if (m_cEfficiencyRPhiViewV)
    m_cEfficiencyRPhiViewV->Clear();
  if (m_cEfficiencyErrRPhiViewU)
    m_cEfficiencyErrRPhiViewU->Clear();
  if (m_cEfficiencyErrRPhiViewV)
    m_cEfficiencyErrRPhiViewV->Clear();

  if (m_3Samples) {
    if (m_cEfficiencyU3Samples)
      m_cEfficiencyU3Samples->Clear();
    if (m_cEfficiencyV3Samples)
      m_cEfficiencyV3Samples->Clear();
    if (m_cEfficiencyErrU3Samples)
      m_cEfficiencyErrU3Samples->Clear();
    if (m_cEfficiencyErrV3Samples)
      m_cEfficiencyErrV3Samples->Clear();

    if (m_cEfficiencyRPhiViewU3Samples)
      m_cEfficiencyRPhiViewU3Samples->Clear();
    if (m_cEfficiencyRPhiViewV3Samples)
      m_cEfficiencyRPhiViewV3Samples->Clear();
    if (m_cEfficiencyErrRPhiViewU3Samples)
      m_cEfficiencyErrRPhiViewU3Samples->Clear();
    if (m_cEfficiencyErrRPhiViewV3Samples)
      m_cEfficiencyErrRPhiViewV3Samples->Clear();
  }

  //Retrieve limits from EPICS
  double effErrorLo = 0.;
  double effWarnLo = 0.;

  requestLimitsFromEpicsPVs("effLimits", effErrorLo, effWarnLo, m_effWarning,  m_effError);

  B2DEBUG(10, " SVD efficiency thresholds taken from EPICS configuration file:");
  B2DEBUG(10, "  EFFICIENCY: normal > " << m_effWarning << " > warning > " << m_effError << " > error with minimum statistics of " <<
          m_statThreshold);

  //build the legend
  m_legProblem->Clear();
  m_legProblem->AddText("ERROR!");
  m_legProblem->AddText("at least one sensor with:");
  m_legProblem->AddText(Form("efficiency < %1.0f%%", m_effError * 100));

  m_legWarning->Clear();
  m_legWarning->AddText("WARNING!");
  m_legWarning->AddText("at least one sensor with:");
  m_legWarning->AddText(Form("%1.0f%% < efficiency < %1.0f%%", m_effError * 100, m_effWarning * 100));

  m_legNormal->Clear();
  m_legNormal->AddText("EFFICIENCY WITHIN LIMITS");
  m_legNormal->AddText(Form("efficiency > %1.0f%%", m_effWarning * 100));

  m_legLowStat->Clear();
  m_legLowStat->AddText("Not enough statistics,");
  m_legLowStat->AddText("check again in a few minutes");

  m_legEmpty->Clear();
  m_legEmpty->AddText("Track/clusters plots are emtpy");

  m_effUstatus = good;
  m_effVstatus = good;
}

void DQMHistAnalysisSVDEfficiencyModule::event()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: event called.");

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents", true);
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  } else {
    B2DEBUG(10, "SVDExpReco/SVDDQM_nEvents found");
  }

  TString tmp = hnEvnts->GetTitle();
  Int_t pos = tmp.Last('~');
  if (pos == -1) pos = 0;

  TString runID = tmp(pos, tmp.Length() - pos);
  B2INFO("DQMHistAnalysisSVDEfficiencyModule::runID = " << runID);

  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.1f");

  // do it by nhand, the interface of the SVDSummaryPlots does not allow to change the title after cstr
  if (m_hEfficiency) {
    m_hEfficiency->reset();
    m_hEfficiency->setRunID(runID);
  }

  if (m_hEfficiencyErr) {
    m_hEfficiencyErr->reset();
    m_hEfficiencyErr->setRunID(runID);
  }

  if (m_3Samples) {
    if (m_hEfficiency3Samples) {
      m_hEfficiency3Samples->reset();
      m_hEfficiency3Samples->setRunID(runID);
    }

    if (m_hEfficiencyErr3Samples) {
      m_hEfficiencyErr3Samples->reset();
      m_hEfficiencyErr3Samples->setRunID(runID);
    }
  }

  Float_t effU = -1;
  Float_t effV = -1;
  Float_t erreffU = -1;
  Float_t erreffV = -1;
  Float_t minEff = 99999;

  // Efficiency for the U and V sides
  TH2F* found_tracksU = (TH2F*)findHist("SVDEfficiency/TrackHitsU");
  TH2F* matched_clusU = (TH2F*)findHist("SVDEfficiency/MatchedHitsU");

  TH2F* found_tracksV = (TH2F*)findHist("SVDEfficiency/TrackHitsV");
  TH2F* matched_clusV = (TH2F*)findHist("SVDEfficiency/MatchedHitsV");

  if (matched_clusU != NULL && found_tracksU != NULL && matched_clusV != NULL && found_tracksV != NULL) {
    B2DEBUG(10, "Before loop on sensors, size :" << m_SVDModules.size());
    m_effUstatus = good;
    m_effVstatus = good;
    for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
      B2DEBUG(10, "module " << i << "," << m_SVDModules[i]);
      int bin = found_tracksU->FindBin(m_SVDModules[i].getLadderNumber(), findBinY(m_SVDModules[i].getLayerNumber(),
                                       m_SVDModules[i].getSensorNumber()));
      // U-side
      float numU = matched_clusU->GetBinContent(bin);
      float denU = found_tracksU->GetBinContent(bin);
      if (denU > 0)
        effU = numU / denU;
      if (effU < minEff) minEff = effU;
      B2DEBUG(10, "effU  = " << numU << "/" << denU << " = " << effU);
      m_hEfficiency->fill(m_SVDModules[i], 1, effU * 100);
      if (denU > 0)
        erreffU = std::sqrt(effU * (1 - effU) / denU);
      m_hEfficiencyErr->fill(m_SVDModules[i], 1, erreffU * 100);
      // V-side
      float numV = matched_clusV->GetBinContent(bin);
      float denV = found_tracksV->GetBinContent(bin);
      if (denV > 0)
        effV = numV / denV;
      if (effV < minEff) minEff = effV;
      B2DEBUG(10, "effV  = " << numV << "/" << denV << " = " << effV);
      m_hEfficiency->fill(m_SVDModules[i], 0, effV * 100);
      if (denV > 0)
        erreffV = std::sqrt(effV * (1 - effV) / denV);
      m_hEfficiencyErr->fill(m_SVDModules[i], 0, erreffV * 100);

      setEffStatus(denU, effU, true);
      setEffStatus(denV, effV, false);

      B2DEBUG(10, "Status U-side is " << m_effUstatus);
      B2DEBUG(10, "Status V-side is " << m_effVstatus);
    }
  } else {
    if (matched_clusU == NULL || found_tracksU == NULL) {
      B2INFO("Histograms needed for U-side Efficiency computation are not found");
      setEffStatus(-1, -1, true);
    }
    if (matched_clusV == NULL || found_tracksV == NULL) {
      B2INFO("Histograms needed for V-side Efficiency computation are not found");
      setEffStatus(-1, -1, false);
    }
  }

  // update summary for U side
  updateCanvases(m_hEfficiency, m_cEfficiencyU, m_cEfficiencyRPhiViewU,  m_effUstatus, minEff, 1);

  // update summary for V side
  updateCanvases(m_hEfficiency, m_cEfficiencyV, m_cEfficiencyRPhiViewV,  m_effVstatus, minEff, 0);

  // update error summary for U side
  updateErrCanvases(m_hEfficiencyErr, m_cEfficiencyErrU, m_cEfficiencyErrRPhiViewU, 1);

  // update error summary for V side
  updateErrCanvases(m_hEfficiencyErr, m_cEfficiencyErrV, m_cEfficiencyErrRPhiViewV, 0);

  if (m_3Samples) {
    /// ------ 3 samples ------
    m_hEfficiency3Samples->getHistogram(0)->Reset();
    m_hEfficiency3Samples->getHistogram(1)->Reset();
    m_hEfficiencyErr3Samples->getHistogram(0)->Reset();
    m_hEfficiencyErr3Samples->getHistogram(1)->Reset();

    // Efficiency for the U and V-side - 3 samples
    TH2F* found3_tracksU = (TH2F*)findHist("SVDEfficiency/TrackHits3U");
    TH2F* matched3_clusU = (TH2F*)findHist("SVDEfficiency/MatchedHits3U");

    TH2F* found3_tracksV = (TH2F*)findHist("SVDEfficiency/TrackHits3V");
    TH2F* matched3_clusV = (TH2F*)findHist("SVDEfficiency/MatchedHits3V");

    if (matched3_clusU != NULL && found3_tracksU != NULL && matched3_clusV != NULL && found3_tracksV != NULL) {
      B2DEBUG(10, "Before loop on sensors, size :" << m_SVDModules.size());
      m_effUstatus = good;
      m_effVstatus = good;
      for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
        B2DEBUG(10, "module " << i << "," << m_SVDModules[i]);
        int bin = found3_tracksU->FindBin(m_SVDModules[i].getLadderNumber(), findBinY(m_SVDModules[i].getLayerNumber(),
                                          m_SVDModules[i].getSensorNumber()));
        // U-side
        float numU = matched3_clusU->GetBinContent(bin);
        float denU = found3_tracksU->GetBinContent(bin);
        if (denU > 0)
          effU = numU / denU;
        if (effU < minEff) minEff = effU;
        B2DEBUG(10, "effU  = " << numU << "/" << denU << " = " << effU);
        m_hEfficiency3Samples->fill(m_SVDModules[i], 1, effU * 100);
        if (denU > 0)
          erreffU = std::sqrt(effU * (1 - effU) / denU);
        m_hEfficiencyErr3Samples->fill(m_SVDModules[i], 1, erreffU * 100);

        // V-side
        float numV = matched3_clusV->GetBinContent(bin);
        float denV = found3_tracksV->GetBinContent(bin);
        if (denV > 0)
          effV = numV / denV;
        if (effV < minEff) minEff = effV;
        B2DEBUG(10, "effV  = " << numV << "/" << denV << " = " << effV);
        m_hEfficiency3Samples->fill(m_SVDModules[i], 0, effV * 100);
        if (denV > 0)
          erreffV = std::sqrt(effV * (1 - effV) / denV);
        m_hEfficiencyErr3Samples->fill(m_SVDModules[i], 0, erreffV * 100);

        setEffStatus(denU, effU, true);
        setEffStatus(denV, effV, false);

        B2DEBUG(10, "Status U-side is " << m_effUstatus);
        B2DEBUG(10, "Status V-side is " << m_effVstatus);
      }
    } else {
      if (matched3_clusU == NULL || found3_tracksU == NULL) {
        B2INFO("Histograms needed for Efficiency computation are not found");
        setEffStatus(-1, -1, true);
      }
      if (matched3_clusV == NULL || found3_tracksV == NULL) {
        B2INFO("Histograms needed for Efficiency computation are not found");
        setEffStatus(-1, -1, false);
      }
    }

    // update summary for U side for 3 samples
    updateCanvases(m_hEfficiency3Samples, m_cEfficiencyU3Samples, m_cEfficiencyRPhiViewU3Samples,  m_effUstatus, minEff, 1);

    // update summary for V side for 3 samples
    updateCanvases(m_hEfficiency3Samples, m_cEfficiencyV3Samples, m_cEfficiencyRPhiViewV3Samples,  m_effUstatus, minEff, 0);

    // update error summary for U side for 3 samples
    updateErrCanvases(m_hEfficiencyErr3Samples, m_cEfficiencyErrU3Samples, m_cEfficiencyErrRPhiViewU3Samples, 1);

    // update error summary for V side for 3 samples
    updateErrCanvases(m_hEfficiencyErr3Samples, m_cEfficiencyErrV3Samples, m_cEfficiencyErrRPhiViewV3Samples, 0);
  }
}

void DQMHistAnalysisSVDEfficiencyModule::endRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency:  endRun called");
}

void DQMHistAnalysisSVDEfficiencyModule::terminate()
{
  B2DEBUG(10, "DQMHistAnalysisSVDEfficiency: terminate called");

  delete m_hEfficiency;
  delete m_cEfficiencyU;
  delete m_cEfficiencyV;
  delete m_hEfficiencyErr;
  delete m_cEfficiencyErrU;
  delete m_cEfficiencyErrV;

  delete m_cEfficiencyRPhiViewU;
  delete m_cEfficiencyRPhiViewV;
  delete m_cEfficiencyErrRPhiViewU;
  delete m_cEfficiencyErrRPhiViewV;

  for (int i = 0; i < (int)m_laddersText.size(); i++) delete m_laddersText[i];
  for (int i = 0; i < (int)m_sensorsText.size(); i++) delete m_sensorsText[i];

  delete m_ly;
  delete m_lx;
  delete m_arrowx;
  delete m_arrowy;

  if (m_3Samples) {
    delete m_hEfficiency3Samples;
    delete m_cEfficiencyU3Samples;
    delete m_cEfficiencyV3Samples;
    delete m_hEfficiencyErr3Samples;
    delete m_cEfficiencyErrU3Samples;
    delete m_cEfficiencyErrV3Samples;

    delete m_cEfficiencyRPhiViewU3Samples;
    delete m_cEfficiencyRPhiViewV3Samples;
    delete m_cEfficiencyErrRPhiViewU3Samples;
    delete m_cEfficiencyErrRPhiViewV3Samples;
  }
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


void DQMHistAnalysisSVDEfficiencyModule::setEffStatus(float den, float eff, bool sideU)
{
  svdStatus* efficiencyStatus;

  if (sideU)
    efficiencyStatus = &m_effUstatus;
  else
    efficiencyStatus = &m_effVstatus;

  if (den < 0) {
    *efficiencyStatus = std::max(noStat, *efficiencyStatus);
  } else if (den < m_statThreshold) {
    *efficiencyStatus = std::max(lowStat, *efficiencyStatus);
  } else if (eff > m_effWarning) {
    *efficiencyStatus = std::max(good, *efficiencyStatus);
  } else if ((eff <= m_effWarning) && (eff > m_effError)) {
    *efficiencyStatus = std::max(warning, *efficiencyStatus);
  } else if ((eff <= m_effError)) {
    *efficiencyStatus = std::max(error, *efficiencyStatus);
  }
}

void DQMHistAnalysisSVDEfficiencyModule::updateCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi,
                                                        svdStatus efficiencyStatus, float minEff, int side)
{
  canvas->Draw();
  canvas->cd();
  if (histo) {
    if (!m_setEfficiencyRange) histo->setMinimum(minEff * 99.9);
    histo->getHistogram(side)->Draw("text colz");
  }
  setStatusOfCanvas(efficiencyStatus, canvas);

  canvas->Modified();
  canvas->Update();

  canvasRPhi->Draw();
  canvasRPhi->cd();
  if (histo) {
    if (m_setEfficiencyRange) histo->getPoly(side, m_efficiencyMin, m_efficiencyMax)->Draw("colz l");
    else histo->getPoly(side)->Draw("colz l");
    drawText();
  }
  setStatusOfCanvas(efficiencyStatus, canvasRPhi, false);

  canvasRPhi->Modified();
  canvasRPhi->Update();
}

void DQMHistAnalysisSVDEfficiencyModule::updateErrCanvases(SVDSummaryPlots* histo, TCanvas* canvas, TCanvas* canvasRPhi, int side)
{
  canvas->Draw();
  canvas->cd();
  if (histo)
    histo->getHistogram(side)->Draw("text colz");

  canvas->Modified();
  canvas->Update();

  canvasRPhi->Draw();
  canvasRPhi->cd();
  if (histo) {
    histo->getPoly(side, 0)->Draw("colz l");
    drawText();
  }

  canvasRPhi->Modified();
  canvasRPhi->Update();
}
