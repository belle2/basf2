/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisTOP.h>
#include <boost/format.hpp>
#include <daq/slc/base/StringUtil.h>
#include <TClass.h>
#include <TDirectory.h>
#include "TROOT.h"

using namespace std;
using namespace Belle2;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisTOP)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisTOPModule::DQMHistAnalysisTOPModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(20, "DQMHistAnalysisTOP: Constructor done.");
}


DQMHistAnalysisTOPModule::~DQMHistAnalysisTOPModule() { }

void DQMHistAnalysisTOPModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistAnalysisTOP: initialized.");

  m_c_goodHitsMean = new TCanvas("TOP/c_good_hits_mean");
  m_c_goodHitsRMS = new TCanvas("TOP/c_good_hits_rms");
  m_c_badHitsMean = new TCanvas("TOP/c_bad_hits_mean");
  m_c_badHitsRMS = new TCanvas("TOP/c_bad_hits_rms");

  m_h_goodHitsMean = new TH1F("TOP/good_hits_mean", "Good hits per event vs. slot number", 16, 0.5, 16.5);
  m_h_goodHitsRMS = new TH1F("TOP/good_hits_rms", "RMS of good hits per event", 16, 0.5, 16.5);
  m_h_badHitsMean = new TH1F("TOP/bad_hits_mean", "Mean of bad hits per event", 16, 0.5, 16.5);
  m_h_badHitsRMS = new TH1F("TOP/bad_hits_rms", "RMS of bad hits per event", 16, 0.5, 16.5);

  m_h_goodHitsMean->GetXaxis()->SetTitle("slot number");
  m_h_goodHitsMean->GetYaxis()->SetTitle("hits per event");
  m_h_goodHitsRMS->GetXaxis()->SetTitle("slot number");
  m_h_goodHitsRMS->GetYaxis()->SetTitle("hits per event");

  m_h_badHitsMean->GetXaxis()->SetTitle("slot number");
  m_h_badHitsMean->GetYaxis()->SetTitle("hits per event");
  m_h_badHitsRMS->GetXaxis()->SetTitle("slot number");
  m_h_badHitsRMS->GetYaxis()->SetTitle("hits per event");

  m_h_goodHitsMean->SetStats(kFALSE);
  m_h_goodHitsRMS->SetStats(kFALSE);
  m_h_badHitsMean->SetStats(kFALSE);
  m_h_badHitsRMS->SetStats(kFALSE);

  m_h_goodHitsMean->SetMinimum(0);
  m_h_goodHitsRMS->SetMinimum(0);
  m_h_badHitsMean->SetMinimum(0);
  m_h_badHitsRMS->SetMinimum(0);

  m_line1 = new TLine(0.5, 220, 16.5, 220);
  m_line2 = new TLine(0.5, 250, 16.5, 250);
  m_line1->SetLineWidth(2);
  m_line2->SetLineWidth(2);
  m_line1->SetLineColor(kRed);
  m_line2->SetLineColor(kRed);

  m_text1 = new TPaveText(2, 400, 8, 500, "NB");
  m_text1->SetFillColor(kWhite);
  m_text1->SetBorderSize(0);
  m_text1->AddText("Expect no entries outside of red lines");
}


void DQMHistAnalysisTOPModule::beginRun()
{
  //B2DEBUG(20, "DQMHistAnalysisTOP: beginRun called.");
}

TCanvas* DQMHistAnalysisTOPModule::find_canvas(TString canvas_name)
{
  TIter nextckey(gROOT->GetListOfCanvases());
  TObject* cobj = NULL;

  while ((cobj = (TObject*)nextckey())) {
    if (cobj->IsA()->InheritsFrom("TCanvas")) {
      if (cobj->GetName() == canvas_name)
        break;
    }
  }
  return (TCanvas*)cobj;
}

TH1* DQMHistAnalysisTOPModule::find_histo_in_canvas(TString histo_name)
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

void DQMHistAnalysisTOPModule::event()
{
  for (int i = 1; i <= 16; i++) {
    string hname1 = str(format("TOP/good_hits_per_event%1%") % (i));;
    string hname2 = str(format("TOP/bad_hits_per_event%1%") % (i));;
    TH1* h1 = findHist(hname1);
    TH1* h2 = findHist(hname2);
    //TH1* h1 = find_histo_in_canvas(hname1);
    //TH1* h2 = find_histo_in_canvas(hname2);
    if (h1 != NULL) {
      m_h_goodHitsMean->SetBinContent(i, h1->GetMean());
      m_h_goodHitsRMS->SetBinContent(i, h1->GetRMS());
    } else {
      m_h_goodHitsMean->SetBinContent(i, 0);
      m_h_goodHitsRMS->SetBinContent(i, 0);
    }
    if (h2 != NULL) {
      m_h_badHitsMean->SetBinContent(i, h2->GetMean());
      m_h_badHitsRMS->SetBinContent(i, h2->GetRMS());
    } else {
      m_h_badHitsMean->SetBinContent(i, 0);
      m_h_badHitsRMS->SetBinContent(i, 0);
    }
  }

  //addHist("", m_h_goodHitsMean->GetName(), m_h_goodHitsMean);

  TCanvas* c1 = find_canvas("TOP/c_hitsPerEvent");
  //TH1* h1=find_histo_in_canvas("TOP/hitsPerEvent");
  if (c1 != NULL) {
    c1->SetName("TOP/c_hitsPerEvent_top");
  }
  //if (h1!=NULL) {
  //  h1->SetName("TOP/hitsPerEvent_top");
  //}

  TCanvas* c2 = find_canvas("TOP/c_window_vs_slot");
  if (c2 != NULL) {
    c2->cd();
    m_line1->Draw();
    m_line2->Draw();
    m_text1->Draw();
  }

  m_c_goodHitsMean->Clear();
  m_c_goodHitsMean->cd();
  m_h_goodHitsMean->Draw();
  m_c_goodHitsMean->Modified();

  m_c_goodHitsRMS->Clear();
  m_c_goodHitsRMS->cd();
  m_h_goodHitsRMS->Draw();
  m_c_goodHitsRMS->Modified();

  m_c_badHitsMean->Clear();
  m_c_badHitsMean->cd();
  m_h_badHitsMean->Draw();
  m_c_badHitsMean->Modified();

  m_c_badHitsRMS->Clear();
  m_c_badHitsRMS->cd();
  m_h_badHitsRMS->Draw();
  m_c_badHitsRMS->Modified();
}

void DQMHistAnalysisTOPModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisTOP : endRun called");
}


void DQMHistAnalysisTOPModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

