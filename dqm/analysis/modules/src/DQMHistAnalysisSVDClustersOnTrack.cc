/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDClustersOnTrack.cc
// Description : module for DQM histogram analysis of SVD sensors occupancies
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDClustersOnTrack.h>
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
REG_MODULE(DQMHistAnalysisSVDClustersOnTrack);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDClustersOnTrackModule::DQMHistAnalysisSVDClustersOnTrackModule()
  : DQMHistAnalysisSVDModule(true)
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVDClustersOnTrack: Constructor done.");

  setDescription("DQM Analysis Module that produces colored canvas for a straightforward interpretation of the SVD Data Quality.");

  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));
  addParam("statThreshold", m_statThreshold, "Minimal number of events to compare histograms", double(10000.));
  addParam("timeThreshold", m_timeThreshold, "Acceptable difference between mean of central peak for present and reference run",
           double(6)); // 6 ns
  addParam("refMCTP", m_refMeanP, "Mean of the signal time peak from Physics reference run", float(0.0)); // Approximate, from exp 20
  addParam("refMCTC", m_refMeanC, "Mean of the signal time peak from Cosmic reference run", float(0.0));  //
  addParam("samples3", m_3Samples, "if True 3 samples histograms analysis is performed", bool(false));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("SVD:"));
}

DQMHistAnalysisSVDClustersOnTrackModule::~DQMHistAnalysisSVDClustersOnTrackModule() { }

void DQMHistAnalysisSVDClustersOnTrackModule::initialize()
{
  B2DEBUG(10, "DQMHistAnalysisSVDClustersOnTrack: initialized.");

  m_cClusterOnTrackTime_L456V = new TCanvas("SVDAnalysis/c_ClusterOnTrackTime_L456V");

  if (m_3Samples)
    m_cClusterOnTrackTimeL456V3Samples = new TCanvas("SVDAnalysis/c_ClusterOnTrackTime_L456V3Samples");

  //register limits for EPICS
  registerEpicsPV(m_pvPrefix + "ratio3_6", "ratio3_6");
  registerEpicsPV(m_pvPrefix + "clusterTimeOnTrackLimits", "clusTimeOnTrkLimits");
}


void DQMHistAnalysisSVDClustersOnTrackModule::beginRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDClustersOnTrack: beginRun called.");

  m_cClusterOnTrackTime_L456V->Clear();

  if (m_3Samples)
    m_cClusterOnTrackTimeL456V3Samples->Clear();

  //Retrieve limits from EPICS
  double timeWarnUp = 0.;
  double timeErrorLo = 0.;
  double timeWarnLo = 0.;
  requestLimitsFromEpicsPVs("clusTimeOnTrkLimits", timeErrorLo, timeWarnLo, timeWarnUp,  m_timeThreshold);
  B2DEBUG(10, " SVD cluster time on track threshold taken from EPICS configuration file:");
  B2DEBUG(10, "  CLUSTER TIME ON TRACK: error > " << m_timeThreshold << " ns with minimum statistics of " << m_statThreshold);

  // cluster time on tracks legend
  m_legProblem->Clear();
  m_legProblem->AddText("ERROR!");
  m_legProblem->AddText(Form("abs(Mean) > %3.1f ns", m_timeThreshold));

  m_legWarning->Clear();
  m_legWarning->AddText("WARNING!");

  m_legNormal->Clear();
  m_legNormal->AddText("TIME SHIFT UNDER LIMIT");
  m_legNormal->AddText(Form("abs(Mean) < %3.1f ns", m_timeThreshold));

  m_legEmpty->Clear();
  m_legEmpty->AddText("Not enough statistics");
}

void DQMHistAnalysisSVDClustersOnTrackModule::event()
{
  B2DEBUG(10, "DQMHistAnalysisSVDClustersOnTrack: event called.");

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents", true);
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  } else {
    B2DEBUG(10, "SVDExpReco/SVDDQM_nEvents found");
  }

  string rtype = getRunType();
  m_runtype = !rtype.empty() ? rtype.c_str() : "physics"; // per default

  if (rtype.empty())
    B2INFO("no run type found, put defaultwise physics");

  TString tmp = hnEvnts->GetTitle();
  Int_t pos = tmp.Last('~');
  if (pos == -1) pos = 0;

  TString runID = tmp(pos, tmp.Length() - pos);
  B2INFO("DQMHistAnalysisSVDClustersOnTrackModule::runID = " << runID);
  Float_t nEvents = hnEvnts->GetEntries();

  // cluster time for clusters of track
  double ratio3_6 = 0.;
  TH1* m_h = findHist("SVDClsTrk/SVDTRK_ClusterTimeV456");

  int status = good;
  if (m_h != NULL) {

    m_hClusterOnTrackTime_L456V.Clear();
    m_h->Copy(m_hClusterOnTrackTime_L456V);
    m_hClusterOnTrackTime_L456V.SetName("ClusterOnTrackTime_L456V");
    m_hClusterOnTrackTime_L456V.SetTitle(Form("ClusterOnTrack Time L456V %s", runID.Data()));

    if (nEvents > m_statThreshold)
      status = getTimeStatus(m_hClusterOnTrackTime_L456V);
    else
      status = lowStat;

    m_cClusterOnTrackTime_L456V->cd();
    m_hClusterOnTrackTime_L456V.Draw();
    setStatusOfCanvas(status, m_cClusterOnTrackTime_L456V);

  } else {
    B2INFO("Histogram SVDClsTrk/c_SVDTRK_ClusterTimeV456 from SVDDQMClustersOnTrack module not found!");
    m_cClusterOnTrackTime_L456V->cd();
    m_hClusterOnTrackTime_L456V.Draw();
    setStatusOfCanvas(noStat, m_cClusterOnTrackTime_L456V);
  }

  if (m_printCanvas)
    m_cClusterOnTrackTime_L456V->Print("c_SVDClusterOnTrackTime_L456V.pdf");


  // cluster time for clusters of track for 3 samples
  if (m_3Samples) {
    m_h = findHist("SVDClsTrk/SVDTRK_Cluster3TimeV456");
    status = good;

    if (m_h != NULL) {
      m_hClusterOnTrackTimeL456V3Samples.Clear();
      m_h->Copy(m_hClusterOnTrackTimeL456V3Samples);
      m_hClusterOnTrackTimeL456V3Samples.SetTitle(Form("ClusterOnTrack Time L456V 3 samples %s", runID.Data()));

      if (nEvents > m_statThreshold)
        status = getTimeStatus(m_hClusterOnTrackTimeL456V3Samples);
      else
        status = lowStat;

      m_cClusterOnTrackTimeL456V3Samples->cd();
      m_hClusterOnTrackTimeL456V3Samples.Draw();
      setStatusOfCanvas(status, m_cClusterOnTrackTimeL456V3Samples);

    } else {
      B2INFO("Histogram SVDClsTrk/c_SVDTRK_Cluster3TimeV456 from SVDDQMClustersOnTrack module not found!");
      m_cClusterOnTrackTimeL456V3Samples->cd();
      m_hClusterOnTrackTimeL456V3Samples.Draw();
      setStatusOfCanvas(noStat, m_cClusterOnTrackTimeL456V3Samples);
    }

    if (m_printCanvas)
      m_cClusterOnTrackTimeL456V3Samples->Print("c_SVDClusterOnTrack3Time_L456V.pdf");

    ratio3_6 = m_hClusterOnTrackTimeL456V3Samples.GetEntries() / m_hClusterOnTrackTime_L456V.GetEntries();
  }

  setEpicsPV("ratio3_6", ratio3_6);
}

void DQMHistAnalysisSVDClustersOnTrackModule::endRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDClustersOnTrack: endRun called");
}


void DQMHistAnalysisSVDClustersOnTrackModule::terminate()
{
  B2DEBUG(10, "DQMHistAnalysisSVDClustersOnTrack: terminate called");

  delete m_cClusterOnTrackTime_L456V;
  delete m_cClusterOnTrackTimeL456V3Samples;
}

int DQMHistAnalysisSVDClustersOnTrackModule::getTimeStatus(TH1F& histo)
{
  int status = good;
  histo.GetXaxis()->SetRange(110, 190); // [-40 ns,40 ns]
  Float_t mean_PeakInCenter = histo.GetMean(); //
  histo.GetXaxis()->SetRange(); // back to [-150 ns,150 ns]
  Float_t difference = 0;

  if (m_runtype == "physics")
    difference = fabs(mean_PeakInCenter - m_refMeanP);

  else if (m_runtype == "cosmic")
    difference = fabs(mean_PeakInCenter - m_refMeanC);

  else {// taking cosmic limits
    B2WARNING("Run type:" << m_runtype << "taken cosmics criteria");
    difference = fabs(mean_PeakInCenter - m_refMeanC);
  }

  if (difference > m_timeThreshold)
    status = error;
  else
    status = good;

  return status;
}
