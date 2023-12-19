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
  m_legError->SetFillColor(kRed);
  m_legError->SetTextColor(kWhite);

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
    printf("%d StripOccupancyU_%d_%d_%d\n", i, tmp_layer, tmp_ladder, tmp_sensor);
  }


  //occupancy plot Y axis title
  m_yTitle = new TText(-0.75, 13, "layer.ladder.sensor");
  m_yTitle->SetTextAngle(90);
  m_yTitle->SetTextSize(0.03);
  m_yTitle->SetTextFont(42);

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

  const int nY = 19;
  TString Ylabels[nY] = {"", "L3.x.1", "L3.x.2",
                         " ", "L4.x.1", "L4.x.2", "L4.x.3",
                         "  ", "L5.x.1", "L5.x.2", "L5.x.3", "L5.x.4",
                         "   ", "L6.x.1", "L6.x.2", "L6.x.3", "L6.x.4", "L6.x.5", "    "
                        };


  m_hOccupancyV =  new TH2F("hOccupancyV", "Average OFFLINE Sensor Occupancy (%), V side ", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyV->SetMarkerSize(1.1);
  m_hOccupancyV->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyV->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyV->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  m_hOccupancyU =  new TH2F("hOccupancyU", "Average OFFLINE Sensor Occupancy (%), U side ", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyU->SetMarkerSize(1.1);
  m_hOccupancyU->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyU->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyU->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  m_hOnlineOccupancyV =  new TH2F("hOnlineOccupancyV", "Average ONLINE Sensor Occupancy (%), V side ", 16, 0.5, 16.5, 19, 0, 19);
  m_hOnlineOccupancyV->SetMarkerSize(1.1);
  m_hOnlineOccupancyV->GetXaxis()->SetTitle("ladder number");
  m_hOnlineOccupancyV->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOnlineOccupancyV->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  m_hOnlineOccupancyU =  new TH2F("hOnlineOccupancyU", "Average ONLINE Sensor Occupancy (%), U side ", 16, 0.5, 16.5, 19, 0, 19);
  m_hOnlineOccupancyU->SetMarkerSize(1.1);
  m_hOnlineOccupancyU->GetXaxis()->SetTitle("ladder number");
  m_hOnlineOccupancyU->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOnlineOccupancyU->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  // 3 samples occupancy
  m_hOccupancyV3Samples =  new TH2F("hOccupancy3V", "Average OFFLINE Sensor Occupancy (%), V side for 3 samples", 16, 0.5, 16.5, 19,
                                    0, 19);
  m_hOccupancyV3Samples->SetMarkerSize(1.1);
  m_hOccupancyV3Samples->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyV3Samples->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyV3Samples->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  if (m_3Samples) {
    m_hOccupancyU3Samples =  new TH2F("hOccupancy3U", "Average OFFLINE Sensor Occupancy (%), U side for 3 samples", 16, 0.5, 16.5, 19,
                                      0, 19);
    m_hOccupancyU3Samples->SetMarkerSize(1.1);
    m_hOccupancyU3Samples->GetXaxis()->SetTitle("ladder number");
    m_hOccupancyU3Samples->GetXaxis()->SetLabelSize(0.04);
    for (unsigned short i = 0; i < nY; i++) m_hOccupancyU3Samples->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

    m_hOnlineOccupancyV3Samples =  new TH2F("hOnlineOccupancy3V", "Average ONLINE Sensor Occupancy (%), V side for 3 samples", 16, 0.5,
                                            16.5, 19, 0, 19);
    m_hOnlineOccupancyV3Samples->SetMarkerSize(1.1);
    m_hOnlineOccupancyV3Samples->GetXaxis()->SetTitle("ladder number");
    m_hOnlineOccupancyV3Samples->GetXaxis()->SetLabelSize(0.04);
    for (unsigned short i = 0; i < nY; i++) m_hOnlineOccupancyV3Samples->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

    m_hOnlineOccupancyU3Samples =  new TH2F("hOnlineOccupancy3U", "Average ONLINE Sensor Occupancy (%), U side for 3 samples", 16, 0.5,
                                            16.5, 19, 0, 19);
    m_hOnlineOccupancyU3Samples->SetMarkerSize(1.1);
    m_hOnlineOccupancyU3Samples->GetXaxis()->SetTitle("ladder number");
    m_hOnlineOccupancyU3Samples->GetXaxis()->SetLabelSize(0.04);
    for (unsigned short i = 0; i < nY; i++) m_hOnlineOccupancyU3Samples->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());
  }

  m_hOccupancyVGroupId0 =  new TH2F("hOccupancyVGroupId0",
                                    "Average OFFLINE Sensor Occupancy (%), V side for cluster time group Id = 0", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyVGroupId0->SetMarkerSize(1.1);
  m_hOccupancyVGroupId0->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyVGroupId0->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyVGroupId0->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

  m_hOccupancyUGroupId0 =  new TH2F("hOccupancyUGroupId0",
                                    "Average OFFLINE Sensor Occupancy (%), U side for cluster time group Id = 0", 16, 0.5, 16.5, 19, 0, 19);
  m_hOccupancyUGroupId0->SetMarkerSize(1.1);
  m_hOccupancyUGroupId0->GetXaxis()->SetTitle("ladder number");
  m_hOccupancyUGroupId0->GetXaxis()->SetLabelSize(0.04);
  for (unsigned short i = 0; i < nY; i++) m_hOccupancyUGroupId0->GetYaxis()->SetBinLabel(i + 1, Ylabels[i].Data());

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
  requestLimitsFromEpicsPVs("clusTimeOnTrkLimits", timeErrorLo, m_statThreshold, timeWarnUp,  m_timeThreshold);
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
  m_legProblem->SetFillColor(kRed);
  m_legWarning = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legWarning->AddText("WARNING!");
  m_legWarning->AddText("at least one sensor with:");
  m_legWarning->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occWarning, m_occError));
  m_legWarning->SetFillColor(kYellow);
  m_legNormal = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legNormal->AddText("OCCUPANCY WITHIN LIMITS");
  m_legNormal->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occEmpty, m_occWarning));
  m_legNormal->SetFillColor(kGreen);
  m_legNormal->SetBorderSize(0.);
  m_legNormal->SetLineColor(kBlack);
  m_legEmpty = new TPaveText(11, findBinY(4, 3) - 2, 16, findBinY(4, 3));
  m_legEmpty->AddText("NO DATA RECEIVED");
  m_legEmpty->AddText("from at least one sensor");
  m_legEmpty->SetFillColor(kBlack);
  m_legEmpty->SetTextColor(kWhite);
  m_legEmpty->SetBorderSize(0.);
  m_legEmpty->SetLineColor(kBlack);

  //ONLINE occupancy plots legend
  m_legOnProblem = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legOnProblem->AddText("ERROR!");
  m_legOnProblem->AddText("at least one sensor with:");
  m_legOnProblem->AddText(Form("online occupancy > %1.1f%%", m_onlineOccError));
  m_legOnProblem->SetFillColor(kRed);
  m_legOnWarning = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legOnWarning->AddText("WARNING!");
  m_legOnWarning->AddText("at least one sensor with:");
  m_legOnWarning->AddText(Form("%1.1f%% < online occupancy < %1.1f%%", m_onlineOccWarning, m_onlineOccError));
  m_legOnWarning->SetFillColor(kYellow);
  m_legOnNormal = new TPaveText(11, findBinY(4, 3) - 3, 16, findBinY(4, 3));
  m_legOnNormal->AddText("OCCUPANCY WITHIN LIMITS");
  m_legOnNormal->AddText(Form("%1.1f%% < online occupancy < %1.1f%%", m_onlineOccEmpty, m_onlineOccWarning));
  m_legOnNormal->SetFillColor(kGreen);
  m_legOnNormal->SetBorderSize(0.);
  m_legOnNormal->SetLineColor(kBlack);
  m_legOnEmpty = new TPaveText(11, findBinY(4, 3) - 2, 16, findBinY(4, 3));
  m_legOnEmpty->AddText("NO DATA RECEIVED");
  m_legOnEmpty->AddText("from at least one sensor");
  m_legOnEmpty->SetFillColor(kBlack);
  m_legOnEmpty->SetTextColor(kWhite);


  // cluster time on tracks legend
  m_legTiProblem = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTiProblem->AddText("ERROR!");
  m_legTiProblem->AddText(Form("abs(Mean) > %3.1f ns", m_timeThreshold));
  m_legTiProblem->SetFillColor(kRed);

  m_legTiNormal = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTiNormal->AddText("TIME SHIFT UNDER LIMIT");
  m_legTiNormal->AddText(Form("abs(Mean) < %3.1f ns", m_timeThreshold));
  m_legTiNormal->SetFillColor(kGreen);
  m_legTiNormal->SetBorderSize(0.);
  m_legTiNormal->SetLineColor(kBlack);

  m_legTiEmpty = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTiEmpty->AddText("NO DATA RECEIVED");
  m_legTiEmpty->SetFillColor(kGreen);
  m_legTiEmpty->SetFillColor(kBlack);
  m_legTiEmpty->SetTextColor(kWhite);


  m_legTi3Problem = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTi3Problem->AddText("ERROR!");
  m_legTi3Problem->AddText(Form("abs(Mean) > %3.1f ns", m_timeThreshold));
  m_legTi3Problem->SetFillColor(kRed);

  m_legTi3Normal = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTi3Normal->AddText("TIME SHIFT UNDER LIMIT");
  m_legTi3Normal->AddText(Form("abs(Mean) < %3.1f ns", m_timeThreshold));
  m_legTi3Normal->SetFillColor(kGreen);
  m_legTi3Normal->SetBorderSize(0.);
  m_legTi3Normal->SetLineColor(kBlack);

  m_legTi3Empty = new TPaveText(0.15, 0.65, 0.35, 0.80, "NDC");
  m_legTi3Empty->AddText("NO DATA RECEIVED");
  m_legTi3Empty->SetFillColor(kGreen);
  m_legTi3Empty->SetFillColor(kBlack);
  m_legTi3Empty->SetTextColor(kWhite);
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


  TString runID = TString((hnEvnts->GetTitle())).Remove(0, 21);
  B2INFO("runID = " << runID);
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
      m_cUnpacker->SetFillColor(kGreen);
      m_cUnpacker->SetFrameFillColor(10);
    } else {
      m_legError->Draw("same");
      m_cUnpacker->SetFillColor(kRed);
      m_cUnpacker->SetFrameFillColor(10);
    }
    if (nEvents > 0)
      setEpicsPV("UnpackError", h->GetEntries() / nEvents);
  } else {
    B2INFO("Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackerDQM not found!");
    m_cUnpacker->SetFillColor(kRed);
  }


  m_cUnpacker->cd();
  h->Draw("colztext");
  h->SetStats(0);

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
      } else {
        B2WARNING("Run type:" << m_runtype);
      }
    } else {
      lowStat = true;
    }

    if (! hasError) {
      m_cClusterOnTrackTime_L456V->SetFillColor(kGreen);
      m_cClusterOnTrackTime_L456V->SetFrameFillColor(10);
    } else {
      m_cClusterOnTrackTime_L456V->SetFillColor(kRed);
      m_cClusterOnTrackTime_L456V->SetFrameFillColor(10);
    }

    if (lowStat) {
      m_cClusterOnTrackTime_L456V->SetFillColor(kGray);
      m_cClusterOnTrackTime_L456V->SetFrameFillColor(10);
    }

  } else {
    B2INFO("Histogram SVDClsTrk/c_SVDTRK_ClusterTimeV456 from SVDDQMClustersOnTrack module not found!");
    m_cClusterOnTrackTime_L456V->SetFillColor(kRed);
  }

  m_cClusterOnTrackTime_L456V->cd();
  m_hClusterOnTrackTime_L456V.Draw();

  if (hasError)
    m_legTiProblem->Draw("same");
  else if (lowStat)
    m_legTiEmpty->Draw("same");
  else
    m_legTiNormal->Draw("same");


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
        m_cClusterOnTrackTimeL456V3Samples->SetFillColor(kGreen);
        m_cClusterOnTrackTimeL456V3Samples->SetFrameFillColor(10);
      } else {
        m_cClusterOnTrackTimeL456V3Samples->SetFillColor(kRed);
        m_cClusterOnTrackTimeL456V3Samples->SetFrameFillColor(10);
      }

      if (lowStat3) {
        m_cClusterOnTrackTimeL456V3Samples->SetFillColor(kGray);
        m_cClusterOnTrackTimeL456V3Samples->SetFrameFillColor(10);
      }

    } else {
      B2INFO("Histogram SVDClsTrk/c_SVDTRK_Cluster3TimeV456 from SVDDQMClustersOnTrack module not found!");
      m_cClusterOnTrackTimeL456V3Samples->SetFillColor(kRed);
    }


    m_cClusterOnTrackTimeL456V3Samples->cd();
    m_hClusterOnTrackTimeL456V3Samples.Draw();

    if (hasError3)
      m_legTi3Problem->Draw("same");
    else if (lowStat3)
      m_legTi3Empty->Draw("same");
    else
      m_legTi3Normal->Draw("same");

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

  m_onlineOccU3Samples = 0;
  m_onlineOccV3Samples = 0;

  m_occU3Samples = 0;
  m_occV3Samples = 0;

  //update titles with exp and run number
  m_hOccupancyU->SetTitle(Form("Average OFFLINE Sensor Occupancy (%%), U side %s", runID.Data()));
  m_hOccupancyU->SetStats(0);
  m_hOccupancyV->SetTitle(Form("Average OFFLINE Sensor Occupancy (%%), V side %s", runID.Data()));
  m_hOccupancyV->SetStats(0);

  m_hOnlineOccupancyU->SetTitle(Form("Average ONLINE Sensor Occupancy (%%), U side %s", runID.Data()));
  m_hOnlineOccupancyU->SetStats(0);
  m_hOnlineOccupancyV->SetTitle(Form("Average ONLINE Sensor Occupancy (%%), V side %s", runID.Data()));
  m_hOnlineOccupancyV->SetStats(0);

  m_hOccupancyUGroupId0->SetTitle(Form("Average OFFLINE Sensor Occupancy (%%), U side for cluster time group Id = 0 %s",
                                       runID.Data()));
  m_hOccupancyUGroupId0->SetStats(0);

  m_hOccupancyVGroupId0->SetTitle(Form("Average OFFLINE Sensor Occupancy (%%), V side for cluster time group Id = 0 %s",
                                       runID.Data()));
  m_hOccupancyVGroupId0->SetStats(0);

  if (m_3Samples) {
    //update titles with exp and run number for 3 samples
    m_hOccupancyU3Samples->SetTitle(Form("Average OFFLINE Sensor Occupancy (%%), U side for 3 samples %s", runID.Data()));
    m_hOccupancyU3Samples->SetStats(0);
    m_hOccupancyV3Samples->SetTitle(Form("Average OFFLINE Sensor Occupancy (%%), V side for 3 samples %s", runID.Data()));
    m_hOccupancyV3Samples->SetStats(0);

    m_hOnlineOccupancyU3Samples->SetTitle(Form("Average ONLINE Sensor Occupancy (%%), U side for 3 samples %s", runID.Data()));
    m_hOnlineOccupancyU3Samples->SetStats(0);
    m_hOnlineOccupancyV3Samples->SetTitle(Form("Average ONLINE Sensor Occupancy (%%), V side for 3 samples %s", runID.Data()));
    m_hOnlineOccupancyV3Samples->SetStats(0);
  }

  //set dedicate gStyle
  const Int_t colNum = 4;
  Int_t palette[colNum] {kBlack,  kGreen, kYellow, kRed};
  gStyle->SetPalette(colNum, palette);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.3f");

  TH1F* htmp = NULL;

  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();


    Int_t bin = m_hOccupancyU->FindBin(tmp_ladder, findBinY(tmp_layer, tmp_sensor));

    //look for U histogram - OFFLINE ZS
    TString tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram not found");
      m_cOccupancyU->SetFillColor(kRed);
    } else {

      Int_t nStrips = 768;

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyU->SetBinContent(bin, occU);

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
        m_cOccupancyU3Samples->SetFillColor(kRed);
      } else {

        Int_t nStrips = 768;

        Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOccupancyU3Samples->SetBinContent(bin, occU);


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
      m_cOccupancyUGroupId0->SetFillColor(kRed);
    } else {

      Int_t nStrips = 768;

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyUGroupId0->SetBinContent(bin, occU);

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
      m_cOccupancyV->SetFillColor(kRed);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyV->SetBinContent(bin, occV);

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
        m_cOccupancyV3Samples->SetFillColor(kRed);
      } else {

        Int_t nStrips = 768;
        if (tmp_layer != 3)
          nStrips = 512;

        Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOccupancyV3Samples->SetBinContent(bin, occV);

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
      m_cOccupancyVGroupId0->SetFillColor(kRed);
    } else {

      Int_t nStrips = 768;

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyVGroupId0->SetBinContent(bin, occU);

      if (occU <= m_occEmpty) {
        if (m_occVGroupId0 < 1) m_occVGroupId0 = 1;
      } else if (occU > m_occWarning) {
        if (occU < m_occError) {
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
      m_cOnlineOccupancyV->SetFillColor(kRed);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t onlineOccV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOnlineOccupancyV->SetBinContent(bin, onlineOccV);

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
        m_cOnlineOccupancyV3Samples->SetFillColor(kRed);
      } else {

        Int_t nStrips = 768;
        if (tmp_layer != 3)
          nStrips = 512;

        Float_t onlineOccV = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOnlineOccupancyV3Samples->SetBinContent(bin, onlineOccV);

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
      m_cOnlineOccupancyU->SetFillColor(kRed);
    } else {

      Int_t nStrips = 768;

      Float_t onlineOccU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOnlineOccupancyU->SetBinContent(bin, onlineOccU);

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
        m_cOnlineOccupancyU3Samples->SetFillColor(kRed);
      } else {

        Int_t nStrips = 768;

        Float_t onlineOccU = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOnlineOccupancyU3Samples->SetBinContent(bin, onlineOccU);

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
  m_cOccupancyU->cd();
  m_hOccupancyU->Draw("text");
  m_yTitle->Draw("same");

  if (m_occUstatus == 0) {
    m_cOccupancyU->SetFillColor(kGreen);
    m_cOccupancyU->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_occUstatus == 3) {
      m_cOccupancyU->SetFillColor(kRed);
      m_cOccupancyU->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_occUstatus == 2) {
      m_cOccupancyU->SetFillColor(kYellow);
      m_cOccupancyU->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_occUstatus == 1) {
      m_cOccupancyU->SetFillColor(kGray);
      m_cOccupancyU->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }
  m_cOccupancyU->Draw();
  m_cOccupancyU->Update();
  m_cOccupancyU->Modified();
  m_cOccupancyU->Update();

  if (m_3Samples) {
    //update summary offline occupancy U canvas for 3 samples
    m_cOccupancyU3Samples->cd();
    m_hOccupancyU3Samples->Draw("text");
    m_yTitle->Draw("same");

    if (m_occU3Samples == 0) {
      m_cOccupancyU3Samples->SetFillColor(kGreen);
      m_cOccupancyU3Samples->SetFrameFillColor(10);
      m_legNormal->Draw("same");
    } else {
      if (m_occU3Samples == 3) {
        m_cOccupancyU3Samples->SetFillColor(kRed);
        m_cOccupancyU3Samples->SetFrameFillColor(10);
        m_legProblem->Draw("same");
      }
      if (m_occU3Samples == 2) {
        m_cOccupancyU3Samples->SetFillColor(kYellow);
        m_cOccupancyU3Samples->SetFrameFillColor(10);
        m_legWarning->Draw("same");
      }
      if (m_occU3Samples == 1) {
        m_cOccupancyU3Samples->SetFillColor(kGray);
        m_cOccupancyU3Samples->SetFrameFillColor(10);
        m_legEmpty->Draw("same");
      }
    }
    m_cOccupancyU3Samples->Draw();
    m_cOccupancyU3Samples->Update();
    m_cOccupancyU3Samples->Modified();
    m_cOccupancyU3Samples->Update();
  }

  //update summary offline occupancy U canvas for groupId0
  m_cOccupancyUGroupId0->cd();
  m_hOccupancyUGroupId0->Draw("text");
  m_yTitle->Draw("same");

  if (m_occUGroupId0 == 0) {
    m_cOccupancyUGroupId0->SetFillColor(kGreen);
    m_cOccupancyUGroupId0->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_occUGroupId0 == 3) {
      m_cOccupancyUGroupId0->SetFillColor(kRed);
      m_cOccupancyUGroupId0->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_occUGroupId0 == 2) {
      m_cOccupancyUGroupId0->SetFillColor(kYellow);
      m_cOccupancyUGroupId0->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_occUGroupId0 == 1) {
      m_cOccupancyUGroupId0->SetFillColor(kGray);
      m_cOccupancyUGroupId0->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }
  m_cOccupancyUGroupId0->Draw();
  m_cOccupancyUGroupId0->Update();
  m_cOccupancyUGroupId0->Modified();
  m_cOccupancyUGroupId0->Update();

  //update summary offline occupancy V canvas
  m_cOccupancyV->cd();
  m_hOccupancyV->Draw("text");
  m_yTitle->Draw("same");

  if (m_occVstatus == 0) {
    m_cOccupancyV->SetFillColor(kGreen);
    m_cOccupancyV->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_occVstatus == 3) {
      m_cOccupancyV->SetFillColor(kRed);
      m_cOccupancyV->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_occVstatus == 2) {
      m_cOccupancyV->SetFillColor(kYellow);
      m_cOccupancyV->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_occVstatus == 1) {
      m_cOccupancyV->SetFillColor(kGray);
      m_cOccupancyV->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }

  m_cOccupancyV->Draw();
  m_cOccupancyV->Update();
  m_cOccupancyV->Modified();
  m_cOccupancyV->Update();

  if (m_3Samples) {
    //update summary offline occupancy V canvas for 3 samples
    m_cOccupancyV3Samples->cd();
    m_hOccupancyV3Samples->Draw("text");
    m_yTitle->Draw("same");

    if (m_occV3Samples == 0) {
      m_cOccupancyV3Samples->SetFillColor(kGreen);
      m_cOccupancyV3Samples->SetFrameFillColor(10);
      m_legNormal->Draw("same");
    } else {
      if (m_occV3Samples == 3) {
        m_cOccupancyV3Samples->SetFillColor(kRed);
        m_cOccupancyV3Samples->SetFrameFillColor(10);
        m_legProblem->Draw("same");
      }
      if (m_occV3Samples == 2) {
        m_cOccupancyV3Samples->SetFillColor(kYellow);
        m_cOccupancyV3Samples->SetFrameFillColor(10);
        m_legWarning->Draw("same");
      }
      if (m_occV3Samples == 1) {
        m_cOccupancyV3Samples->SetFillColor(kGray);
        m_cOccupancyV3Samples->SetFrameFillColor(10);
        m_legEmpty->Draw("same");
      }
    }

    m_cOccupancyV3Samples->Draw();
    m_cOccupancyV3Samples->Update();
    m_cOccupancyV3Samples->Modified();
    m_cOccupancyV3Samples->Update();
  }

  //update summary offline occupancy V canvas for groupId0
  m_cOccupancyVGroupId0->cd();
  m_hOccupancyVGroupId0->Draw("text");
  m_yTitle->Draw("same");

  if (m_occVGroupId0 == 0) {
    m_cOccupancyVGroupId0->SetFillColor(kGreen);
    m_cOccupancyVGroupId0->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_occVGroupId0 == 3) {
      m_cOccupancyVGroupId0->SetFillColor(kRed);
      m_cOccupancyVGroupId0->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_occVGroupId0 == 2) {
      m_cOccupancyVGroupId0->SetFillColor(kYellow);
      m_cOccupancyVGroupId0->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_occVGroupId0 == 1) {
      m_cOccupancyVGroupId0->SetFillColor(kGray);
      m_cOccupancyVGroupId0->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }
  m_cOccupancyVGroupId0->Draw();
  m_cOccupancyVGroupId0->Update();
  m_cOccupancyVGroupId0->Modified();
  m_cOccupancyVGroupId0->Update();

  //update summary online occupancy U canvas
  m_cOnlineOccupancyU->cd();
  m_hOnlineOccupancyU->Draw("text");
  m_yTitle->Draw("same");

  if (m_onlineOccUstatus == 0) {
    m_cOnlineOccupancyU->SetFillColor(kGreen);
    m_cOnlineOccupancyU->SetFrameFillColor(10);
    m_legOnNormal->Draw("same");
  } else {
    if (m_onlineOccUstatus == 3) {
      m_cOnlineOccupancyU->SetFillColor(kRed);
      m_cOnlineOccupancyU->SetFrameFillColor(10);
      m_legOnProblem->Draw("same");
    }
    if (m_onlineOccUstatus == 2) {
      m_cOnlineOccupancyU->SetFillColor(kYellow);
      m_cOnlineOccupancyU->SetFrameFillColor(10);
      m_legOnWarning->Draw("same");
    }
    if (m_onlineOccUstatus == 1) {
      m_cOnlineOccupancyU->SetFillColor(kGray);
      m_cOnlineOccupancyU->SetFrameFillColor(10);
      m_legOnEmpty->Draw("same");
    }
  }

  m_cOnlineOccupancyU->Draw();
  m_cOnlineOccupancyU->Update();
  m_cOnlineOccupancyU->Modified();
  m_cOnlineOccupancyU->Update();

  //update summary online occupancy V canvas
  m_cOnlineOccupancyV->cd();
  m_hOnlineOccupancyV->Draw("text");
  m_yTitle->Draw("same");

  if (m_onlineOccVstatus == 0) {
    m_cOnlineOccupancyV->SetFillColor(kGreen);
    m_cOnlineOccupancyV->SetFrameFillColor(10);
    m_legOnNormal->Draw("same");
  } else {
    if (m_onlineOccVstatus == 3) {
      m_cOnlineOccupancyV->SetFillColor(kRed);
      m_cOnlineOccupancyV->SetFrameFillColor(10);
      m_legOnProblem->Draw("same");
    }
    if (m_onlineOccVstatus == 2) {
      m_cOnlineOccupancyV->SetFillColor(kYellow);
      m_cOnlineOccupancyV->SetFrameFillColor(10);
      m_legOnWarning->Draw("same");
    }
    if (m_onlineOccVstatus == 1) {
      m_cOnlineOccupancyV->SetFillColor(kGray);
      m_cOnlineOccupancyV->SetFrameFillColor(10);
      m_legOnEmpty->Draw("same");
    }
  }

  m_cOnlineOccupancyV->Draw();
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
    m_cOnlineOccupancyU3Samples->cd();
    m_hOnlineOccupancyU3Samples->Draw("text");
    m_yTitle->Draw("same");

    if (m_onlineOccU3Samples == 0) {
      m_cOnlineOccupancyU3Samples->SetFillColor(kGreen);
      m_cOnlineOccupancyU3Samples->SetFrameFillColor(10);
      m_legOnNormal->Draw("same");
    } else {
      if (m_onlineOccU3Samples == 3) {
        m_cOnlineOccupancyU3Samples->SetFillColor(kRed);
        m_cOnlineOccupancyU3Samples->SetFrameFillColor(10);
        m_legOnProblem->Draw("same");
      }
      if (m_onlineOccU3Samples == 2) {
        m_cOnlineOccupancyU3Samples->SetFillColor(kYellow);
        m_cOnlineOccupancyU3Samples->SetFrameFillColor(10);
        m_legOnWarning->Draw("same");
      }
      if (m_onlineOccU3Samples == 1) {
        m_cOnlineOccupancyU3Samples->SetFillColor(kGray);
        m_cOnlineOccupancyU3Samples->SetFrameFillColor(10);
        m_legOnEmpty->Draw("same");
      }
    }

    m_cOnlineOccupancyU3Samples->Draw();
    m_cOnlineOccupancyU3Samples->Update();
    m_cOnlineOccupancyU3Samples->Modified();
    m_cOnlineOccupancyU3Samples->Update();

    //update summary online occupancy V canvas for 3 samples
    m_cOnlineOccupancyV3Samples->cd();
    m_hOnlineOccupancyV3Samples->Draw("text");
    m_yTitle->Draw("same");

    if (m_onlineOccV3Samples == 0) {
      m_cOnlineOccupancyV3Samples->SetFillColor(kGreen);
      m_cOnlineOccupancyV3Samples->SetFrameFillColor(10);
      m_legOnNormal->Draw("same");
    } else {
      if (m_onlineOccV3Samples == 3) {
        m_cOnlineOccupancyV3Samples->SetFillColor(kRed);
        m_cOnlineOccupancyV3Samples->SetFrameFillColor(10);
        m_legOnProblem->Draw("same");
      }
      if (m_onlineOccV3Samples == 2) {
        m_cOnlineOccupancyV3Samples->SetFillColor(kYellow);
        m_cOnlineOccupancyV3Samples->SetFrameFillColor(10);
        m_legOnWarning->Draw("same");
      }
      if (m_onlineOccV3Samples == 1) {
        m_cOnlineOccupancyV3Samples->SetFillColor(kGray);
        m_cOnlineOccupancyV3Samples->SetFrameFillColor(10);
        m_legOnEmpty->Draw("same");
      }
    }

    m_cOnlineOccupancyV3Samples->Draw();
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
  delete m_yTitle;

  delete m_cUnpacker;

  delete m_hOccupancyU;
  delete m_cOccupancyU;
  delete m_hOccupancyV;
  delete m_cOccupancyV;

  delete m_hOnlineOccupancyU;
  delete m_cOnlineOccupancyU;
  delete m_hOnlineOccupancyV;
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

