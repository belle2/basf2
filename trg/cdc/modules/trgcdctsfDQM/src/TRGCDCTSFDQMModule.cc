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
// Filename : TRGCDCTSFModule.cc
// Section  : TRG CDCTSF
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG CDCTSF
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <trg/cdc/modules/trgcdctsfDQM/TRGCDCTSFDQMModule.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <TDirectory.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(TRGCDCTSFDQM);


TRGCDCTSFDQMModule::TRGCDCTSFDQMModule() : HistoModule()
{

  setDescription("DQM for CDCTSF Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TSFMOD", m_TSFMOD,
           "TSF module number",
           0);
  addParam("generatePostscript", m_generatePostscript,
           "Genarete postscript file or not",
           false);
  addParam("postScriptName", m_postScriptName,
           "postscript file name",
           string("gdldqm.ps"));


}

void TRGCDCTSFDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = NULL;
  //dirDQM = oldDir->mkdir("TRGCDCTSF");
  if (!oldDir->Get("TRGCDCTSF"))dirDQM = oldDir->mkdir("TRGCDCTSF");
  else dirDQM = (TDirectory*)oldDir->Get("TRGCDCTSF");
  dirDQM->cd();
  //Total number of TSF hits per event in each superlayer
  h_nhit = new TH1I(Form("hCDCTSF_nhit_mod%d", m_TSFMOD), Form("nhit_mod%d", m_TSFMOD), 16, 0, 16);
  h_nhit->SetTitle(Form("Exp%d Run%d SuperLayer%d", _exp, _run, m_TSFMOD));
  h_nhit->GetXaxis()->SetTitle("Total number of TSF hits/event");
  //Total number of hits in each TSF
  h_nhit_tsf = new TH1I(Form("hCDCTSF_nhit_tsf_mod%d", m_TSFMOD), Form("nhit_tsf_mod%d", m_TSFMOD), 200, 0, 200);
  h_nhit_tsf->SetTitle(Form("Exp%d Run%d SuperLayer%d", _exp, _run, m_TSFMOD));
  h_nhit_tsf->GetXaxis()->SetTitle("TSF ID");
  h_nhit_tsf->GetYaxis()->SetTitle("Total number of hits");
  //Validity of hits in each super layer
  h_valid = new TH1I(Form("hCDCTSF_valid_mod%d", m_TSFMOD), Form("valid__mod%d", m_TSFMOD), 10, 0, 10);
  h_valid->SetTitle(Form("Exp%d Run%d SuperLayer%d", _exp, _run, m_TSFMOD));
  h_valid->GetXaxis()->SetTitle("Validity");
  h_valid->GetYaxis()->SetTitle("#of tsf hits");
  //Timing of hits in each super layer
  h_timing = new TH1I(Form("hCDCTSF_timing_mod%d", m_TSFMOD), Form("timing__mod%d", m_TSFMOD), 520, -5, 515);
  h_timing->SetTitle(Form("Exp%d Run%d SuperLayer%d", _exp, _run, m_TSFMOD));
  h_timing->GetXaxis()->SetTitle("Timing");
  h_timing->GetYaxis()->SetTitle("#of tsf hits");
  oldDir->cd();
}

void TRGCDCTSFDQMModule::beginRun()
{

  dirDQM->cd();

  h_nhit->Reset();
  h_nhit_tsf->Reset();

  oldDir->cd();
}

void TRGCDCTSFDQMModule::initialize()
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM

  char c_name[100];
  sprintf(c_name, "TRGCDCTSFUnpackerStore%d", m_TSFMOD);
  entAry.isRequired(c_name);
  if (!entAry || !entAry.getEntries()) return;

}

void TRGCDCTSFDQMModule::endRun()
{
  dirDQM->cd();

  //Draw and save histograms
  if (m_generatePostscript) {
    gStyle->SetOptStat(0);
    TCanvas c1("c1", "", 0, 0, 500, 300);
    c1.cd();

    TPostScript* ps_nhit = new TPostScript((m_postScriptName + ".tsf_total_nhit_superlayer" + to_string(m_TSFMOD) + ".ps").c_str(),
                                           112);
    h_nhit->Draw();
    c1.Update();
    ps_nhit->Close();

    TPostScript* ps_nhit_tsf = new TPostScript((m_postScriptName + ".tsf_nhit_superlayer" + to_string(m_TSFMOD) + ".ps").c_str(), 112);
    h_nhit_tsf->Draw();
    c1.Update();
    ps_nhit_tsf->Close();

  }

  oldDir->cd();
}

void TRGCDCTSFDQMModule::event()
{

  dirDQM->cd();


  //Fill
  int nhit = 0;
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    nhit += entAry[ii]->m_netfhit;
  }
  h_nhit->Fill(nhit);

  /* cppcheck-suppress variableScope */
  int id;
  /* cppcheck-suppress variableScope */
  int v ;
  /* cppcheck-suppress variableScope */
  int rt;
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    id = entAry[ii]->m_trackerhit0id;
    v  = entAry[ii]->m_trackerhit0v;
    rt = entAry[ii]->m_trackerhit0rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit1id;
    v  = entAry[ii]->m_trackerhit1v;
    rt = entAry[ii]->m_trackerhit1rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit2id;
    v  = entAry[ii]->m_trackerhit2v;
    rt = entAry[ii]->m_trackerhit2rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit3id;
    v  = entAry[ii]->m_trackerhit3v;
    rt = entAry[ii]->m_trackerhit3rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit4id;
    v  = entAry[ii]->m_trackerhit4v;
    rt = entAry[ii]->m_trackerhit4rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit5id;
    v  = entAry[ii]->m_trackerhit5v;
    rt = entAry[ii]->m_trackerhit5rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit6id;
    v  = entAry[ii]->m_trackerhit6v;
    rt = entAry[ii]->m_trackerhit6rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit7id;
    v  = entAry[ii]->m_trackerhit7v;
    rt = entAry[ii]->m_trackerhit7rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit8id;
    v  = entAry[ii]->m_trackerhit8v;
    rt = entAry[ii]->m_trackerhit8rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit9id;
    v  = entAry[ii]->m_trackerhit9v;
    rt = entAry[ii]->m_trackerhit9rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }

    // to fill 5 more TS for 15 TS version, only for TSF 1,3,5
    if (entAry[ii]->m_N2DTS == 10) continue;

    id = entAry[ii]->m_trackerhit10id;
    v  = entAry[ii]->m_trackerhit10v;
    rt = entAry[ii]->m_trackerhit10rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit11id;
    v  = entAry[ii]->m_trackerhit11v;
    rt = entAry[ii]->m_trackerhit11rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit12id;
    v  = entAry[ii]->m_trackerhit12v;
    rt = entAry[ii]->m_trackerhit12rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit13id;
    v  = entAry[ii]->m_trackerhit13v;
    rt = entAry[ii]->m_trackerhit13rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }
    id = entAry[ii]->m_trackerhit14id;
    v  = entAry[ii]->m_trackerhit14v;
    rt = entAry[ii]->m_trackerhit14rt;
    if (v != 0) {
      h_nhit_tsf->Fill(id);
      h_valid->Fill(v);
      h_timing->Fill(rt);
    }


  }

  oldDir->cd();

}


