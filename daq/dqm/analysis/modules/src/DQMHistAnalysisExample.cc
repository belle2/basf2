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
}


void DQMHistAnalysisExampleModule::beginRun()
{
  //B2INFO("DQMHistAnalysisExample: beginRun called.");
}

void DQMHistAnalysisExampleModule::event()
{
  TH1* h = findHist("FirstDet/h_HitXPositionCh01");
  if (h != NULL) {
    setIntValue("firstdet.myresult", 1);
  } else {
    setIntValue("firstdet.myresult", 0);
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

