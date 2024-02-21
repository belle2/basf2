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

  string run_type = "default";
  TH1* hrtype = findHist("DQMInfo/rtype");
  if (hrtype != NULL) {
    run_type = string(hrtype->GetTitle());
    B2INFO("DQMHistReference: hrtype: " << string(hrtype->GetName()));
  }
  B2INFO("DQMHistReference: run_type " << run_type);

  for (auto& it : m_pnode) {
    // clear ref histos from memory
    delete it->ref_clone;
    delete it;
  }
  m_pnode.clear();
  B2INFO("DQMHistReference: clear m_pnode. size: " << m_pnode.size());

  //if (m_refFile != NULL) delete m_refFile;
  TFile* m_refFile = new TFile(m_referenceFile.c_str());

  if (m_refFile->IsZombie()) {
    B2INFO("DQMHistReference: reference file " << m_referenceFile << " does not exist. No references will be used!");
    m_refFile->Close();
    delete m_refFile;
    return;
  }

  B2INFO("DQMHistReference: use reference file " << m_referenceFile);

  TIter nextkey(m_refFile->GetListOfKeys());
  TKey* key;
  while ((key = (TKey*)nextkey())) {
    if (key->IsFolder() && string(key->GetName()) == string("ref")) {
      TDirectory* refdir = (TDirectory*)key->ReadObj();
      TIter nextDetDir(refdir->GetListOfKeys());
      TKey* detDir;
      // detector folders
      while ((detDir = (TKey*)nextDetDir())) {
        if (!detDir->IsFolder())  continue;
        TIter nextTypeDir(((TDirectory*)detDir->ReadObj())->GetListOfKeys());
        TKey* typeDir;
        TDirectory* foundDir = NULL;
        // run type folders (get the run type corresponding folder or use default one)
        while ((typeDir = (TKey*)nextTypeDir())) {
          if (!typeDir->IsFolder()) continue;
          if (string(typeDir->GetName()) == run_type) {
            foundDir = (TDirectory*)typeDir->ReadObj();
            break;
          }
          if (string(typeDir->GetName()) == "default") foundDir = (TDirectory*)typeDir->ReadObj();
        }
        string dirname = detDir->GetName();
        if (foundDir) B2INFO("Reading reference histograms for " << dirname << " from run type folder: " << foundDir->GetName());
        else {
          B2INFO("No run type specific or default references available for " << dirname);
          continue;
        }

        TIter next(foundDir->GetListOfKeys());
        TKey* hh;

        while ((hh = (TKey*)next())) {
          if (hh->IsFolder()) continue;
          TObject* obj = hh->ReadObj();
          if (obj->IsA()->InheritsFrom("TH1")) {
            TH1* h = (TH1*)obj;
            string histname = h->GetName();
            if (h->GetDimension() == 1) {
              auto n = new REFNODE;
              n->orghist_name = dirname + "/" + histname;
              n->refhist_name = "ref/" + dirname + "/" + histname;
              TH1* histo = (TH1*)h->Clone();
              histo->SetName((n->refhist_name).c_str());
              histo->SetDirectory(0);
              n->ref_clone = histo;
              n->canvas = nullptr;
              m_pnode.push_back(n);
            }
          }
        }
      }
    }
  }

  B2INFO("DQMHistReference: insert reference to m_pnode. size: " << m_pnode.size());
  m_refFile->Close();
  delete m_refFile;

}

void DQMHistReferenceModule::event()
{
  char mbstr[100];

  time_t now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] before ref loop");

  for (auto& it : m_pnode) {

    TH1* hist2 = it->ref_clone;
    if (!hist2) continue;

    hist2->SetLineStyle(2);
    hist2->SetLineColor(3);
    hist2->SetFillColor(0);
    hist2->SetStats(kFALSE);

    TH1* hist1 = findHistInCanvas(it->orghist_name, &(it->canvas));

    TCanvas* canvas = it->canvas;

    // if there is no histogram on canvas we plot the reference anyway.
    if (!canvas) {
      B2DEBUG(1, "No canvas found for refernce histogram " << it->orghist_name);
      continue;
    }
    if (!hist1) {
      B2DEBUG(1, "Canvas is without histogram -> displaying only reference " << it->orghist_name);
      canvas->cd();
      hist2->Draw();
      canvas->Modified();
      canvas->Update();
      continue;
    }

    if (hist1->Integral() == 0) continue;

    /* consider adding coloring option....
      double data = 0;
      if (m_color) {
      data = hist1->KolmogorovTest(hist2, ""); // returns p value (0 bad, 1 good), N - do not compare normalized
      }
    */

    if (abs(hist2->Integral()) > 0) {
      hist2->Scale(hist1->Integral() / hist2->Integral());
    }

    canvas->cd();

    if (hist2->GetMaximum() > hist1->GetMaximum())
      hist1->SetMaximum(1.1 * hist2->GetMaximum());

    hist2->Draw("hist,same");

    canvas->Modified();
    canvas->Update();

  }
  now = time(0);
  strftime(mbstr, sizeof(mbstr), "%c", localtime(&now));
  B2INFO("[" << mbstr << "] after ref loop");


}

void DQMHistReferenceModule::endRun()
{
  B2DEBUG(1, "DQMHistReference: endRun called");
}


void DQMHistReferenceModule::terminate()
{
  B2DEBUG(1, "DQMHistReference: terminate called");
  //  if (m_refFile) delete m_refFile;
}

