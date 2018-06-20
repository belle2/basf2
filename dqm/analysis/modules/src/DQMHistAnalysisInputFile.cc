//+
// File : DQMHistAnalysisInputFile.cc
// Description :
//
// Author : Boqun Wang
// Date : Jun - 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisInputFile.h>

#include <daq/slc/base/StringUtil.h>
#include <TROOT.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInputFile)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputFileModule::DQMHistAnalysisInputFileModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("InputRootFile", m_input_name, "Name of the input root file", std::string("input_histo.root"));
  addParam("SelectFolders", m_folders, "List of folders for which to process, empty for all", std::vector<std::string>());
  B2DEBUG(1, "DQMHistAnalysisInputFile: Constructor done.");
}

void DQMHistAnalysisInputFileModule::initialize()
{
  m_expno = m_runno = 0;
  m_count = 0;
  m_file = new TFile(m_input_name.c_str());
  m_eventMetaDataPtr.registerInDataStore();
  B2INFO("DQMHistAnalysisInputFile: initialized.");
}

void DQMHistAnalysisInputFileModule::beginRun()
{
  B2INFO("DQMHistAnalysisInputFile: beginRun called.");
}

void DQMHistAnalysisInputFileModule::event()
{
  if (m_count >= 1) {
    m_eventMetaDataPtr.create();
    m_eventMetaDataPtr->setEndOfData();
    return;
  }

  std::vector<TH1*> hs;

  m_file->cd();
  TIter next(m_file->GetListOfKeys());
  TKey* key = NULL;
  while ((key = (TKey*)next())) {
    TClass* cl = gROOT->GetClass(key->GetClassName());
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
    TIter dnext(d->GetListOfKeys());
    TKey* dkey;
    while ((dkey = (TKey*)dnext())) {
      TClass* dcl = gROOT->GetClass(dkey->GetClassName());
      if (!dcl->InheritsFrom("TH1")) continue;
      TH1* h = (TH1*)dkey->ReadObj();
      if (h->InheritsFrom("TH2")) h->SetOption("col");
      else h->SetOption("hist");
      std::string hname = h->GetName();

      h->SetName((dirname + "/" + hname).c_str());
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

  resetHist();
  for (size_t i = 0; i < hs.size(); i++) {
    TH1* h = hs[i];
    addHist("", h->GetName(), h);
    B2DEBUG(1, "Found : " << h->GetName() << " : " << h->GetEntries());
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

void DQMHistAnalysisInputFileModule::endRun()
{
  B2INFO("DQMHistAnalysisInputFile : endRun called");
}


void DQMHistAnalysisInputFileModule::terminate()
{
  B2INFO("terminate called");
}

