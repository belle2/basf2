/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisRunNr.cc
// Description : DQM Analysis for RunNr Check
//-


#include <dqm/analysis/modules/DQMHistAnalysisRunNr.h>
#include <TROOT.h>
#include <TPaveText.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisRunNr);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisRunNrModule::DQMHistAnalysisRunNrModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("DAQ"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:DAQ:RunNr:"));

  B2DEBUG(99, "DQMHistAnalysisRunNr: Constructor done.");
}

DQMHistAnalysisRunNrModule::~DQMHistAnalysisRunNrModule()
{
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisRunNrModule::initialize()
{
  m_monObj = getMonitoringObject("daq");

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cRunNr = new TCanvas((m_histogramDirectoryName + "/c_RunNr").data());

  // m_monObj->addCanvas(m_cRunNr);// useful?

#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    mychid.resize(2);
    SEVCHK(ca_create_channel((m_pvPrefix + "Alarm").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "RunNr").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");

    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
  B2DEBUG(99, "DQMHistAnalysisRunNr: initialized.");
}

void DQMHistAnalysisRunNrModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisRunNr: beginRun called.");

  m_cRunNr->Clear();

  // make sure we reset at run start to retrigger the alarm
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    double mean = 0.0; // must be double, mean of histogram -> runnr
    int status = 0; // must be int, epics alarm status 0 = no data, 2 = o.k., 4 = not o.k.
    // if status & runnr valid
    SEVCHK(ca_put(DBR_INT, mychid[0], (void*)&status), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&mean), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisRunNrModule::event()
{
  if (!m_cRunNr) return;
  double mean = 0.0; // must be double, mean of histogram -> runnr
  int status = 0; // must be int, epics alarm status 0 = no data, 2 = o.k., 4 = not o.k.

  m_cRunNr->cd();
  m_cRunNr->Clear();

  auto leg = new TPaveText(0.6, 0.6, 0.95, 0.95, "NDC");
  leg->SetFillColor(kWhite);

  auto name = "hRunnr";
  TH1* hh1 = findHist(name);
  if (hh1 == NULL) {
    hh1 = findHist(m_histogramDirectoryName, name);
  }
  if (hh1) {
    hh1->SetStats(kFALSE); // get rid of annoying box, we have our own
    hh1->Draw("hist");
    mean = hh1->GetMean();
    if (hh1->GetEntries() > 0) {
      leg->AddText("Contains Run: Entries");
      // loop over bins and check if more than one is used
      int nfilled = 0;
      for (int i = 0; i <= hh1->GetXaxis()->GetNbins() + 1; i++) {
        // resizeable histogram, thus there should never be under or overflow. still we loop over them
        if (hh1->GetBinContent(i) > 0) {
          nfilled++;
          TString tmp;
          tmp.Form("%ld: %ld", (long int)hh1->GetXaxis()->GetBinCenter(i), (long int)hh1->GetBinContent(i));
          leg->AddText(tmp);
        }
      }
      if (nfilled > 1) {
        status = 4;
      } else if (nfilled == 1) {
        status = 2;
      }// else nfilled=0, status stays 0
    }
  }

  if (status == 0) {
    // no data (empty) or no histogram
    m_cRunNr->Pad()->SetFillColor(kGray);// Magenta or Gray
    leg->AddText("No data yet");
  } else if (status == 2) {
    // only one run
    m_cRunNr->Pad()->SetFillColor(kGreen);// Green
  } else { /*if ( status==4 )*/
    // anything else is bad
    m_cRunNr->Pad()->SetFillColor(kRed);// Red
  }

  leg->Draw();

  m_cRunNr->Modified();
  m_cRunNr->Update();


#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    // if status & runnr valid
    SEVCHK(ca_put(DBR_INT, mychid[0], (void*)&status), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&mean), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisRunNrModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisRunNr: terminate called");
  // should delete canvas here, maybe hist, too? Who owns it?
#ifdef _BELLE2_EPICS
  if (getUseEpics()) {
    for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

