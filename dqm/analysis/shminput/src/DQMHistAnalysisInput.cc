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
  addParam("AutoCanvas", m_autocanvas, "Automatic creation of canvas", true);
  addParam("AutoCanvasFolders", m_acfolders, "List of histograms to automatically create canvases, empty for all",
           std::vector<std::string>());
  addParam("ExcludeFolders", m_exclfolders, "List of folders to exclude from create canvases, empty for none, \"all\" for all",
           std::vector<std::string>());
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
  if (m_autocanvas && m_enable_run_info) {
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

    // the following lines prevent any histogram outside a directory to be processed
    auto split_result = StringSplit(a.Data(), '/');
    if (split_result.size() <= 1) continue;
    auto dirname = split_result.at(0); // extract dirname, get hist name is in histogram itself

    hs.push_back(h);
    if (std::string(h->GetName()) == std::string("DQMInfo/expno")) expno = h->GetTitle();
    if (std::string(h->GetName()) == std::string("DQMInfo/runno")) runno = h->GetTitle();
    if (std::string(h->GetName()) == std::string("DQMInfo/rtype")) rtype = h->GetTitle();
    if (m_autocanvas) {

      bool give_canvas = false;
      if (m_exclfolders.size() == 0) { //If none specified, canvases for all histograms
        give_canvas = true;
      } else {
        bool in_excl_folder = false;
        if (m_exclfolders.size() == 1 && m_exclfolders[0] == "all") {
          in_excl_folder = true;
        } else {
          for (auto& excl_folder : m_exclfolders) {
            if (excl_folder == dirname) {
              in_excl_folder = true;
              break;
            }
          }
        }

        if (in_excl_folder) {
          for (auto& wanted_folder : m_acfolders) {
            B2DEBUG(1, "==" << wanted_folder << "==" << dirname << "==");
            if (wanted_folder == std::string(h->GetName())) {
              give_canvas = true;
              break;
            }
          }
        } else {
          give_canvas = true;
        }
      }

      if (give_canvas) {
        B2DEBUG(1, "Auto Hist->Canvas for " << a);
        a.ReplaceAll("/", "_");
        std::string name = a.Data();
        if (m_cs.find(name) == m_cs.end()) {
          // no canvas exists yet, create one
          if (split_result.size() > 1) {
            std::string hname = split_result.at(1);
            if ((dirname + "/" + hname) == "softwaretrigger/skim") hname = "skim_hlt";
            TCanvas* c = new TCanvas((dirname + "/c_" + hname).c_str(), ("c_" + hname).c_str());
            m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
          } else {
            // but this case is explicity excluded above?
            std::string hname = a.Data();
            TCanvas* c = new TCanvas(("c_" + hname).c_str(), ("c_" + hname).c_str());
            m_cs.insert(std::pair<std::string, TCanvas*>(name, c));
          }
        }
        TCanvas* c = m_cs[name]; // access already created canvas
        B2DEBUG(1, "DQMHistAnalysisInput: new canvas " << c->GetName());
        c->cd();
        if (h->GetDimension() == 1) {
          // assume users are expecting non-0-suppressed axis
          if (h->GetMinimum() > 0) h->SetMinimum(0);
          h->Draw("hist");
        } else if (h->GetDimension() == 2) {
          // ... but not in 2d
          h->Draw("colz");
        }
        c->Update();
      }
    }
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
  for (size_t i = 0; i < hs.size(); i++) {
    TH1* h = hs[i];
    addHist("", h->GetName(), h);
    B2DEBUG(1, "Found : " << h->GetName() << " : " << h->GetEntries());
  }
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

