/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki, Luka Santelj                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisARICH.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

// framework
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// Dataobject classes
#include <framework/database/DBObjPtr.h>
//#include <framework/dbobjects/RunInfo.h>


#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TColor.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TClass.h>
#include <TStyle.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TImage.h>
#include <TPad.h>

#include <sstream>
#include <fstream>
#include <math.h>
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
  m_c_mergerHit = new TCanvas("ARICH/c_mergerHitModified");

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

  //Draw lines on mergerHits histogram for shifters to divide sectors
  TH1* m_h_mergerHit = findHist("ARICH/mergerHit");/**<The number of hits in each Merger Boards*/
  if (m_h_mergerHit != NULL) {
    m_c_mergerHit->Clear();
    m_c_mergerHit->cd();
    m_h_mergerHit->Draw("hist");
    gPad->Update();
    for (int i = 0; i < 5; i++) {
      m_LineForMB[i]->DrawLine(12 * (i + 1) + 0.5, 0, 12 * (i + 1) + 0.5, gPad->GetUymax());
    }
    m_c_mergerHit->Modified();
  } else {
    B2INFO("Histogram named mergerHit is not found.");
  }

  //Draw 2D hit map of channels and APDs
  TH1* m_h_chHit = findHist("ARICH/chipHit");/**<The number of hits in each chip */
  if (m_h_chHit != NULL) {
    int nevt = 0;
    TH1* htmp = findHist("ARICH/hitsPerEvent");
    if (htmp) nevt = htmp->GetEntries();
    m_apdHist->fillFromTH1(m_h_chHit);
    if (nevt) m_apdHist->Scale(1. / float(nevt));
    m_apdHist->setPoly(m_apdPoly);
    m_c_apdHist->Clear();
    m_c_apdHist->cd();
    m_apdPoly->SetMaximum(0.01);
    m_apdPoly->SetMinimum(0.0);
    m_apdPoly->Draw("colz");
    m_apdPoly->GetXaxis()->SetTickLength(0);
    m_apdPoly->GetYaxis()->SetTickLength(0);
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


