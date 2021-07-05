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

#include <TDirectory.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <iostream>

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
           false);
  addParam("postScriptName", m_postScriptName,
           "postscript file name",
           string("cdct3ddqm.ps"));


}

void TRGCDCT3DDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = NULL;
  //dirDQM = oldDir->mkdir("TRGCDCT3D");
  if (!oldDir->Get("TRGCDCT3D"))dirDQM = oldDir->mkdir("TRGCDCT3D");
  else dirDQM = (TDirectory*)oldDir->Get("TRGCDCT3D");
  dirDQM->cd();
  //dz distribution
  h_dz = new TH1D(Form("hdz_mod%d", m_T3DMOD), Form("hdz_mod%d", m_T3DMOD), 80, -40, 40);
  h_dz->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_dz->GetXaxis()->SetTitle("dz");
  //phi distribution
  h_phi = new TH1D(Form("hphi_mod%d", m_T3DMOD), Form("hphi_mod%d", m_T3DMOD), 100, -6, 6);
  h_phi->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_phi->GetXaxis()->SetTitle("phi");
  //tanlambda distribution
  h_tanlambda = new TH1D(Form("htanlambda_mod%d", m_T3DMOD), Form("htanlambda_mod%d", m_T3DMOD), 100, -2.5, 2.5);
  h_tanlambda->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_tanlambda->GetXaxis()->SetTitle("tanlambda");
  //pt distribution
  h_pt = new TH1D(Form("hpt_mod%d", m_T3DMOD), Form("hpt_mod%d", m_T3DMOD), 100, 0, 3);
  h_pt->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_pt->GetXaxis()->SetTitle("pt");

  //2D phi distribution
  h_phi_2D = new TH1D(Form("hphi_2D_mod%d", m_T3DMOD), Form("hphi_2D_mod%d", m_T3DMOD), 100, -6, 6);
  h_phi_2D->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_phi_2D->GetXaxis()->SetTitle("phi_2D");
  //2D pt distribution
  h_pt_2D = new TH1D(Form("hpt_2D_mod%d", m_T3DMOD), Form("hpt_2D_mod%d", m_T3DMOD), 100, 0, 3);
  h_pt_2D->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_pt_2D->GetXaxis()->SetTitle("pt_2D");
  //TSF1 ID
  h_ID_TSF1 = new TH1D(Form("hID_TSF1_mod%d", m_T3DMOD), Form("hID_TSF1_mod%d", m_T3DMOD), 160, 160, 319);
  h_ID_TSF1->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_ID_TSF1->GetXaxis()->SetTitle("ID_TSF1");
  //TSF3 ID
  h_ID_TSF3 = new TH1D(Form("hID_TSF3_mod%d", m_T3DMOD), Form("hID_TSF3_mod%d", m_T3DMOD), 224, 512, 735);
  h_ID_TSF3->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_ID_TSF3->GetXaxis()->SetTitle("ID_TSF3");
  //TSF5 ID
  h_ID_TSF5 = new TH1D(Form("hID_TSF5_mod%d", m_T3DMOD), Form("hID_TSF5_mod%d", m_T3DMOD), 288, 992, 1279);
  h_ID_TSF5->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_ID_TSF5->GetXaxis()->SetTitle("ID_TSF5");
  //TSF7 ID
  h_ID_TSF7 = new TH1D(Form("hID_TSF7_mod%d", m_T3DMOD), Form("hID_TSF7_mod%d", m_T3DMOD), 352, 1600, 1951);
  h_ID_TSF7->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_ID_TSF7->GetXaxis()->SetTitle("ID_TSF7");
  //TSF1 rt
  h_rt_TSF1 = new TH1D(Form("hrt_TSF1_mod%d", m_T3DMOD), Form("hrt_TSF1_mod%d", m_T3DMOD), 512, 0, 511);
  h_rt_TSF1->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_rt_TSF1->GetXaxis()->SetTitle("rt_TSF1");
  //TSF3 rt
  h_rt_TSF3 = new TH1D(Form("hrt_TSF3_mod%d", m_T3DMOD), Form("hrt_TSF3_mod%d", m_T3DMOD), 512, 0, 511);
  h_rt_TSF3->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_rt_TSF3->GetXaxis()->SetTitle("rt_TSF3");
  //TSF5 rt
  h_rt_TSF5 = new TH1D(Form("hrt_TSF5_mod%d", m_T3DMOD), Form("hrt_TSF5_mod%d", m_T3DMOD), 512, 0, 511);
  h_rt_TSF5->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_rt_TSF5->GetXaxis()->SetTitle("rt_TSF5");
  //TSF7 rt
  h_rt_TSF7 = new TH1D(Form("hrt_TSF7_mod%d", m_T3DMOD), Form("hrt_TSF7_mod%d", m_T3DMOD), 512, 0, 511);
  h_rt_TSF7->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_rt_TSF7->GetXaxis()->SetTitle("rt_TSF7");
  //TSF1 validity
  h_validity_TSF1 = new TH1D(Form("hvalidity_TSF1_mod%d", m_T3DMOD), Form("hvalidity_TSF1_mod%d", m_T3DMOD), 16, 0, 15);
  h_validity_TSF1->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_validity_TSF1->GetXaxis()->SetTitle("validity_TSF1");
  //TSF3 validity
  h_validity_TSF3 = new TH1D(Form("hvalidity_TSF3_mod%d", m_T3DMOD), Form("hvalidity_TSF3_mod%d", m_T3DMOD), 16, 0, 15);
  h_validity_TSF3->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_validity_TSF3->GetXaxis()->SetTitle("validity_TSF3");
  //TSF5 validity
  h_validity_TSF5 = new TH1D(Form("hvalidity_TSF5_mod%d", m_T3DMOD), Form("hvalidity_TSF5_mod%d", m_T3DMOD), 16, 0, 15);
  h_validity_TSF5->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_validity_TSF5->GetXaxis()->SetTitle("validity_TSF5");
  //TSF7 validity
  h_validity_TSF7 = new TH1D(Form("hvalidity_TSF7_mod%d", m_T3DMOD), Form("hvalidity_TSF7_mod%d", m_T3DMOD), 16, 0, 15);
  h_validity_TSF7->SetTitle(Form("Exp%d Run%d 3Dmodule%d", _exp, _run, m_T3DMOD));
  h_validity_TSF7->GetXaxis()->SetTitle("validity_TSF7");

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
  h_ID_TSF1->Reset();

  oldDir->cd();
}

void TRGCDCT3DDQMModule::initialize()
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM

  char c_name_3D[100];
  sprintf(c_name_3D, "FirmTRGCDC3DFitterTracks%d", m_T3DMOD);
  char c_name_2D[100];
  sprintf(c_name_2D, "FirmTRGCDC2DFinderTracks%d", m_T3DMOD);
  entAry.isRequired(c_name_3D);
  entAry_2D.isRequired(c_name_2D);
  char c_name_TSF[100];
  sprintf(c_name_TSF, "FirmCDCTriggerSegmentHits%d", m_T3DMOD);
  entAry_TSF.isRequired(c_name_TSF);
  if (!entAry || !entAry.getEntries()) return;
  if (!entAry_2D || !entAry_2D.getEntries()) return;
  if (!entAry_TSF || !entAry_TSF.getEntries()) return;

}

void TRGCDCT3DDQMModule::endRun()
{
  dirDQM->cd();

  //Draw and save histograms
  if (m_generatePostscript) {
    //gStyle->SetOptStat(0);
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

  //Fill 3D histo
  if (!(!entAry || !entAry.getEntries())) {
    for (int i = 0; i < entAry.getEntries(); i++) {
      h_dz->Fill(entAry[i]->getZ0());
      h_phi->Fill(entAry[i]->getPhi0());
      h_tanlambda->Fill(entAry[i]->getTanLambda());
      if (entAry[i]->getOmega() != 0)
        h_pt->Fill(fabs(1. / entAry[i]->getOmega() * 0.3 * 1.5 * 0.01));
    }
  }
  //Fill 2D histo
  if (!(!entAry_2D || !entAry_2D.getEntries())) {
    for (int i = 0; i < entAry_2D.getEntries(); i++) {
      h_phi_2D->Fill(entAry_2D[i]->getPhi0());
      if (entAry_2D[i]->getOmega() != 0)
        h_pt_2D->Fill(fabs(1. / entAry_2D[i]->getOmega() * 0.3 * 1.5 * 0.01));
    }
  }
  //Fill stereo TSF histo
  if (!(!entAry_TSF || !entAry_TSF.getEntries())) {
    for (int i = 0; i < entAry_TSF.getEntries(); i++) {
      int sl = entAry_TSF[i]->getISuperLayer();
      int id = entAry_TSF[i]->getSegmentID();
      int pr = entAry_TSF[i]->getPriorityPosition();
      int lr = entAry_TSF[i]->getLeftRight();
      int rt = entAry_TSF[i]->priorityTime();
      int validity = lr * 4 + pr;
      if (lr == 0 || pr == 0) continue;
      if (sl == 1)  {
        h_ID_TSF1->Fill(id);
        h_rt_TSF1->Fill(rt);
        h_validity_TSF1->Fill(validity);
      } else if (sl == 3)  {
        h_ID_TSF3->Fill(id);
        h_rt_TSF3->Fill(rt);
        h_validity_TSF3->Fill(validity);
      } else if (sl == 5)  {
        h_ID_TSF5->Fill(id);
        h_rt_TSF5->Fill(rt);
        h_validity_TSF5->Fill(validity);
      } else if (sl == 7)  {
        h_ID_TSF7->Fill(id);
        h_rt_TSF7->Fill(rt);
        h_validity_TSF7->Fill(validity);
      }


    }
  }

  oldDir->cd();

}


