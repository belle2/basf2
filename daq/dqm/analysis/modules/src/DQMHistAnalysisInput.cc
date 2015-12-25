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
  B2DEBUG(1, "DQMHistAnalysisInput: Constructor done.");
}


DQMHistAnalysisInputModule::~DQMHistAnalysisInputModule() { }

void DQMHistAnalysisInputModule::initialize()
{
  m_expno = m_runno = 0;
  m_count = 0;
  try {
    m_memory.open(m_mempath.c_str(), m_memsize);
  } catch (const IOException& e) {
    B2FATAL("Failed to open HistMemory : " << m_mempath.c_str()
            << ":" << m_memsize);
  }
  m_eventMetaDataPtr.registerPersistent();
  B2INFO("DQMHistAnalysisInput: initialized.");
}


void DQMHistAnalysisInputModule::beginRun()
{
  //B2INFO("DQMHistAnalysisInput: beginRun called.");
}

void DQMHistAnalysisInputModule::event()
{
  while (true) {
    std::vector<TH1*>& h(m_memory.deserialize());
    if (h.size() > 0) {
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
      return;
    }
    sleep(1);
  }
}

void DQMHistAnalysisInputModule::endRun()
{
  B2INFO("DQMHistAnalysisInput : endRun called");
}


void DQMHistAnalysisInputModule::terminate()
{
  B2INFO("terminate called")
}

