//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGCDCT3DModule.cc
// Section  : TRG CDCT3D
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG CDCT3D
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <trg/cdc/modules/trgcdct3dDQM/TRGCDCT3DDQMModule.h>

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

REG_MODULE(TRGCDCT3DDQM);


TRGCDCT3DDQMModule::TRGCDCT3DDQMModule() : HistoModule()
{

  setDescription("DQM for CDCT3D Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("T3DMOD", m_T3DMOD,
           "T3D module number",
           0);
  addParam("generatePostscript", m_generatePostscript,
           "Genarete postscript file or not",
           true);
  addParam("postScriptName", m_postScriptName,
           "postscript file name",
           string("cdct3ddqm.ps"));


}

void TRGCDCT3DDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = NULL;
  dirDQM = oldDir->mkdir("TRGCDCT3D");
  dirDQM->cd();
  //dz distribution
  h_dz = new TH1D("hdz", "ndz", 80, -40, 40);
  h_dz->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_dz->GetXaxis()->SetTitle("dz");
  //phi distribution
  h_phi = new TH1D("hphi", "nphi", 100, -6, 6);
  h_phi->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_phi->GetXaxis()->SetTitle("phi");
  //tanlambda distribution
  h_tanlambda = new TH1D("htanlambda", "ntanlambda", 100, -2.5, 2.5);
  h_tanlambda->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_tanlambda->GetXaxis()->SetTitle("tanlambda");
  //pt distribution
  h_pt = new TH1D("hpt", "npt", 100, -2.5, 2.5);
  h_pt->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_pt->GetXaxis()->SetTitle("pt");

  //2D phi distribution
  h_phi_2D = new TH1D("hphi_2D", "nphi_2D", 100, -6, 6);
  h_phi_2D->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_phi_2D->GetXaxis()->SetTitle("phi_2D");
  //2D pt distribution
  h_pt_2D = new TH1D("hpt_2D", "npt_2D", 100, -2.5, 2.5);
  h_pt_2D->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_pt_2D->GetXaxis()->SetTitle("pt_2D");
  oldDir->cd();
}

void TRGCDCT3DDQMModule::beginRun()
{

  dirDQM->cd();

  h_dz->Reset();
  h_phi->Reset();
  h_tanlambda->Reset();
  h_pt->Reset();
  h_phi_2D->Reset();
  h_pt_2D->Reset();

  oldDir->cd();
}

void TRGCDCT3DDQMModule::initialize()
{

  entAry.isRequired("FirmTRGCDC3DFitterTracks");
  entAry_2D.isRequired("FirmTRGCDC2DFinderTracks");
  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();
  REG_HISTOGRAM
  defineHisto();

}

void TRGCDCT3DDQMModule::endRun()
{
  dirDQM->cd();

  //Draw and save histograms
  if (m_generatePostscript) {
    gStyle->SetOptStat(0);
    TCanvas c1("c1", "", 0, 0, 500, 300);
    c1.cd();

    TPostScript* ps_dz = new TPostScript((m_postScriptName + ".dz_T3DModule" + to_string(m_T3DMOD) + ".ps").c_str(), 112);
    h_dz->Draw();
    c1.Update();
    ps_dz->Close();

    TPostScript* ps_phi = new TPostScript((m_postScriptName + ".phi_T3DModule" + to_string(m_T3DMOD) + ".ps").c_str(), 112);
    h_phi->Draw();
    c1.Update();
    ps_phi->Close();

    TPostScript* ps_tanlambda = new TPostScript((m_postScriptName + ".tanlambda_T3DModule" + to_string(m_T3DMOD) + ".ps").c_str(), 112);
    h_tanlambda->Draw();
    c1.Update();
    ps_tanlambda->Close();

    TPostScript* ps_pt = new TPostScript((m_postScriptName + ".pt_T3DModule" + to_string(m_T3DMOD) + ".ps").c_str(), 112);
    h_pt->Draw();
    c1.Update();
    ps_pt->Close();

    TPostScript* ps_phi_2D = new TPostScript((m_postScriptName + ".phi_2D_T3DModule" + to_string(m_T3DMOD) + ".ps").c_str(), 112);
    h_phi_2D->Draw();
    c1.Update();
    ps_phi_2D->Close();

    TPostScript* ps_pt_2D = new TPostScript((m_postScriptName + ".pt_2D_T3DModule" + to_string(m_T3DMOD) + ".ps").c_str(), 112);
    h_pt_2D->Draw();
    c1.Update();
    ps_pt_2D->Close();
  }

  oldDir->cd();
}

void TRGCDCT3DDQMModule::event()
{

  dirDQM->cd();

  if (!entAry || !entAry.getEntries()) {
    return;
  }
  //Fill
  h_dz->Fill(entAry[0]->getZ0());
  h_phi->Fill(entAry[0]->getPhi0());
  h_tanlambda->Fill(entAry[0]->getTanLambda());
  if (entAry[0]->getOmega() != 0)
    h_pt->Fill(1. / entAry[0]->getOmega() * 0.3 * 1.5 * 0.01);

  if (!entAry_2D || !entAry_2D.getEntries()) {
    return;
  }
  h_phi_2D->Fill(entAry_2D[0]->getPhi0());
  if (entAry_2D[0]->getOmega() != 0)
    h_pt_2D->Fill(1. / entAry_2D[0]->getOmega() * 0.3 * 1.5 * 0.01);

  oldDir->cd();

}


