/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <dqm/analysis/modules/DQMHistAnalysisOutputMonObj.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include <time.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisOutputMonObj)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisOutputMonObjModule::DQMHistAnalysisOutputMonObjModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("Filename", m_filename, "Output root filename (if not set mon_e{exp}r{run}.root is used", std::string(""));
  addParam("TreeFile", m_treeFile, "If set, entry to run summary TTree from TreeFile is made", std::string(""));
  addParam("ProcID", m_procID, "Processing id (online,proc10, etc.)", std::string("online"));
  addParam("run", m_run, "Run number", 0);
  addParam("exp", m_exp, "Experiment number", 0);
  addParam("nevt", m_nevt, "Number of events", 0);
  addParam("runtype", m_runtype, "Run type", std::string(""));

  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: Constructor done.");
}


DQMHistAnalysisOutputMonObjModule::~DQMHistAnalysisOutputMonObjModule() { }

void DQMHistAnalysisOutputMonObjModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: initialized.");
  // create file metadata
  m_metaData = new DQMFileMetaData();
  m_metaData->setProcessingID(m_procID);
}


void DQMHistAnalysisOutputMonObjModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: beginRun called.");
}


void DQMHistAnalysisOutputMonObjModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisOutputMonObj: event called.");
}

void DQMHistAnalysisOutputMonObjModule::endRun()
{
  B2INFO("DQMHistAnalysisOutputMonObj: endRun called");

  StoreObjPtr<EventMetaData> lastEvtMeta;

  B2INFO("open file");
  TH1* hrun = findHist("DQMInfo/runno");
  TH1* hexp = findHist("DQMInfo/expno");

  int run = hrun ? std::stoi(hrun->GetTitle()) : m_run;
  int exp = hexp ? std::stoi(hexp->GetTitle()) : m_exp;
  TString fname;
  if (m_filename.length()) fname = m_filename;
  else fname = TString::Format("mon_e%04dr%06d_%s.root", exp, run, m_procID.c_str());

  TH1* runtype = findHist("DQMInfo/rtype");
  if (runtype) m_metaData->setRunType(std::string(runtype->GetTitle()));
  else m_metaData->setRunType(m_runtype);
  TH1* hnevt = findHist("DAQ/Nevent");
  if (hnevt) m_metaData->setNEvents(hnevt->GetEntries());
  else m_metaData->setNEvents(m_nevt);

  TFile f(fname, "NEW");

  if (f.IsZombie()) {
    B2WARNING("File " << LogVar("MonitoringObject file",
                                fname) << " already exists and it will not be rewritten. If desired please delete file and re-run.");
    return;
  }

  // set meta data info
  //  m_metaData->setNEvents(lastEvtMeta->getEvent());
  m_metaData->setExperimentRun(exp, run);
  time_t ts = lastEvtMeta->getTime() / 1e9;
  struct tm* timeinfo;
  timeinfo = localtime(&ts);
  m_metaData->setRunDate(asctime(timeinfo));

  m_metaData->Write();
  // get list of existing monitoring objects
  const MonObjList& objts =  getMonObjList();
  // write them to the output file
  for (const auto& obj : objts)(obj.second)->Write();

  f.Close();

  if (m_treeFile.length() > 0) addTreeEntry();

}

void DQMHistAnalysisOutputMonObjModule::addTreeEntry()
{

  TFile* treeFile = new TFile(m_treeFile.c_str(), "update");
  auto* tree = (TTree*)treeFile->Get("tree");

  if (tree == NULL) tree = new TTree("tree", "tree");

  int run = m_metaData->getRun();
  int expe = m_metaData->getExperiment();
  int nevt = m_metaData->getNEvents();
  //int rune = 0;
  //int expee = 0;
  char* rel = const_cast<char*>(m_metaData->getRelease().c_str());
  char* db = const_cast<char*>(m_metaData->getDatabaseGlobalTag().c_str());
  char* datee = const_cast<char*>(m_metaData->getRunDate().c_str());
  char* rtype = const_cast<char*>(m_metaData->getRunType().c_str());
  char* procID = const_cast<char*>(m_metaData->getProcessingID().c_str());

  auto b_run = tree->GetBranch("run");
  auto b_exp = tree->GetBranch("exp");
  auto b_release = tree->GetBranch("release");
  auto b_gt = tree->GetBranch("gt");
  auto b_datetime = tree->GetBranch("datetime");
  auto b_rtype = tree->GetBranch("rtype");
  auto b_procID = tree->GetBranch("procID");
  auto b_nevt = tree->GetBranch("nevt");

  // this still needs to be sorted out
  /*if(b_run){
    b_run->SetAddress(&rune);
    b_exp->SetAddress(&expee);
    for(int ie = 0; ie<b_run->GetEntries(); ie++){
      b_run->GetEntry(ir);
      b_exp->GetEntry(ir);
      if(rune == run && expee = expe){

      }
      }
      }*/


  if (!b_run) tree->Branch("run", &run, "run/I");
  else b_run->SetAddress(&run);
  if (!b_exp) tree->Branch("exp", &expe, "exp/I");
  else b_exp->SetAddress(&expe);
  if (!b_nevt) tree->Branch("nevt", &nevt, "nevt/I");
  else b_nevt->SetAddress(&nevt);
  if (!b_release) tree->Branch("release", rel, "release/C");
  else b_release->SetAddress(rel);
  if (!b_gt) tree->Branch("gt", db, "gt/C");
  else b_gt->SetAddress(db);
  if (!b_datetime) tree->Branch("datetime", datee, "datetime/C");
  else b_datetime->SetAddress(datee);
  if (!b_rtype) tree->Branch("rtype", rtype, "rtype/C");
  else b_rtype->SetAddress(rtype);
  if (!b_procID) tree->Branch("procID", procID, "procID/C");
  else b_procID->SetAddress(procID);


  const MonObjList& objts =  getMonObjList();
  // write them to the output file
  for (const auto& obj : objts) {
    std::map<std::string, float>& vars = const_cast<std::map<std::string, float>&>((obj.second)->getVariables());
    std::map<std::string, float>& upErr = const_cast<std::map<std::string, float>&>((obj.second)->getUpError());
    std::map<std::string, float>& lowErr = const_cast<std::map<std::string, float>&>((obj.second)->getLowError());

    const std::vector<std::pair<std::string, std::string>>& strVars = (obj.second)->getStringVariables();

    for (auto& var : vars) {
      std::string brname = obj.first + "_" + var.first;
      auto branch = tree->GetBranch((brname).c_str());
      if (!branch) {
        branch = tree->Branch((brname).c_str(), &(var.second));
        fillBranch(branch);
      } else branch->SetAddress(&(var.second));

      auto vvE1 = upErr.find(var.first);
      auto vvE2 = lowErr.find(var.first);

      if (vvE1 != upErr.end() && vvE2 == lowErr.end()) {
        auto errBranch = tree->GetBranch((brname).c_str() + TString("_err"));
        if (!errBranch) {
          errBranch = tree->Branch((brname).c_str() + TString("_err"), &(vvE1->second));
          fillBranch(errBranch);
        } else errBranch->SetAddress(&(vvE1->second));
      }

      if (vvE1 != upErr.end() && vvE2 != lowErr.end()) {
        auto errBranch1 = tree->GetBranch((brname).c_str() + TString("_upErr"));
        if (!errBranch1) {
          errBranch1 = tree->Branch((brname).c_str() + TString("_upErr"), &(vvE1->second));
          fillBranch(errBranch1);
        } else errBranch1->SetAddress(&(vvE1->second));

        auto errBranch2 = tree->GetBranch((brname).c_str() + TString("_dwErr"));
        if (!errBranch2) {
          errBranch2 = tree->Branch((brname).c_str() + TString("_dwErr"), &(vvE2->second));
          fillBranch(errBranch2);
        } else errBranch2->SetAddress(&(vvE2->second));

      }
    }

    for (auto& var : strVars) {
      std::string brname = obj.first + "_" + var.first;
      char* cc = const_cast<char*>((var.second).c_str());
      auto branch = tree->GetBranch((brname).c_str());
      if (!branch) {
        std::string ty = brname + "/C";
        branch = tree->Branch((brname).c_str(), cc, ty.c_str());
        fillBranch(branch);
      } else branch->SetAddress(cc);
    }
  }

  tree->Fill();
  tree->Write(0, TObject::kWriteDelete, 0);
  treeFile->Close();

}

void DQMHistAnalysisOutputMonObjModule::fillBranch(TBranch* branch)
{
  TTree* tree = (TTree*)branch->GetTree();
  int nentr = tree->GetEntries();
  for (int i = 0; i < nentr; i++) {
    tree->GetEntry(i);
    branch->Fill();
  }
}



void DQMHistAnalysisOutputMonObjModule::terminate()
{
  B2INFO("DQMHistAnalysisOutputMonObj: terminate called");
// Attention, we can not do that in Terminate, as then the memFile is already closed by previous task!

}

