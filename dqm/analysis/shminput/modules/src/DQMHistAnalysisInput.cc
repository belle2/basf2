/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInput.cc
// Description :
//-


#include <dqm/analysis/shminput/DQMHistAnalysisInput.h>

#include <TROOT.h>
#include <TKey.h>

#include <ctime>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInput);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputModule::DQMHistAnalysisInputModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistMemoryPath", m_mempath, "Path to Input Hist memory", std::string(""));
  addParam("HistMemorySize", m_memsize, "Size of Input Hist memory", 10000000);
  addParam("HistMemoryName", m_memname, "Name of Input Hist memory", std::string(""));
  addParam("ShmId", m_shm_id, "ID of shared memory", 0);
  addParam("SemId", m_sem_id, "ID of semaphore", 0);
  addParam("RefreshInterval", m_interval, "Refresh interval of histograms", 10);
  addParam("RemoveEmpty", m_remove_empty, "Remove empty histograms", false);
  addParam("EnableRunInfo", m_enable_run_info, "Enable Run Info", false);
  B2DEBUG(1, "DQMHistAnalysisInput: Constructor done.");
}


DQMHistAnalysisInputModule::~DQMHistAnalysisInputModule() { }

void DQMHistAnalysisInputModule::initialize()
{
  if (m_memory != nullptr) delete m_memory;
  if (m_mempath != "")
    m_memory = new DqmMemFile(m_mempath.c_str());
  else
    m_memory = new DqmMemFile(m_shm_id, m_sem_id);
  if (m_enable_run_info) {
    m_c_info = new TCanvas("DQMInfo/c_info", "");
    m_c_info->SetTitle("");
  } else {
    m_c_info = NULL;
  }
  m_eventMetaDataPtr.registerInDataStore();
  B2INFO("DQMHistAnalysisInput: initialized.");
}


void DQMHistAnalysisInputModule::beginRun()
{
  B2INFO("DQMHistAnalysisInput: beginRun called.");
  clearHistList();
}

void DQMHistAnalysisInputModule::event()
{
  initHistListBeforeEvent();

  sleep(m_interval);
  std::vector<TH1*> hs;
  char mbstr[100];

  time_t now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] before LoadMemFile");

  TMemFile* file = m_memory->LoadMemFile();

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] after LoadMemFile");

  const TDatime& mt = file->GetModificationDate();
  TDatime mmt(mt.Convert());
  std::string expno("UNKNOWN"), runno("UNKNOWN"), rtype("UNKNOWN");

  file->cd();
  TIter next(file->GetListOfKeys());
  TKey* key = nullptr;

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] before input loop");

  while ((key = (TKey*)next())) {
    auto obj = key->ReadObj();
    if (obj == nullptr) continue; // would be strange, but better check
    if (!key->IsA()->InheritsFrom("TH1")) continue; //
    TH1* h = (TH1*)obj; // we are sure its a TH1

    if (m_remove_empty && h->GetEntries() == 0) continue;
    // Remove ":" from folder name, workaround!
    TString a = h->GetName();
    a.ReplaceAll(":", "");
    h->SetName(a);
    B2DEBUG(1, "DQMHistAnalysisInput: get histo " << a.Data());

    // the following line prevent any histogram outside a directory to be processed
    if (StringSplit(a.Data(), '/').size() <= 1) continue;

    hs.push_back(h);

    // the following workaround need to be improved
    if (std::string(h->GetName()) == std::string("DQMInfo/expno")) expno = h->GetTitle();
    if (std::string(h->GetName()) == std::string("DQMInfo/runno")) runno = h->GetTitle();
    if (std::string(h->GetName()) == std::string("DQMInfo/rtype")) rtype = h->GetTitle();
  }

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] after input loop");

  if (expno == std::string("UNKNOWN") || runno == std::string("UNKNOWN")) {
    if (m_c_info != NULL) m_c_info->SetTitle((m_memname + ": Last Updated " + mmt.AsString()).c_str());
  } else {
    if (m_c_info != NULL) m_c_info->SetTitle((m_memname + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Updated "
                                                + mmt.AsString()).c_str());
    m_expno = std::stoi(expno);
    m_runno = std::stoi(runno);
  }
  B2INFO("DQMHistAnalysisInput: " << m_memname + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Updated " +
         mmt.AsString());


  m_count++;
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);
  m_eventMetaDataPtr->setTime(mt.Convert());

  //setExpNr(m_expno); // redundant access from MetaData
  //setRunNr(m_runno); // redundant access from MetaData
  // ExtractRunType();// Run Type is processed above already, just take it
  setRunType(rtype);
  ExtractEvent(hs);

  // this code must be run after "event processed" has been extracted
  bool anyupdate = false; // flag if any histogram updated at all
  for (auto& h : hs) {
    anyupdate |= addHist("", h->GetName(), h);
    B2DEBUG(1, "Found : " << h->GetName() << " : " << h->GetEntries());
  }

  // if no histogram was updated, we could stop processing
  setReturnValue(anyupdate);
}

void DQMHistAnalysisInputModule::endRun()
{
  B2INFO("DQMHistAnalysisInput : endRun called");

  TIter nextckey(gROOT->GetListOfCanvases());
  TObject* cobj = NULL;

  while ((cobj = dynamic_cast<TObject*>(nextckey()))) {
    if (cobj->IsA()->InheritsFrom("TCanvas")) {
      (dynamic_cast<TCanvas*>(cobj))->Clear();
    }
  }
}

void DQMHistAnalysisInputModule::terminate()
{
  B2INFO("terminate called");
}

