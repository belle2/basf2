/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDGeneral.cc
// Description : module for DQM histogram analysis of SVD sensors occupancies
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDGeneral.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TAxis.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDGeneral);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDGeneralModule::DQMHistAnalysisSVDGeneralModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVDGeneral: Constructor done.");

  setDescription("DQM Analysis Module that produces colored canvas for a straightforward interpretation of the SVD Data Quality.");

  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("SVDrefHisto.root"));
  addParam("unpackerErrorLevel", m_unpackError, "Maximum bin_content/ # events allowed before throwing ERROR", double(0.00001));
  addParam("occLevel_Error", m_occError, "Maximum Occupancy (%) allowed for safe operations (red)", double(5));
  addParam("occLevel_Warning", m_occWarning, "Occupancy (%) at WARNING level (orange)", double(3));
  addParam("occLevel_Empty", m_occEmpty, "Maximum Occupancy (%) for which the sensor is considered empty", double(0));
  addParam("onlineOccLevel_Error", m_onlineOccError, "Maximum OnlineOccupancy (%) allowed for safe operations (red)", double(10));
  addParam("onlineOccLevel_Warning", m_onlineOccWarning, "OnlineOccupancy (%) at WARNING level (orange)", double(5));
  addParam("onlineOccLevel_Empty", m_onlineOccEmpty, "Maximum OnlineOccupancy (%) for which the sensor is considered empty",
           double(0));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));
  addParam("statThreshold", m_statThreshold, "Minimal number of events to compare histograms", double(10000.));
  addParam("timeThreshold", m_timeThreshold, "Acceptable difference between mean of central peak for present and reference run",
           double(6)); // 6 ns
  addParam("refMCTP", m_refMeanP, "Mean of the signal time peak from Physics reference run", float(0.0)); // Approximate, from exp 20
  addParam("refMCTC", m_refMeanC, "Mean of the signal time peak from Cosmic reference run", float(0.0));  //
  addParam("additionalPlots", m_additionalPlots, "Flag to produce additional plots",   bool(false));
  addParam("samples3", m_3Samples, "if True 3 samples histograms analysis is performed", bool(false));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("SVD:"));
}

DQMHistAnalysisSVDGeneralModule::~DQMHistAnalysisSVDGeneralModule() { }

void DQMHistAnalysisSVDGeneralModule::initialize()
{
  B2DEBUG(10, "DQMHistAnalysisSVDGeneral: initialized.");

  m_legError = new TPaveText(-1, 54, 3, 57.5);
  m_legError->AddText("ERROR!!");
  m_legError->SetFillColor(c_ColorDefault);
  m_legError->SetTextColor(c_ColorDefault);

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

  //occupancy chart chip
  m_cOccupancyChartChip = new TCanvas("SVDOccupancy/c_OccupancyChartChip");

  //strip occupancy per sensor
  if (m_additionalPlots)
    m_sensors = m_SVDModules.size();
  else
    m_sensors = 2;

  m_cStripOccupancyU = new TCanvas*[m_sensors];
  m_cStripOccupancyV = new TCanvas*[m_sensors];
  for (int i = 0; i < m_sensors; i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();
    m_cStripOccupancyU[i] = new TCanvas(Form("SVDOccupancy/c_StripOccupancyU_%d_%d_%d", tmp_layer, tmp_ladder, tmp_sensor));
    m_cStripOccupancyV[i] = new TCanvas(Form("SVDOccupancy/c_StripOccupancyV_%d_%d_%d", tmp_layer, tmp_ladder, tmp_sensor));
  }

  gROOT->cd();
  m_cUnpacker = new TCanvas("SVDAnalysis/c_SVDDataFormat");
  m_cUnpacker->SetGrid(1);
  m_cOccupancyU = new TCanvas("SVDAnalysis/c_SVDOccupancyU");
  m_cOccupancyV = new TCanvas("SVDAnalysis/c_SVDOccupancyV");

  m_cOnlineOccupancyU = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyU");
  m_cOnlineOccupancyV = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyV");

  m_cClusterOnTrackTime_L456V = new TCanvas("SVDAnalysis/c_ClusterOnTrackTime_L456V");

  if (m_3Samples) {
    m_cOccupancyU3Samples = new TCanvas("SVDAnalysis/c_SVDOccupancyU3Samples");
    //  m_cOccupancyU->SetGrid(1);
    m_cOccupancyV3Samples = new TCanvas("SVDAnalysis/c_SVDOccupancyV3Samples");
    //  m_cOccupancyV->SetGrid(1);

    m_cOnlineOccupancyU3Samples = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyU3Samples");
    //  m_cOnlineOccupancyU->SetGrid(1);
    m_cOnlineOccupancyV3Samples = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyV3Samples");
    //  m_cOnlineOccupancyV->SetGrid(1);
    m_cClusterOnTrackTimeL456V3Samples = new TCanvas("SVDAnalysis/c_ClusterOnTrackTime_L456V3Samples");
  }

  m_cOccupancyUGroupId0 = new TCanvas("SVDAnalysis/c_SVDOccupancyUGroupId0");
  m_cOccupancyVGroupId0 = new TCanvas("SVDAnalysis/c_SVDOccupancyVGroupId0");

  m_hOccupancy =  new SVDSummaryPlots("hOccupancy@view", "Average OFFLINE Sensor Occupancy (%), @view/@side Side");
  m_hOccupancy->setStats(0);

  m_hOnlineOccupancy  =  new SVDSummaryPlots("hOnlineOccupancy@view", "Average ONLINE Sensor Occupancy (%), @view/@side Side");
  m_hOnlineOccupancy->setStats(0);

  m_hOccupancyGroupId0  =  new SVDSummaryPlots("hOccupancyGroupId0@view",
                                               "Average OFFLINE Sensor Occupancy (%), @view/@side Side for cluster time group Id = 0");
  m_hOccupancyGroupId0->setStats(0);

  if (m_3Samples) {
    m_hOccupancy3Samples  =  new SVDSummaryPlots("hOccupancy3@view",
                                                 "Average OFFLINE Sensor Occupancy (%), @view/@side Side for 3 samples");
    m_hOnlineOccupancy->setStats(0);

    m_hOnlineOccupancy3Samples  =  new SVDSummaryPlots("hOnlineOccupancy3@view",
                                                       "Average ONLINE Sensor Occupancy (%), @view/@side Side for 3 samples");
    m_hOnlineOccupancy3Samples->setStats(0);
  }


  //register limits for EPICS
  registerEpicsPV(m_pvPrefix + "ratio3_6", "ratio3_6");
  registerEpicsPV(m_pvPrefix + "UnpackError", "UnpackError");
  registerEpicsPV(m_pvPrefix + "occupancyLimits", "occLimits");
  registerEpicsPV(m_pvPrefix + "occupancyOnlineLimits", "occOnlineLimits");
  registerEpicsPV(m_pvPrefix + "clusterTimeOnTrackLimits", "clusTimeOnTrkLimits");
}


void DQMHistAnalysisSVDGeneralModule::beginRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDGeneral: beginRun called.");
  m_cUnpacker->Clear();
  m_cOccupancyU->Clear();
  m_cOccupancyV->Clear();

  m_cOnlineOccupancyU->Clear();
  m_cOnlineOccupancyV->Clear();
  m_cOccupancyChartChip->Clear();
  for (int i = 0; i < m_sensors; i++) {
    m_cStripOccupancyU[i]->Clear();
    m_cStripOccupancyV[i]->Clear();
  }

  m_cClusterOnTrackTime_L456V->Clear();

  if (m_3Samples) {
    m_cOccupancyU3Samples->Clear();
    m_cOccupancyV3Samples->Clear();
    m_cOnlineOccupancyU3Samples->Clear();
    m_cOnlineOccupancyV3Samples->Clear();
    m_cClusterOnTrackTimeL456V3Samples->Clear();
  }
  m_cOccupancyUGroupId0->Clear();
  m_cOccupancyVGroupId0->Clear();

  //Retrieve limits from EPICS
  double oocErrorLoOff = 0.;
  double oocErrorLoOn = 0.;
  requestLimitsFromEpicsPVs("occLimits", oocErrorLoOff, m_occEmpty, m_occWarning,  m_occError);
  requestLimitsFromEpicsPVs("occOnlineLimits", oocErrorLoOn, m_onlineOccEmpty, m_onlineOccWarning,  m_onlineOccError);

  B2DEBUG(10, " SVD occupancy thresholds taken from EPICS configuration file:");
  B2DEBUG(10, "  ONLINE OCCUPANCY: empty < " << m_onlineOccEmpty << " normal < " << m_onlineOccWarning << " warning < " <<
          m_onlineOccError <<
          " < error");
  B2DEBUG(10, "  OFFLINE OCCUPANCY: empty < " << m_occEmpty << " normal < " << m_occWarning << " warning < " << m_occError <<
          " < error with minimum statistics of " << m_occEmpty);

  double timeWarnUp = 0.;
  double timeErrorLo = 0.;
  double timeWarnLo = 0.;
  requestLimitsFromEpicsPVs("clusTimeOnTrkLimits", timeErrorLo, timeWarnLo, timeWarnUp,  m_timeThreshold);
  B2DEBUG(10, " SVD cluster time on track threshold taken from EPICS configuration file:");
  B2DEBUG(10, "  CLUSTER TIME ON TRACK: error > " << m_timeThreshold << " ns with minimum statistics of " << m_statThreshold);

  double unpackWarnLo = 0.;
  double unpackWarnUp = 0.;
  double unpackErrorLo = 0.;
  requestLimitsFromEpicsPVs("UnpackError", unpackErrorLo, unpackWarnLo, unpackWarnUp,  m_unpackError);
  B2DEBUG(10, " SVD unpack error threshold taken from EPICS configuration file:");
  B2DEBUG(10, "  DATA UNPACK: error > " << m_unpackError);

  // Create text panel
  //OFFLINE occupancy plots legend
  m_legProblem = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legProblem->AddText("ERROR!");
  m_legProblem->AddText("at least one sensor with:");
  m_legProblem->AddText(Form("occupancy > %1.1f%%", m_occError));
  m_legProblem->SetFillColor(c_ColorDefault);
  m_legProblem->SetLineColor(kBlack);

  m_legWarning = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legWarning->AddText("WARNING!");
  m_legWarning->AddText("at least one sensor with:");
  m_legWarning->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occWarning, m_occError));
  m_legWarning->SetFillColor(c_ColorDefault);
  m_legWarning->SetLineColor(kBlack);

  m_legNormal = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legNormal->AddText("OCCUPANCY WITHIN LIMITS");
  m_legNormal->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occEmpty, m_occWarning));
  m_legNormal->SetFillColor(c_ColorDefault);
  m_legNormal->SetLineColor(kBlack);

  m_legEmpty = new TPaveText(11, findBinY(4, 3) - 2, 16, findBinY(4, 3));
  m_legEmpty->AddText("NO DATA RECEIVED");
  m_legEmpty->AddText("from at least one sensor");
  m_legEmpty->SetTextColor(c_ColorDefault);
  m_legEmpty->SetLineColor(kBlack);

  //ONLINE occupancy plots legend
  m_legOnProblem = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legOnProblem->AddText("ERROR!");
  m_legOnProblem->AddText("at least one sensor with:");
  m_legOnProblem->AddText(Form("online occupancy > %1.1f%%", m_onlineOccError));
  m_legOnProblem->SetFillColor(c_ColorDefault);
  m_legOnProblem->SetLineColor(kBlack);

  m_legOnWarning = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legOnWarning->AddText("WARNING!");
  m_legOnWarning->AddText("at least one sensor with:");
  m_legOnWarning->AddText(Form("%1.1f%% < online occupancy < %1.1f%%", m_onlineOccWarning, m_onlineOccError));
  m_legOnWarning->SetFillColor(c_ColorDefault);
  m_legOnWarning->SetLineColor(kBlack);

  m_legOnNormal = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legOnNormal->AddText("OCCUPANCY WITHIN LIMITS");
  m_legOnNormal->AddText(Form("%1.1f%% < online occupancy < %1.1f%%", m_onlineOccEmpty, m_onlineOccWarning));
  m_legOnNormal->SetFillColor(c_ColorDefault);
  m_legOnNormal->SetLineColor(kBlack);

  m_legOnEmpty = new TPaveText(11, findBinY(4, 3) - 2, 16, findBinY(4, 3));
  m_legOnEmpty->AddText("NO DATA RECEIVED");
  m_legOnEmpty->AddText("from at least one sensor");
  m_legOnEmpty->SetFillColor(c_ColorDefault);
  m_legOnEmpty->SetLineColor(kBlack);


  // cluster time on tracks legend
  m_legTiProblem = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTiProblem->AddText("ERROR!");
  m_legTiProblem->AddText(Form("abs(Mean) > %3.1f ns", m_timeThreshold));
  m_legTiProblem->SetFillColor(c_ColorDefault);
  m_legTiProblem->SetLineColor(kBlack);

  m_legTiNormal = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTiNormal->AddText("TIME SHIFT UNDER LIMIT");
  m_legTiNormal->AddText(Form("abs(Mean) < %3.1f ns", m_timeThreshold));
  m_legTiNormal->SetFillColor(c_ColorDefault);
  m_legTiNormal->SetLineColor(kBlack);

  m_legTiEmpty = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTiEmpty->AddText("Not enough statistics");
  m_legTiEmpty->SetFillColor(c_ColorDefault);
  m_legTiEmpty->SetLineColor(kBlack);

  m_legTi3Problem = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTi3Problem->AddText("ERROR!");
  m_legTi3Problem->AddText(Form("abs(Mean) > %3.1f ns", m_timeThreshold));
  m_legTi3Problem->SetFillColor(c_ColorDefault);
  m_legTi3Problem->SetLineColor(kBlack);

  m_legTi3Normal = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTi3Normal->AddText("TIME SHIFT UNDER LIMIT");
  m_legTi3Normal->AddText(Form("abs(Mean) < %3.1f ns", m_timeThreshold));
  m_legTi3Normal->SetFillColor(c_ColorDefault);
  m_legTi3Normal->SetLineColor(kBlack);

  m_legTi3Empty = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTi3Empty->AddText("Not enough statistics");
  m_legTi3Empty->SetFillColor(kBlack);
  m_legTi3Empty->SetLineColor(kBlack);
}

void DQMHistAnalysisSVDGeneralModule::event()
{
  B2DEBUG(10, "DQMHistAnalysisSVDGeneral: event called.");

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents", true);
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  } else {
    B2DEBUG(10, "SVDExpReco/SVDDQM_nEvents found");
  }

  TH1* rtype = findHist("DQMInfo/rtype");
  if (rtype)
    B2DEBUG(10, "DQMInfo/rtype found");

  m_runtype = rtype ? rtype->GetTitle() : "physics"; // per default

  TString tmp = hnEvnts->GetTitle();
  Int_t pos = tmp.Last('~');
  if (pos == -1) pos = 0;

  TString runID = tmp(pos, tmp.Length() - pos);
  B2INFO("DQMHistAnalysisSVDGeneralModule::runID = " << runID);
  Float_t nEvents = hnEvnts->GetEntries();

  //check DATA FORMAT
  TH1* h = findHist("SVDUnpacker/DQMUnpackerHisto");

  //test ERROR:
  //  h->SetBinContent(100,0.01);

  if (h != NULL) {
    h->SetTitle(Form("SVD Data Format Monitor %s", runID.Data()));
    //check if number of errors is above the allowed limit
    bool hasError = false;
    for (int un = 0; un < h->GetNcells(); un++)
      if (h->GetBinContent(un) / nEvents > m_unpackError)
        hasError = true;
    if (! hasError) {
      m_cUnpacker->cd();
      h->Draw("colztext");
      h->SetStats(0);
      colorizeCanvas(m_cUnpacker, c_StatusGood);
    } else {
      m_cUnpacker->cd();
      h->Draw("colztext");
      h->SetStats(0);
      m_legError->Draw();
      colorizeCanvas(m_cUnpacker, c_StatusError);
    }
    if (nEvents > 0)
      setEpicsPV("UnpackError", h->GetEntries() / nEvents);
  } else {
    B2INFO("Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackerDQM not found!");
    m_cUnpacker->cd(1);
    colorizeCanvas(m_cUnpacker, c_StatusDefault);
  }

  m_cUnpacker->Modified();
  m_cUnpacker->Update();

  if (m_printCanvas)
    m_cUnpacker->Print("c_SVDDataFormat.pdf");

  //occupancy chart
  TH1F* hChart = (TH1F*)findHist("SVDExpReco/SVDDQM_StripCountsChip");

  if (hChart != NULL) {
    m_hOccupancyChartChip.Clear();
    hChart->Copy(m_hOccupancyChartChip);
    m_hOccupancyChartChip.SetName("SVDOccupancyChart");
    m_hOccupancyChartChip.SetTitle(Form("SVD OFFLINE Occupancy per chip %s", runID.Data()));
    m_hOccupancyChartChip.Scale(1 / nEvents / 128);
    m_cOccupancyChartChip->cd();
    //    m_hOccupancyChartChip->SetStats(0);
    m_hOccupancyChartChip.Draw();
  }
  m_cOccupancyChartChip->Modified();
  m_cOccupancyChartChip->Update();

  if (m_printCanvas)
    m_cOccupancyChartChip->Print("c_OccupancyChartChip.pdf");

  // cluster time for clusters of track
  double ratio3_6 = 0.;
  TH1* m_h = findHist("SVDClsTrk/SVDTRK_ClusterTimeV456");
  bool hasError = false;
  bool lowStat = false;

  if (m_h != NULL) {
    m_hClusterOnTrackTime_L456V.Clear();
    m_h->Copy(m_hClusterOnTrackTime_L456V);
    m_hClusterOnTrackTime_L456V.GetXaxis()->SetRange(110, 190); // [-40 ns,40 ns]
    Float_t mean_PeakInCenter = m_hClusterOnTrackTime_L456V.GetMean(); //
    m_hClusterOnTrackTime_L456V.GetXaxis()->SetRange(); // back to [-150 ns,150 ns]
    m_hClusterOnTrackTime_L456V.SetTitle(Form("ClusterOnTrack Time L456V %s", runID.Data()));

    if (nEvents > m_statThreshold) {
      if (m_runtype == "physics") {
        Float_t difference_physics = fabs(mean_PeakInCenter - m_refMeanP);
        if (difference_physics > m_timeThreshold) {
          hasError = true;
        }
      } else if (m_runtype == "cosmic") {
        Float_t difference_cosmic = fabs(mean_PeakInCenter - m_refMeanC);
        if (difference_cosmic > m_timeThreshold) {
          hasError = true;
        }
      } else { // taking cosmic limits
        B2WARNING("Run type:" << m_runtype << "taken cosmics criteria");
        Float_t difference_cosmic = fabs(mean_PeakInCenter - m_refMeanC);
        if (difference_cosmic > m_timeThreshold)
          hasError = true;
      }
    } else {
      lowStat = true;
    }

    if (! hasError) {
      m_cClusterOnTrackTime_L456V->cd();
      m_hClusterOnTrackTime_L456V.Draw();
      colorizeCanvas(m_cClusterOnTrackTime_L456V, c_StatusGood);
    } else {
      m_cClusterOnTrackTime_L456V->cd();
      m_hClusterOnTrackTime_L456V.Draw();
      colorizeCanvas(m_cClusterOnTrackTime_L456V, c_StatusError);
    }

    if (lowStat) {
      m_cClusterOnTrackTime_L456V->cd();
      m_hClusterOnTrackTime_L456V.Draw();
      colorizeCanvas(m_cClusterOnTrackTime_L456V, c_StatusTooFew);
    }

  } else {
    B2INFO("Histogram SVDClsTrk/c_SVDTRK_ClusterTimeV456 from SVDDQMClustersOnTrack module not found!");
    m_cClusterOnTrackTime_L456V->cd();
    m_hClusterOnTrackTime_L456V.Draw();
    colorizeCanvas(m_cClusterOnTrackTime_L456V, c_StatusDefault);
  }

  if (hasError)
    m_legTiProblem->Draw();
  else if (lowStat)
    m_legTiEmpty->Draw();
  else
    m_legTiNormal->Draw();


  m_cClusterOnTrackTime_L456V->Modified();
  m_cClusterOnTrackTime_L456V->Update();

  if (m_printCanvas)
    m_cClusterOnTrackTime_L456V->Print("c_SVDClusterOnTrackTime_L456V.pdf");


  // cluster time for clusters of track for 3 samples
  if (m_3Samples) {
    m_h = findHist("SVDClsTrk/SVDTRK_Cluster3TimeV456");
    bool hasError3 = false;
    bool lowStat3 = false;

    if (m_h != NULL) {
      m_hClusterOnTrackTimeL456V3Samples.Clear();
      m_h->Copy(m_hClusterOnTrackTimeL456V3Samples);
      m_hClusterOnTrackTimeL456V3Samples.GetXaxis()->SetRange(110, 190); // [-40 ns,40 ns]
      Float_t mean_PeakInCenter = m_hClusterOnTrackTimeL456V3Samples.GetMean(); //
      m_hClusterOnTrackTimeL456V3Samples.GetXaxis()->SetRange(); // back to [-150 ns,150 ns]
      m_hClusterOnTrackTimeL456V3Samples.SetTitle(Form("ClusterOnTrack Time L456V 3 samples %s", runID.Data()));

      if (nEvents > m_statThreshold) {
        if (m_runtype == "physics") {
          Float_t difference_physics = fabs(mean_PeakInCenter - m_refMeanP);
          if (difference_physics > m_timeThreshold) {
            hasError3 = true;
          }
        } else if (m_runtype == "cosmic") {
          Float_t difference_cosmic = fabs(mean_PeakInCenter - m_refMeanC);
          if (difference_cosmic > m_timeThreshold) {
            hasError3 = true;
          }
        } else {
          B2WARNING("Run type:" << m_runtype);
        }
      } else {
        lowStat3 = true;
      }
      if (! hasError3) {
        m_cClusterOnTrackTimeL456V3Samples->cd();
        m_hClusterOnTrackTimeL456V3Samples.Draw();
        colorizeCanvas(m_cClusterOnTrackTimeL456V3Samples, c_StatusGood);
      } else {
        m_cClusterOnTrackTimeL456V3Samples->cd();
        m_hClusterOnTrackTimeL456V3Samples.Draw();
        colorizeCanvas(m_cClusterOnTrackTimeL456V3Samples, c_StatusError);
      }

      if (lowStat3) {
        m_cClusterOnTrackTimeL456V3Samples->cd();
        m_hClusterOnTrackTimeL456V3Samples.Draw();
        colorizeCanvas(m_cClusterOnTrackTimeL456V3Samples, c_StatusTooFew);
      }

    } else {
      B2INFO("Histogram SVDClsTrk/c_SVDTRK_Cluster3TimeV456 from SVDDQMClustersOnTrack module not found!");
      m_cClusterOnTrackTimeL456V3Samples->cd();
      m_hClusterOnTrackTimeL456V3Samples.Draw();
      colorizeCanvas(m_cClusterOnTrackTimeL456V3Samples, c_StatusDefault);
    }

    if (hasError3)
      m_legTi3Problem->Draw();
    else if (lowStat3)
      m_legTi3Empty->Draw();
    else
      m_legTi3Normal->Draw();

    m_cClusterOnTrackTimeL456V3Samples->Modified();
    m_cClusterOnTrackTimeL456V3Samples->Update();

    if (m_printCanvas)
      m_cClusterOnTrackTimeL456V3Samples->Print("c_SVDClusterOnTrack3Time_L456V.pdf");

    ratio3_6 = m_hClusterOnTrackTimeL456V3Samples.GetEntries() / m_hClusterOnTrackTime_L456V.GetEntries();
  }

  setEpicsPV("ratio3_6", ratio3_6);

  //check MODULE OCCUPANCY online & offline
  //reset canvas color
  m_occUstatus = 0;
  m_occVstatus = 0;
  m_onlineOccUstatus = 0;
  m_onlineOccVstatus = 0;
  m_occUGroupId0 = 0;
  m_occVGroupId0 = 0;

  m_onlineOccU3Samples = 0;
  m_onlineOccV3Samples = 0;

  m_occU3Samples = 0;
  m_occV3Samples = 0;

  //update titles with exp and run number

  m_hOccupancy->reset();
  m_hOccupancy->setStats(0);
  m_hOccupancy->setRunID(runID);

  m_hOnlineOccupancy->reset();
  m_hOnlineOccupancy->setStats(0);
  m_hOnlineOccupancy->setRunID(runID);

  m_hOccupancyGroupId0->reset();
  m_hOccupancyGroupId0->setStats(0);
  m_hOccupancyGroupId0->setRunID(runID);

  if (m_3Samples) {
    m_hOccupancy3Samples->reset();
    m_hOccupancy3Samples->setStats(0);
    m_hOccupancy3Samples->setRunID(runID);

    m_hOnlineOccupancy3Samples->reset();
    m_hOnlineOccupancy3Samples->setStats(0);
    m_hOnlineOccupancy3Samples->setRunID(runID);
  }

  //set dedicate gStyle
  const Int_t colNum = 4;
  Int_t palette[colNum] {kBlack,  c_ColorGood, c_ColorWarning, c_ColorError};
  gStyle->SetPalette(colNum, palette);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.3f");

  TH1F* htmp = NULL;

  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();

    //look for U histogram - OFFLINE ZS
    TString tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram not found");
      m_cOccupancyU->Draw();
      m_cOccupancyU->cd();
      m_hOccupancy->getHistogram(1)->Draw("text");
      colorizeCanvas(m_cOccupancyU, c_StatusDefault);
    } else {

      Int_t nStrips = 768;

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancy->fill(m_SVDModules[i], 1, occU);

      if (occU <= m_occEmpty) {
        if (m_occUstatus < 1) m_occUstatus = 1;
      } else if (occU > m_occWarning) {
        if (occU < m_occError) {
          if (m_occUstatus < 2) m_occUstatus = 2;
        } else {
          if (m_occUstatus < 3) m_occUstatus = 3;
        }
      }

      //produce the occupancy plot
      if (m_additionalPlots) {
        m_hStripOccupancyU[i].Clear();
        htmp->Copy(m_hStripOccupancyU[i]);
        m_hStripOccupancyU[i].Scale(1 / nEvents);
        m_hStripOccupancyU[i].SetName(Form("%d_%d_%d_OccupancyU", tmp_layer, tmp_ladder, tmp_sensor));
        m_hStripOccupancyU[i].SetTitle(Form("SVD Sensor %d_%d_%d U-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                            tmp_sensor, runID.Data()));
      } else {
        if (i == 0 || i == 1) {
          m_hStripOccupancyU[i].Clear();
          htmp->Copy(m_hStripOccupancyU[i]);
          m_hStripOccupancyU[i].Scale(1 / nEvents);
          m_hStripOccupancyU[i].SetName(Form("%d_%d_%d_OccupancyU", tmp_layer, tmp_ladder, tmp_sensor));
          m_hStripOccupancyU[i].SetTitle(Form("SVD Sensor %d_%d_%d U-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                              tmp_sensor, runID.Data()));
        }
      }
    }

    if (m_3Samples) {
      //look for U histogram - OFFLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_Strip3CountU", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("Occupancy U histogram not found for 3 samples");
        m_cOccupancyU3Samples->Draw();
        m_cOccupancyU3Samples->cd();
        m_hOccupancy3Samples->getHistogram(1)->Draw("text");
        colorizeCanvas(m_cOccupancyU3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;

        Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOccupancy3Samples->fill(m_SVDModules[i], 1, occU);


        if (occU <= m_occEmpty) {
          if (m_occU3Samples < 1) m_occU3Samples = 1;
        } else if (occU > m_occWarning) {
          if (occU < m_occError) {
            if (m_occU3Samples < 2) m_occU3Samples = 2;
          } else {
            if (m_occU3Samples < 3) m_occU3Samples = 3;
          }
        }
      }
    }

    // groupId0 side U
    TString tmpnameGrpId0 = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountGroupId0U", tmp_layer, tmp_ladder, tmp_sensor);
    htmp = (TH1F*)findHist(tmpnameGrpId0.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram for group Id0 not found");
      m_cOccupancyUGroupId0->Draw();
      m_cOccupancyUGroupId0->cd();
      m_hOccupancyGroupId0->getHistogram(1)->Draw("text");
      colorizeCanvas(m_cOccupancyUGroupId0, c_StatusDefault);
    } else {

      Int_t nStrips = 768;

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyGroupId0->fill(m_SVDModules[i], 1, occU);

      if (occU <= m_occEmpty) {
        if (m_occUGroupId0 < 1) m_occUGroupId0 = 1;
      } else if (occU > m_occWarning) {
        if (occU < m_occError) {
          if (m_occUGroupId0 < 2) m_occUGroupId0 = 2;
        } else {
          if (m_occUGroupId0 < 3) m_occUGroupId0 = 3;
        }
      }
    }

    //look for V histogram - OFFLINE ZS
    tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountV", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy V histogram not found");
      m_cOccupancyV->Draw();
      m_cOccupancyV->cd();
      m_hOccupancy->getHistogram(0)->Draw("text");
      colorizeCanvas(m_cOccupancyV, c_StatusDefault);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancy->fill(m_SVDModules[i], 0, occV);

      if (occV <= m_occEmpty) {
        if (m_occVstatus < 1) m_occVstatus = 1;
      } else if (occV > m_occWarning) {
        if (occV < m_occError) {
          if (m_occVstatus < 2) m_occVstatus = 2;
        } else {
          if (m_occVstatus < 3) m_occVstatus = 3;
        }
      }
      //produce the occupancy plot
      if (m_additionalPlots) {
        m_hStripOccupancyV[i].Clear();
        htmp->Copy(m_hStripOccupancyV[i]);
        m_hStripOccupancyV[i].Scale(1 / nEvents);
        m_hStripOccupancyV[i].SetName(Form("%d_%d_%d_OccupancyV", tmp_layer, tmp_ladder, tmp_sensor));
        m_hStripOccupancyV[i].SetTitle(Form("SVD Sensor %d_%d_%d V-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                            tmp_sensor, runID.Data()));
      } else {
        if (i == 0 || i == 1) {
          m_hStripOccupancyV[i].Clear();
          htmp->Copy(m_hStripOccupancyV[i]);
          m_hStripOccupancyV[i].Scale(1 / nEvents);
          m_hStripOccupancyV[i].SetName(Form("%d_%d_%d_OccupancyV", tmp_layer, tmp_ladder, tmp_sensor));
          m_hStripOccupancyV[i].SetTitle(Form("SVD Sensor %d_%d_%d V-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                              tmp_sensor, runID.Data()));
        }
      }
    }

    if (m_3Samples) {
      //look for V histogram - OFFLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_Strip3CountV", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("Occupancy V histogram not found");
        m_cOccupancyV3Samples->Draw();
        m_cOccupancyV3Samples->cd();
        m_hOccupancy3Samples->getHistogram(0)->Draw("text");
        colorizeCanvas(m_cOccupancyV3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;
        if (tmp_layer != 3)
          nStrips = 512;

        Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOccupancy3Samples->fill(m_SVDModules[i], 0, occV);

        if (occV <= m_occEmpty) {
          if (m_occV3Samples < 1) m_occV3Samples = 1;
        } else if (occV > m_occWarning) {
          if (occV < m_occError) {
            if (m_occV3Samples < 2) m_occV3Samples = 2;
          } else {
            if (m_occV3Samples < 3) m_occV3Samples = 3;
          }
        }
      }
    }

    // groupId0 side V
    tmpnameGrpId0 = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountGroupId0V", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpnameGrpId0.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram for group Id0 not found");
      m_cOccupancyVGroupId0->Draw();
      m_cOccupancyVGroupId0->cd();
      m_hOccupancyGroupId0->getHistogram(0)->Draw("text");
      colorizeCanvas(m_cOccupancyVGroupId0, c_StatusDefault);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyGroupId0->fill(m_SVDModules[i], 0, occV);

      if (occV <= m_occEmpty) {
        if (m_occVGroupId0 < 1) m_occVGroupId0 = 1;
      } else if (occV > m_occWarning) {
        if (occV < m_occError) {
          if (m_occVGroupId0 < 2) m_occVGroupId0 = 2;
        } else {
          if (m_occVGroupId0 < 3) m_occVGroupId0 = 3;
        }
      }
    }

    //look for V histogram - ONLINE ZS
    tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStripCountV", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("OnlineOccupancy V histogram not found");
      m_cOnlineOccupancyV->Draw();
      m_cOnlineOccupancyV->cd();
      m_hOnlineOccupancy->getHistogram(0)->Draw("text");
      colorizeCanvas(m_cOnlineOccupancyV, c_StatusDefault);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t onlineOccV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOnlineOccupancy->fill(m_SVDModules[i], 0, onlineOccV);


      for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
        htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
      }
      htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");

      if (onlineOccV <= m_onlineOccEmpty) {
        if (m_onlineOccVstatus < 1) m_onlineOccVstatus = 1;
      } else if (onlineOccV > m_onlineOccWarning) {
        if (onlineOccV < m_onlineOccError) {
          if (m_onlineOccVstatus < 2) m_onlineOccVstatus = 2;
        } else {
          if (m_onlineOccVstatus < 3) m_onlineOccVstatus = 3;
        }
      }
    }

    if (m_3Samples) {
      //look for V histogram - ONLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStrip3CountV", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("OnlineOccupancy3 V histogram not found");
        m_cOnlineOccupancyV3Samples->Draw();
        m_cOnlineOccupancyV3Samples->cd();
        m_hOnlineOccupancy3Samples->getHistogram(0)->Draw("text");
        colorizeCanvas(m_cOnlineOccupancyV3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;
        if (tmp_layer != 3)
          nStrips = 512;

        Float_t onlineOccV = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOnlineOccupancy3Samples->fill(m_SVDModules[i], 0, onlineOccV);

        for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
          htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
        }
        htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");

        if (onlineOccV <= m_onlineOccEmpty) {
          if (m_onlineOccV3Samples < 1) m_onlineOccV3Samples = 1;
        } else if (onlineOccV > m_onlineOccWarning) {
          if (onlineOccV < m_onlineOccError) {
            if (m_onlineOccV3Samples < 2) m_onlineOccV3Samples = 2;
          } else {
            if (m_onlineOccV3Samples < 3) m_onlineOccV3Samples = 3;
          }
        }
      }
    }


    //look for U histogram - ONLINE ZS
    tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("OnlineOccupancy U histogram not found");
      m_cOnlineOccupancyU->Draw();
      m_cOnlineOccupancyU->cd();
      m_hOnlineOccupancy->getHistogram(1)->Draw("text");
      colorizeCanvas(m_cOnlineOccupancyU, c_StatusDefault);
    } else {

      Int_t nStrips = 768;

      Float_t onlineOccU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOnlineOccupancy->fill(m_SVDModules[i], 1, onlineOccU);

      for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
        htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
      }
      htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");

      if (onlineOccU <= m_onlineOccEmpty) {
        if (m_onlineOccUstatus < 1) m_onlineOccUstatus = 1;
      } else if (onlineOccU > m_onlineOccWarning) {
        if (onlineOccU < m_onlineOccError) {
          if (m_onlineOccUstatus < 2) m_onlineOccUstatus = 2;
        } else {
          if (m_onlineOccUstatus < 3) m_onlineOccUstatus = 3;
        }
      }
    }

    if (m_3Samples) {
      //look for U histogram - ONLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStrip3CountU", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("OnlineOccupancy3 U histogram not found");
        m_cOnlineOccupancyU3Samples->Draw();
        m_cOnlineOccupancyU3Samples->cd();
        m_hOnlineOccupancy3Samples->getHistogram(1)->Draw("text");
        colorizeCanvas(m_cOnlineOccupancyU3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;

        Float_t onlineOccU = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOnlineOccupancy3Samples->fill(m_SVDModules[i], 1, onlineOccU);

        for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
          htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
        }
        htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");

        if (onlineOccU <= m_onlineOccEmpty) {
          if (m_onlineOccU3Samples < 1) m_onlineOccU3Samples = 1;
        } else if (onlineOccU > m_onlineOccWarning) {
          if (onlineOccU < m_onlineOccError) {
            if (m_onlineOccU3Samples < 2) m_onlineOccU3Samples = 2;
          } else {
            if (m_onlineOccU3Samples < 3) m_onlineOccU3Samples = 3;
          }
        }
      }
    }

    //update sensor occupancy canvas U and V
    if (m_additionalPlots) {
      m_cStripOccupancyU[i]->cd();
      m_hStripOccupancyU[i].Draw("histo");
      m_cStripOccupancyV[i]->cd();
      m_hStripOccupancyV[i].Draw("histo");
    } else {
      if (i == 0 || i == 1) {
        m_cStripOccupancyU[i]->cd();
        m_hStripOccupancyU[i].Draw("histo");
        m_cStripOccupancyV[i]->cd();
        m_hStripOccupancyV[i].Draw("histo");
      }
    }
  }

  //update summary offline occupancy U canvas
  m_cOccupancyU->Draw();
  m_cOccupancyU->cd();
  m_hOccupancy->getHistogram(1)->Draw("text");

  if (m_occUstatus == 0) {
    colorizeCanvas(m_cOccupancyU, c_StatusGood);
    m_legNormal->Draw();
  } else {
    if (m_occUstatus == 3) {
      colorizeCanvas(m_cOccupancyU, c_StatusError);
      m_legProblem->Draw();
    }
    if (m_occUstatus == 2) {
      colorizeCanvas(m_cOccupancyU, c_StatusWarning);
      m_legWarning->Draw();
    }
    if (m_occUstatus == 1) {
      colorizeCanvas(m_cOccupancyU, c_StatusTooFew);
      m_legEmpty->Draw();
    }
  }
  m_cOccupancyU->Update();
  m_cOccupancyU->Modified();
  m_cOccupancyU->Update();

  if (m_3Samples) {
    //update summary offline occupancy U canvas for 3 samples
    m_cOccupancyU3Samples->Draw();
    m_cOccupancyU3Samples->cd();
    m_hOccupancy3Samples->getHistogram(1)->Draw("text");

    if (m_occU3Samples == 0) {
      colorizeCanvas(m_cOccupancyU3Samples, c_StatusGood);
      m_legNormal->Draw();
    } else {
      if (m_occU3Samples == 3) {
        colorizeCanvas(m_cOccupancyU3Samples, c_StatusError);
        m_legProblem->Draw();
      }
      if (m_occU3Samples == 2) {
        colorizeCanvas(m_cOccupancyU3Samples, c_StatusWarning);
        m_legWarning->Draw();
      }
      if (m_occU3Samples == 1) {
        colorizeCanvas(m_cOccupancyU3Samples, c_StatusTooFew);
        m_legEmpty->Draw();
      }
    }
    m_cOccupancyU3Samples->Update();
    m_cOccupancyU3Samples->Modified();
    m_cOccupancyU3Samples->Update();
  }

  //update summary offline occupancy U canvas for groupId0
  m_cOccupancyUGroupId0->Draw();
  m_cOccupancyUGroupId0->cd();
  m_hOccupancyGroupId0->getHistogram(1)->Draw("text");

  if (m_occUGroupId0 == 0) {
    colorizeCanvas(m_cOccupancyUGroupId0, c_StatusGood);
    m_legNormal->Draw();
  } else {
    if (m_occUGroupId0 == 3) {
      colorizeCanvas(m_cOccupancyUGroupId0, c_StatusError);
      m_legProblem->Draw();
    }
    if (m_occUGroupId0 == 2) {
      colorizeCanvas(m_cOccupancyUGroupId0, c_StatusWarning);
      m_legWarning->Draw();
    }
    if (m_occUGroupId0 == 1) {
      colorizeCanvas(m_cOccupancyUGroupId0, c_StatusTooFew);
      m_legEmpty->Draw();
    }
  }
  m_cOccupancyUGroupId0->Update();
  m_cOccupancyUGroupId0->Modified();
  m_cOccupancyUGroupId0->Update();

  //update summary offline occupancy V canvas
  m_cOccupancyV->Draw();
  m_cOccupancyV->cd();
  m_hOccupancy->getHistogram(0)->Draw("text");

  if (m_occVstatus == 0) {
    colorizeCanvas(m_cOccupancyV, c_StatusGood);
    m_legNormal->Draw();
  } else {
    if (m_occVstatus == 3) {
      colorizeCanvas(m_cOccupancyV, c_StatusError);
      m_legProblem->Draw();
    }
    if (m_occVstatus == 2) {
      colorizeCanvas(m_cOccupancyV, c_StatusWarning);
      m_legWarning->Draw();
    }
    if (m_occVstatus == 1) {
      colorizeCanvas(m_cOccupancyV, c_StatusTooFew);
      m_legEmpty->Draw();
    }
  }

  m_cOccupancyV->Update();
  m_cOccupancyV->Modified();
  m_cOccupancyV->Update();

  if (m_3Samples) {
    //update summary offline occupancy V canvas for 3 samples
    m_cOccupancyV3Samples->Draw();
    m_cOccupancyV3Samples->cd();
    m_hOccupancy3Samples->getHistogram(0)->Draw("text");

    if (m_occV3Samples == 0) {
      colorizeCanvas(m_cOccupancyV3Samples, c_StatusGood);
      m_legNormal->Draw();
    } else {
      if (m_occV3Samples == 3) {
        colorizeCanvas(m_cOccupancyV3Samples, c_StatusError);
        m_legProblem->Draw();
      }
      if (m_occV3Samples == 2) {
        colorizeCanvas(m_cOccupancyV3Samples, c_StatusWarning);
        m_legWarning->Draw();
      }
      if (m_occV3Samples == 1) {
        colorizeCanvas(m_cOccupancyV3Samples, c_StatusTooFew);
        m_legEmpty->Draw();
      }
    }

    m_cOccupancyV3Samples->Update();
    m_cOccupancyV3Samples->Modified();
    m_cOccupancyV3Samples->Update();
  }

  //update summary offline occupancy V canvas for groupId0
  m_cOccupancyVGroupId0->Draw();
  m_cOccupancyVGroupId0->cd();
  m_hOccupancyGroupId0->getHistogram(0)->Draw("text");

  if (m_occVGroupId0 == 0) {
    colorizeCanvas(m_cOccupancyVGroupId0, c_StatusGood);
    m_legNormal->Draw();
  } else {
    if (m_occVGroupId0 == 3) {
      colorizeCanvas(m_cOccupancyVGroupId0, c_StatusError);
      m_legProblem->Draw();
    }
    if (m_occVGroupId0 == 2) {
      colorizeCanvas(m_cOccupancyVGroupId0, c_StatusWarning);
      m_legWarning->Draw();
    }
    if (m_occVGroupId0 == 1) {
      colorizeCanvas(m_cOccupancyVGroupId0, c_StatusTooFew);
      m_legEmpty->Draw();
    }
  }
  m_cOccupancyVGroupId0->Update();
  m_cOccupancyVGroupId0->Modified();
  m_cOccupancyVGroupId0->Update();

  //update summary online occupancy U canvas
  m_cOnlineOccupancyU->Draw();
  m_cOnlineOccupancyU->cd();
  m_hOnlineOccupancy->getHistogram(1)->Draw("text");

  if (m_onlineOccUstatus == 0) {
    colorizeCanvas(m_cOnlineOccupancyU, c_StatusGood);
    m_legOnNormal->Draw();
  } else {
    if (m_onlineOccUstatus == 3) {
      colorizeCanvas(m_cOnlineOccupancyU, c_StatusError);
      m_legOnProblem->Draw();
    }
    if (m_onlineOccUstatus == 2) {
      colorizeCanvas(m_cOnlineOccupancyU, c_StatusWarning);
      m_legOnWarning->Draw();
    }
    if (m_onlineOccUstatus == 1) {
      colorizeCanvas(m_cOnlineOccupancyU, c_StatusTooFew);
      m_legOnEmpty->Draw();
    }
  }

  m_cOnlineOccupancyU->Update();
  m_cOnlineOccupancyU->Modified();
  m_cOnlineOccupancyU->Update();

  //update summary online occupancy V canvas
  m_cOnlineOccupancyV->Draw();
  m_cOnlineOccupancyV->cd();
  m_hOnlineOccupancy->getHistogram(0)->Draw("text");

  if (m_onlineOccVstatus == 0) {
    colorizeCanvas(m_cOnlineOccupancyV, c_StatusGood);
    m_legOnNormal->Draw();
  } else {
    if (m_onlineOccVstatus == 3) {
      colorizeCanvas(m_cOnlineOccupancyV, c_StatusError);
      m_legOnProblem->Draw();
    }
    if (m_onlineOccVstatus == 2) {
      colorizeCanvas(m_cOnlineOccupancyV, c_StatusWarning);
      m_legOnWarning->Draw();
    }
    if (m_onlineOccVstatus == 1) {
      colorizeCanvas(m_cOnlineOccupancyV, c_StatusTooFew);
      m_legOnEmpty->Draw();
    }
  }

  m_cOnlineOccupancyV->Update();
  m_cOnlineOccupancyV->Modified();
  m_cOnlineOccupancyV->Update();

  if (m_printCanvas) {
    m_cOccupancyU->Print("c_SVDOccupancyU.pdf");
    m_cOccupancyV->Print("c_SVDOccupancyV.pdf");
    m_cOnlineOccupancyU->Print("c_SVDOnlineOccupancyU.pdf");
    m_cOnlineOccupancyV->Print("c_SVDOnlineOccupancyV.pdf");
  }

  if (m_3Samples) {
    //update summary online occupancy U canvas for 3 samples
    m_cOnlineOccupancyU3Samples->Draw();
    m_cOnlineOccupancyU3Samples->cd();
    m_hOnlineOccupancy3Samples->getHistogram(1)->Draw("text");

    if (m_onlineOccU3Samples == 0) {
      colorizeCanvas(m_cOnlineOccupancyU3Samples, c_StatusGood);
      m_legOnNormal->Draw();
    } else {
      if (m_onlineOccU3Samples == 3) {
        colorizeCanvas(m_cOnlineOccupancyU3Samples, c_StatusError);
        m_legOnProblem->Draw();
      }
      if (m_onlineOccU3Samples == 2) {
        colorizeCanvas(m_cOnlineOccupancyU3Samples, c_StatusWarning);
        m_legOnWarning->Draw();
      }
      if (m_onlineOccU3Samples == 1) {
        colorizeCanvas(m_cOnlineOccupancyU3Samples, c_StatusTooFew);
        m_legOnEmpty->Draw();
      }
    }

    m_cOnlineOccupancyU3Samples->Update();
    m_cOnlineOccupancyU3Samples->Modified();
    m_cOnlineOccupancyU3Samples->Update();

    //update summary online occupancy V canvas for 3 samples
    m_cOnlineOccupancyV3Samples->Draw();
    m_cOnlineOccupancyV3Samples->cd();
    m_hOnlineOccupancy3Samples->getHistogram(0)->Draw("text");

    if (m_onlineOccV3Samples == 0) {
      colorizeCanvas(m_cOnlineOccupancyV3Samples, c_StatusGood);
      m_legOnNormal->Draw();
    } else {
      if (m_onlineOccV3Samples == 3) {
        colorizeCanvas(m_cOnlineOccupancyV3Samples, c_StatusError);
        m_legOnProblem->Draw();
      }
      if (m_onlineOccV3Samples == 2) {
        colorizeCanvas(m_cOnlineOccupancyV3Samples, c_StatusWarning);
        m_legOnWarning->Draw();
      }
      if (m_onlineOccV3Samples == 1) {
        colorizeCanvas(m_cOnlineOccupancyV3Samples, c_StatusTooFew);
        m_legOnEmpty->Draw();
      }
    }

    m_cOnlineOccupancyV3Samples->Update();
    m_cOnlineOccupancyV3Samples->Modified();
    m_cOnlineOccupancyV3Samples->Update();
  }
}

void DQMHistAnalysisSVDGeneralModule::endRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDGeneral: endRun called");
}


void DQMHistAnalysisSVDGeneralModule::terminate()
{
  B2DEBUG(10, "DQMHistAnalysisSVDGeneral: terminate called");

  delete m_refFile;
  delete m_legProblem;
  delete m_legWarning;
  delete m_legNormal;
  delete m_legEmpty;
  delete m_legError;
  delete m_legOnProblem;
  delete m_legOnWarning;
  delete m_legOnNormal;
  delete m_legOnEmpty;
  delete m_legOnError;

  delete m_cUnpacker;

  delete m_cOccupancyU;
  delete m_cOccupancyV;

  delete m_hOccupancy;
  delete m_hOnlineOccupancy;
  delete m_hOccupancyGroupId0;
  delete m_hOccupancy3Samples;
  delete m_hOnlineOccupancy3Samples;

  delete m_cOnlineOccupancyU;
  delete m_cOnlineOccupancyV;

  delete m_cOccupancyChartChip;

  for (int module = 0; module < m_sensors; module++) {
    delete m_cStripOccupancyU[module];
    delete m_cStripOccupancyV[module];
  }
  delete m_cStripOccupancyU;
  delete m_cStripOccupancyV;

  delete m_cClusterOnTrackTime_L456V;
}

Int_t DQMHistAnalysisSVDGeneralModule::findBinY(Int_t layer, Int_t sensor)
{
  if (layer == 3)
    return sensor; //2
  if (layer == 4)
    return 2 + 1 + sensor; //6
  if (layer == 5)
    return 6 + 1 + sensor; // 11
  if (layer == 6)
    return 11 + 1 + sensor; // 17
  else
    return -1;
}

