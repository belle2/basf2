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
  addParam("HistMemoryPath", m_mempath, "Path to Input Hist memory");
  addParam("StatFileName", m_statname, "Filename for status export", std::string(""));
  addParam("RefreshInterval", m_interval, "Refresh interval of histograms", 10);
  addParam("EnableRunInfo", m_enable_run_info, "Enable Run Info", false);
  B2DEBUG(1, "DQMHistAnalysisInput2: Constructor done.");
}

void DQMHistAnalysisInput2Module::initialize()
{
  if (m_enable_run_info) {
    m_c_info = new TCanvas("DQMInfo/c_info", "");
    m_c_info->SetTitle("");
  }
  m_eventMetaDataPtr.registerInDataStore();
  B2DEBUG(1, "DQMHistAnalysisInput2: initialized.");
}

void DQMHistAnalysisInput2Module::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisInput2: beginRun called.");
  clearRefList();
  clearCanvases();

  m_last_beginrun = time(0);
  write_state();
  m_lasttime -= std::chrono::seconds(1); // just change
}

void DQMHistAnalysisInput2Module::addToHistList(std::vector<TH1*>& inputHistList, std::string dirname, TKey* key)
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

  // Remove ":" from folder name, workaround!
  // Is this really needed?
  if (hname.find(":") != std::string::npos) {
    B2ERROR("histogram or folder name with : is problematic");
  }
  TString a = h->GetName();
  a.ReplaceAll(":", "");
  h->SetName(a);
  B2DEBUG(1, "DQMHistAnalysisInput2: get histo " << a.Data());

  // Histograms in the inputHistList list will be taken care of later (delete)
  inputHistList.push_back(h);
}

void DQMHistAnalysisInput2Module::event()
{
  m_last_event = time(0);
  write_state();

  TH1::AddDirectory(false);
  initHistListBeforeEvent();

// we create a metadata and set the latest exp/run
// if do not, execution will be terminate if file is not updated/zombie
// if we only put dummy exp/run, we trigger begin/endRun ...
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);

  const std::filesystem::file_time_type ftime = std::filesystem::last_write_time(m_mempath);

  if (m_lasttime == ftime) {
    B2INFO("File not updated! -> Sleep");
    sleep(m_interval);
    setReturnValue(false);
    return;
  }
  m_last_file_update = time(0);
  m_lasttime = ftime;

  char mbstr[100];
  time_t now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] before Load File");

  std::unique_ptr<TFile> pFile = std::unique_ptr<TFile> (new TFile(m_mempath.c_str(), "READ"));
  if (pFile->IsZombie()) {
    B2WARNING("DQMHistAnalysisInput2: " << m_mempath + " is Zombie -> Sleep");
    sleep(m_interval);
    setReturnValue(false);
    return;
  }

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] after LoadFile");

  const TDatime& mt = pFile->GetModificationDate();
  TDatime mmt(mt.Convert());
  std::string expno("UNKNOWN"), runno("UNKNOWN"), rtype("UNKNOWN");

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] before input loop");

  // check if histograms were updated since last DQM event (based on number of processed events)
  if (m_nevent != getEventProcessed()) {
    m_lastChange = std::string(mbstr);
  }
  // update number of processed events
  m_nevent = getEventProcessed();

  /** Input vector for histograms */
  std::vector<TH1*> inputHistList;

  // first check sub-directories
  pFile->cd();
  TIter next(pFile->GetListOfKeys());
  TKey* key = NULL;
  while ((key = (TKey*)next())) {
    TClass* cl = gROOT->GetClass(key->GetClassName());
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
      pFile->cd();
    } else if (cl->InheritsFrom("TH1")) {
      addToHistList(inputHistList, "", key);
    }
  }

  for (auto& h : inputHistList) {
    if (std::string(h->GetName()) == std::string("DQMInfo/expno")) expno = h->GetTitle();
    if (std::string(h->GetName()) == std::string("DQMInfo/runno")) runno = h->GetTitle();
    if (std::string(h->GetName()) == std::string("DQMInfo/rtype")) rtype = h->GetTitle();
  }

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] before Close File");

  // we are done with reading, so close it
  pFile->Close();

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] before delete File");
  pFile = nullptr;

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] after input loop");

  if (expno == std::string("UNKNOWN") || runno == std::string("UNKNOWN")) {
    B2WARNING("DQMHistAnalysisInput2: " << m_mempath + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Updated " +
              mmt.AsString());
    setReturnValue(false);
    for (auto& h : inputHistList)  delete h;
    return;
  } else {
    if (m_c_info != NULL) m_c_info->SetTitle((m_mempath + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Changed "
                                                + m_lastChange + ", Last Updated "
                                                + mmt.AsString() + ", Last DQM event " + std::string(mbstr)).c_str());
    m_expno = std::stoi(expno);
    m_runno = std::stoi(runno);
  }
  B2INFO("DQMHistAnalysisInput2: " << m_mempath + ": Exp " + expno + ", Run " + runno + ", RunType " + rtype + ", Last Updated " +
         mmt.AsString());


  m_count++;
  // Override with updated values
  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);
  m_eventMetaDataPtr->setTime(mt.Convert());

  //setExpNr(m_expno); // redundant access from MetaData
  //setRunNr(m_runno); // redundant access from MetaData
  // ExtractRunType();// Run Type is processed above already, just take it
  setRunType(rtype);
  ExtractNEvent(inputHistList);

  if (m_lastRun != m_runno or m_lastExp != m_expno) {
    // Run change detected
    m_lastRun = m_runno;
    m_lastExp = m_expno;
    // we cannot do that in beginRun(), otherwise all histos are cleare before first event
    clearHistList();
    resetDeltaList();
  }

  // this code must be run after "event processed" has been extracted
  bool anyupdate = false; // flag if any histogram updated at all
  for (auto& h : inputHistList) {
    anyupdate |= addHist("", h->GetName(), h);
    B2DEBUG(1, "Found : " << h->GetName() << " : " << h->GetEntries());
  }

  if (anyupdate) {
    m_last_content_update = time(0);
  } else {
    // if there is no update, sleep a moment
    sleep(m_interval);
  }

  // if no histogram was updated, we could stop processing
  setReturnValue(anyupdate);
}

void DQMHistAnalysisInput2Module::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisInput2: endRun called");
}

void DQMHistAnalysisInput2Module::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisInput2: terminate called");
  if (m_c_info) delete m_c_info;
  clearlist(); // necessary in the Input Module! Otherwise ROOT may clean before we do
}

void DQMHistAnalysisInput2Module::write_state(void)
{
  if (m_statname == "") return;
  FILE* fh = fopen(m_statname.c_str(), "wt+");
  if (fh) {
    char mbstr[100];
    time_t now = time(0);
    strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
    fprintf(fh, "%s,%s,%s,", m_statname.c_str(), m_mempath.c_str(), mbstr);
    strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_last_event));
    fprintf(fh, "%s,", mbstr);
    strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_last_beginrun));
    fprintf(fh, "%s,", mbstr);
    strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_last_file_update));
    fprintf(fh, "%s,", mbstr);
    strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&m_last_content_update));
    fprintf(fh, "%s\n", mbstr);

    fclose(fh);
  }
}
