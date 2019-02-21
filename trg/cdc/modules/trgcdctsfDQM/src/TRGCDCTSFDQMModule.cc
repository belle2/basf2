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
#include <trg/cdc/modules/trgcdctsfUnpacker/trgcdctsfUnpackerModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dbobjects/RunInfo.h>
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
  h_nhit = new TH1I(Form("hCDCTSF_nhit_mod%d", m_TSFMOD), Form("nhit_mod%d", m_TSFMOD), 10, 0, 10);
  h_nhit->SetTitle(Form("Exp%d Run%d SuperLayer%d", _exp, _run, m_TSFMOD));
  h_nhit->GetXaxis()->SetTitle("Total number of TSF hits/event");
  //Total number of hits in each TSF
  h_nhit_tsf = new TH1I(Form("hCDCTSF_nhit_tsf_mod%d", m_TSFMOD), Form("nhit_tsf_mod%d", m_TSFMOD), 200, 0, 200);
  h_nhit_tsf->SetTitle(Form("Exp%d Run%d SuperLayer%d", _exp, _run, m_TSFMOD));
  h_nhit_tsf->GetXaxis()->SetTitle("TSF ID");
  h_nhit_tsf->GetYaxis()->SetTitle("Total number of hits");
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
  REG_HISTOGRAM
  defineHisto();

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

  int id = 0;
  int v  = 0;
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    id = entAry[ii]->m_trackerhit0id;
    v  = entAry[ii]->m_trackerhit0v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit1id;
    v  = entAry[ii]->m_trackerhit1v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit2id;
    v  = entAry[ii]->m_trackerhit2v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit3id;
    v  = entAry[ii]->m_trackerhit3v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit4id;
    v  = entAry[ii]->m_trackerhit4v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit5id;
    v  = entAry[ii]->m_trackerhit5v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit6id;
    v  = entAry[ii]->m_trackerhit6v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit7id;
    v  = entAry[ii]->m_trackerhit7v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit8id;
    v  = entAry[ii]->m_trackerhit8v;
    if (v != 0)h_nhit_tsf->Fill(id);
    id = entAry[ii]->m_trackerhit9id;
    v  = entAry[ii]->m_trackerhit9v;
    if (v != 0)h_nhit_tsf->Fill(id);
  }

  oldDir->cd();

}


