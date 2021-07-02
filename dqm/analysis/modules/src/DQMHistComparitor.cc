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
#include <daq/slc/base/StringUtil.h>
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
REG_MODULE(DQMHistComparitor)

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
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

TH1* DQMHistComparitorModule::find_histo_in_canvas(TString histo_name)
{
  StringList s = StringUtil::split(histo_name.Data(), '/');
  std::string dirname = s[0];
  std::string hname = s[1];
  std::string canvas_name = dirname + "/c_" + hname;

  TIter nextckey(gROOT->GetListOfCanvases());
  TObject* cobj = NULL;

  while ((cobj = (TObject*)nextckey())) {
    if (cobj->IsA()->InheritsFrom("TCanvas")) {
      if (cobj->GetName() == canvas_name)
        break;
    }
  }
  if (cobj == NULL) return NULL;

  TIter nextkey(((TCanvas*)cobj)->GetListOfPrimitives());
  TObject* obj = NULL;

  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TH1")) {
      if (obj->GetName() == histo_name)
        return (TH1*)obj;
    }
  }
  return NULL;
}

TH1* DQMHistComparitorModule::GetHisto(TString histoname)
{
  TH1* hh1;
  gROOT->cd();
  hh1 = findHist(histoname.Data());
  if (hh1 == NULL) {
    B2DEBUG(20, "findHisto failed " << histoname << " not in memfile");

    // first search reference root file ... if ther is one
    if (m_refFile && m_refFile->IsOpen()) {
      TDirectory* d = m_refFile;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      B2DEBUG(20, myl);
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2DEBUG(20, "Cd Dir " << tok << " from " << d->GetPath());
            d = e;
          } else {
            B2DEBUG(20, "cd failed " << tok << " from " << d->GetPath());
          }
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2DEBUG(20, "Histo " << histoname << " found in ref file");
          hh1 = (TH1*)obj;
        } else {
          B2DEBUG(20, "Histo " << histoname << " found in ref file but wrong type");
        }
      } else {
        // seems find will only find objects, not keys, thus get the object on first access
        TIter next(d->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)next())) {
          TObject* obj2 = key->ReadObj() ;
          if (obj2->InheritsFrom("TH1")) {
            if (obj2->GetName() == tok) {
              hh1 = (TH1*)obj2;
              B2DEBUG(20, "Histo " << histoname << " found as key -> readobj");
              break;
            }
          }
        }
        if (hh1 == NULL) B2DEBUG(20, "Histo " << histoname << " NOT found in ref file " << tok);
      }
    }

    if (hh1 == NULL) {
      B2DEBUG(20, "Histo " << histoname << " not in memfile or ref file");

      TDirectory* d = gROOT;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2DEBUG(20, "Cd Dir " << tok);
            d = e;
          } else B2DEBUG(20, "cd failed " << tok);
          d->cd();
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2DEBUG(20, "Histo " << histoname << " found in mem");
          hh1 = (TH1*)obj;
        }
      } else {
        B2DEBUG(20, "Histo " << histoname << " NOT found in mem");
      }
    }
  }

  if (hh1 == NULL) {
    B2DEBUG(20, "Histo " << histoname << " not found");
  }

  return hh1;
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

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
#endif
  for (auto& it : m_histlist) {
    if (it.size() != 7) {
      B2WARNING("Histolist with wrong nr of parameters (" << it.size() << "), hist1=" << it.at(0));
      continue;
    }

    // Do not check for histogram existance here, as it might not be
    // in memfile when analysis task is started
    TString a = it.at(2).c_str();

    auto n = new CMPNODE;
    n->histo1 = it.at(0);
    n->histo2 = it.at(1);
    TCanvas* c = new TCanvas(a);
    n->canvas = c;

    n->min_entries = atoi(it.at(3).c_str());
    n->warning = atof(it.at(4).c_str());
    n->error = atof(it.at(5).c_str());
    n->epicsflag = false;

#ifdef _BELLE2_EPICS
    if (it.at(6) != "") {
      SEVCHK(ca_create_channel(it.at(6).c_str(), NULL, NULL, 10, &n->mychid), "ca_create_channel failure");
      n->epicsflag = true;
    }
#endif
    m_pnode.push_back(n);
  }
#ifdef _BELLE2_EPICS
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif

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
    hist1 = find_histo_in_canvas(it->histo1);
    if (!hist1) B2DEBUG(20, "NOT Found " << it->histo1);
    hist2 = GetHisto(it->histo2);
    if (!hist2) B2DEBUG(20, "NOT Found " << it->histo2);
    // Dont compare if any of hists is missing ... TODO We should clean CANVAS, raise some error if we cannot compare
    if (!hist1) continue;
    if (!hist2) continue;

    B2DEBUG(20, "Compare " << it->histo1 << " with ref " << it->histo2);
    // if compare normalized ... does not work!
    // hist2->Scale(hist1->GetEntries()/hist2->GetEntries());

    double data = hist1->KolmogorovTest(hist2, ""); // returns p value (0 bad, 1 good), N - do not compare normalized
    //     data = hist1->Chi2Test(hist2);// return p value (0 bad, 1 good), ignores normalization
    //     data= BinByBinTest(hits1,hist2);// user function (like Peters test)
    //     printf(" %.2f %.2f %.2f\n",(float)data,it->warning,it->error);
#ifdef _BELLE2_EPICS
    if (it->epicsflag) SEVCHK(ca_put(DBR_DOUBLE, it->mychid, (void*)&data), "ca_set failure");
#endif
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

    // if draw normalized
    TH1* h;
    if (1) {
      h = (TH1*)hist2->Clone(); // Anoying ... Maybe an memory leak? TODO
      if (abs(hist2->Integral()) > 0)
        h->Scale(hist1->Integral() / hist2->Integral());
    } else {
      hist2->Draw("hist");
    }

    h->SetFillColor(0);
    h->SetStats(kFALSE);
    hist1->SetFillColor(0);
    if (h->GetMaximum() > hist1->GetMaximum())
      hist1->SetMaximum(1.1 * h->GetMaximum());
    hist1->Draw("hist");
    h->Draw("hist,same");

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
#ifdef _BELLE2_EPICS
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
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
