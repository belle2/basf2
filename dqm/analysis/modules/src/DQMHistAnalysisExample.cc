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
  setDescription("Example DQMHistAnalysisModule! with base features");

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("test"));
  addParam("histogramName", m_histogramName, "Name of Histogram", std::string("testHist"));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:TEST"));
  B2DEBUG(20, "DQMHistAnalysisExample: Constructor done.");
}


DQMHistAnalysisExampleModule::~DQMHistAnalysisExampleModule()
{
  // if this function is not needed, please remove
}

void DQMHistAnalysisExampleModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisExample: initialized.");
  TString a = m_histogramName;
  a.ReplaceAll("/", "_");
  m_canvas = new TCanvas("c_" + a);
  m_function = new TF1("f_" + a, TString("gaus"), -100, 100);
}

void DQMHistAnalysisExampleModule::beginRun()
{
  // if this function is not needed, please remove
  B2DEBUG(20, "DQMHistAnalysisExample : beginRun called");
}

void DQMHistAnalysisExampleModule::event()
{
  TH1* h = findHist(m_histogramName);
  if (h != NULL) {
    m_canvas->Clear();
    m_canvas->cd();
    h->Fit(m_function, "R");
    h->Draw();
    m_canvas->Modified();
    B2DEBUG(20, "mean " << m_function->GetParameter(1));
    B2DEBUG(20, "sigma" << m_function->GetParameter(2));
  } else {
    B2DEBUG(20, "Histo " << m_histogramName << " not found");
  }
}

void DQMHistAnalysisExampleModule::endRun()
{
  // if this function is not needed, please remove
  B2DEBUG(20, "DQMHistAnalysisExample : endRun called");
}


void DQMHistAnalysisExampleModule::terminate()
{
  // if this function is not needed, please remove
  B2DEBUG(20, "terminate called");
}

