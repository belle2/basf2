/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistReferenceModule.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TClass.h>
#include <TKey.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistReference);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistReferenceModule::DQMHistReferenceModule() : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("ReferenceFile", m_referenceFileName, "Name of the reference histrogram files", string(""));
  B2DEBUG(1, "DQMHistReference: Constructor done.");
}


DQMHistReferenceModule::~DQMHistReferenceModule() { }

void DQMHistReferenceModule::initialize()
{
  B2DEBUG(1, "DQMHistReference: initialized.");
}

void DQMHistReferenceModule::beginRun()
{
  B2DEBUG(1, "DQMHistReference: beginRun called.");

  loadReferenceHistos();
}

void DQMHistReferenceModule::loadReferenceHistos()
{
  TH1::AddDirectory(false); // do not store any histograms

  B2DEBUG(1, "DQMHistReference: reading references from input root file");

  string run_type = getRunType();
  if (run_type == "") run_type = "default";

  B2INFO("DQMHistReference: run_type " << run_type);

  TFile* refFile = new TFile(m_referenceFileName.c_str(), "READ");

  if (refFile->IsZombie()) {
    B2INFO("DQMHistReference: reference file " << m_referenceFileName << " does not exist. No references will be used!");
    refFile->Close();
    delete refFile;
    return;
  }

  B2INFO("DQMHistReference: use reference file " << m_referenceFileName);

  TIter nextkey(refFile->GetListOfKeys());
  TKey* key;
  while ((key = (TKey*)nextkey())) {
    if (key->IsFolder() && string(key->GetName()) == string("ref")) {
      TDirectory* refdir = (TDirectory*)key->ReadObj(); // ReadObj -> I own it
      TIter nextDetDir(refdir->GetListOfKeys());
      TKey* detDir;
      // detector folders
      while ((detDir = (TKey*)nextDetDir())) {
        if (!detDir->IsFolder()) continue;
        TIter nextTypeDir(((TDirectory*)detDir->ReadObj())->GetListOfKeys());
        TKey* typeDir;
        TDirectory* foundDir = NULL;
        // run type folders (get the run type corresponding folder or use default one)
        while ((typeDir = (TKey*)nextTypeDir())) {
          if (!typeDir->IsFolder()) continue;
          if (string(typeDir->GetName()) == run_type) {
            foundDir = (TDirectory*)typeDir->ReadObj(); // ReadObj -> I own it
            break;
          }
          if (string(typeDir->GetName()) == "default") foundDir = (TDirectory*)typeDir->ReadObj(); // ReadObj -> I own it
        }
        string dirname = detDir->GetName();
        if (!foundDir) {
          B2INFO("No run type specific or default references available for " << dirname);
        } else {
          B2INFO("Reading reference histograms for " << dirname << " from run type folder: " << foundDir->GetName());

          TIter next(foundDir->GetListOfKeys());
          TKey* hh;

          while ((hh = (TKey*)next())) {
            if (hh->IsFolder()) continue;
            TObject* obj = hh->ReadObj(); // ReadObj -> I own it
            if (obj->IsA()->InheritsFrom("TH1")) {
              TH1* h = (TH1*)obj;
              string histname = h->GetName();
              std::string name = dirname + "/" + histname;
              auto& n = getRefList()[name];
              n.m_orghist_name = name;
              n.m_refhist_name = "ref/" + name;
              h->SetName((n.m_refhist_name).c_str());
              h->SetDirectory(0);
              n.setRefHist(h); // transfer ownership!
              n.setRefCopy(nullptr);
              n.setCanvas(nullptr);
            } else {
              delete obj;
            }
          }
          delete foundDir; // always non-zero
        }
      }
      delete refdir; // always non-zero
    }
  }

  B2INFO("DQMHistReference: read references done");
  refFile->Close();
  delete refFile;
}

void DQMHistReferenceModule::event()
{
  B2DEBUG(1, "DQMHistReference: event called");
}

void DQMHistReferenceModule::endRun()
{
  B2DEBUG(1, "DQMHistReference: endRun called");
}

void DQMHistReferenceModule::terminate()
{
  B2DEBUG(1, "DQMHistReference: terminate called");
}

