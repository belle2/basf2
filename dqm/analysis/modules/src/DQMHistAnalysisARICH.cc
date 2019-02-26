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

  B2DEBUG(20, "DQMHistAnalysisARICH: initialized.");
}

void DQMHistAnalysisARICHModule::beginRun()
{
}

void DQMHistAnalysisARICHModule::event()
{
  //Draw lines on mergerHits histogram for shifters to divide sectors
  m_h_mergerHit = findHist("ARICH/mergerHit");
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

}

void DQMHistAnalysisARICHModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisARICH : endRun called");
}

void DQMHistAnalysisARICHModule::terminate()
{

  B2DEBUG(20, "terminate called");
}


