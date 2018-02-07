//+
// File : DQMHistComparitor.cc
// Description : DQM Histogram analysis module, compares histo with references
//
// Author : Bjoern Spruck, Uni Mainz
// Date : yesterday
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistComparitor.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TClass.h>
#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TKey.h>
#include <boost/lexical_cast.hpp>

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
  B2DEBUG(1, "DQMHistComparitor: Constructor done.");
}


DQMHistComparitorModule::~DQMHistComparitorModule() { }


TH1* DQMHistComparitorModule::GetHisto(TString histoname)
{
  TH1* hh1;
  hh1 = findHist(histoname.Data());
  if (hh1 == NULL) {
    B2INFO("Histo " << histoname << " not in memfile");
    // the following code sux ... is there no root function for that?


    // first search reference root file ... if ther is one
    if (m_refFile && m_refFile->IsOpen()) {
      TDirectory* d = m_refFile;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      B2INFO(myl);
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2INFO("Cd Dir " << tok);
            d = e;
          } else {
            B2INFO("cd failed " << tok);
          }
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2INFO("Histo " << histoname << " found in ref file");
          hh1 = (TH1*)obj;
        } else {
          B2INFO("Histo " << histoname << " found in ref file but wrong type");
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
              B2INFO("Histo " << histoname << " found as key -> readobj");
              break;
            }
          }
        }
        if (hh1 == NULL) B2INFO("Histo " << histoname << " NOT found in ref file " << tok);
      }
    }

    if (hh1 == NULL) {
      B2INFO("Histo " << histoname << " not in memfile or ref file");
      // the following code sux ... is there no root function for that?

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
            B2INFO("Cd Dir " << tok);
            d = e;
          } else B2INFO("cd failed " << tok);
          d->cd();
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2INFO("Histo " << histoname << " found in mem");
          hh1 = (TH1*)obj;
        }
      } else {
        B2INFO("Histo " << histoname << " NOT found in mem");
      }
    }
  }

  if (hh1 == NULL) {
    B2INFO("Histo " << histoname << " not found");
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
  SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
#endif
  for (auto& it : m_histlist) {
    if (it.size() != 7) {
      B2WARNING("Histolist with wrong nr of parameters (" << it.size() << "), hist1=" << it.at(0));
      continue;
    }

    TH1* hist1, *hist2;

    hist1 = GetHisto(it.at(0));
    if (!hist1) continue;
    hist2 = GetHisto(it.at(1));
    if (!hist2) continue;
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

    hist1 = GetHisto(it->histo1);
    if (!hist1) continue;
    hist2 = GetHisto(it->histo2);
    if (!hist2) continue;

    // if compare normalized ... does not work!
//     hist2->Scale(hist1->GetEntries()/hist2->GetEntries());

    double data = 0.0;
    data = hist1->KolmogorovTest(hist2, ""); // returns p value (0 bad, 1 good), N - do not compare normalized
//     data = hist1->Chi2Test(hist2);// return p value (0 bad, 1 good), ignores normalization
//     data= BinByBinTest(hits1,hist2);// user function (like Peters test)
//     printf(" %.2f %.2f %.2f\n",(float)data,it->warning,it->error);
#ifdef _BELLE2_EPICS
    if (it->epicsflag) SEVCHK(ca_put(DBR_DOUBLE, it->mychid, (void*)&data), "ca_set failure");
#endif
    it->canvas->cd();
    hist2->SetLineStyle(2);// 2 or 3
    hist2->SetLineColor(1);
    // if draw normalized
    if (1) {
      TH1* h = (TH1*)hist2->Clone(); // Anoying ... Maybe an memory leak? TODO
      h->Scale(hist1->GetEntries() / hist2->GetEntries());
      h->Draw();
    } else {
      hist2->Draw("hist");
    }
    hist1->Draw("hist,same");
    it->canvas->Pad()->SetFrameFillStyle(1);
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
