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

using namespace std;
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
  double dummy_lowerAlarm, dummy_lowerWarn, dummy_upperWarn, dummy_upperAlarm;

  // read thresholds from EPICS
  requestLimitsFromEpicsPVs("abortRate", dummy_lowerAlarm, dummy_lowerWarn, dummy_upperWarn, buffThreshold);
  requestLimitsFromEpicsPVs("minNoEvents", dummy_lowerAlarm, buffMinEvents, dummy_upperWarn, dummy_upperAlarm);

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
  m_cAbortRate = new TCanvas("TrackingAnalysis/c_AbortRate");

  m_cAbortRateIN  = new TCanvas("TrackingAnalysis/c_AbortRateIN");
  m_cAbortRateOUT  = new TCanvas("TrackingAnalysis/c_AbortRateOUT");
  m_cAbortRateIN_BF  = new TCanvas("TrackingAnalysis/c_AbortRateIN_BF");
  m_cAbortRateOUT_BF  = new TCanvas("TrackingAnalysis/c_AbortRateOUT_BF");

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

  //

}

void DQMHistAnalysisTrackingAbortModule::event()
{


  //check Tracking Abort Rate AFTER the filter
  TH1* hAbort_in = findHist("TrackingAbort/EventsWithAborts_IN");
  TH1* hAbort_out = findHist("TrackingAbort/EventsWithAborts_OUT");

  int nEventsIN;
  int nEventsOUT;

  if (hAbort_in != nullptr && hAbort_out) {

    nEventsIN = hAbort_in->GetEntries();
    m_monObj->setVariable("nEvents_inActiveVeto", nEventsIN);
    nEventsOUT = hAbort_out->GetEntries();
    m_monObj->setVariable("nEvents_outActiveVeto", nEventsOUT);
    const int nEvents = nEventsIN + nEventsOUT;

    TH1F* hAbort = (TH1F*)hAbort_in->Add((TH1F*)hAbort_out, 1);
    bool hasError = false;
    double abortRate = hAbort->GetMean();
    hAbort->SetTitle(Form("Fraction of Events in which Tracking Aborts = %.4f %%", abortRate * 100));

    if (nEvents >= m_statThreshold) {
      m_monObj->setVariable("abortRate", abortRate);
      setEpicsPV("abortRate", abortRate);

      double abortRate_in = hAbort_in->GetMean();
      m_monObj->setVariable("abortRate_inActiveVeto", abortRate_in);
      setEpicsPV("abortRate_inActiveVeto", abortRate_in);

      double abortRate_out = hAbort_out->GetMean();
      m_monObj->setVariable("abortRate_outActiveVeto", abortRate_out);
      setEpicsPV("abortRate_outActiveVeto", abortRate_out);

    }

    //check if number of errors is above the allowed limit
    if (abortRate > m_failureRateThreshold)
      hasError = true;

    m_cAbortRateIN->cd();
    hAbort_in->Draw();

    m_cAbortRateOUT->cd();
    hAbort_out->Draw();

    m_cAbortRate->cd();
    hAbort->Draw();


    if (nEvents < m_statThreshold) colorizeCanvas(m_cAbortRate, EStatus::c_StatusTooFew);
    else if (hasError) colorizeCanvas(m_cAbortRate, EStatus::c_StatusError);
    else colorizeCanvas(m_cAbortRate, EStatus::c_StatusGood);

  } else { // histograms not found
    B2WARNING("Histograms TrackingAbort/EventsWithAborts_{IN,OUT} from TrackingAbort DQM not found!");
    m_cAbortRate->SetFillColor(kGray);
  }

  m_cAbortRate->Modified();
  m_cAbortRate->Update();

  if (m_printCanvas)
    m_cAbortRate->Print("c_AbortRate.pdf");


  //check Tracking Abort Rate BEFORE the filter

  TH1* hAbort_in_BF = findHist("TrackingAbort_before_filter/EventsWithAborts_IN");
  TH1* hAbort_out_BF = findHist("TrackingAbort_before_filter/EventsWithAborts_OUT");

  int nEventsIN_BF;
  int nEventsOUT_BF;

  if (hAbort_in_BF != nullptr && hAbort_out_BF) {

    nEventsIN_BF = hAbort_in_BF->GetEntries();
    m_monObj->setVariable("nEventsBeforeFilter_inActiveVeto", nEventsIN_BF);
    nEventsOUT_BF = hAbort_out_BF->GetEntries();
    m_monObj->setVariable("nEventsBeforeFilter_outActiveVeto", nEventsOUT);
    const int nEvents_BF = nEventsIN_BF + nEventsOUT_BF;

    TH1F* hAbort_BF = (TH1F*)hAbort_in_BF->Add((TH1F*)hAbort_out_BF, 1);
    double abortRate_BF = hAbort_BF->GetMean();
    hAbort_BF->SetTitle(Form("[Before Filter] Fraction of Events in which Tracking Aborts = %.4f %%", abortRate_BF * 100));

    if (nEvents_BF >= m_statThreshold) {
      m_monObj->setVariable("abortRateBeforeFilter", abortRate_BF);
      setEpicsPV("abortRateBeforeFilter", abortRate_BF);

      double abortRate_in = hAbort_in_BF->GetMean();
      m_monObj->setVariable("abortRateBeforeFilter_inActiveVeto", abortRate_in);
      setEpicsPV("abortRateBeforeFilter_inActiveVeto", abortRate_in);
      double abortRate_out = hAbort_out_BF->GetMean();
      m_monObj->setVariable("abortRateBeforeFilter_outActiveVeto", abortRate_out);
      setEpicsPV("abortRateBeforeFilter_outActiveVeto", abortRate_out);
    }

    m_cAbortRateIN_BF->cd();
    hAbort_in_BF->Draw();

    m_cAbortRateOUT_BF->cd();
    hAbort_out_BF->Draw();

  } else { // histograms not found
    B2WARNING("Histograms TrackingAbort/EventsWithAborts_{IN,OUT} from TrackingAbort DQM not found!");
  }

  //scale average istograms
  TH1F* hAverage_in = (TH1F*)findHist("TrackingAbort/averages_IN");
  if (hAverage_in != nullptr) scaleAndSendToMirabelle(hAverage_in, nEventsIN, "_inActiveVeto");

  TH1F* hAverage_out = (TH1F*)findHist("TrackingAbort/averages_OUT");
  if (hAverage_out != nullptr) scaleAndSendToMirabelle(hAverage_out, nEventsOUT, "_outActiveVeto");

  TH1F* hAverage_in_BF = (TH1F*)findHist("TrackingAbort_before_filter/averages_IN");
  if (hAverage_in_BF != nullptr) scaleAndSendToMirabelle(hAverage_in_BF, nEventsIN_BF, "BeforeFilter_inActiveVeto");

  TH1F* hAverage_out_BF = (TH1F*)findHist("TrackingAbort_before_filter/averages_OUT");
  if (hAverage_out_BF != nullptr) scaleAndSendToMirabelle(hAverage_out_BF, nEventsOUT_BF, "BeforeFilter_outActiveVeto");

}

void DQMHistAnalysisTrackingAbortModule::scaleAndSendToMirabelle(TH1F* hAverage, int nEvents, TString tag)
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




