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
           "Maximum Fraction of Events in which Tracking Aborts before turning Canvas to Red", double(0.5));
  addParam("minNoEvents", m_statThreshold, "Minimum Number of Events before scaring CR shifters", int(1000));
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

  TH1* hAbort = findHist("TrackingHLTDQM/NumberTrackingErrorFlags");
  if (hAbort != NULL) {

    bool hasError = false;
    int nEvents = hAbort->GetEntries();
    double abortRate = hAbort->GetMean();
    hAbort->SetTitle(Form("Fraction of Events in which Tracking aborts = %.6f ", abortRate));

    m_monObj->setVariable("abortRate", abortRate);
    //check if number of errors is above the allowed limit
    if (abortRate > m_failureRateThreshold)
      hasError = true;

    if (nEvents < m_statThreshold) m_cAbortRate->SetFillColor(kGray);
    else if (hasError) m_cAbortRate->SetFillColor(kRed);
    else m_cAbortRate->SetFillColor(kGreen);
    m_cAbortRate->SetFrameFillColor(10);

    m_cAbortRate->cd();
    hAbort->Draw();

  } else { // histogram not found
    B2WARNING("Histogram TrackingHLTDQM/NumberTrackingErrorFlags from Tracking DQM not found!");
    m_cAbortRate->SetFillColor(kGray);
  }

  m_cAbortRate->Modified();
  m_cAbortRate->Update();


  if (m_printCanvas)
    m_cAbortRate->Print("c_AbortRate.pdf");

  // add average number of tracks per event to Mirabelle
  double averageNTracks = -1;
  TH1* hnTracks = findHist("TrackingHLTDQM/NoOfTracks");
  if (hnTracks != NULL)
    averageNTracks = hnTracks->GetMean();
  double averageNVXDTracks = -1;
  TH1* hnVXDTracks = findHist("TrackingHLTDQM/NoOfTracksInVXDOnly");
  if (hnVXDTracks != NULL)
    averageNVXDTracks = hnVXDTracks->GetMean();
  double averageNCDCTracks = -1;
  TH1* hnCDCTracks = findHist("TrackingHLTDQM/NoOfTracksInCDCOnly");
  if (hnCDCTracks != NULL)
    averageNCDCTracks = hnCDCTracks->GetMean();
  double averageNVXDCDCTracks = -1;
  TH1* hnVXDCDCTracks = findHist("TrackingHLTDQM/NoOfTracksInVXDCDC");
  if (hnVXDCDCTracks != NULL)
    averageNVXDCDCTracks = hnVXDCDCTracks->GetMean();

  m_monObj->setVariable("nTracksPerEvent", averageNTracks);
  m_monObj->setVariable("nVXDTracksPerEvent", averageNVXDTracks);
  m_monObj->setVariable("nCDCTracksPerEvent", averageNCDCTracks);
  m_monObj->setVariable("nVXDCDCTracksPerEvent", averageNVXDCDCTracks);

}

void DQMHistAnalysisTrackingHLTModule::terminate()
{

  delete m_rtype;

}


