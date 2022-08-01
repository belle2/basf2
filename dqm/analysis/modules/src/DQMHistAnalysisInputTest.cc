/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInputTest.cc
// Description : Module for offline testing of histogram analysis code.
//-


#include <dqm/analysis/modules/DQMHistAnalysisInputTest.h>

#include <TROOT.h>

//#include <iostream>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInputTest);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputTestModule::DQMHistAnalysisInputTestModule()
  : DQMHistAnalysisModule()
{
  addParam("Experiment", m_expno, "Experiment Nr", 26u);
  addParam("Run", m_runno, "Run Number List", 1u);
  addParam("Events", m_events, "Number of events for each run", 100u);
  B2DEBUG(1, "DQMHistAnalysisInputTest: Constructor done.");
}

void DQMHistAnalysisInputTestModule::initialize()
{
  m_eventMetaDataPtr.registerInDataStore();
  B2INFO("DQMHistAnalysisInputTest: initialized.");

  m_func = new TF1("mygaus", "gaus(0)", 0, 1000);
  m_func->SetParameters(100, 100, 30); // scale, mean, sigma

  m_testHisto = (TH1*) new TH1F("testHisto", "test Histo", 1000, 0, 1000);

  addDeltaPar("test", "testHisto", 1, 1600, 5);
}

void DQMHistAnalysisInputTestModule::beginRun()
{
  B2INFO("DQMHistAnalysisInputTest: beginRun called. Run: " << m_runno);

  m_count = 0;
  m_testHisto->Reset();

}

void DQMHistAnalysisInputTestModule::event()
{
  // attention, the first call to event is BEFORE the first call to begin run!

  B2INFO("DQMHistAnalysisInputTest: event called.");

  if (m_count > m_events) {
    m_eventMetaDataPtr.create();
    m_eventMetaDataPtr->setEndOfData();
    return;
  }

  // change mean from 100 -> 900 over the full "run"
  m_func->SetParameter(1, 100 + m_count * 800. / m_events);
  m_testHisto->FillRandom("mygaus", 500);

  resetHist();

  addHist("test", m_testHisto->GetName(), m_testHisto);

  /*{
    auto c=new TCanvas();
    c->cd();
    m_testHisto->Draw("hist");
    TString fn;
    fn.Form("testHist_%d.png",m_count);
    c->Print(fn);
  }*/

  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);
  m_eventMetaDataPtr->setTime(m_count * 1e9);
  B2INFO("DQMHistAnalysisInputTest: event finished. count: " << m_count);

  PlotDelta();
  m_count++;
}

void DQMHistAnalysisInputTestModule::PlotDelta(void)
{
  B2INFO("Delta");
  for (auto a : getDeltaList()) {
    B2INFO(a.first << " " << a.second->m_type << " " << a.second->m_parameter
           << " " << a.second->m_amountDeltas << " " << a.second->m_deltaHists.size());
  }
  std::string name = "test/testHisto";
  {
    auto c = new TCanvas();
    c->Divide(3, 3);
    c->cd(1);
    auto h = getHist(name);
    if (h) h->Draw("hist");

    c->cd(2);
    auto it = getDeltaList().find(name);
    if (it != getDeltaList().end()) {
      h = it->second->m_lastHist;
      if (h) h->Draw("hist");

      for (unsigned int i = 0; i < it->second->m_amountDeltas; i++) {
        c->cd(i + 4);
        h = getDelta(name, i);
        if (h) {
          h->Draw("hist");
          c->cd(3);
          if (i == 0) {
            h->Draw();
          } else {
            h->SetLineColor(i + 2);
            h->Draw("same");
          }
        }
        if (i + 4 == 9) break;
      }
    }

    c->cd(0);

    TString fn;
    fn.Form("testHist_Delta_%d.png", m_count);
    c->Print(fn);
  }
}

void DQMHistAnalysisInputTestModule::endRun()
{
  B2INFO("Histo");
  for (auto a : getHistList()) {
    B2INFO(a.first);
  }
  PlotDelta();

  B2INFO("DQMHistAnalysisInputTest : endRun called");
}


void DQMHistAnalysisInputTestModule::terminate()
{
  B2INFO("terminate called");
}
