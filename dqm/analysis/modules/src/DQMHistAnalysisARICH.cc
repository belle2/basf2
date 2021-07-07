/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisARICH.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TClass.h>
#include <TROOT.h>

#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisARICH);

DQMHistAnalysisARICHModule::DQMHistAnalysisARICHModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Modify and analyze the data quality histograms of ARICH");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("debug", m_debug, "debug mode", false);
  addParam("alert", m_enableAlert, "Enable color alert", true);
}

DQMHistAnalysisARICHModule::~DQMHistAnalysisARICHModule()
{
}

void DQMHistAnalysisARICHModule::initialize()
{
  gROOT->cd();

  //definition of new TObjects for modification and analysis
  for (int i = 0; i < 5; i++) {
    m_LineForMB[i] = new TLine();
    m_LineForMB[i]->SetLineStyle(3);
    m_LineForMB[i]->SetLineWidth(1);
    m_LineForMB[i]->SetLineColor(kBlack);
  }

  m_apdHist = new ARICHChannelHist("tmpChHist", "tmpChHist", 2); /**<ARICH TObject to draw hit map for each APD*/
  m_apdPoly = new TH2Poly();
  m_apdPoly->SetName("ARICH/apdHitMap");
  m_apdPoly->SetTitle("# of hits/APD/event");
  m_apdPoly->SetOption("colz");
  m_c_apdHist = new TCanvas("ARICH/c_apdHist");

  B2DEBUG(20, "DQMHistAnalysisARICH: initialized.");
}

void DQMHistAnalysisARICHModule::beginRun()
{
}

void DQMHistAnalysisARICHModule::event()
{

  //Show alert by empty bins = red and strange entries = yellow
  //Draw lines on mergerHits histogram for shifters to divide sectors
  TH1* m_h_mergerHit = findHist("ARICH/mergerHit");/**<The number of hits in each Merger Boards*/
  m_c_mergerHit = find_canvas("ARICH/c_mergerHit");
  if (m_h_mergerHit != NULL && m_c_mergerHit != NULL) {
    m_c_mergerHit->Clear();
    m_c_mergerHit->cd();
    m_h_mergerHit->SetMinimum(0);
    m_h_mergerHit->Draw("hist");
    gPad->Update();

    int alertMerger = 0;/**<Alert level variable for shifter plot (0:no problem, 1:need to check, 2:contact experts immediately)*/
    double mean = m_h_mergerHit->Integral() / 72;
    for (int i = 0; i < 72; i++) {
      int hit = m_h_mergerHit->GetBinContent(i + 1);
      if ((bool)hit ^ (bool)m_h_mergerHit->GetEntries()) {
        //only if the empty bin is not a masked merger, show alert.
        auto itr = std::find(maskedMergers.begin(), maskedMergers.end(), i + 1);
        if (itr == maskedMergers.end()) {
          alertMerger = 2;
          break;
        }
      }
      if (hit > mean * 100 && alertMerger < 1) alertMerger = 1;
    }
    if (m_enableAlert && m_minStats < m_h_mergerHit->GetEntries()) m_c_mergerHit->SetFillColor(alertColor[alertMerger]);

    //Draw lines divide the sectors
    for (int i = 0; i < 5; i++) {
      m_LineForMB[i]->DrawLine(12 * (i + 1) + 0.5, 0, 12 * (i + 1) + 0.5, gPad->GetUymax());
    }

    m_c_mergerHit->Modified();
  } else {
    B2INFO("Histogram/canvas named mergerHit is not found.");
  }


  //Show alert by the ratio of center 2 bins to side 2bins. <1.5 = red, <2 = yellow
  TH1* m_h_bits = findHist("ARICH/bits");/**<The number of hits in each timing bit*/
  m_c_bits = find_canvas("ARICH/c_bits");
  if (m_h_bits != NULL && m_c_bits != NULL) {
    m_c_bits->Clear();
    m_c_bits->cd();
    m_h_bits->SetMinimum(0);
    m_h_bits->Draw("hist");
    gPad->Update();

    int alertBits = 0;/**<Alert level variable for shifter plot (0:no problem, 1:need to check, 2:contact experts immediately)*/

    double side = m_h_bits->GetBinContent(2) + m_h_bits->GetBinContent(5);
    double center = m_h_bits->GetBinContent(3) + m_h_bits->GetBinContent(4);
    if (center / side < 2) alertBits = 1;
    if (center / side < 1.5) alertBits = 2;
    if (m_enableAlert && m_minStats < m_h_bits->GetEntries()) m_c_bits->SetFillColor(alertColor[alertBits]);

    m_c_bits->Modified();
  } else {
    B2INFO("Histogram/canvas named bits is not found.");
  }

  //Show alert by no entry = red and 0 peak = yellow
  TH1* m_h_hitsPerEvent = findHist("ARICH/hitsPerEvent");/**<The number of hits in each triggered event*/
  m_c_hitsPerEvent = find_canvas("ARICH/c_hitsPerEvent");
  if (m_h_hitsPerEvent != NULL && m_c_hitsPerEvent  != NULL) {
    m_c_hitsPerEvent->Clear();
    m_c_hitsPerEvent->cd();
    m_h_hitsPerEvent->SetMinimum(0);
    m_h_hitsPerEvent->Draw("hist");
    gPad->Update();

    int alertHitsPerEvent = 0;/**<Alert level variable for shifter plot (0:no problem, 1:need to check, 2:contact experts immediately)*/
    double mean = m_h_hitsPerEvent->GetMean();
    if (mean < 1) alertHitsPerEvent = 1;
    double entry = m_h_hitsPerEvent->GetEntries();
    if (entry == 0) alertHitsPerEvent = 2;
    if (m_enableAlert) m_c_hitsPerEvent->SetFillColor(alertColor[alertHitsPerEvent]);

    m_c_hitsPerEvent->Modified();
  } else {
    B2INFO("Histogram/canvas named hitsPerEvent is not found.");
  }

  //Draw 2D hit map of channels and APDs
  TH1* m_h_chHit = findHist("ARICH/chipHit");/**<The number of hits in each chip */
  if (m_h_chHit != NULL) {
    int nevt = 0;
    TH1* htmp = findHist("ARICH/hitsPerEvent");
    if (htmp) nevt = htmp->GetEntries();
    m_apdHist->fillFromTH1(m_h_chHit);
    if (nevt) m_apdHist->Scale(1. / float(nevt));
    m_apdPoly->SetMaximum(0.1);
    m_apdHist->setPoly(m_apdPoly);
    m_apdPoly->SetMinimum(0.0001);
    m_c_apdHist->Clear();
    m_c_apdHist->cd();
    m_apdPoly->Draw("colz");
    m_apdPoly->GetXaxis()->SetTickLength(0);
    m_apdPoly->GetYaxis()->SetTickLength(0);
    gPad->SetLogz();
    m_c_apdHist->Update();
  } else {
    B2INFO("Histogram named chipHit is not found.");
  }

}

void DQMHistAnalysisARICHModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisARICH : endRun called");
}

void DQMHistAnalysisARICHModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

TCanvas* DQMHistAnalysisARICHModule::find_canvas(TString canvas_name)
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
