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
  addParam("Prefix", m_Prefix, "Prefix HLT or ERECO", std::string(""));

  B2DEBUG(99, "DQMHistAnalysisRunNr: Constructor done.");
}

DQMHistAnalysisRunNrModule::~DQMHistAnalysisRunNrModule()
{
}

void DQMHistAnalysisRunNrModule::initialize()
{
  m_monObj = getMonitoringObject("daq");

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cRunNr = new TCanvas((m_histogramDirectoryName + "/c_RunNr").data());

  // m_monObj->addCanvas(m_cRunNr);// useful?

  registerEpicsPV("DAQ:" + Prefix + ":RunNr:RunNr", "RunNr");
  registerEpicsPV("DAQ:" + Prefix + ":RunNr:Alarm", "Alarm");

  B2DEBUG(99, "DQMHistAnalysisRunNr: initialized.");
}

void DQMHistAnalysisRunNrModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisRunNr: beginRun called.");

  m_cRunNr->Clear();

  // make sure we reset at run start to retrigger the alarm
  double mean = 0.0; // must be double, mean of histogram -> runnr
  int status = 0; // must be int, epics alarm status 0 = no data, 2 = o.k., 4 = not o.k.
  // if status & runnr valid
  setEpicsPV("Alarm", status);
  setEpicsPV("RunNr", mean);
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


  setEpicsPV("Alarm", status);
  setEpicsPV("RunNr", mean);
}

void DQMHistAnalysisRunNrModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisRunNr: terminate called");
}

