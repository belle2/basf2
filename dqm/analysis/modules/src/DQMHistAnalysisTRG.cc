/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisTRGModule.cc
// Description : Test Module for Delta Histogram Access
//-


#include <dqm/analysis/modules/DQMHistAnalysisTRG.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTRG);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTRGModule::DQMHistAnalysisTRGModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  // Parameter definition
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("TRG:"));
  B2DEBUG(1, "DQMHistAnalysisTRGModule: Constructor done.");

}

DQMHistAnalysisTRGModule::~DQMHistAnalysisTRGModule()
{
  // destructor not needed
  // EPICS singleton deletion not urgent -> can be done by framework
}

void DQMHistAnalysisTRGModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: initialized.");

  gROOT->cd();
  m_canvas = new TCanvas("TRG/c_Test");

  //ECLTRG timing
  addDeltaPar("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_ECLTRG", HistDelta::c_Entries, 1000, 1); // update each 1000 entries
  registerEpicsPV(m_pvPrefix + "ECLTRG_timing_mean", "ECLTRG_timing_mean");

  //update PV
  updateEpicsPVs(
    5.0); // -> now trigger update. this may be optional, framework can take care unless we want to now the result immediately
}

void DQMHistAnalysisTRGModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: beginRun called.");
}

void DQMHistAnalysisTRGModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: endRun called.");
}

void DQMHistAnalysisTRGModule::event()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: event called.");
  doHistAnalysis();
}

void DQMHistAnalysisTRGModule::doHistAnalysis()
{
  m_canvas->Clear();
  m_canvas->cd(0);

  //update ECLTRG timing
  double ECLTRG_timing_mean = 0.0;
  auto hist =  getDelta("EventT0DQMdir", "m_histEventT0_TOP_hadron_L1_ECLTRG", 0, true);// only if updated
  if (hist) {
    hist->Draw();
    ECLTRG_timing_mean = hist->GetMean();
    B2DEBUG(1, "ECLTRG_timing_mean:" << ECLTRG_timing_mean);
    setEpicsPV("ECLTRG_timing_mean", ECLTRG_timing_mean);
    updateEpicsPVs(
      5.0); // -> now trigger update. this may be optional, framework can take care unless we want to now the result immediately
  }

  // Tag canvas as updated ONLY if things have changed.
  UpdateCanvas(m_canvas->GetName(), hist != nullptr);

  // this if left over from jsroot, may not be needed anymore (to check)
  m_canvas->Update();

}

void DQMHistAnalysisTRGModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisTRGModule: terminate called");
}

