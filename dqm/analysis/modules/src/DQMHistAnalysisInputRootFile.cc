//+
// File : DQMHistAnalysisInputRootFile.cc
// Description : Module for offline testing of histogram analysis code.
//               Root file containing DQM histograms can be used as input.
//
// Author : Boqun Wang
// Date : Jun - 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisInputRootFile.h>

#include <TKey.h>
#include <TROOT.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInputRootFile)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputRootFileModule::DQMHistAnalysisInputRootFileModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("InputRootFile", m_input_name, "Name of the input root file", std::string("input_histo.root"));
  addParam("SelectFolders", m_folders, "List of folders for which to process, empty for all", std::vector<std::string>());
  addParam("SelectHistograms", m_histograms, "List of histogram name patterns, empty for all. Support wildcard matching (* and ?).",
           std::vector<std::string>());
  addParam("Experiment", m_expno, "Experiment Nr" , 7u);
  addParam("RunNr", m_runno, "Run Nr" , 1u);
  addParam("TotalEvents", m_total_events, "Number of Events" , 1u);
  addParam("EventInterval", m_interval, "Time between events (seconds)" , 20u);
  B2DEBUG(1, "DQMHistAnalysisInputRootFile: Constructor done.");
}

void DQMHistAnalysisInputRootFileModule::initialize()
{
  if (m_file != nullptr) delete m_file;
  m_file = new TFile(m_input_name.c_str());
  m_eventMetaDataPtr.registerInDataStore();
  B2INFO("DQMHistAnalysisInputRootFile: initialized.");
}

bool DQMHistAnalysisInputRootFileModule::hname_pattern_match(std::string pattern, std::string text)
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
}

void DQMHistAnalysisInputRootFileModule::event()
{
  B2INFO("DQMHistAnalysisInputRootFile: event called.");
  sleep(m_interval);

  if (m_count >= m_total_events) {
    m_eventMetaDataPtr.create();
    m_eventMetaDataPtr->setEndOfData();
    return;
  }

  std::vector<TH1*> hs;
  unsigned long long int ts = 0;
  m_file->cd();
  TIter next(m_file->GetListOfKeys());
  TKey* key = NULL;
  while ((key = (TKey*)next())) {
    TClass* cl = gROOT->GetClass(key->GetClassName());
    if (ts == 0) ts = key->GetDatime().Convert();
    if (!cl->InheritsFrom("TDirectory")) continue;
    TDirectory* d = (TDirectory*)key->ReadObj();
    std::string dirname = d->GetName();

    bool pass = false;
    if (m_folders.size() == 0) {
      pass = true;
    } else {
      for (auto& wanted_folder : m_folders) {
        if (wanted_folder == dirname) {
          pass = true;
          break;
        }
      }
    }
    if (!pass) continue;

    d->cd();
    TIter nextd(d->GetListOfKeys());

    TKey* dkey;
    while ((dkey = (TKey*)nextd())) {
      TClass* dcl = gROOT->GetClass(dkey->GetClassName());
      if (!dcl->InheritsFrom("TH1")) continue;
      TH1* h = (TH1*)dkey->ReadObj();
      if (h->InheritsFrom("TH2")) h->SetOption("col");
      else h->SetOption("hist");
      Double_t scale = (m_count + 1.0) / m_total_events;
      h->Scale(scale);
      std::string hname = h->GetName();

      bool hpass = false;
      if (m_histograms.size() == 0) {
        hpass = true;
      } else {
        for (auto& hpattern : m_histograms) {
          if (hname_pattern_match(hpattern, dirname + "/" + hname)) {
            hpass = true;
            break;
          }
        }
      }
      if (!hpass) continue;

      if (hname.find("/") == std::string::npos) h->SetName((dirname + "/" + hname).c_str());
      hs.push_back(h);

      std::string name = dirname + "_" + hname;
      if (m_cs.find(name) == m_cs.end()) {
        TCanvas* c = new TCanvas((dirname + "/c_" + hname).c_str(), ("c_" + hname).c_str());
        m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
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
    m_file->cd();
  }

  // if no histograms are found in the sub-directories
  // searc the top folder
  if (hs.size() == 0) {
    TIter nexth(m_file->GetListOfKeys());
    TKey* keyh = NULL;
    while ((keyh = (TKey*)nexth())) {
      TClass* cl = gROOT->GetClass(keyh->GetClassName());
      TH1* h;
      if (cl->InheritsFrom("TH1")) { h = (TH1*)keyh->ReadObj(); hs.push_back(h); }
      else continue;
      Double_t scale = (m_count + 1.0) / m_total_events;
      h->Scale(scale);
      std::string name = h->GetName();
      name.replace(name.find("/"), 1, "/c_");
      if (m_cs.find(name) == m_cs.end()) {
        TCanvas* c = new TCanvas(name.c_str(), name.c_str());
        m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
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
  }

  resetHist();
  for (size_t i = 0; i < hs.size(); i++) {
    TH1* h = hs[i];
    addHist("", h->GetName(), h);
    B2DEBUG(1, "Found : " << h->GetName() << " : " << h->GetEntries());
  }
  m_count++;
  m_eventMetaDataPtr.create();

  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);
  m_eventMetaDataPtr->setTime(ts * 1e9);

}

void DQMHistAnalysisInputRootFileModule::endRun()
{
  B2INFO("DQMHistAnalysisInputRootFile : endRun called");
}


void DQMHistAnalysisInputRootFileModule::terminate()
{
  B2INFO("terminate called");
}

