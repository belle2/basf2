/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisInputSrv.h>

#include <TKey.h>
#include <TSystem.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInputSrv)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputSrvModule::DQMHistAnalysisInputSrvModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistMemoryPath", m_mempath, "Path to Input Hist memory", string(""));
  addParam("HistMemorySize", m_memsize, "Size of Input Hist memory", 10000000);
  addParam("RefreshInterval", m_interval, "Refresh interval of histograms in ms", 2000);
  B2DEBUG(20, "DQMHistAnalysisInputSrv: Constructor done.");
}


DQMHistAnalysisInputSrvModule::~DQMHistAnalysisInputSrvModule() { }

void DQMHistAnalysisInputSrvModule::initialize()
{
  if (m_memory != nullptr) delete m_memory;
  m_memory = new DqmMemFile(m_mempath.c_str());
  m_eventMetaDataPtr.registerInDataStore();
  //m_serv = new THttpServer("http:8081");
  //m_serv->SetReadOnly(kFALSE);
  B2DEBUG(20, "DQMHistAnalysisInputSrv: initialized.");
}


void DQMHistAnalysisInputSrvModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisInputSrv: beginRun called.");
}

void DQMHistAnalysisInputSrvModule::event()
{
  std::vector<TH1*> h;
  TMemFile* file = m_memory->LoadMemFile();
  file->cd();
  TIter next(file->GetListOfKeys());
  TKey* key = NULL;
  while ((key = (TKey*)next())) {
    h.push_back((TH1*)key->ReadObj());
  }
  resetHist();
  for (size_t i = 0; i < h.size(); i++) {
    addHist("", h[i]->GetName(), h[i]);
    B2DEBUG(2, "Found : " << h[i]->GetName() << " : " << h[i]->GetEntries());
  }
  m_count++;
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);

  TTimer t(m_interval, kFALSE);// in ms

  do { // call at least once!
    //m_serv->ProcessRequests();
    //gSystem->Sleep(10);  // 10 ms sleep
  } while (!t.CheckTimer(gSystem->Now()));

}

void DQMHistAnalysisInputSrvModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisInputSrv: endRun called");
}


void DQMHistAnalysisInputSrvModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisInputSrv: terminate called");
}

