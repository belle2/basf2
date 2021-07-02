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
//
// Author : Boqun Wang
// Date : Jun - 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisInputRootFile.h>

#include <daq/slc/base/StringUtil.h>
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
  addParam("FileList", m_file_list, "List of input files" , std::vector<std::string> {"input_histo.root"});
  addParam("SelectHistograms", m_histograms, "List of histogram name patterns, empty for all. Support wildcard matching (* and ?).",
           std::vector<std::string>());
  addParam("Experiment", m_expno, "Experiment Nr" , 7u);
  addParam("RunList", m_run_list, "Run Number List" , std::vector<unsigned int> {1u});
  addParam("EventsList", m_events_list, "Number of events for each run" , std::vector<unsigned int> {10u});
  addParam("EventInterval", m_interval, "Time between events (seconds)" , 20u);
  addParam("NullHistogramMode", m_null_histo_mode, "Test mode for null histograms" , false);
  addParam("AutoCanvas", m_autocanvas, "Automatic creation of canvas", true);
  B2DEBUG(1, "DQMHistAnalysisInputRootFile: Constructor done.");
}

void DQMHistAnalysisInputRootFileModule::initialize()
{
  if (m_file != nullptr) delete m_file;
  if (m_file_list.size() == 0) B2ERROR("File list is empty.");
  if (m_run_list.size() == 0) B2ERROR("Run list is empty.");
  if (m_events_list.size() == 0) B2ERROR("Events list is empty.");
  if (m_run_list.size() != m_events_list.size()) B2ERROR("Run list does not have the same size as events list.");
  if (m_run_list.size() != m_file_list.size()) B2ERROR("Run list does not have the same size as file list.");
  m_run_idx = 0;
  m_file = new TFile(m_file_list[m_run_idx].c_str());
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
  B2INFO("DQMHistAnalysisInputRootFile: beginRun called. Run: " << m_run_list[m_run_idx]);
}

void DQMHistAnalysisInputRootFileModule::event()
{
  B2INFO("DQMHistAnalysisInputRootFile: event called.");
  sleep(m_interval);

  if (m_count > m_events_list[m_run_idx]) {
    m_run_idx++;
    if (m_run_idx == m_run_list.size()) {
      m_eventMetaDataPtr.create();
      m_eventMetaDataPtr->setEndOfData();
      return;
    }
    m_count = 0;
    if (m_file != nullptr) {
      m_file->Close();
      delete m_file;
    }
    m_file = new TFile(m_file_list[m_run_idx].c_str());
  }

  if (m_null_histo_mode) {
    m_eventMetaDataPtr.create();
    m_eventMetaDataPtr->setExperiment(m_expno);
    m_eventMetaDataPtr->setRun(m_run_list[m_run_idx]);
    m_eventMetaDataPtr->setEvent(m_count);
    m_eventMetaDataPtr->setTime(0);
    B2INFO("DQMHistAnalysisInputRootFile: event finished. count: " << m_count);
    m_count++;
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

    d->cd();
    TIter nextd(d->GetListOfKeys());

    TKey* dkey;
    while ((dkey = (TKey*)nextd())) {
      TClass* dcl = gROOT->GetClass(dkey->GetClassName());
      if (!dcl->InheritsFrom("TH1")) continue;
      TH1* h = (TH1*)dkey->ReadObj();
      if (h->InheritsFrom("TH2")) h->SetOption("col");
      else h->SetOption("hist");
      Double_t scale = 1.0 * m_count / m_events_list[m_run_idx];
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

      if (m_autocanvas) {
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
      if (!cl->InheritsFrom("TH1")) continue;
      h = (TH1*)keyh->ReadObj();

      bool hpass = false;
      if (m_histograms.size() == 0) {
        hpass = true;
      } else {
        for (auto& hpattern : m_histograms) {
          if (hname_pattern_match(hpattern, h->GetName())) {
            hpass = true;
            break;
          }
        }
      }
      if (!hpass) continue;

      hs.push_back(h);
      Double_t scale = 1.0 * m_count / m_events_list[m_run_idx];
      h->Scale(scale);
      if (m_autocanvas) {
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
  }

  resetHist();
  for (size_t i = 0; i < hs.size(); i++) {
    TH1* h = hs[i];
    addHist("", h->GetName(), h);
    B2DEBUG(1, "Found : " << h->GetName() << " : " << h->GetEntries());
  }
  m_eventMetaDataPtr.create();

  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_run_list[m_run_idx]);
  m_eventMetaDataPtr->setEvent(m_count);
  m_eventMetaDataPtr->setTime(ts * 1e9);
  B2INFO("DQMHistAnalysisInputRootFile: event finished. count: " << m_count);

  m_count++;
}

void DQMHistAnalysisInputRootFileModule::endRun()
{
  B2INFO("DQMHistAnalysisInputRootFile : endRun called");
}


void DQMHistAnalysisInputRootFileModule::terminate()
{
  B2INFO("terminate called");
}

