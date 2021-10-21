/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisTrackingHLT.h>

#include <TROOT.h>
#include <TString.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTrackingHLT)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTrackingHLTModule::DQMHistAnalysisTrackingHLTModule()
  : DQMHistAnalysisModule()
{

  setDescription("DQM Analysis Module of the Tracking HLT Plots.");

  addParam("failureRateThreshold", m_failureRateThreshold,
           "Maximum Fraction of Events in which Tracking Aborts before turning Canvas to Red", double(1.));
  addParam("minNoEvents", m_statThreshold, "Minimum Numner of Events before scaring CR shifters", int(1000));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));

}

void DQMHistAnalysisTrackingHLTModule::initialize()
{

  gROOT->cd();
  m_cAbortRate = new TCanvas("TrackingAnalysis/c_AbortRate");
  // add MonitoringObject
  m_monObj = getMonitoringObject("trackingHLT");

  m_rtype = findHist("DQMInfo/rtype");
  m_runtype = m_rtype ? m_rtype->GetTitle() : "";
}


void DQMHistAnalysisTrackingHLTModule::event()
{

  //check Tracking Abort Rate
  int nEvents = 0;
  double averageAbortRate = 0;
  bool hasError = false;

  TH1* hAbort = findHist("TrackingHLTDQM/NumberTrackingErrorFlags");
  if (hAbort != NULL) {
    nEvents = hAbort->GetEntries();
    averageAbortRate = hAbort->GetMean();
    hAbort->SetTitle(Form("Average Fraction of Events in which Tracking aborts = %.6f ", averageAbortRate));

    m_monObj->setVariable("avgAbortRate", averageAbortRate);
    //check if number of errors is above the allowed limit
    if (averageAbortRate > m_failureRateThreshold)
      hasError = true;
  } else {
    hasError = true;
    B2INFO("Histogram TrackingHLTDQM/NumberTrackingErrorFlags from Tracking DQM not found!");
  }

  if (nEvents < m_statThreshold) m_cAbortRate->SetFillColor(kGray);
  else if (hasError) m_cAbortRate->SetFillColor(kRed);
  else m_cAbortRate->SetFillColor(kGreen);

  m_cAbortRate->cd();
  hAbort->Draw();

  m_cAbortRate->Modified();
  m_cAbortRate->Update();

  if (m_printCanvas)
    m_cAbortRate->Print("c_AbortRate.pdf");

  // add average number of tracks per event to Mirabelle
  double averageNTracks = -1;
  TH1* hnTracks = findHist("TrackingHLTDQM/NoOfTracks");
  if (hnTracks != NULL)
    averageNTracks = hnTracks->GetMean();

  m_monObj->setVariable("nTracksPerEvent", averageNTracks);

}

void DQMHistAnalysisTrackingHLTModule::terminate()
{

  delete m_rtype;

}


