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
REG_MODULE(DQMHistAnalysisSVDGeneral)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDGeneralModule::DQMHistAnalysisSVDGeneralModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2INFO("DQMHistAnalysisSVDGeneral: Constructor done.");

  setDescription("DQM Analysis Module that produces colored canvas for a straightforward interpretation of the SVD Data Quality.");

  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("SVDrefHisto.root"));
  addParam("unpackerErrorLevel", m_unpackError, "Maximum bin_content/ # events allowed before throwing ERROR", double(0.00001));
  addParam("occLevel_Error", m_occError, "Maximum Occupancy (%) allowed for safe operations (red)", float(5));
  addParam("occLevel_Warning", m_occWarning, "Occupancy (%) at WARNING level (orange)", float(3));
  addParam("occLevel_Empty", m_occEmpty, "Maximum Occupancy (%) for which the sensor is considered empty", float(0));
  addParam("onlineOccLevel_Error", m_onlineOccError, "Maximum OnlineOccupancy (%) allowed for safe operations (red)", float(10));
  addParam("onlineOccLevel_Warning", m_onlineOccWarning, "OnlineOccupancy (%) at WARNING level (orange)", float(5));
  addParam("onlineOccLevel_Empty", m_onlineOccEmpty, "Maximum OnlineOccupancy (%) for which the sensor is considered empty",
           float(0));
  addParam("statThreshold", m_statThreshold, "Minimal number of events to compare histograms", int(10000));
  addParam("timeThreshold", m_timeThreshold, "Acceptable difference between mean of central peak for present and reference run",
           float(6)); // 6 ns
  addParam("refMCTP", m_refMeanP, "Mean of the signal time peak from Physics reference run", float(0.0)); // Approximate, from exp 20
  addParam("refMCTC", m_refMeanC, "Mean of the signal time peak from Cosmic reference run", float(0.0));  //
}

DQMHistAnalysisSVDGeneralModule::~DQMHistAnalysisSVDGeneralModule() { }

void DQMHistAnalysisSVDGeneralModule::initialize()
{
  B2INFO("DQMHistAnalysisSVDGeneral: initialized.");
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

    TH1F* ref_occ = (TH1F*)m_refFile->Get("refOccupancy");
    if (!ref_occ)
      B2WARNING("SVD DQMHistAnalysis: Occupancy Level Reference not found! using module parameters");
    else {
      m_occEmpty = ref_occ->GetBinContent(1);
      m_occWarning = ref_occ->GetBinContent(2);
      m_occError = ref_occ->GetBinContent(3);
    }

    TH1F* ref_onlineOcc = (TH1F*)m_refFile->Get("refOnlineOccupancy");
    if (!ref_onlineOcc)
      B2WARNING("SVD DQMHistAnalysis: OnlineOccupancy Level Reference not found! using module parameters");
    else {
      m_onlineOccEmpty = ref_onlineOcc->GetBinContent(1);
      m_onlineOccWarning = ref_onlineOcc->GetBinContent(2);
      m_onlineOccError = ref_onlineOcc->GetBinContent(3);
    }
  } else
    B2WARNING("SVD DQMHistAnalysis: reference root file (" << m_refFileName << ") not found, or closed, using module parameters");

  B2INFO(" SVD occupancy thresholds:");
  B2INFO("ONLINE OCCUPANCY: empty < " << m_onlineOccEmpty << " normal < " << m_onlineOccWarning << "warning < " << m_onlineOccError <<
         " < error");
  B2INFO("OFFLINE OCCUPANCY: empty < " << m_occEmpty << " normal < " << m_occWarning << "warning < " << m_occError << " < error");

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
  m_cStripOccupancyU = new TCanvas*[nSensors];
  m_cStripOccupancyV = new TCanvas*[nSensors];
  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();
    m_cStripOccupancyU[i] = new TCanvas(Form("SVDOccupancy/c_StripOccupancyU_%d_%d_%d", tmp_layer, tmp_ladder, tmp_sensor));
    m_cStripOccupancyV[i] = new TCanvas(Form("SVDOccupancy/c_StripOccupancyV_%d_%d_%d", tmp_layer, tmp_ladder, tmp_sensor));
  }


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
  m_legWarning->SetFillColor(kOrange);
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
  m_legOnWarning->SetFillColor(kOrange);
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


  //occupancy plot Y axis title
  m_yTitle = new TText(-0.75, 13, "layer.ladder.sensor");
  m_yTitle->SetTextAngle(90);
  m_yTitle->SetTextSize(0.03);
  m_yTitle->SetTextFont(42);

  gROOT->cd();
  m_cUnpacker = new TCanvas("SVDAnalysis/c_SVDDataFormat");
  m_cUnpacker->SetGrid(1);
  m_cOccupancyU = new TCanvas("SVDAnalysis/c_SVDOccupancyU");
  //  m_cOccupancyU->SetGrid(1);
  m_cOccupancyV = new TCanvas("SVDAnalysis/c_SVDOccupancyV");
  //  m_cOccupancyV->SetGrid(1);
  m_cOnlineOccupancyU = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyU");
  //  m_cOnlineOccupancyU->SetGrid(1);
  m_cOnlineOccupancyV = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyV");
  //  m_cOnlineOccupancyV->SetGrid(1);
  m_cClusterOnTrackTime_L456V = new TCanvas("SVDAnalysis/c_ClusterOnTrackTime_L456V");

  const int nY = 19;
  TString Ylabels[nY] = {"", "L3.x.1", "L3.x.2",
                         "", "L4.x.1", "L4.x.2", "L4.x.3",
                         "", "L5.x.1", "L5.x.2", "L5.x.3", "L5.x.4",
                         "", "L6.x.1", "L6.x.2", "L6.x.3", "L6.x.4", "L6.x.5", ""
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

  rtype = findHist("DQMInfo/rtype");
  runtype = rtype ? rtype->GetTitle() : "";
}


void DQMHistAnalysisSVDGeneralModule::beginRun()
{
  B2INFO("DQMHistAnalysisSVDGeneral: beginRun called.");
  m_cUnpacker->Clear();
  m_cOccupancyU->Clear();
  m_cOccupancyV->Clear();
  m_cOnlineOccupancyU->Clear();
  m_cOnlineOccupancyV->Clear();
  m_cOccupancyChartChip->Clear();
  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    m_cStripOccupancyU[i]->Clear();
    m_cStripOccupancyV[i]->Clear();
  }
  m_cClusterOnTrackTime_L456V->Clear();
}

void DQMHistAnalysisSVDGeneralModule::event()
{

  //SETUP gSTYLE - all plots
  //  gStyle->SetOptStat(0);
  //  gStyle->SetTitleY(.97);

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents");
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  }
  TString runID = TString((hnEvnts->GetTitle())).Remove(0, 21);
  B2INFO("runID = " << runID);
  Float_t nEvents = hnEvnts->GetEntries();

  //check DATA FORMAT
  TH1* h = findHist("SVDUnpacker/DQMUnpackerHisto");

  //test ERROR:
  //  h->SetBinContent(100,0.01);

  if (h != NULL) {
    h->SetTitle("SVD Data Format Monitor " + runID);
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
  } else {
    B2INFO("Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackerDQM not found!");
    m_cUnpacker->SetFillColor(kRed);
  }

  m_cUnpacker->cd();
  h->Draw("colztext");
  h->SetStats(0);

  m_cUnpacker->Modified();
  m_cUnpacker->Update();

  //occupancy chart
  TH1F* hChart = (TH1F*)findHist("SVDExpReco/SVDDQM_StripCountsChip");

  if (hChart != NULL) {
    m_hOccupancyChartChip.Clear();
    hChart->Copy(m_hOccupancyChartChip);
    m_hOccupancyChartChip.SetName("SVDOccupancyChart");
    m_hOccupancyChartChip.SetTitle("SVD OFFLINE Occupancy per chip " + runID);
    m_hOccupancyChartChip.Scale(1 / nEvents / 128);
    m_cOccupancyChartChip->cd();
    //    m_hOccupancyChartChip->SetStats(0);
    m_hOccupancyChartChip.Draw();
  }
  m_cOccupancyChartChip->Modified();
  m_cOccupancyChartChip->Update();

  // cluster time for clusters of track
  TH1* m_h = findHist("SVDClsTrk/SVDTRK_ClusterTimeV456");
  if (m_h != NULL) {
    m_hClusterOnTrackTime_L456V.Clear();
    m_h->Copy(m_hClusterOnTrackTime_L456V);
    m_hClusterOnTrackTime_L456V.GetXaxis()->SetRange(110, 190); // [-40 ns,40 ns]
    Float_t mean_PeakInCenter = m_hClusterOnTrackTime_L456V.GetMean(); //
    m_hClusterOnTrackTime_L456V.GetXaxis()->SetRange(); // back to [-150 ns,150 ns]
    m_hClusterOnTrackTime_L456V.SetTitle("ClusterOnTrack Time L456V " + runID);
    bool hasError = false;
    if (nEvents > m_statThreshold) {
      if (runtype == "physics") {
        Float_t difference_physics = fabs(mean_PeakInCenter - m_refMeanP);
        if (difference_physics > m_timeThreshold) {
          hasError = true;
        }
      } else if (runtype == "cosmic") {
        Float_t difference_cosmic = fabs(mean_PeakInCenter - m_refMeanC);
        if (difference_cosmic > m_timeThreshold) {
          hasError = true;
        }
      } else {
        B2WARNING("Run type:" << runtype);
      }
    } else {
      m_cClusterOnTrackTime_L456V->SetFillColor(kGray);
      m_cClusterOnTrackTime_L456V->SetFrameFillColor(10);
    }
    if (! hasError) {
      m_cClusterOnTrackTime_L456V->SetFillColor(kGreen);
      m_cClusterOnTrackTime_L456V->SetFrameFillColor(10);
    } else {
      m_legError->Draw("same");
      m_cClusterOnTrackTime_L456V->SetFillColor(kRed);
      m_cClusterOnTrackTime_L456V->SetFrameFillColor(10);
    }
  } else {
    B2INFO("Histogram SVDClsTrk/c_SVDTRK_ClusterTimeV456 from SVDDQMClustersOnTrack module not found!");
    m_cClusterOnTrackTime_L456V->SetFillColor(kRed);
  }

  m_cClusterOnTrackTime_L456V->cd();
  m_hClusterOnTrackTime_L456V.Draw();

  m_cClusterOnTrackTime_L456V->Modified();
  m_cClusterOnTrackTime_L456V->Update();

  //check MODULE OCCUPANCY online & offline

  //reset canvas color
  m_occUstatus = 0;
  m_occVstatus = 0;
  m_onlineOccUstatus = 0;
  m_onlineOccVstatus = 0;

  //update titles with exp and run number
  m_hOccupancyU->SetTitle("Average OFFLINE Sensor Occupancy (%), U side " + runID);
  m_hOccupancyU->SetStats(0);
  m_hOccupancyV->SetTitle("Average OFFLINE Sensor Occupancy (%), V side " + runID);
  m_hOccupancyV->SetStats(0);

  m_hOnlineOccupancyU->SetTitle("Average ONLINE Sensor Occupancy (%), U side " + runID);
  m_hOnlineOccupancyU->SetStats(0);
  m_hOnlineOccupancyV->SetTitle("Average ONLINE Sensor Occupancy (%), V side " + runID);
  m_hOnlineOccupancyV->SetStats(0);


  //set dedicate gStyle
  const Int_t colNum = 4;
  Int_t palette[colNum] {kBlack,  kGreen, kOrange, kRed};
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

      //test ERRORS
      /*
      if(bin == m_hOccupancyU->FindBin(2, findBinY(3, 1))){
      occU = 0; //0,1,1.5,1.8,2,3
      m_hOccupancyU->SetBinContent(bin, occU);
      }
      */

      if (occU <= m_occEmpty) {
        if (m_occUstatus < 1) m_occUstatus = 1;
      } else if (occU > m_occWarning) {
        if (occU < m_occError) {
          if (m_occUstatus < 2) m_occUstatus = 2;
        } else {
          if (m_occUstatus < 3) m_occUstatus = 3;
        }
      }

      //      B2INFO(" x = " << tmp_ladder << ", y = " << tmp_layer * 10 + tmp_sensor << " U occ = " << occU << " status = " << m_occUstatus);

      //produce the occupancy plot
      m_hStripOccupancyU[i].Clear();
      htmp->Copy(m_hStripOccupancyU[i]);
      m_hStripOccupancyU[i].Scale(1 / nEvents);
      m_hStripOccupancyU[i].SetName(Form("%d_%d_%d_OccupancyU", tmp_layer, tmp_ladder, tmp_sensor));
      m_hStripOccupancyU[i].SetTitle(Form("SVD Sensor %d_%d_%d U-Strip OFFLINE Occupancy vs Strip Number", tmp_layer, tmp_ladder,
                                          tmp_sensor));
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

      //test ERRORS
      /*
      if(bin == m_hOccupancyV->FindBin(3, findBinY(3, 1))){
      occV = 1.8; //0,1,1.5,1.8,2,3
      m_hOccupancyV->SetBinContent(bin, occV);
      }
      */

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
      m_hStripOccupancyV[i].Clear();
      htmp->Copy(m_hStripOccupancyV[i]);
      m_hStripOccupancyV[i].Scale(1 / nEvents);
      m_hStripOccupancyV[i].SetName(Form("%d_%d_%d_OccupancyV", tmp_layer, tmp_ladder, tmp_sensor));
      m_hStripOccupancyV[i].SetTitle(Form("SVD Sensor %d_%d_%d V-Strip OFFLINE Occupancy vs Strip Number", tmp_layer, tmp_ladder,
                                          tmp_sensor));

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
      //test ERRORS
      /*
        if(bin == m_hOnlineOccupancyV->FindBin(3, findBinY(3, 1))){
        onlineOccV = 1.8; //0,1,1.5,1.8,2,3
        m_hOnlineOccupancyV->SetBinContent(bin, onlineOccV);
        }
      */

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
      //test ERRORS
      /*
        if(bin == m_hOnlineOccupancyU->FindBin(3, findBinY(3, 1))){
        onlineOccU = 1.8; //0,1,1.5,1.8,2,3
        m_hOnlineOccupancyU->SetBinContent(bin, onlineOccU);
        }
      */

      if (onlineOccU <= m_onlineOccEmpty) {
        if (m_onlineOccUstatus < 1) m_onlineOccUstatus = 1;
      } else if (onlineOccU > m_onlineOccWarning) {
        if (onlineOccU < m_onlineOccError) {
          if (m_onlineOccUstatus < 2) m_onlineOccUstatus = 2;
        } else {
          if (m_onlineOccUstatus < 3) m_onlineOccUstatus = 3;
        }
      }

      //B2INFO(" x = " << tmp_ladder << ", y = " << tmp_layer * 10 + tmp_sensor << " V occ = " << occV << " status = " << m_occVstatus);
    }

    //update sensor occupancy canvas U and V
    m_cStripOccupancyU[i]->cd();
    m_hStripOccupancyU[i].Draw("histo");
    m_cStripOccupancyV[i]->cd();
    m_hStripOccupancyV[i].Draw("histo");
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
      m_cOccupancyU->SetFillColor(kOrange);
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
      m_cOccupancyV->SetFillColor(kOrange);
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
      m_cOnlineOccupancyU->SetFillColor(kOrange);
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
      m_cOnlineOccupancyV->SetFillColor(kOrange);
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

}

void DQMHistAnalysisSVDGeneralModule::endRun()
{
  B2INFO("DQMHistAnalysisSVDGeneral: endRun called");
}


void DQMHistAnalysisSVDGeneralModule::terminate()
{
  B2INFO("DQMHistAnalysisSVDGeneral: terminate called");

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

  for (int module = 0; module < nSensors; module++) {
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

