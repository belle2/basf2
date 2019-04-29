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

  channelHist = new Belle2::ARICHChannelHist("ARICHExpert/chHist",
                                             "# of hits/channel"); /**<ARICH TObject to draw hit map for each channel*/
  m_c_channelHist = new TCanvas("ARICHExpert/c_channelHist");
  apdHist = new Belle2::ARICHChannelHist("ARICHExpert/apdHist", "# of hits/APD", 2); /**<ARICH TObject to draw hit map for each APD*/
  m_c_apdHist = new TCanvas("ARICHExpert/c_apdHist");
  hapdHist = new Belle2::ARICHChannelHist("ARICHExpert/hapdHist", "# of hits/HAPD",
                                          1); /**<ARICH TObject to draw hit map for each HAPD*/
  m_c_hapdHist = new TCanvas("ARICHExpert/c_hapdHist");

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
    m_h_mergerHit->Draw();
    gPad->Update();
    for (int i = 0; i < 5; i++) {
      m_LineForMB[i]->DrawLine(12 * (i + 1) + 0.5, 0, 12 * (i + 1) + 0.5, gPad->GetUymax());
    }
    m_c_mergerHit->Modified();
  } else {
    B2INFO("Histogram named mergerHit is not found.");
  }

  //Draw 2D hit map of channels and APDs
  TH1* m_h_chHit = findHist("ARICH/chHit");/**<The number of hits in each channels*/
  if (m_h_chHit != NULL) {
    for (int i = 1; i < 421; i++) {
      int apdHit[4] = {};
      for (int j = 0; j < 144; j++) {
        int ch = (i - 1) * 144 + j;
        channelHist->setBinContent(i, j, m_h_chHit->GetBinContent(ch + 1));
        apdHit[j / 36] += m_h_chHit->GetBinContent(ch + 1);
      }
      for (int j = 0; j < 4; j++) {
        apdHist->setBinContent(i, j, apdHit[j]);
      }
    }
    m_c_channelHist->Clear();
    m_c_channelHist->cd();
    channelHist->Draw();
    m_c_apdHist->Clear();
    m_c_apdHist->cd();
    apdHist->Draw();
  } else {
    B2INFO("Histogram named chHit is not found.");
  }

  //Draw 2D hit map of HAPDs
  TH1* m_h_hapdHit = findHist("ARICH/hapdHit");/**<The number of hits in each HAPDs*/
  if (m_h_hapdHit != NULL) {
    for (int i = 1; i < 421; i++) {
      hapdHist->setBinContent(i, m_h_hapdHit->GetBinContent(i));
    }
    m_c_hapdHist->Clear();
    m_c_hapdHist->cd();
    hapdHist->Draw();
  } else {
    B2INFO("Histogram named hapdHit is not found.");
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


