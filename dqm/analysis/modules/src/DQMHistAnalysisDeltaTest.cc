/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisDeltaTest.cc
// Description : Analysis of PXD Reduction
//-


#include <dqm/analysis/modules/DQMHistAnalysisDeltaTest.h>
#include <TROOT.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisDeltaTest);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisDeltaTestModule::DQMHistAnalysisDeltaTestModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("test"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:TEST"));
  addParam("useEpics", m_useEpics, "Whether to update EPICS PVs.", false);
  B2DEBUG(1, "DQMHistAnalysisDeltaTest: Constructor done.");

}

DQMHistAnalysisDeltaTestModule::~DQMHistAnalysisDeltaTestModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisDeltaTestModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaTest: initialized.");

  m_monObj = getMonitoringObject("test");

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cTest = new TCanvas((m_histogramDirectoryName + "/c_Test").data());

  m_monObj->addCanvas(m_cTest);

#ifdef _BELLE2_EPICS
  mychid.resize(2);
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    SEVCHK(ca_create_channel((m_pvPrefix + "TEST1").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "TEST2").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");

    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}

void DQMHistAnalysisDeltaTestModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaTest: beginRun called.");
}

void DQMHistAnalysisDeltaTestModule::event()
{
  double data_Test1 = 0.0;
  double data_Test2 = 0.0;

  m_cTest->Clear();
  m_cTest->Divide(2, 2);

  // more handy to have it as a full name, but find/get functions
  // can work with one or two parameters
  std::string name = m_histogramDirectoryName + "/testHisto";

  // get basic histogram (run integrated up)
  m_cTest->cd(1);
  auto hh1 = findHist(m_histogramDirectoryName, "testHisto");
  if (hh1) {
    hh1->Draw("hist");
    data_Test2 = hh1->GetMean();
  }

  // get most recent delta
  m_cTest->cd(2);
  auto hd1 = getDelta(m_histogramDirectoryName, "testHisto");
  if (hd1) {
    hd1->Draw("hist");
    data_Test1 = hd1->GetMean();
  } else {
    // Depending on you analysis, you want to see/plot
    // the histogram even before the first condition for update is met
    // thus, no delta histogram is available
    if (hh1) {
      auto a = (TH1*)hh1->DrawClone("hist");
      a->SetLineColor(2);
    }
    // but, as the statistics is low, we dont want to have it updated yet (e.g. in EPICS)
    // data_Test1=hd1->GetMean(); //< thus, do not uncomment
  }

  // plot all delta histograms, thus make recent shifts more visible
  // it would be nicer to plot oldest first, left as exercise for reader
  m_cTest->cd(3);
  for (int i = 0; i < 99; i++) {
    auto h = getDelta(name, i);
    if (h == nullptr) break;
    if (i == 0) {
      h->Draw("hist");
    } else {
      h->SetLineColor(kBlue + 4 - i * 3); // a bit of shading
      h->Draw("same");
    }
  }

  // I see no reason to access that histogram, but just as low-level example.
  m_cTest->cd(4);
  auto it = getDeltaList().find(name);
  if (it != getDeltaList().end()) {
    auto h = it->second->m_lastHist;
    if (h) h->Draw("hist");
  }

  m_cTest->cd(0);
  m_cTest->Update();

  TString fn;
  static int plot_count = 0;
  fn.Form("ana_testHist_Delta_%d.png", plot_count++);
  m_cTest->Print(fn);

  // actually, we would prefer to only update the epics variable
  // if the main histogram or delta~ has changed. but there is until now no easy
  // way to check that
  if (true) {
    // currently, monObj are only used at end of run, thus this is not necessary
    // but, in the future we may want to use finer granularity.
    m_monObj->setVariable("data_Test1", data_Test1);
    m_monObj->setVariable("data_Test2", data_Test2);

#ifdef _BELLE2_EPICS
    if (m_useEpics) {
      SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&data_Test1), "ca_set failure");
      SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&data_Test2), "ca_set failure");
      // write out
      SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
    }
#endif
  }
}

void DQMHistAnalysisDeltaTestModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisDeltaTest: terminate called");
}

