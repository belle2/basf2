/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisTrackingAbort.h>

#include <TROOT.h>
#include <TString.h>
#include <TH1F.h>
#include <TH2F.h>

#include <TMath.h>
#include <iostream>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTrackingAbort);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTrackingAbortModule::DQMHistAnalysisTrackingAbortModule()
  : DQMHistAnalysisModule()
{

  setDescription("DQM Analysis Module of the Tracking HLT Plots.");

  addParam("failureRateThreshold", m_failureRateThreshold,
           "Maximum Fraction of Events in which Tracking Aborts before turning Canvas to Red. Will be taken from Epics by default, \
    this value is only taken if Epics is not available!", double(m_failureRateThreshold));
  addParam("minNoEvents", m_statThreshold,
           "Minimum Number of Events before scaring CR shifters. Will be taken from Epics by default, \
    this value is only taken if Epics is not available!", int(m_statThreshold));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(m_printCanvas));

}



void DQMHistAnalysisTrackingAbortModule::beginRun()
{
  // get the abort rate and statThreshold from epics
  double buffThreshold(NAN);
  double buffMinEvents(NAN);
  double dummyLowerAlarm, dummyLowerWarn, dummyUpperWarn, dummyUpperAlarm;

  // read thresholds from EPICS
  requestLimitsFromEpicsPVs("abortRate", dummyLowerAlarm, dummyLowerWarn, dummyUpperWarn, buffThreshold);
  requestLimitsFromEpicsPVs("minNoEvents", dummyLowerAlarm, buffMinEvents, dummyUpperWarn, dummyUpperAlarm);

  if (!std::isnan(buffThreshold)) {
    B2INFO(getName() << ": Setting failure rate threshold from EPICS. New failureRateThreshold " << buffThreshold);
    m_failureRateThreshold = buffThreshold;
  }
  if (!std::isnan(buffMinEvents)) {
    B2INFO(getName() << ": Setting min number of events threshold from EPICS. New minNoEvents " << buffMinEvents);
    m_statThreshold = buffMinEvents;
  }

}


void DQMHistAnalysisTrackingAbortModule::initialize()
{

  gROOT->cd();
  m_cAbortRate = new TCanvas("TrackingAnalysis/c_TrackingAbort");
  m_cAbortRate_BF = new TCanvas("TrackingAnalysis/c_TrackingAbort_BF");

  m_cAbortRateIN  = new TCanvas("TrackingAnalysis/c_TrackingAbortIN");
  m_cAbortRateOUT  = new TCanvas("TrackingAnalysis/c_TrackingAbortOUT");
  m_cAbortRateIN_BF  = new TCanvas("TrackingAnalysis/c_TrackingAbortIN_BF");
  m_cAbortRateOUT_BF  = new TCanvas("TrackingAnalysis/c_TrackingAbortOUT_BF");

  // add MonitoringObject
  m_monObj = getMonitoringObject("trackingHLT");

  // register the PVs for setting thresholds
  registerEpicsPV("TRACKING:minNoEvents", "minNoEvents");

  // variables to be monitored via EPICS
  //abort rates
  registerEpicsPV("trackingHLT:abortRate", "abortRate");
  registerEpicsPV("trackingHLT:abortRate_inActiveVeto", "abortRate_inActiveVeto");
  registerEpicsPV("trackingHLT:abortRate_outActiveVeto", "abortRate_outActiveVeto");
  registerEpicsPV("trackingHLT:abortRateBeforeFilter", "abortRateBeforeFilter");
  registerEpicsPV("trackingHLT:abortRateBeforeFilter_inActiveVeto", "abortRateBeforeFilter_inActiveVeto");
  registerEpicsPV("trackingHLT:abortRateBeforeFilter_outActiveVeto", "abortRateBeforeFilter_outActiveVeto");


  //abort rate AFTER the Filter:
  //must have THE SAME binning as mEventsWithAborts from TrackingAbortDQM
  std::string histoName = "EventsWithAborts";
  std::string histoTitle = "Fraction of Events With at Least one Abort [After Filter]";
  m_hAbort = new TH1F(TString::Format("%s", histoName.c_str()),
                      TString::Format("%s", histoTitle.c_str()),
                      2, -0.5, 1.5);
  m_hAbort->GetYaxis()->SetTitle("Number of Events");
  m_hAbort->GetXaxis()->SetBinLabel(1, "No Abort");
  m_hAbort->GetXaxis()->SetBinLabel(2, "At Least One Abort");
  m_hAbort->SetMinimum(0.1);

  //abort rate BEFORE the Filter:
  //must have THE SAME binning as mEventsWithAborts from TrackingAbortDQM
  histoName = "EventsWithAborts_beforeFilter";
  histoTitle = "Fraction of Events With at Least one Abort [Before Filter]";
  m_hAbort_BF = new TH1F(TString::Format("%s", histoName.c_str()),
                         TString::Format("%s", histoTitle.c_str()),
                         2, -0.5, 1.5);
  m_hAbort_BF->GetYaxis()->SetTitle("Number of Events");
  m_hAbort_BF->GetXaxis()->SetBinLabel(1, "No Abort");
  m_hAbort_BF->GetXaxis()->SetBinLabel(2, "At Least One Abort");
  m_hAbort_BF->SetMinimum(0.1);

}

void DQMHistAnalysisTrackingAbortModule::event()
{

  //check Tracking Abort Rate AFTER the filter
  TH1* hAbortIn = findHist("TrackingAbort/EventsWithAborts_IN");
  TH1* hAbortOut = findHist("TrackingAbort/EventsWithAborts_OUT");

  int nEventsIN = 0;
  int nEventsOUT = 0;

  if (hAbortIn != nullptr && hAbortOut != nullptr) {

    nEventsIN = hAbortIn->GetEntries();
    m_monObj->setVariable("nEvents_inActiveVeto", nEventsIN);
    nEventsOUT = hAbortOut->GetEntries();
    m_monObj->setVariable("nEvents_outActiveVeto", nEventsOUT);
    const int nEvents = nEventsIN + nEventsOUT;

    m_hAbort->SetBinContent(1, hAbortIn->GetBinContent(1) + hAbortOut->GetBinContent(1));
    m_hAbort->SetBinContent(2, hAbortIn->GetBinContent(2) + hAbortOut->GetBinContent(2));

    const double abortRate = (double)m_hAbort->GetBinContent(2) / (m_hAbort->GetBinContent(1) + m_hAbort->GetBinContent(2));
    m_hAbort->SetTitle(Form("[After Filter] Fraction of Events in which Tracking Aborts = %.2f %%", abortRate * 100));

    if (nEvents >= m_statThreshold) {
      m_monObj->setVariable("abortRate", abortRate);
      setEpicsPV("abortRate", abortRate);

      const double abortRate_in = hAbortIn->GetMean();
      m_monObj->setVariable("abortRate_inActiveVeto", abortRate_in);
      setEpicsPV("abortRate_inActiveVeto", abortRate_in);

      const double abortRate_out = hAbortOut->GetMean();
      m_monObj->setVariable("abortRate_outActiveVeto", abortRate_out);
      setEpicsPV("abortRate_outActiveVeto", abortRate_out);

    }

    //check if number of errors is above the allowed limit
    const bool hasError = abortRate > m_failureRateThreshold;

    m_cAbortRateIN->cd();
    hAbortIn->Draw();

    m_cAbortRateOUT->cd();
    hAbortOut->Draw();

    m_cAbortRate->cd();
    m_hAbort->Draw();


    auto state = makeStatus(nEvents >= m_statThreshold, false, hasError);
    colorizeCanvas(m_cAbortRate, state);

  } else { // histograms not found
    colorizeCanvas(m_cAbortRate, EStatus::c_StatusTooFew);
  }

  m_cAbortRate->Modified();
  m_cAbortRate->Update();

  //check Tracking Abort Rate BEFORE the filter

  TH1* hAbortIn_BF = findHist("TrackingAbort_before_filter/EventsWithAborts_IN");
  TH1* hAbortOut_BF = findHist("TrackingAbort_before_filter/EventsWithAborts_OUT");

  int nEventsINbf = 0;
  int nEventsOUTbf = 0;

  if (hAbortIn_BF != nullptr && hAbortOut_BF != nullptr) {

    nEventsINbf = hAbortIn_BF->GetEntries();
    m_monObj->setVariable("nEventsBeforeFilter_inActiveVeto", nEventsINbf);
    nEventsOUTbf = hAbortOut_BF->GetEntries();
    m_monObj->setVariable("nEventsBeforeFilter_outActiveVeto", nEventsOUTbf);
    const int nEvents_BF = nEventsINbf + nEventsOUTbf;

    m_hAbort_BF->SetBinContent(1, hAbortIn_BF->GetBinContent(1) + hAbortOut_BF->GetBinContent(1));
    m_hAbort_BF->SetBinContent(2, hAbortIn_BF->GetBinContent(2) + hAbortOut_BF->GetBinContent(2));

    const double abortRate_BF = (double)m_hAbort_BF->GetBinContent(2) / (m_hAbort_BF->GetBinContent(1) + m_hAbort_BF->GetBinContent(2));
    m_hAbort_BF->SetTitle(Form("[Before Filter] Fraction of Events in which Tracking Aborts = %.2f %%", abortRate_BF * 100));

    if (nEvents_BF >= m_statThreshold) {
      m_monObj->setVariable("abortRateBeforeFilter", abortRate_BF);
      setEpicsPV("abortRateBeforeFilter", abortRate_BF);

      const double abortRate_in = hAbortIn_BF->GetMean();
      m_monObj->setVariable("abortRateBeforeFilter_inActiveVeto", abortRate_in);
      setEpicsPV("abortRateBeforeFilter_inActiveVeto", abortRate_in);
      const double abortRate_out = hAbortOut_BF->GetMean();
      m_monObj->setVariable("abortRateBeforeFilter_outActiveVeto", abortRate_out);
      setEpicsPV("abortRateBeforeFilter_outActiveVeto", abortRate_out);
    }

    m_cAbortRateIN_BF->cd();
    hAbortIn_BF->Draw();

    m_cAbortRateOUT_BF->cd();
    hAbortOut_BF->Draw();

    m_cAbortRate_BF->cd();
    m_hAbort_BF->Draw();

  }

  //scale tracking abort reason histograms
  TH1F* hAbortReason_in = (TH1F*)findHist("TrackingAbort/TrkAbortReason_IN");
  if (hAbortReason_in != nullptr) scaleAndSendToMirabelle(hAbortReason_in, nEventsIN, "_inActiveVeto");
  TH1F* hAbortReason_out = (TH1F*)findHist("TrackingAbort/TrkAbortReason_OUT");
  if (hAbortReason_out != nullptr) scaleAndSendToMirabelle(hAbortReason_out, nEventsOUT, "_outActiveVeto");
  TH1F* hAbortReason_in_BF = (TH1F*)findHist("TrackingAbort_before_filter/TrkAbortReason_IN");
  if (hAbortReason_in_BF != nullptr) scaleAndSendToMirabelle(hAbortReason_in_BF, nEventsIN, "BeforeFilter_inActiveVeto");
  TH1F* hAbortReason_out_BF = (TH1F*)findHist("TrackingAbort_before_filter/TrkAbortReason_OUT");
  if (hAbortReason_out_BF != nullptr) scaleAndSendToMirabelle(hAbortReason_out_BF, nEventsOUT, "BeforeFilter_outActiveVeto");


  //scale average histograms
  TH1F* hAverage_in = (TH1F*)findHist("TrackingAbort/averages_IN");
  if (hAverage_in != nullptr) scaleAndSendToMirabelle(hAverage_in, nEventsIN, "_inActiveVeto");

  TH1F* hAverage_out = (TH1F*)findHist("TrackingAbort/averages_OUT");
  if (hAverage_out != nullptr) scaleAndSendToMirabelle(hAverage_out, nEventsOUT, "_outActiveVeto");

  TH1F* hAverage_in_BF = (TH1F*)findHist("TrackingAbort_before_filter/averages_IN");
  if (hAverage_in_BF != nullptr) scaleAndSendToMirabelle(hAverage_in_BF, nEventsINbf, "BeforeFilter_inActiveVeto");

  TH1F* hAverage_out_BF = (TH1F*)findHist("TrackingAbort_before_filter/averages_OUT");
  if (hAverage_out_BF != nullptr) scaleAndSendToMirabelle(hAverage_out_BF, nEventsOUTbf, "BeforeFilter_outActiveVeto");

  // average SVD L3U Occupancy to Mirabelle
  TH1* hL3UOccIn = findHist("TrackingAbort/SVDL3UOcc_IN");
  if (hL3UOccIn != nullptr) m_monObj->setVariable("svdL3UOcc_inActiveVeto", hL3UOccIn->GetMean());
  TH1* hL3UOccOut = findHist("TrackingAbort/SVDL3UOcc_OUT");
  if (hL3UOccOut != nullptr) m_monObj->setVariable("svdL3UOcc_outActiveVeto", hL3UOccOut->GetMean());
  TH1* hL3UOccIn_BF = findHist("TrackingAbort_before_filter/SVDL3UOcc_IN");
  if (hL3UOccIn_BF != nullptr) m_monObj->setVariable("svdL3UOccBeforeFilter_inActiveVeto", hL3UOccIn_BF->GetMean());
  TH1* hL3UOccOut_BF = findHist("TrackingAbort_before_filter/SVDL3UOcc_OUT");
  if (hL3UOccOut_BF != nullptr) m_monObj->setVariable("svdL3UOccBeforeFilter_outActiveVeto", hL3UOccOut_BF->GetMean());


  // average n CDC extra hits to Mirabelle
  TH1* hCDCExtraHitsIn = findHist("TrackingAbort/nCDCExtraHits_IN");
  if (hCDCExtraHitsIn != nullptr) m_monObj->setVariable("nCDCExtraHits_inActiveVeto", hCDCExtraHitsIn->GetMean());
  TH1* hCDCExtraHitsOut = findHist("TrackingAbort/nCDCExtraHits_OUT");
  if (hCDCExtraHitsOut != nullptr) m_monObj->setVariable("nCDCExtraHits_outActiveVeto", hCDCExtraHitsOut->GetMean());
  TH1* hCDCExtraHitsIn_BF = findHist("TrackingAbort_before_filter/nCDCExtraHits_IN");
  if (hCDCExtraHitsIn_BF != nullptr) m_monObj->setVariable("nCDCExtraHitsBeforeFilter_inActiveVeto", hCDCExtraHitsIn_BF->GetMean());
  TH1* hCDCExtraHitsOut_BF = findHist("TrackingAbort_before_filter/nCDCExtraHits_OUT");
  if (hCDCExtraHitsOut_BF != nullptr) m_monObj->setVariable("nCDCExtraHitsBeforeFilter_outActiveVeto",
                                                              hCDCExtraHitsOut_BF->GetMean());

  if (m_printCanvas) {
    m_cAbortRate->Print("c_TrackingAbort.pdf[");
    m_cAbortRate->Print("c_TrackingAbort.pdf");
    m_cAbortRateIN->Print("c_TrackingAbort.pdf");
    m_cAbortRateOUT->Print("c_TrackingAbort.pdf");
    m_cAbortRateIN_BF->Print("c_TrackingAbort.pdf");
    m_cAbortRateOUT_BF->Print("c_TrackingAbort.pdf");
    m_cAbortRateOUT_BF->Print("c_TrackingAbort.pdf]");
  }


}

void DQMHistAnalysisTrackingAbortModule::terminate()
{
  delete m_hAbort;
  delete m_hAbort_BF;
  delete m_cAbortRate;
  delete m_cAbortRate_BF;
  delete m_cAbortRateIN;
  delete m_cAbortRateIN_BF;
  delete m_cAbortRateOUT;
  delete m_cAbortRateOUT_BF;

}

void DQMHistAnalysisTrackingAbortModule::scaleAndSendToMirabelle(TH1F* hAverage, const int nEvents, const TString& tag)
{

  //scale the histogram to the number of events
  hAverage->Scale(1. / nEvents);

  const int nBins = hAverage->GetNbinsX();
  for (int bin = 1; bin < nBins + 1; bin++) {
    const TString binLabel = hAverage->GetXaxis()->GetBinLabel(bin);
    const TString varName = TString::Format("%s%s", binLabel.Data(), tag.Data());
    const float varValue = hAverage->GetBinContent(bin);
    m_monObj->setVariable(varName.Data(), varValue);
  }


}




