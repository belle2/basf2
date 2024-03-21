/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInput2.cc
// Description :
//-


#include <dqm/analysis/modules/DQMHistAnalysisInput2.h>

#include <TROOT.h>
#include <TKey.h>

#include <ctime>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInput2);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInput2Module::DQMHistAnalysisInput2Module()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistMemoryPath", m_mempath, "Path to Input Hist memory", std::string(""));
  addParam("RefreshInterval", m_interval, "Refresh interval of histograms", 10);
  addParam("RemoveEmpty", m_remove_empty, "Remove empty histograms", false);
  addParam("EnableRunInfo", m_enable_run_info, "Enable Run Info", false);
  B2DEBUG(1, "DQMHistAnalysisInput: Constructor done.");
}


DQMHistAnalysisInput2Module::~DQMHistAnalysisInput2Module() { }

void DQMHistAnalysisInput2Module::initialize()
{
  if (m_enable_run_info) {
    m_c_info = new TCanvas("DQMInfo/c_info", "");
    m_c_info->SetTitle("");
  } else {
    m_c_info = NULL;
  }
  m_eventMetaDataPtr.registerInDataStore();
  B2INFO("DQMHistAnalysisInput: initialized.");
}


void DQMHistAnalysisInput2Module::beginRun()
{
  B2INFO("DQMHistAnalysisInput: beginRun called.");
  clearHistList();
  resetDeltaList();
  clearCanvases();
}

void DQMHistAnalysisInput2Module::event()
{
  TH1::AddDirectory(false);
  initHistListBeforeEvent();

  std::vector<TH1*> hs;
  char mbstr[100];

  time_t now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] before Close File");

  if (m_file) {
    m_file->Close();

    now = time(0);
    strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
    B2INFO("[" << mbstr << "] before delete File");
    delete m_file;
    m_file = nullptr;
  }

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] before Load File");

  m_file = new TFile(m_mempath.c_str(), "READ");
  if (m_file->IsZombie()) {
    B2WARNING("DQMHistAnalysisInput: " << m_mempath + " is Zombie");
    setReturnValue(false);
    return;
  }

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] after LoadFile");

  const TDatime& mt = m_file->GetModificationDate();
  TDatime mmt(mt.Convert());
  std::string expno("UNKNOWN"), runno("UNKNOWN"), rtype("UNKNOWN");

  m_file->cd();
  TIter next(m_file->GetListOfKeys());
  TKey* key = nullptr;

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] before input loop");

  // check if histograms were updated since last DQM event (based on number of processed events)
  if (m_nevent != getEventProcessed()) {
    m_lastChange = std::string(mbstr);
  }
  // update number of processed events
  m_nevent = getEventProcessed();

  while ((key = (TKey*)next())) {
    auto obj = key->ReadObj();
    if (obj == nullptr) continue; // would be strange, but better check
    if (!obj->IsA()->InheritsFrom("TH1")) continue; // other non supported (yet?)
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
    B2WARNING("DQMHistAnalysisInput: " << m_mempath + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Updated " +
              mmt.AsString());
    setReturnValue(false);
    return;
  } else {
    if (m_c_info != NULL) m_c_info->SetTitle((m_mempath + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Changed "
                                                + m_lastChange + ", Last Updated "
                                                + mmt.AsString() + ", Last DQM event " + std::string(mbstr)).c_str());
    m_expno = std::stoi(expno);
    m_runno = std::stoi(runno);
  }
  B2INFO("DQMHistAnalysisInput: " << m_mempath + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Updated " +
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

  // if there is no update, sleep a moment
  if (!anyupdate) sleep(m_interval);

  // if no histogram was updated, we could stop processing
  setReturnValue(anyupdate);
}

void DQMHistAnalysisInput2Module::endRun()
{
  B2INFO("DQMHistAnalysisInput : endRun called");
}

void DQMHistAnalysisInput2Module::terminate()
{
  B2INFO("terminate called");
}

