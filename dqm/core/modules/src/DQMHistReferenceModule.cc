/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/core/modules/DQMHistReferenceModule.h>
#include <TROOT.h>
#include <TStyle.h>
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

  TIter nextRefDirKey(refFile->GetListOfKeys());
  TKey* refDirKey;
  while ((refDirKey = (TKey*)nextRefDirKey())) {
    if (refDirKey->IsFolder() && string(refDirKey->GetName()) == string("ref")) {
      TDirectory* refDir = (TDirectory*)refDirKey->ReadObj(); // ReadObj -> I own it, delete later
      TIter nextDetDirKey(refDir->GetListOfKeys());
      TKey* detDirKey;
      // detector folders
      while ((detDirKey = (TKey*)nextDetDirKey())) {
        if (!detDirKey->IsFolder()) continue;
        TDirectory* detDir = ((TDirectory*)detDirKey->ReadObj());
        TIter nextRunTypeDirKey(detDir->GetListOfKeys()); // ReadObj -> Now I own this, so delete later
        TKey* runtypeDirKey;
        TDirectory* runtypeDir = nullptr;
        // run type folders (get the run type corresponding folder or use default one)
        while ((runtypeDirKey = (TKey*)nextRunTypeDirKey())) {
          if (!runtypeDirKey->IsFolder()) continue;
          if (string(runtypeDirKey->GetName()) == run_type) {
            if (runtypeDir) delete runtypeDir; // if default was loaded before
            runtypeDir = (TDirectory*)runtypeDirKey->ReadObj(); // ReadObj -> I own it, delete later
            break; // break directly, otherwise "default" could overwrite it
          }
          // else we would check if default, which we load as backup
          if (string(runtypeDirKey->GetName()) == "default") runtypeDir = (TDirectory*)runtypeDirKey->ReadObj(); // ReadObj -> I own it
        }
        string detName = detDir->GetName();
        if (!runtypeDir) {
          B2INFO("No run type specific or default references available for " << detName);
        } else {
          B2INFO("Reading reference histograms for " << detName << " from run type folder: " << runtypeDirKey->GetName());

          TIter nextHistkey(runtypeDir->GetListOfKeys());
          TKey* histKey;
          // now read histograms
          while ((histKey = (TKey*)nextHistkey())) {
            if (histKey->IsFolder()) continue;
            if (gROOT->GetClass(histKey->GetClassName())->InheritsFrom("TH1")) {
              addRefHist(detName, (TH1*)histKey->ReadObj()); // ReadObj -> I own it, tranfer ownership to function
            }
          }
          delete runtypeDir; // always non-zero as checked above ... runtype or "default"
        }
        delete detDir; // always non-zero ... detector subdir name
      }
      delete refDir; // always non-zero ... "ref" folder
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

