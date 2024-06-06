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

}

void DQMHistAnalysisTrackingAbortModule::event()
{

  //check Tracking Abort Rate AFTER the filter
  TH1* hAbortIn = findHist("TrackingAbort/EventsWithAborts_IN");
  TH1* hAbortOut = findHist("TrackingAbort/EventsWithAborts_OUT");

  int nEventsIN;
  int nEventsOUT;

  if (hAbortIn != nullptr && hAbortOut) {

    nEventsIN = hAbortIn->GetEntries();
    m_monObj->setVariable("nEvents_inActiveVeto", nEventsIN);
    nEventsOUT = hAbortOut->GetEntries();
    m_monObj->setVariable("nEvents_outActiveVeto", nEventsOUT);
    const int nEvents = nEventsIN + nEventsOUT;

    TH1F* hAbort = (TH1F*)hAbortIn->Add((TH1F*)hAbortOut, 1);
    const double abortRate = hAbort->GetMean();
    hAbort->SetTitle(Form("Fraction of Events in which Tracking Aborts = %.4f %%", abortRate * 100));

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
    hAbort->Draw();


    if (nEvents < m_statThreshold) colorizeCanvas(m_cAbortRate, EStatus::c_StatusTooFew);
    else if (hasError) colorizeCanvas(m_cAbortRate, EStatus::c_StatusError);
    else colorizeCanvas(m_cAbortRate, EStatus::c_StatusGood);

  } else { // histograms not found
    m_cAbortRate->SetFillColor(kGray);
  }

  m_cAbortRate->Modified();
  m_cAbortRate->Update();

  //check Tracking Abort Rate BEFORE the filter

  TH1* hAbortIn_BF = findHist("TrackingAbort_before_filter/EventsWithAborts_IN");
  TH1* hAbortOut_BF = findHist("TrackingAbort_before_filter/EventsWithAborts_OUT");

  int nEventsINbf;
  int nEventsOUTbf;

  if (hAbortIn_BF != nullptr && hAbortOut_BF) {

    nEventsINbf = hAbortIn_BF->GetEntries();
    m_monObj->setVariable("nEventsBeforeFilter_inActiveVeto", nEventsINbf);
    nEventsOUTbf = hAbortOut_BF->GetEntries();
    m_monObj->setVariable("nEventsBeforeFilter_outActiveVeto", nEventsOUT);
    const int nEvents_BF = nEventsINbf + nEventsOUTbf;

    TH1F* hAbort_BF = (TH1F*)hAbortIn_BF->Add((TH1F*)hAbortOut_BF, 1);
    const double abortRate_BF = hAbort_BF->GetMean();
    hAbort_BF->SetTitle(Form("[Before Filter] Fraction of Events in which Tracking Aborts = %.4f %%", abortRate_BF * 100));

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

  }

  //scale average istograms
  TH1F* hAverage_in = (TH1F*)findHist("TrackingAbort/averages_IN");
  if (hAverage_in != nullptr) scaleAndSendToMirabelle(hAverage_in, nEventsIN, "_inActiveVeto");

  TH1F* hAverage_out = (TH1F*)findHist("TrackingAbort/averages_OUT");
  if (hAverage_out != nullptr) scaleAndSendToMirabelle(hAverage_out, nEventsOUT, "_outActiveVeto");

  TH1F* hAverage_in_BF = (TH1F*)findHist("TrackingAbort_before_filter/averages_IN");
  if (hAverage_in_BF != nullptr) scaleAndSendToMirabelle(hAverage_in_BF, nEventsINbf, "BeforeFilter_inActiveVeto");

  TH1F* hAverage_out_BF = (TH1F*)findHist("TrackingAbort_before_filter/averages_OUT");
  if (hAverage_out_BF != nullptr) scaleAndSendToMirabelle(hAverage_out_BF, nEventsOUTbf, "BeforeFilter_outActiveVeto");

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

void DQMHistAnalysisTrackingAbortModule::scaleAndSendToMirabelle(TH1F* hAverage, const int nEvents, const TString& tag)
{

  //scale the histogram to the number of events
  hAverage->Scale(1. / nEvents);

  int nBins = hAverage->GetNbinsX();
  for (int bin = 1; bin < nBins + 1; bin++) {
    TString binLabel = hAverage->GetXaxis()->GetBinLabel(bin);
    TString varName = TString::Format("%s%s", binLabel.Data(), tag.Data());
    float varValue = hAverage->GetBinContent(bin);
    m_monObj->setVariable(varName.Data(), varValue);
  }


}




