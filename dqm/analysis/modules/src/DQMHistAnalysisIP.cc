/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisIP.cc
// Description : Mean for IP position with delta histogramming
//-


#include <dqm/analysis/modules/DQMHistAnalysisIP.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisIP);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisIPModule::DQMHistAnalysisIPModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!
  setDescription("Modify and analyze the data quality histograms of IP Monitoring");

  //Parameter definition
  addParam("HistoName", m_histoName, "Name of Histogram (excl dir)", std::string(""));
  addParam("HistoDirectory", m_histoDirectory, "Name of Histogram dir", std::string(""));
  addParam("PVName", m_pvPrefix, "PV Prefix and Name", std::string("DQM:TEST:hist:"));
  addParam("MonitorPrefix", m_monPrefix, "Monitor Prefix");// force to be set!
  addParam("minEntries", m_minEntries, "minimum number of new Entries for a fit", 1000);
  B2DEBUG(20, "DQMHistAnalysisIP: Constructor done.");
}

void DQMHistAnalysisIPModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisIP: initialized.");

  m_monObj = getMonitoringObject("ip");

  // register delta
  if (!hasDeltaPar(m_histoDirectory, m_histoName)) addDeltaPar(m_histoDirectory, m_histoName, HistDelta::c_Entries, m_minEntries, 1);

  // prefer to change canvas name to monitorPrefix, but then changes on the web gui are needed :-(
  m_c1 = new TCanvas((m_histoDirectory + "/" + m_histoName +  "_fit").data());

  m_line = new TLine(0, 10, 0, 0);
  m_line->SetVertical(true);
  m_line->SetLineColor(8);
  m_line->SetLineWidth(3);
  m_line2 = new TLine(0, 10, 0, 0);
  m_line2->SetVertical(true);
  m_line2->SetLineColor(9);
  m_line2->SetLineWidth(3);

  m_monObj->addCanvas(m_c1);

  registerEpicsPV(m_pvPrefix + "Mean", "Mean");
  registerEpicsPV(m_pvPrefix + "RMS", "RMS");
  registerEpicsPV(m_pvPrefix + "Median", "Median");
  updateEpicsPVs(5.0);
}


void DQMHistAnalysisIPModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisIP: beginRun called.");
  m_c1->Clear();
}

void DQMHistAnalysisIPModule::event()
{
  auto delta = getDelta(m_histoDirectory, m_histoName);
  // do not care about initial filling handling. we wont show or update unless we reach the min req entries
  UpdateCanvas(m_c1->GetName(), delta != nullptr);
  if (delta != nullptr) {
    m_c1->Clear();
    m_c1->cd();// necessary!
    delta->Draw("hist");
    // we modify the current delta histogram, that is bad habit
    // but as long as no one else uses it, it may be o.k.
    // for more severe changes, maybe better work on a clone?
    delta->ResetStats(); // kills the Mean from filling, now only use bin values excl over/underflow
    double x = delta->GetMean();// must be double bc of EPICS below
    double w = delta->GetRMS();// must be double bc of EPICS below
    double q = 0.5; // array size one for quantiles
    double m = 0; // array of size 1 for result = median
    delta->ComputeIntegral(); // precaution
    delta->GetQuantiles(1, &m, &q);
    double y1 = delta->GetMaximum();
    double y2 = delta->GetMinimum();
    B2DEBUG(20, "Fit " << x << "," << w << "," << y1 << "," << y2);
    m_line->SetY1(y1 + (y1 - y2) * 0.05);
    m_line->SetX1(x);
    m_line->SetX2(x);
    m_line2->SetY1(y1 + (y1 - y2) * 0.05);
    m_line2->SetX1(m);
    m_line2->SetX2(m);
    delta->GetXaxis()->SetRangeUser(x - 3 * w, x + 3 * w);
    m_line->Draw();
    m_line2->Draw();
    m_c1->Modified();
    m_c1->Update();

    m_monObj->setVariable(m_monPrefix + "_median", m);
    m_monObj->setVariable(m_monPrefix + "_mean", x);
    m_monObj->setVariable(m_monPrefix + "_width", w);

    B2DEBUG(20, "Now update Epics PVs");
    setEpicsPV("Mean", x);
    setEpicsPV("RMS", w);
    setEpicsPV("Median", m);
    updateEpicsPVs(5.0);
  }
}
