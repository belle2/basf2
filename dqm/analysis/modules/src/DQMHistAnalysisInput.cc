//+
// File : DQMHistAnalysisInput.cc
// Description :
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 25 - Dec - 2015
//-


#include <dqm/analysis/modules/DQMHistAnalysisInput.h>

using namespace Belle2;
typedef std::vector<std::string> StringList;

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
  addParam("HistMemoryPath", m_mempath, "Path to Input Hist memory", std::string(""));
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
  std::vector<TH1*> hs;
  TMemFile* file = m_memory->LoadMemFile();
  file->cd();
  TIter next(file->GetListOfKeys());
  TKey* key = NULL;
  while ((key = (TKey*)next())) {
    TH1* h = (TH1*)key->ReadObj();
    hs.push_back(h);
    TString a = h->GetName();

    StringList s;
    const std::string& str = a.Data();
    const char type = '/';
    size_t max = 0;
    size_t current = 0, found;
    while ((found = str.find_first_of(type, current)) != std::string::npos) {
      s.push_back(std::string(str, current, found - current));
      current = found + 1;
    }
    if (str.size() - current > 0) {
      s.push_back(std::string(str, current, str.size() - current));
    }
    while (max > 0 && s.size() < max) {
      s.push_back("");
    }

    a.ReplaceAll("/", "_");
    std::string name = a.Data();
    if (m_cs.find(name) == m_cs.end()) {
      if (s.size() > 1) {
        std::string dirname = s[0];
        std::string hname = s[1];
        TCanvas* c = new TCanvas((dirname + "/c_" + hname).c_str(), ("c_" + hname).c_str());
        m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
      } else {
        std::string hname = a.Data();
        TCanvas* c = new TCanvas(("c_" + hname).c_str(), ("c_" + hname).c_str());
        m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
      }
    }
    TCanvas* c = m_cs[name];
    c->cd();
    if (h->GetDimension() == 1) {
      h->Draw("hist");
    } else if (h->GetDimension() == 2) {
      h->Draw("colz");
    }
    c->Update();
  }
  resetHist();
  for (size_t i = 0; i < hs.size(); i++) {
    TH1* h = hs[i];
    addHist("", h->GetName(), h);
    B2INFO("Found : " << h->GetName() << " : " << h->GetEntries());
    std::string vname = h->GetName();
    setFloatValue(vname + ".entries", h->GetEntries());
    if (h->GetDimension() == 1) {
      setFloatValue(vname + ".rms", h->GetRMS());
      setFloatValue(vname + ".rmserr", h->GetRMSError());
      setFloatValue(vname + ".mean", h->GetMean());
      setFloatValue(vname + ".meanerr", h->GetMeanError());
    } else if (h->GetDimension() == 2) {
      setFloatValue(vname + ".xrms", h->GetRMS(1));
      setFloatValue(vname + ".xrmserr", h->GetRMSError(1));
      setFloatValue(vname + ".xmean", h->GetMean(1));
      setFloatValue(vname + ".xmeanerr", h->GetMeanError(1));
      setFloatValue(vname + ".yrms", h->GetRMS(2));
      setFloatValue(vname + ".yrmserr", h->GetRMSError(2));
      setFloatValue(vname + ".ymean", h->GetMean(2));
      setFloatValue(vname + ".ymeanerr", h->GetMeanError(2));
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

