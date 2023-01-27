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
#include <TROOT.h>
#include <TClass.h>

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

  //Parameter definition
  addParam("HistoName", m_histoName, "Name of Histogram (excl dir)", std::string(""));
  addParam("HistoDirectory", m_histoDirectory, "Name of Histogram dir", std::string(""));
  addParam("PVName", m_pvPrefix, "PV Prefix", std::string("DQM:TEST:hist:"));
  addParam("MonitorPrefix", m_monPrefix, "Monitor Prefix");// force to be set!
  addParam("useEpics", m_useEpics, "Whether to update EPICS PVs.", false);
  addParam("minEntries", m_minEntries, "minimum number of new Entries for a fit", 1000);
  B2DEBUG(20, "DQMHistAnalysisIP: Constructor done.");
}


DQMHistAnalysisIPModule::~DQMHistAnalysisIPModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisIPModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisIP: initialized.");

  m_monObj = getMonitoringObject("ip");

  addDeltaPar(m_histoDirectory, m_histoName, HistDelta::c_Entries, m_minEntries, 1); // register delta

  auto  a = m_histoDirectory + "/" + m_histoName;
  m_c1 = new TCanvas((a +
                      "_fit").data()); // prefer to change canvas name to monitorPrefix, but then changes on the web gui are needed :-(

  m_line = new TLine(0, 10, 0, 0);
  m_line->SetVertical(true);
  m_line->SetLineColor(8);
  m_line->SetLineWidth(3);
  m_line2 = new TLine(0, 10, 0, 0);
  m_line2->SetVertical(true);
  m_line2->SetLineColor(9);
  m_line2->SetLineWidth(3);

  m_monObj->addCanvas(m_c1);

  // need the function to get parameter names
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    std::string aa;
    aa = m_pvPrefix + "Mean";
    SEVCHK(ca_create_channel(aa.c_str(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    aa = m_pvPrefix + "RMS";
    SEVCHK(ca_create_channel(aa.c_str(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
    aa = m_pvPrefix + "Median";
    SEVCHK(ca_create_channel(aa.c_str(), NULL, NULL, 10, &mychid[2]), "ca_create_channel failure");
    // Read LO and HI limits from EPICS, seems this needs additional channels?
    // SEVCHK(ca_get(DBR_DOUBLE,mychid[i],(void*)&data),"ca_get failure"); // data is only valid after ca_pend_io!!
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
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

#ifdef _BELLE2_EPICS
    if (m_useEpics) {
      B2DEBUG(20, "Update EPICS");
      if (mychid[0]) SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&x), "ca_set failure");
      if (mychid[1]) SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&w), "ca_set failure");
      if (mychid[2]) SEVCHK(ca_put(DBR_DOUBLE, mychid[2], (void*)&m), "ca_set failure");
      SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
    }
#endif
  } else {
    B2DEBUG(20, "Histo " << m_histoName << " not found in " << m_histoDirectory);
  }
}

void DQMHistAnalysisIPModule::terminate()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto i = 0; i < m_parameters; i++) {
      if (mychid[i]) SEVCHK(ca_clear_channel(mychid[i]), "ca_clear_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
  B2DEBUG(20, "DQMHistAnalysisIP: terminate called");
}

