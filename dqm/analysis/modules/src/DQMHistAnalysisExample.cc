/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisExample.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisExample);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisExampleModule::DQMHistAnalysisExampleModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoName", m_histoname, "Name of Histogram (incl dir)", std::string(""));
  addParam("Function", m_function, "Fit function definition", std::string("gaus"));
  B2DEBUG(20, "DQMHistAnalysisExample: Constructor done.");
}


DQMHistAnalysisExampleModule::~DQMHistAnalysisExampleModule() { }

void DQMHistAnalysisExampleModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisExample: initialized.");
  TString a = m_histoname;
  a.ReplaceAll("/", "_");
  m_c = new TCanvas("c_" + a);
  m_f = new TF1("f_" + a, TString(m_function), -100, 100);
}


void DQMHistAnalysisExampleModule::beginRun()
{
  //B2DEBUG(20, "DQMHistAnalysisExample: beginRun called.");
}

void DQMHistAnalysisExampleModule::event()
{
  TH1* h = findHist(m_histoname);
  if (h != NULL) {
    m_c->Clear();
    m_c->cd();
    h->Fit(m_f, "R");
    h->Draw();
    m_c->Modified();
    B2DEBUG(20, "mean " << m_f->GetParameter(1));
    B2DEBUG(20, "sigma" << m_f->GetParameter(2));
  } else {
    B2DEBUG(20, "Histo " << m_histoname << " not found");
  }
}

void DQMHistAnalysisExampleModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisExample : endRun called");
}


void DQMHistAnalysisExampleModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

