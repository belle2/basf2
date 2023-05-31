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

  // Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("test"));
  addParam("histogramName", m_histogramName, "Name of Histogram", std::string("testHist"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:TEST:"));
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: Constructor done.");

}

DQMHistAnalysisDeltaEpicsMonObjExampleModule::~DQMHistAnalysisDeltaEpicsMonObjExampleModule()
{
  // destructor not needed
  // EPICS singleton deletion not urgent -> can be done by framework
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: initialized.");

  gROOT->cd(); // this seems to be important before creating new Canvas, or strange things happen
  m_canvas = new TCanvas((m_histogramDirectoryName + "/c_Test").data());

  m_monObj = getMonitoringObject("test");
  m_monObj->addCanvas(m_canvas);

  // delta parameters, two examples:
  addDeltaPar(m_histogramDirectoryName, m_histogramName, HistDelta::c_Events,
              100000, 1); // update each 100k events (from daq histogram)
  // addDeltaPar(m_histogramDirectoryName, m_histogramName, HistDelta::c_Entries, 10000, 1); // update each 10000 entries

  registerEpicsPV(m_pvPrefix + "mean", "mean");
  registerEpicsPV(m_pvPrefix + "width", "width");
  registerEpicsPV(m_pvPrefix + "alarm", "alarm");
  updateEpicsPVs(
    5.0); // -> now trigger update. this may be optional, framework can take care unless we want to now the result immediately
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: beginRun called.");
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: endRun called.");
  // MiraBelle export code should run at end of Run
  // but it still "remembers" the state from last event call.
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
  int data_alarm = 0;

  m_canvas->Clear();
  m_canvas->cd(0);

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

    // whatever condition
    data_alarm = 2; // good, green
    if (data_mean > 0.5 || data_mean < -0.5 || data_width > 0.5) data_alarm = 3; // warning, yellow
    if (data_mean > 0.7 || data_mean < -0.7 || data_width > 0.7) data_alarm = 4; // error, red
    // maybe other condition, e.g. too low to judge -> 0

    hist->Draw("hist");

    // the following will be replaced by a base class function
    auto color = kWhite;
    switch (data_alarm) {
      case 2:
        color = kGreen;
        break;
      case 3:
        color = kYellow;
        break;
      case 4:
        color = kRed;
        break;
      default:
        color = kGray;
        break;
    }
    m_canvas->Pad()->SetFillColor(color);
  }

  // Tag canvas as updated ONLY if things have changed.
  UpdateCanvas(m_canvas->GetName(), hist != nullptr);
  // Remark: if you do not tag, you may have an empty Canvas (as it is cleared above)

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
      setEpicsPV("alarm", data_alarm);
      // until now, chnages are buffered only locally, not written out to network for performance reason
      updateEpicsPVs(
        5.0); // -> now trigger update. this may be optional, framework can take care unless we want to now the result immediately
    }
  }
}

void DQMHistAnalysisDeltaEpicsMonObjExampleModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaEpicsMonObjExample: terminate called");
}

