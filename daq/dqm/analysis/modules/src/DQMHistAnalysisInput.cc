//+
// File : DQMHistAnalysisInput.cc
// Description :
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 25 - Dec - 2015
//-


#include <daq/dqm/analysis/modules/DQMHistAnalysisInput.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputModule::DQMHistAnalysisInputModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistMemoryPath", m_mempath, "Path to Input Hist memory", string(""));
  addParam("HistMemorySize", m_memsize, "Size of Input Hist memory", 10000000);
  addParam("RefreshInterval", m_interval, "Refresh interval of histograms", 10);
  B2DEBUG(1, "DQMHistAnalysisInput: Constructor done.");
}


DQMHistAnalysisInputModule::~DQMHistAnalysisInputModule() { }

void DQMHistAnalysisInputModule::initialize()
{
  m_expno = m_runno = 0;
  m_count = 0;
  m_memory = new DqmMemFile(m_mempath.c_str());
  m_eventMetaDataPtr.registerInDataStore();
  B2INFO("DQMHistAnalysisInput: initialized.");
}


void DQMHistAnalysisInputModule::beginRun()
{
  //B2INFO("DQMHistAnalysisInput: beginRun called.");
}

void DQMHistAnalysisInputModule::event()
{
  sleep(m_interval);
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
    B2INFO("Found : " << h[i]->GetName() << " : " << h[i]->GetEntries());
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
}

void DQMHistAnalysisInputModule::endRun()
{
  B2INFO("DQMHistAnalysisInput : endRun called");
}


void DQMHistAnalysisInputModule::terminate()
{
  B2INFO("terminate called");
}

