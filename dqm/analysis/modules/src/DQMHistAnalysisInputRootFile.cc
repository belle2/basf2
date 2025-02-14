/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInputRootFile.cc
// Description : Module for offline testing of histogram analysis code.
//               Root file containing DQM histograms can be used as input.
//-


#include <dqm/analysis/modules/DQMHistAnalysisInputRootFile.h>

#include <TROOT.h>
#include <TKey.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInputRootFile);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputRootFileModule::DQMHistAnalysisInputRootFileModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("FileList", m_fileList, "List of input files", std::vector<std::string> {"input_histo.root"});
  addParam("SelectHistograms", m_histograms, "List of histogram name patterns, empty for all. Support wildcard matching (* and ?).",
           std::vector<std::string>());
  addParam("EventsList", m_eventsList, "Number of events for each run", std::vector<int>());
  addParam("Experiment", m_expno, "Experiment Nr override", 0);
  addParam("RunList", m_runList, "Run Number List", std::vector<int>());
  addParam("RunType", m_runType, "Run Type override", std::string(""));
  addParam("FillNEvent", m_fillNEvent, "NEvent override", 0);
  addParam("EventInterval", m_interval, "Time between events (seconds)", 20u);
  addParam("EnableRunInfo", m_enable_run_info, "Enable Run Info", false);
  B2DEBUG(1, "DQMHistAnalysisInputRootFile: Constructor done.");
}

void DQMHistAnalysisInputRootFileModule::initialize()
{
  if (m_fileList.size() == 0) B2FATAL("File list is empty.");
  if (m_eventsList.size() == 0) {
    m_eventsList.resize(m_fileList.size(), 1); // default one events per file
  }
  if (m_fileList.size() != m_eventsList.size()) B2ERROR("File list does not have the same size as events list.");
  if (m_runList.size() != 0 and m_runList.size() != m_fileList.size()) B2ERROR("Run list does not have the same size as file list.");
  m_run_idx = 0;
  m_file = new TFile(m_fileList[m_run_idx].c_str());
  m_eventMetaDataPtr.registerInDataStore();

  if (m_enable_run_info) {
    m_c_info = new TCanvas("DQMInfo/c_info", "");
    m_c_info->SetTitle("");
  }
  if (m_expno > 0)  m_h_expno = new TH1F("DQMInfo/expno", "", 1, 0, 1);
  if (m_runList.size() != 0)  m_h_runno = new TH1F("DQMInfo/runno", "", 1, 0, 1);
  if (m_runType != "")  m_h_rtype = new TH1F("DQMInfo/rtype", m_runType.c_str(), 1, 0, 1);
  if (m_fillNEvent > 0) {
    m_h_fillNEvent = new TH1F("DAQ/Nevent", "", 1, 0, 1);
    m_h_fillNEvent->Fill(0., m_fillNEvent);
    m_h_fillNEvent->SetEntries(m_fillNEvent);
  }
  B2INFO("DQMHistAnalysisInputRootFile: initialized.");
}

bool DQMHistAnalysisInputRootFileModule::hnamePatternMatch(std::string pattern, std::string text)
{
  boost::replace_all(pattern, "\\", "\\\\");
  boost::replace_all(pattern, "^", "\\^");
  boost::replace_all(pattern, ".", "\\.");
  boost::replace_all(pattern, "$", "\\$");
  boost::replace_all(pattern, "|", "\\|");
  boost::replace_all(pattern, "(", "\\(");
  boost::replace_all(pattern, ")", "\\)");
  boost::replace_all(pattern, "[", "\\[");
  boost::replace_all(pattern, "]", "\\]");
  boost::replace_all(pattern, "*", "\\*");
  boost::replace_all(pattern, "+", "\\+");
  boost::replace_all(pattern, "?", "\\?");
  boost::replace_all(pattern, "/", "\\/");

  boost::replace_all(pattern, "\\?", ".");
  boost::replace_all(pattern, "\\*", ".*");

  boost::regex bpattern(pattern);

  return regex_match(text, bpattern);
}

void DQMHistAnalysisInputRootFileModule::beginRun()
{
  B2INFO("DQMHistAnalysisInputRootFile: beginRun called.");
  clearRefList();
}

void DQMHistAnalysisInputRootFileModule::addToHistList(std::vector<TH1*>& inputHistList, std::string dirname, TKey* key)
{
  TH1* h = (TH1*)key->ReadObj();
  if (h == nullptr) return; // would be strange, but better check
  std::string hname = h->GetName();

  if (hname.find("/") == std::string::npos) {
    if (dirname != "") {
      hname = dirname + "/" + hname;
      h->SetName(hname.c_str());
    } else {
      // histo in root dir without prefix -> do not keep!
      delete h;
      return;
    }
  }

  bool hpass = false;
  if (m_histograms.size() == 0) {// workaround for run info???
    hpass = true;
  } else {
    for (auto& hpattern : m_histograms) {
      if (hnamePatternMatch(hpattern, h->GetName())) {
        hpass = true;
        break;
      }
    }
  }
  if (!hpass) {
    delete h;
    return;
  }

  // Remove ":" from folder name, workaround!
  // Is this really needed?
  if (hname.find(":") != std::string::npos) {
    B2ERROR("histogram or folder name with : is problematic");
  }
  TString a = h->GetName();
  a.ReplaceAll(":", "");
  h->SetName(a);
  B2DEBUG(1, "DQMHistAnalysisInput2: get histo " << a.Data());


  Double_t scale = 1.0 * m_count / m_eventsList[m_run_idx];
  h->Scale(scale);
  h->SetEntries(h->GetEntries()*scale); // empty hists are not marked for update!

  // Histograms in the inputHistList list will be taken care of later (delete)
  inputHistList.push_back(h);
}

void DQMHistAnalysisInputRootFileModule::event()
{
  B2INFO("DQMHistAnalysisInputRootFile: event called.");
  TH1::AddDirectory(false);

  if (m_count == 0) {
    // special handling for 1 event to get proper max scaling
    if (m_eventsList[m_run_idx] == 1) m_count++;
    else m_eventsList[m_run_idx]--;
  } else {
    sleep(m_interval);
  }

  // Check for run change
  if (m_count > m_eventsList[m_run_idx]) {
    m_run_idx++;
    if (m_run_idx == m_fileList.size()) {
      m_eventMetaDataPtr.create();
      m_eventMetaDataPtr->setEndOfData();
      return;
    }
    m_count = 0;
    // special handling for 1 event to get proper max scaling
    if (m_eventsList[m_run_idx] == 1) m_count = 1;
    else m_eventsList[m_run_idx]--;
    // open next file
    if (m_file != nullptr) {
      m_file->Close();
      delete m_file;
    }
    m_file = new TFile(m_fileList[m_run_idx].c_str());
  }

  // Clear only after EndOfRun check, otherwise we won't have any histograms for MiraBelle
  // which expects analysis run in endRun function
  initHistListBeforeEvent();

  /** Input vector for histograms */
  std::vector<TH1*> inputHistList;

  unsigned long long int ts = 0;
  m_file->cd();
  TIter next(m_file->GetListOfKeys());
  TKey* key = NULL;
  while ((key = (TKey*)next())) {
    TClass* cl = gROOT->GetClass(key->GetClassName());
    if (ts == 0) ts = key->GetDatime().Convert();
    if (cl->InheritsFrom("TDirectory")) {
      TDirectory* d = (TDirectory*)key->ReadObj();
      std::string dirname = d->GetName();

      d->cd();
      TIter nextd(d->GetListOfKeys());

      TKey* dkey;
      while ((dkey = (TKey*)nextd())) {
        TClass* dcl = gROOT->GetClass(dkey->GetClassName());
        if (!dcl->InheritsFrom("TH1")) continue;
        addToHistList(inputHistList, dirname, dkey);
      }
      m_file->cd();
    } else if (cl->InheritsFrom("TH1")) {
      addToHistList(inputHistList, "", key);
    }
  }

  auto expno = m_expno;
  auto runno = m_runList.size() ? m_runList[m_run_idx] : 0;
  auto rtype = m_runType;

  if (m_h_expno) {
    m_h_expno->SetTitle(std::to_string(expno).c_str());
    inputHistList.push_back((TH1*)(m_h_expno->Clone()));
  }
  if (m_h_runno) {
    m_h_runno->SetTitle(std::to_string(runno).c_str());
    inputHistList.push_back((TH1*)(m_h_runno->Clone()));
  }
  if (m_h_rtype) {
    m_h_rtype->SetTitle(rtype.c_str());
    inputHistList.push_back((TH1*)(m_h_rtype->Clone()));
  }
  if (m_fillNEvent > 0) {
    inputHistList.push_back((TH1*)(m_h_fillNEvent->Clone()));
  }
  // check for no-override
  for (auto& h : inputHistList) {
    if (std::string(h->GetName()) == std::string("DQMInfo/expno")) {
      if (expno == 0) {
        expno = atoi(h->GetTitle());
      } else {
        h->SetTitle(std::to_string(expno).c_str());
      }
    }
    if (std::string(h->GetName()) == std::string("DQMInfo/runno")) {
      if (runno == 0) {
        runno = atoi(h->GetTitle());
      } else {
        h->SetTitle(std::to_string(runno).c_str());
      }
    }
    if (std::string(h->GetName()) == std::string("DQMInfo/rtype")) {
      if (rtype == "") {
        rtype = h->GetTitle();
      } else {
        h->SetTitle(rtype.c_str());
      }
    }
    if (std::string(h->GetName()) == std::string("DAQ/Nevent")) {
      if (m_fillNEvent > 0) {
        Double_t scale = 1.0 * m_count / m_eventsList[m_run_idx];
        h->Scale(scale);
        h->SetEntries(h->GetEntries()*scale); // empty hists are not marked for update!
      }
    }
  }

  if (m_c_info != nullptr) m_c_info->SetTitle(("OFFLINE: Exp " + std::to_string(expno) + ", Run " + std::to_string(
                                                   runno) + ", RunType " + rtype + ", Last Changed NEVER, Last Updated NEVER, Last DQM event NEVER").c_str());

  m_count++;
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(expno);
  m_eventMetaDataPtr->setRun(runno);
  m_eventMetaDataPtr->setEvent(m_count);
  m_eventMetaDataPtr->setTime(ts * 1e9);

  ExtractRunType(inputHistList);
  ExtractNEvent(inputHistList);

  if (m_lastRun != runno or m_lastExp != expno) {
    // Run change detected
    m_lastRun = runno;
    m_lastExp = expno;
    // we cannot do that in beginRun(), otherwise all histos are cleare before first event
    clearHistList();
  }

  // this code must be run after "event processed" has been extracted
  for (size_t i = 0; i < inputHistList.size(); i++) {
    TH1* h = inputHistList[i];
    addHist("", h->GetName(), h);
    B2DEBUG(1, "Found : " << h->GetName() << " : " << h->GetEntries());
  }

  B2INFO("DQMHistAnalysisInputRootFile: event finished. count: " << m_count);
}
