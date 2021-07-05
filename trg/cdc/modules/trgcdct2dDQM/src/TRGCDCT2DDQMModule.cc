/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGCDCT2DDQMModule.cc
// Section  : TRG CDCT2D
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG CDCT2DDQM
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <trg/cdc/modules/trgcdct2dDQM/TRGCDCT2DDQMModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
//#include <framework/dbobjects/RunInfo.h>
#include <framework/datastore/DataStore.h>

#include <TDirectory.h>
#include <TRandom3.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Belle2;

REG_MODULE(TRGCDCT2DDQM);


TRGCDCT2DDQMModule::TRGCDCT2DDQMModule() : HistoModule()
{

  setDescription("DQM for CDCT2D Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("generatePostscript", m_generatePostscript,
           "Genarete postscript file or not",
           false);
  addParam("postScriptName", m_postScriptName,
           "postscript file name",
           string("t2ddqm.ps"));


}

void TRGCDCT2DDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = NULL;
  //dirDQM = oldDir->mkdir("TRGCDCT2D");
  if (!oldDir->Get("TRGCDCT2D"))dirDQM = oldDir->mkdir("TRGCDCT2D");
  else dirDQM = (TDirectory*)oldDir->Get("TRGCDCT2D");
  dirDQM->cd();

  //TSF hit distribution as a function of tsfid
  h_tsfhit = new TH1I("hCDCT2D_tsfhit", "hCDCT2D_tsfhit", 2500, 0, 2500);
  h_tsfhit->SetTitle("TSF hit in 2D module");
  h_tsfhit->GetXaxis()->SetTitle("TSF ID");

  //2D phi distribution
  h_phi = new TH1D("hCDCT2D_phi", "hCDCT2D_phi", 80, -1, 7);
  h_phi->SetTitle("2D track phi");
  h_phi->GetXaxis()->SetTitle("rad");

  //2D pt distribution
  h_pt = new TH1D("hCDCT2D_pt", "hCDCT2D_pt", 30, 0, 3);
  h_pt->SetTitle("2D track pt");
  h_pt->GetXaxis()->SetTitle("GeV");

  //2D omega distribution
  h_omega = new TH1D("hCDCT2D_omega", "hCDCT2D_omega", 80, -0.02, 0.02);
  h_omega->SetTitle("2D track omega");
  h_omega->GetXaxis()->SetTitle("");


  //2D foundtime distribution
  h_time = new TH1D("hCDCT2D_time", "hCDCT2D_time", 100, -50, 50);
  h_time->SetTitle("2D track foundtime");
  h_time->GetXaxis()->SetTitle("CLK 32ns");

  oldDir->cd();
}

void TRGCDCT2DDQMModule::beginRun()
{

  dirDQM->cd();

  h_tsfhit->Reset();
  h_phi->Reset();
  h_pt->Reset();
  h_omega->Reset();
  h_time->Reset();

  oldDir->cd();
}

void TRGCDCT2DDQMModule::initialize()
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM

  char c_name_tsf[100];
  sprintf(c_name_tsf, "CDCTriggerSegmentHits");
  entAry_tsf.isRequired(c_name_tsf);
  if (!entAry_tsf || !entAry_tsf.getEntries()) return;
  char c_name_t2d[100];
  sprintf(c_name_t2d, "CDCTrigger2DFinderTracks");
  entAry_t2d.isRequired(c_name_t2d);
  if (!entAry_t2d || !entAry_t2d.getEntries()) return;

}

void TRGCDCT2DDQMModule::endRun()
{
  dirDQM->cd();

  //Draw and save histograms
  if (m_generatePostscript) {
    gStyle->SetOptStat(0);
    TCanvas c1("c1", "", 0, 0, 500, 300);
    c1.cd();

    TPostScript* ps_tsfhit = new TPostScript((m_postScriptName + ".tsfhit" + ".ps").c_str(), 112);
    h_tsfhit->Draw();
    c1.Update();
    ps_tsfhit->Close();

    TPostScript* ps_phi = new TPostScript((m_postScriptName + ".phi" + ".ps").c_str(), 112);
    h_phi->Draw();
    c1.Update();
    ps_phi->Close();

    TPostScript* ps_pt = new TPostScript((m_postScriptName + ".pt" + ".ps").c_str(), 112);
    h_pt->Draw();
    c1.Update();
    ps_pt->Close();

    TPostScript* ps_omega = new TPostScript((m_postScriptName + ".omega" + ".ps").c_str(), 112);
    h_omega->Draw();
    c1.Update();
    ps_omega->Close();

    TPostScript* ps_time = new TPostScript((m_postScriptName + ".time" + ".ps").c_str(), 112);
    h_time->Draw();
    c1.Update();
    ps_time->Close();
  }

  oldDir->cd();
}

void TRGCDCT2DDQMModule::event()
{

  dirDQM->cd();


  //Fill
  for (int ii = 0; ii < entAry_tsf.getEntries(); ii++) {
    int id = entAry_tsf[ii]->getSegmentID();
    h_tsfhit->Fill(id);
  }
  for (int ii = 0; ii < entAry_t2d.getEntries(); ii++) {
    double phi = entAry_t2d[ii]->getPhi0();
    h_phi->Fill(phi);
    double pt = entAry_t2d[ii]->getPt();
    h_pt->Fill(pt);
    double omega = entAry_t2d[ii]->getOmega();
    h_omega->Fill(omega);
    double time = entAry_t2d[ii]->getTime();
    h_time->Fill(time);
  }


  oldDir->cd();

}


