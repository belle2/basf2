//+
// File : DQMHistAnalysisPlotOnly.cc
// Description : DQM ANalalysis, plots a list of histograms into canvases
//
// Author : B. Spruck
// Date : 13 - Oct - 2017
//-


#include <dqm/analysis/modules/DQMHistAnalysisPlotOnly.h>
#include <TSystem.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPlotOnly)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


DQMHistAnalysisPlotOnlyModule::DQMHistAnalysisPlotOnlyModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoList", m_histlist, "histname [, canvasname]");
  B2DEBUG(1, "DQMHistAnalysisPlotOnly: Constructor done.");
}

TH1* DQMHistAnalysisPlotOnlyModule::GetHisto(TString histoname)
{
  TH1* hh1;
  hh1 = findHist(histoname.Data());
  if (hh1 == NULL) {
    B2DEBUG(20, "Histo " << histoname << " not in memfile");
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
          B2DEBUG(20, "Cd Dir " << tok);
          d = e;
        }
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

  if (hh1 == NULL) {
    B2DEBUG(20, "Histo " << histoname << " not found");
  }

  return hh1;
}

DQMHistAnalysisPlotOnlyModule::~DQMHistAnalysisPlotOnlyModule() { }

void DQMHistAnalysisPlotOnlyModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisPlotOnly: initialized.");

  m_canvasList.clear();
  for (auto& it : m_histlist) {
    TString a = "";
    if (it.size() == 1) {
      a = TString(it.at(0).c_str());
      auto p = a.Last('/');
      if (p == kNPOS) a = "c_" + a;
      else a.Insert(p + 1, "c_");
    } else if (it.size() == 2) {
      a = it.at(1).c_str();
    } else continue;
    TCanvas* c = new TCanvas(a);
    m_canvasList[it.at(0)] = c;
  }
}


void DQMHistAnalysisPlotOnlyModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisPlotOnly: beginRun called.");
}

void DQMHistAnalysisPlotOnlyModule::event()
{
  for (auto& it : m_canvasList) {
    gROOT->cd();
//    TH1 *hh1 = findHist(TString(it.first.c_str()));
    TH1* hh1 = GetHisto(it.first.c_str());
    if (hh1 == NULL) {
      B2DEBUG(10, "Hist " << it.first.c_str() << " not found");
      continue;
//         hh1 = findHistLocal(it.first);
    }

//    if (it.second) {
    it.second->cd();
    if (hh1) {
      if (hh1->GetDimension() == 1) {
        hh1->Draw("hist");
      } else if (hh1->GetDimension() == 2) {
        hh1->Draw("colz");
      }
    }
    it.second->Modified();
    it.second->Update();
//    }

  }
}

void DQMHistAnalysisPlotOnlyModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisPlotOnly: endRun called");
}


void DQMHistAnalysisPlotOnlyModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisPlotOnly: terminate called");
}

