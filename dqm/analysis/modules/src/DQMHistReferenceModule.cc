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
  addParam("ReferenceFile", m_referenceFile, "Name of the reference histrogram files", string(""));
  B2DEBUG(1, "DQMHistReference: Constructor done.");
}


DQMHistReferenceModule::~DQMHistReferenceModule() { }

void DQMHistReferenceModule::initialize()
{
  gStyle->SetOptStat(0);
  gStyle->SetStatStyle(1);
  gStyle->SetOptDate(22);// Date and Time in Bottom Right, does no work

  B2DEBUG(1, "DQMHistReference: initialized.");
}

void DQMHistReferenceModule::beginRun()
{
  B2DEBUG(1, "DQMHistReference: beginRun called.");
  m_firstInRun = true;
}

void DQMHistReferenceModule::loadReferenceHistos()
{
  B2DEBUG(1, "DQMHistReference: reading references from input root file");

  string run_type = getRunType();
  if (run_type == "") run_type = "default";

  B2INFO("DQMHistReference: run_type " << run_type);

  for (auto& it : m_pnode) {
    // clear ref histos from memory
    if (it.m_refHist) it.setRefHist(nullptr);
    if (it.m_refCopy) it.setRefCopy(nullptr);
  }
  m_pnode.clear();
  B2INFO("DQMHistReference: clear m_pnode. size: " << m_pnode.size());

  TFile* refFile = new TFile(m_referenceFile.c_str(), "READ");

  if (refFile->IsZombie()) {
    B2INFO("DQMHistReference: reference file " << m_referenceFile << " does not exist. No references will be used!");
    refFile->Close();
    delete refFile;
    return;
  }

  B2INFO("DQMHistReference: use reference file " << m_referenceFile);

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
              if (h->GetDimension() == 1) {
                string histname = h->GetName();
                m_pnode.push_back(RefHistObject());
                auto& n = m_pnode.back();
                n.m_orghist_name = dirname + "/" + histname;
                n.m_refhist_name = "ref/" + dirname + "/" + histname;
                h->SetName((n.m_refhist_name).c_str());
                h->SetDirectory(0);
                n.setRefHist(h); // transfer ownership!
                n.setRefCopy(nullptr);
                n.setCanvas(nullptr);

              } else {
                delete h;
              }
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

  B2INFO("DQMHistReference: insert reference to m_pnode. size: " << m_pnode.size());
  refFile->Close();
  delete refFile;
}

void DQMHistReferenceModule::event()
{
  TH1::AddDirectory(false); // do not store any histograms

  if (m_firstInRun) {
    loadReferenceHistos();
    m_firstInRun = false;
  }

  char mbstr[100];

  time_t now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] before ref loop");


  for (auto& it : m_pnode) {
    TH1* ref = it.getRefHist();
    if (!ref) continue; // No reference, continue
    TCanvas* canvas = it.getCanvas();
    TH1* hist1 = findHistInCanvas(it.m_orghist_name, &(canvas));

    // if there is no histogram on canvas we plot the reference anyway.
    if (!hist1) {
      B2DEBUG(1, "Canvas is without histogram -> no display " << it.m_orghist_name);
      // Display something could be confusing for shifters
//       B2DEBUG(1, "Canvas is without histogram -> displaying only reference " << it.orghist_name);
//       canvas->cd();
//       hist2->Draw();
//       canvas->Modified();
//       canvas->Update();
      continue;
    }
    if (!canvas) {
      B2DEBUG(1, "No canvas found for reference histogram " << it.m_orghist_name);
      continue;
    }
    if (hist1->Integral() == 0) continue; // empty histogram -> continue

    /* consider adding coloring option....
      double data = 0;
      if (m_color) {
      data = hist1->KolmogorovTest(hist2, ""); // returns p value (0 bad, 1 good), N - do not compare normalized
      }
    */

    if (abs(ref->Integral()) > 0) { // only if we have entries in reference
      TH1* refCopy = it.getRefCopy();
      if (refCopy) {
        refCopy->Reset();
        refCopy->Add(ref);
        refCopy->Scale(hist1->Integral() / refCopy->Integral());
      } else {
        refCopy = scaleReference(hist1, ref);
      }

      //Adjust the y scale to cover the reference
      if (refCopy->GetMaximum() > hist1->GetMaximum())
        hist1->SetMaximum(1.1 * refCopy->GetMaximum());

      canvas->cd();
      refCopy->Draw("hist,same");

      canvas->Modified();
      canvas->Update();
      B2DEBUG(2, "Adding ref: " << it.m_orghist_name << " " << ref->GetName() << " " << ref);
      addRef("", it.m_orghist_name, ref);
    }
  }

  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%F %T", localtime(&now));
  B2INFO("[" << mbstr << "] after ref loop");
}

void DQMHistReferenceModule::endRun()
{
  B2DEBUG(1, "DQMHistReference: endRun called");
}


void DQMHistReferenceModule::terminate()
{
  B2DEBUG(1, "DQMHistReference: terminate called");
  for (auto& it : m_pnode) {
    // clear ref histos from memory (but converting to smart pointers)
    //if (it.m_refHist) delete it.m_refHist;
    //if (it.m_refCopy) delete it.m_refCopy;
    if (it.m_refHist) it.setRefHist(nullptr);
    if (it.m_refCopy) it.setRefCopy(nullptr);
  }
  m_pnode.clear();
}

