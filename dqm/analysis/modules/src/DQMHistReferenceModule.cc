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
#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>
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
  addParam("ListRefFiles", m_listRefFiles, "List of reference histrogram files");
  B2DEBUG(1, "DQMHistReference: Constructor done.");
}


DQMHistReferenceModule::~DQMHistReferenceModule() { }

TH1* DQMHistReferenceModule::find_histo_in_canvas(REFNODE* node)
{

  if (!node->canvas) {
    node->canvas = find_canvas(node->canvas_name);
  }
  if (!node->canvas) return nullptr;

  TIter nextkey((node->canvas)->GetListOfPrimitives());
  TObject* obj = nullptr;

  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TH1")) {
      if (obj->GetName() == node->histo1) {
        return (TH1*)obj;
      }
    }
  }
  return nullptr;
}

TCanvas* DQMHistReferenceModule::find_canvas(TString canvas_name)
{

  TIter nextckey(gROOT->GetListOfCanvases());
  TObject* cobj = nullptr;

  while ((cobj = (TObject*)nextckey())) {
    if (cobj->IsA()->InheritsFrom("TCanvas")) {
      if (cobj->GetName() == canvas_name) {
        return (TCanvas*)cobj;
      }
    }
  }
  return nullptr;
}

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
    auto search = m_listRefFiles.find(run_type);
    if (search == m_listRefFiles.end()) {
      run_type = "default";
    }
  }
  B2INFO("DQMHistReference: run_type " << run_type);

  if (m_refFile != NULL) delete m_refFile;
  m_refFile = new TFile(m_listRefFiles[run_type].c_str());
  B2INFO("DQMHistReference: use reference file " << m_listRefFiles[run_type]);

  for (auto& it : m_pnode) {
    // clear ref histos from memory
    delete it->ref_clone;
    delete it;
  }
  m_pnode.clear();
  B2INFO("DQMHistReference: clear m_pnode. size: " << m_pnode.size());

  TIter nextkey(m_refFile->GetListOfKeys());
  TKey* key;
  while ((key = (TKey*)nextkey())) {
    if (key->IsFolder() && string(key->GetName()) == string("ref")) {
      TDirectory* refdir = (TDirectory*)key->ReadObj();
      TIter nextdir(refdir->GetListOfKeys());
      TKey* dir;

      while ((dir = (TKey*)nextdir())) {
        if (dir->IsFolder()) {
          string dirname = dir->GetName();
          TIter next(((TDirectory*)dir->ReadObj())->GetListOfKeys());
          TKey* hh;

          while ((hh = (TKey*)next())) {
            if (!hh->IsFolder()) {
              TObject* obj = hh->ReadObj();
              if (obj->IsA()->InheritsFrom("TH1")) {
                TH1* h = (TH1*)obj;
                string histname = h->GetName();
                if (h->GetDimension() == 1) {
                  auto n = new REFNODE;
                  n->histo1 = dirname + "/" + histname;
                  n->histo2 = "ref/" + dirname + "/" + histname;
                  TH1* histo = (TH1*)h->Clone();
                  histo->SetName(n->histo2);
                  histo->SetDirectory(0);
                  n->ref_clone = histo;
                  n->canvas_name = dirname + "/c_" + histname;
                  n->canvas = nullptr;
                  m_pnode.push_back(n);
                }
              }
            }
          }

        }
      }
    }
  }
  B2INFO("DQMHistReference: insert reference to m_pnode. size: " << m_pnode.size());
  m_refFile->Close();
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

    TH1* hist1 = find_histo_in_canvas(it);

    TCanvas* canvas = it->canvas;

    // if there is no histogram on canvas we plot the reference anyway.
    if (!canvas) {
      B2DEBUG(1, "No canvas found for refernce histogram " << it->canvas_name);
      continue;
    }
    if (!hist1) {
      B2DEBUG(1, "Canvas is without histogram -> displaying only reference " << it->histo1);
      canvas->cd();
      hist2->Draw();
      canvas->Modified();
      canvas->Update();
      continue;
    }

    if (hist1->GetDimension() != 1) continue;
    if (hist1->Integral() == 0) continue;

    B2DEBUG(1, "Compare " << it->histo1 << " with ref " << it->histo2);

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
  if (m_refFile) delete m_refFile;
}

