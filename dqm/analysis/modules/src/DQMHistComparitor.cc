/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistComparitor.cc
// Description : DQM Histogram analysis module, compares histo with references
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistComparitor.h>
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
REG_MODULE(DQMHistComparitor);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistComparitorModule::DQMHistComparitorModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoList", m_histlist, "['histname', 'refhistname', 'canvas', 'minentry', 'warnlvl', 'errlvl', 'pvname'],[...],...");
  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("refHisto.root"));
  addParam("ColorAlert", m_color, "Whether to show the color alert", true);
  B2DEBUG(1, "DQMHistComparitor: Constructor done.");
}

DQMHistComparitorModule::~DQMHistComparitorModule()
{
}

void DQMHistComparitorModule::initialize()
{
  m_refFile = NULL;
  if (m_refFileName != "") {
    m_refFile = new TFile(m_refFileName.data());
  }

  gStyle->SetOptStat(0);
  gStyle->SetStatStyle(1);
  gStyle->SetOptDate(22);// Date and Time in Bottom Right, does no work

  for (auto& it : m_histlist) {
    if (it.size() != 7) {
      B2WARNING("Histolist with wrong nr of parameters (" << it.size() << "), hist1=" << it.at(0));
      continue;
    }

    auto n = new CMPNODE;
    n->histo1 = it.at(0);
    n->histo2 = it.at(1);
    TCanvas* c = new TCanvas(it.at(2).c_str());
    n->canvas = c;

    n->min_entries = atoi(it.at(3).c_str());
    n->warning = atof(it.at(4).c_str());
    n->error = atof(it.at(5).c_str());
    n->pvname = it.at(6);
    if (n->pvname != "") {
      registerEpicsPV(n->pvname);
    }
    m_pnode.push_back(n);
  }

  B2DEBUG(20, "DQMHistComparitor: initialized.");
}


void DQMHistComparitorModule::beginRun()
{
  B2DEBUG(20, "DQMHistComparitor: beginRun called.");
}

void DQMHistComparitorModule::event()
{
  for (auto& it : m_pnode) {

    TH1* hist1, *hist2;

    B2DEBUG(20, "== Search for " << it->histo1 << " with ref " << it->histo2 << "==");
    hist1 = findHist(it->histo1, true); // only if changed
    if (!hist1) B2DEBUG(20, "NOT Found " << it->histo1);
    if (!hist1) continue;
    hist2 = findRefHist(it->histo2, ERefScaling::c_RefScaleEntries, hist1);
    if (!hist2) B2DEBUG(20, "NOT Found " << it->histo2);
    // Dont compare if any of hists is missing ... TODO We should clean CANVAS, raise some error if we cannot compare
    if (!hist2) continue;

    B2DEBUG(20, "Compare " << it->histo1 << " with ref " << it->histo2);
    // if compare normalized ... does not work!
    // hist2->Scale(hist1->GetEntries()/hist2->GetEntries());

    double data = hist1->KolmogorovTest(hist2, ""); // returns p value (0 bad, 1 good), N - do not compare normalized
    //     data = hist1->Chi2Test(hist2);// return p value (0 bad, 1 good), ignores normalization
    //     data= BinByBinTest(hits1,hist2);// user function (like Peters test)
    //     printf(" %.2f %.2f %.2f\n",(float)data,it->warning,it->error);
    if (it->pvname != "") setEpicsPV(it->pvname, data);
    it->canvas->cd();
    hist2->SetLineStyle(3);// 2 or 3
    hist2->SetLineColor(3);

    TIter nextkey(it->canvas->GetListOfPrimitives());
    TObject* obj = NULL;
    while ((obj = (TObject*)nextkey())) {
      if (obj->IsA()->InheritsFrom("TH1")) {
        if (string(obj->GetName()) == string(hist2->GetName())) {
          delete obj;
        }
      }
    }

    hist2->Draw("hist");
    hist2->SetFillColor(0);
    hist2->SetStats(kFALSE);
    hist1->SetFillColor(0);
    if (hist2->GetMaximum() > hist1->GetMaximum())
      hist1->SetMaximum(1.1 * hist2->GetMaximum());
    hist1->Draw("hist");
    hist2->Draw("hist,same");

    it->canvas->Pad()->SetFrameFillColor(10);
    if (m_color) {
      if (hist1->GetEntries() < it->min_entries) {
        // not enough Entries
        it->canvas->Pad()->SetFillColor(6);// Magenta
      } else {
        if (data < it->error) {
          it->canvas->Pad()->SetFillColor(2);// Red
        } else if (data < it->warning) {
          it->canvas->Pad()->SetFillColor(5);// Yellow
        } else {
          it->canvas->Pad()->SetFillColor(0);// White
        }
      }
    } else {
      it->canvas->Pad()->SetFillColor(0);// White
    }
    it->canvas->Modified();
    it->canvas->Update();
  }
}

void DQMHistComparitorModule::endRun()
{
  B2DEBUG(20, "DQMHistComparitor: endRun called");
}


void DQMHistComparitorModule::terminate()
{
  B2DEBUG(20, "DQMHistComparitor: terminate called");
  if (m_refFile) delete m_refFile;
}
