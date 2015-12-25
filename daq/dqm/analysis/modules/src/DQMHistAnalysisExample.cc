//+
// File : DQMHistAnalysisExample.cc
// Description :
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 25 - Dec - 2015
//-


#include <daq/dqm/analysis/modules/DQMHistAnalysisExample.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisExample)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisExampleModule::DQMHistAnalysisExampleModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(1, "DQMHistAnalysisExample: Constructor done.");
}


DQMHistAnalysisExampleModule::~DQMHistAnalysisExampleModule() { }

void DQMHistAnalysisExampleModule::initialize()
{
  B2INFO("DQMHistAnalysisExample: initialized.");
  m_f = new TF1("f1", "gaus(0)", -20, 20);
}


void DQMHistAnalysisExampleModule::beginRun()
{
  //B2INFO("DQMHistAnalysisExample: beginRun called.");
}

void DQMHistAnalysisExampleModule::event()
{
  TH1* h = findHist("FirstDet/h_HitXPositionCh01");
  if (h != NULL) {
    h->Fit("f1");
    setFloatValue("firstdet.fit.mean", m_f->GetParameter(1));
    setFloatValue("firstdet.fit.sigma", m_f->GetParameter(2));
  } else {
    setFloatValue("firstdet.fit.mean", 0);
    setFloatValue("firstdet.fit.sigma", 0);
  }
}

void DQMHistAnalysisExampleModule::endRun()
{
  B2INFO("DQMHistAnalysisExample : endRun called");
}


void DQMHistAnalysisExampleModule::terminate()
{
  B2INFO("terminate called")
}

