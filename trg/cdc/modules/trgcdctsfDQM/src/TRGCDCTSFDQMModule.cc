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
  dirDQM = oldDir->mkdir("TRGCDCTSF");
  dirDQM->cd();
  h_nhit = new TH1I("hCDCTSF_nhit", "nhit", 7, 0, 7);
  h_nhit->SetTitle("test");
  h_nhit->GetXaxis()->SetTitle("test");
  oldDir->cd();
}

void TRGCDCTSFDQMModule::beginRun()
{

  dirDQM->cd();

  h_nhit->Reset();

  oldDir->cd();
}

void TRGCDCTSFDQMModule::initialize()
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();
  REG_HISTOGRAM
  defineHisto();

}

void TRGCDCTSFDQMModule::endRun()
{
  if (m_generatePostscript) {
    TPostScript* ps = new TPostScript(m_postScriptName.c_str(), 112);
    gStyle->SetOptStat(0);
    TCanvas c1("c1", "", 0, 0, 500, 300);
    c1.cd();

    h_nhit->Draw();
    c1.Update();
    ps->Close();
  }
}

void TRGCDCTSFDQMModule::event()
{

  StoreArray<TRGCDCTSFUnpackerStore> entAry;
  if (!entAry || !entAry.getEntries()) return;

  int nhit = 0;
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    nhit += entAry[ii]->m_netfhit;
  }

  h_nhit->Fill(nhit);

}


