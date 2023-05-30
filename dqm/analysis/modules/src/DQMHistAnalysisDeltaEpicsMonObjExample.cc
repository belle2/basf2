/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisDeltaEpicsMonObjExample.cc
// Description : Test Module for Delta Histogram Access
//-


#include <dqm/analysis/modules/DQMHistAnalysisDeltaEpicsMonObjExample.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisDeltaEpicsMonObjExample);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisDeltaEpicsMonObjExampleModule::DQMHistAnalysisDeltaEpicsMonObjExampleModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("test"));
  addParam("histogramName", m_histogramName, "Name of Histogram", std::string("testHist"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:TEST:"));
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: Constructor done.");

}

DQMHistAnalysisDeltaEpicsMonObjExampleModule::~DQMHistAnalysisDeltaEpicsMonObjExampleModule()
{
  // destructor not needed
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: initialized.");

  m_monObj = getMonitoringObject("test");

  gROOT->cd(); // this seems to be important, or strange things happen

  m_canvas = new TCanvas((m_histogramDirectoryName + "/c_Test").data());

  m_monObj->addCanvas(m_canvas);

  registerEpicsPV(m_pvPrefix + "mean", "mean");
  registerEpicsPV(m_pvPrefix + "width", "width");
  updateEpicsPVs(5.0);
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: beginRun called.");
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: endRun called.");
  doHistAnalysis(true);
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::event()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: event called.");
  doHistAnalysis(false);
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::doHistAnalysis(bool forMiraBelle)
{
  double data_mean = 0.0;
  double data_width = 0.0;

  m_canvas->Clear();

  // more handy to have it as a full name, but find/get functions
  // can work with one or two parameters
  // std::string fullname = m_histogramDirectoryName + "/" + m_histogramName;

  // In the following, enable one

  // get most recent delta, but only if updated since last call
  auto hist =  getDelta(m_histogramDirectoryName, m_histogramName, 0, true);// only if updated
  // get basic histogram (run integrated up) but only if updated since last call
  // auto hist = findHist(m_histogramDirectoryName, m_histogramName, true);// only if updated

  // the following cases do not make sense, unless you want to archieve something special.
  // get most recent delta even if not updated
  // auto hist =  getDelta(m_histogramDirectoryName, m_histogramName, 0, false);// even if no update
  // get basic histogram (run integrated up) even if not updated
  // auto hist = findHist(m_histogramDirectoryName, m_histogramName, false);// even if no update

  if (hist) {
    data_mean = hist->GetMean();
    data_width = hist->GetRMS();
  }

  // Tag canvas as updated ONLY if things have changed.
  UpdateCanvas(m_canvas->GetName(), hist != nullptr);

  // this if left over from jsroot, may not be needed anymore (to check)
  m_canvas->Update();

  // actually, we would prefer to only update the epics variable
  // if the main histogram or delta~ has changed.

  if (hist) {
    if (forMiraBelle) {
      // currently, monObj are only used at end of run, thus this is not necessary
      // but, if in the future we want to use finer granularity, this code needs change.
      m_monObj->setVariable("mean", data_mean);
      m_monObj->setVariable("width", data_width);
    } else {
      // we do not want to fill epics again at end of run, as this would be identical to the last event called before.
      setEpicsPV("mean", data_mean);
      setEpicsPV("width", data_width);
      // write out
      updateEpicsPVs(5.0);
    }
  }
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: terminate called");
  // MiraBelle export code should run at end of Run
  // but it still "remembers" the state from last event call.
}

