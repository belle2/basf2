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
#include <TH1F.h>
#include <TH2F.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTrackingHLT);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTrackingHLTModule::DQMHistAnalysisTrackingHLTModule()
  : DQMHistAnalysisModule()
{

  setDescription("DQM Analysis Module of the Tracking HLT Plots.");

  addParam("failureRateThreshold", m_failureRateThreshold,
           "Maximum Fraction of Events in which Tracking Aborts before turning Canvas to Red", double(m_failureRateThreshold));
  addParam("minNoEvents", m_statThreshold, "Minimum Number of Events before scaring CR shifters", int(m_statThreshold));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(m_printCanvas));

}

void DQMHistAnalysisTrackingHLTModule::initialize()
{

  gROOT->cd();
  m_cAbortRate = new TCanvas("TrackingAnalysis/c_AbortRate");
  m_cAbortRateHER = new TCanvas("TrackingAnalysis/c_AbortRateHER");
  m_cAbortRateLER = new TCanvas("TrackingAnalysis/c_AbortRateLER");

  // add MonitoringObject
  m_monObj = getMonitoringObject("trackingHLT");

}

void DQMHistAnalysisTrackingHLTModule::event()
{

  //check Tracking Abort Rate

  TH1* hAbort = findHist("TrackingHLTDQM/NumberTrackingErrorFlags");
  if (hAbort != nullptr) {

    bool hasError = false;
    int nEvents = hAbort->GetEntries();
    double abortRate = hAbort->GetMean();
    hAbort->SetTitle(Form("Fraction of Events in which Tracking aborts = %.4f %%", abortRate * 100));

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


  // check tracking abort rate VS time after last HER injection and time within a beam cycle HER
  TH2F* hAbortHER = dynamic_cast<TH2F*>(findHist("TrackingHLTDQM/TrkAbortVsTimeHER"));
  TH2F* hAllHER = dynamic_cast<TH2F*>(findHist("TrackingHLTDQM/allEvtsVsTimeHER"));
  if (hAbortHER != nullptr && hAllHER != nullptr) {

    TH2F* hAbortRateHER = new TH2F(*hAbortHER);

    for (int i = 0; i < hAbortRateHER->GetXaxis()->GetNbins(); i++)
      for (int j = 0; j < hAbortRateHER->GetYaxis()->GetNbins(); j++) {
        int den = hAllHER->GetBinContent(i + 1, j + 1);
        int num = hAbortHER->GetBinContent(i + 1, j + 1);

        if (den > 0) hAbortRateHER->SetBinContent(i + 1, j + 1, num * 1. / den);
        else  hAbortRateHER->SetBinContent(i + 1, j + 1, 0);
      }

    m_cAbortRateHER->cd();
    hAbortRateHER->Draw("colz");


  } else { // histogram not found
    B2WARNING("Histograms TrackingHLTDQM/TrkAbortVsTimeHER or allEvtsVsTimeHER from Tracking DQM not found!");
    m_cAbortRateHER->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cAbortRateHER->Print("c_AbortRateHER.pdf");


  // check tracking abort rate VS time after last LER injection and time within a beam cycle LER
  TH2F* hAbortLER = dynamic_cast<TH2F*>(findHist("TrackingHLTDQM/TrkAbortVsTimeLER"));
  TH2F* hAllLER = dynamic_cast<TH2F*>(findHist("TrackingHLTDQM/allEvtsVsTimeLER"));
  if (hAbortLER != nullptr && hAllLER != nullptr) {

    TH2F* hAbortRateLER = new TH2F(*hAbortLER);

    for (int i = 0; i < hAbortRateLER->GetXaxis()->GetNbins(); i++)
      for (int j = 0; j < hAbortRateLER->GetYaxis()->GetNbins(); j++) {
        int den = hAllLER->GetBinContent(i + 1, j + 1);
        int num = hAbortLER->GetBinContent(i + 1, j + 1);

        if (den > 0) hAbortRateLER->SetBinContent(i + 1, j + 1, num * 1. / den);
        else  hAbortRateLER->SetBinContent(i + 1, j + 1, 0);
      }

    m_cAbortRateLER->cd();
    hAbortRateLER->Draw("colz");

  } else { // histogram not found
    B2WARNING("Histograms TrackingHLTDQM/TrkAbortVsTimeLER or allEvtsVsTimeLER from Tracking DQM not found!");
    m_cAbortRateLER->SetFillColor(kGray);
  }
  if (m_printCanvas)
    m_cAbortRateLER->Print("c_AbortRateLER.pdf");

  // add average number of tracks per event to Mirabelle
  TH1* hnTracks = findHist("TrackingHLTDQM/NoOfTracks");
  if (hnTracks != nullptr) {
    double averageNTracks = hnTracks->GetMean();
    m_monObj->setVariable("nTracksPerEvent", averageNTracks);
  }

  TH1* hnVXDTracks = findHist("TrackingHLTDQM/NoOfTracksInVXDOnly");
  if (hnVXDTracks != nullptr) {
    double averageNVXDTracks = hnVXDTracks->GetMean();
    m_monObj->setVariable("nVXDTracksPerEvent", averageNVXDTracks);
  }

  TH1* hnCDCTracks = findHist("TrackingHLTDQM/NoOfTracksInCDCOnly");
  if (hnCDCTracks != nullptr) {
    double averageNCDCTracks = hnCDCTracks->GetMean();
    m_monObj->setVariable("nCDCTracksPerEvent", averageNCDCTracks);
  }

  TH1* hnVXDCDCTracks = findHist("TrackingHLTDQM/NoOfTracksInVXDCDC");
  if (hnVXDCDCTracks != nullptr) {
    double averageNVXDCDCTracks = hnVXDCDCTracks->GetMean();
    m_monObj->setVariable("nVXDCDCTracksPerEvent", averageNVXDCDCTracks);
  }

}



