//+
// File : DQMHistAnalysisExample.cc
// Description : Sequential ROOT output module for pbasf2
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 13 - Aug - 2010
//         6 - Sep - 2012,  Use of DataStoreStreamer, clean up
//         9 - Dec - 2013, Modification for DAQ use
//-


#include <daq/slc/apps/dqmviewd/modules/DQMHistAnalysisExample.h>
#include <daq/slc/apps/dqmviewd/modules/DQMHistAnalysis.h>

#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisExample)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisExampleModule::DQMHistAnalysisExampleModule() : DQMHistAnalysisModule()
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

