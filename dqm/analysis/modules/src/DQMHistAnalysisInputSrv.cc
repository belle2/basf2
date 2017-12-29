//+
// File : DQMHistAnalysisInputSrv.cc
// Description :
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on wrok from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-


#include <dqm/analysis/modules/DQMHistAnalysisInputSrv.h>
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
  B2DEBUG(1, "DQMHistAnalysisInputSrv: Constructor done.");
}


DQMHistAnalysisInputSrvModule::~DQMHistAnalysisInputSrvModule() { }

void DQMHistAnalysisInputSrvModule::initialize()
{
  m_expno = m_runno = 0;
  m_count = 0;
  m_memory = new DqmMemFile(m_mempath.c_str());
  m_eventMetaDataPtr.registerInDataStore();
  //m_serv = new THttpServer("http:8081");
  //m_serv->SetReadOnly(kFALSE);
  B2INFO("DQMHistAnalysisInputSrv: initialized.");
}


void DQMHistAnalysisInputSrvModule::beginRun()
{
  B2INFO("DQMHistAnalysisInputSrv: beginRun called.");
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
    std::string vname = h[i]->GetName();
    setFloatValue(vname + ".entries", h[i]->GetEntries());
    if (h[i]->GetDimension() == 1) {
      setFloatValue(vname + ".rms", h[i]->GetRMS());
      setFloatValue(vname + ".rmserr", h[i]->GetRMSError());
      setFloatValue(vname + ".mean", h[i]->GetMean());
      setFloatValue(vname + ".meanerr", h[i]->GetMeanError());
    } else if (h[i]->GetDimension() == 2) {
      setFloatValue(vname + ".xrms", h[i]->GetRMS(1));
      setFloatValue(vname + ".xrmserr", h[i]->GetRMSError(1));
      setFloatValue(vname + ".xmean", h[i]->GetMean(1));
      setFloatValue(vname + ".xmeanerr", h[i]->GetMeanError(1));
      setFloatValue(vname + ".yrms", h[i]->GetRMS(2));
      setFloatValue(vname + ".yrmserr", h[i]->GetRMSError(2));
      setFloatValue(vname + ".ymean", h[i]->GetMean(2));
      setFloatValue(vname + ".ymeanerr", h[i]->GetMeanError(2));
    }
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
  B2INFO("DQMHistAnalysisInputSrv: endRun called");
}


void DQMHistAnalysisInputSrvModule::terminate()
{
  B2INFO("DQMHistAnalysisInputSrv: terminate called");
}

