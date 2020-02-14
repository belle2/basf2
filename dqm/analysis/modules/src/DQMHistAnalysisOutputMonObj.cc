/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <dqm/analysis/modules/DQMHistAnalysisOutputMonObj.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include "TFile.h"
#include "TString.h"
#include <time.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisOutputMonObj)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisOutputMonObjModule::DQMHistAnalysisOutputMonObjModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoFile", m_filename, "Output Histogram Filename", std::string(""));
  addParam("ProcID", m_procID, "Processing id (online,proc10, etc.)", std::string("online"));
  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: Constructor done.");
}


DQMHistAnalysisOutputMonObjModule::~DQMHistAnalysisOutputMonObjModule() { }

void DQMHistAnalysisOutputMonObjModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: initialized.");

  // create file metadata
  m_metaData = new DQMFileMetaData();
  m_metaData->setProcessingID(m_procID);
}


void DQMHistAnalysisOutputMonObjModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: beginRun called.");
}


void DQMHistAnalysisOutputMonObjModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: event called.");
}

void DQMHistAnalysisOutputMonObjModule::endRun()
{
  B2INFO("DQMHistAnalysisOutputMonObj: endRun called");

  StoreObjPtr<EventMetaData> lastEvtMeta;

  B2INFO("open file");
  int run = lastEvtMeta->getRun();
  int exp = lastEvtMeta->getExperiment();
  TString fname;
  if (m_filename.length()) fname = m_filename;
  else fname = TString::Format("mon_e%04dr%06d.root", exp, run);
  TFile f(fname, "recreate");

  // set meta data info
  m_metaData->setNEvents(lastEvtMeta->getEvent());
  m_metaData->setExperimentRun(exp, run);
  time_t ts = lastEvtMeta->getTime() / 1e9;
  struct tm* timeinfo;
  timeinfo = localtime(&ts);
  m_metaData->setRunDate(asctime(timeinfo));
  m_metaData->Write();
  // get list of existing monitoring objects
  const MonObjList& objts =  getMonObjList();
  // write them to the output file
  for (const auto& obj : objts)(obj.second)->Write();

  f.Write();
  f.Close();

}


void DQMHistAnalysisOutputMonObjModule::terminate()
{
  B2INFO("DQMHistAnalysisOutputMonObj: terminate called");
// Attention, we can not do that in Terminate, as then the memFile is already closed by previous task!
}

