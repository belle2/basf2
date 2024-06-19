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
  setDescription("DQM Analysis for RunNr/Mixing Check");

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("DAQ"));
  addParam("Prefix", m_prefix, "Prefix HLT or ERECO");

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
  m_legend = new TPaveText(0.6, 0.6, 0.95, 0.95, "NDC");

  registerEpicsPV("DAQ:" + m_prefix + ":RunNr:RunNr", "RunNr");
  registerEpicsPV("DAQ:" + m_prefix + ":RunNr:Alarm", "Alarm");

  B2DEBUG(99, "DQMHistAnalysisRunNr: initialized.");
}

void DQMHistAnalysisRunNrModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisRunNr: beginRun called.");

  m_cRunNr->Clear();
  m_cRunNr->Pad()->SetFillColor(c_ColorTooFew);// Magenta or Gray
  m_legend->Clear();
  m_legend->SetFillColor(kWhite);
  m_legend->AddText("No data yet");
  m_legend->Draw();

  // make sure we reset at run start to retrigger the alarm
  double mean = 0.0; // must be double, mean of histogram -> runnr
  int status = 0; // must be int, epics alarm status 0 = no data, 2 = o.k., 4 = not o.k.
  // if status & runnr valid
  setEpicsPV("Alarm", status); // reset alarm status at run start
  setEpicsPV("RunNr", mean);
}

void DQMHistAnalysisRunNrModule::event()
{
  if (!m_cRunNr) return;

  auto name = "hRunnr";
  auto hist = findHist(m_histogramDirectoryName, name, true); // only if updated
  if (hist) {
    double mean = 0.0; // must be double, mean of histogram -> runnr
    int status = c_StatusTooFew; // must be int, epics alarm status 0 = no data, 2 = o.k., 4 = not o.k.
    m_cRunNr->Clear();
    m_cRunNr->cd();
    m_legend->Clear();
    m_legend->SetFillColor(kWhite);
    hist->SetStats(kFALSE); // get rid of annoying box, we have our own
    hist->Draw("hist");
    mean = hist->GetMean();
    if (hist->GetEntries() > 0) {
      m_legend->AddText("Contains Run: Entries");
      // loop over bins and check if more than one is used
      int nfilled = 0;
      for (int i = 0; i <= hist->GetXaxis()->GetNbins() + 1; i++) {
        // resizeable histogram, thus there should never be under or overflow. still we loop over them
        if (hist->GetBinContent(i) > 0) {
          nfilled++;
          TString tmp;
          tmp.Form("%ld: %ld", (long int)hist->GetXaxis()->GetBinCenter(i), (long int)hist->GetBinContent(i));
          m_legend->AddText(tmp);
        }
      }
      // Check number of bins filled
      if (nfilled > 1) {
        // problem
        status = c_StatusError;
      } else if (nfilled == 1) {
        // ok
        status = c_StatusGood;
      }// else nfilled=0, status stays 0 (no data)
    }

    if (status == c_StatusTooFew) {
      // no data (empty) or no histogram
      m_cRunNr->Pad()->SetFillColor(c_ColorTooFew);// Magenta or Gray
      m_legend->AddText("No data yet");
    } else if (status == c_StatusGood) {
      // only one run
      m_cRunNr->Pad()->SetFillColor(c_ColorGood);// Green
    } else { /*if ( status==4 )*/
      // anything else is bad
      m_cRunNr->Pad()->SetFillColor(c_ColorError);// Red
    }

    m_legend->Draw();

    m_cRunNr->Modified();
    m_cRunNr->Update();
    UpdateCanvas(m_cRunNr);

    setEpicsPV("Alarm", status);
    setEpicsPV("RunNr", mean);
  }
}

void DQMHistAnalysisRunNrModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisRunNr: terminate called");
  if (m_cRunNr) delete m_cRunNr;
  if (m_legend) delete m_legend;
}

