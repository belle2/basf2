/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/grl/modules/TRGGRLDQM/TRGGRLDQMModule.h>

#include <framework/datastore/StoreObjPtr.h>

#include "trg/grl/dataobjects/TRGGRLUnpackerStore.h"

#include <TDirectory.h>
#include <iostream>


using namespace std;
using namespace Belle2;

REG_MODULE(TRGGRLDQM);

TRGGRLDQMModule::TRGGRLDQMModule() : HistoModule()
{

  setDescription("DQM for GRL Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

}

void TRGGRLDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = NULL;
  dirDQM = oldDir->mkdir("TRGGRL");
  dirDQM->cd();

  h_N_track  = new TH1I("h_N_track",  "CDCTRG 2D N_track", 8, 0, 8);
  h_N_track->GetXaxis()->SetTitle("CDCTRG 2D N_track");

  h_phi_i  = new TH1F("h_phi_i",  "phi_i CDCTRG 2D [10 degrees]", 36, 0, 360);
  h_phi_i->GetXaxis()->SetTitle("#phi_{i} CDCTRG 2D [10 degrees]");
  h_phi_CDC  = new TH1F("h_phi_CDC",  "phi_CDC CDCTRG 2D [10 degrees]", 36, 0, 360);
  h_phi_CDC->GetXaxis()->SetTitle("#phi_{CDC} CDCTRG 2D [10 degrees]");
  h_sector_CDC  = new TH1F("h_sector_CDC",  "Sector_CDC CDCTRG 2D [45 degrees]", 8, -22.5, 337.5);
  h_sector_CDC->GetXaxis()->SetTitle("Sector_{CDC} CDCTRG 2D [45 degrees]");
  h_sector_KLM  = new TH1F("h_sector_KLM",  "Sector_KLM [45 degrees]", 8, -22.5, 337.5);
  h_sector_KLM->GetXaxis()->SetTitle("Sector_{KLM} [45 degrees]");
  h_slot_CDC  = new TH1F("h_slot_CDC",  "Slot_CDC CDCTRG 2D [22.5 degrees]", 16, 0, 360);
  h_slot_CDC->GetXaxis()->SetTitle("Slot_{CDC} CDCTRG 2D [22.5 degrees]");
  h_slot_TOP  = new TH1F("h_slot_TOP",  "Slot_TOP [22.5 degrees]", 16, 0, 360);
  h_slot_TOP->GetXaxis()->SetTitle("Slot_{TOP} [22.5 degrees]");

  h_E_ECL  = new TH1F("h_E_ECL",  "ECL cluster energy [5 MeV]", 2048, 0, 10.24);
  h_E_ECL->GetXaxis()->SetTitle("ECL cluster energy [5 MeV]");
  h_theta_ECL  = new TH1F("h_theta_ECL",  "ECL cluster theta [1.4 degrees]", 128, 0, 180);
  h_theta_ECL->GetXaxis()->SetTitle("ECL cluster #theta [1.4 degrees]");
  h_phi_ECL  = new TH1F("h_phi_ECL",  "ECL cluster phi [1.4 degrees]", 256, 0, 360);
  h_phi_ECL->GetXaxis()->SetTitle("ECL cluster #phi [1.4 degrees]");

  h_CDCL1  = new TH1F("h_CDCL1",  "CDCTRG 2D -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_CDCL1->GetXaxis()->SetTitle("CDCTRG 2D -> L1 trg [ns]");
  h_ECLL1  = new TH1F("h_ECLL1",  "ECLTRG -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_ECLL1->GetXaxis()->SetTitle("ECLTRG -> L1 trg [ns]");
  h_TOPL1  = new TH1F("h_TOPL1",  "TOPTRG -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_TOPL1->GetXaxis()->SetTitle("TOPTRG -> L1 trg [ns]");
  h_KLML1  = new TH1F("h_KLML1",  "KLMTRG -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_KLML1->GetXaxis()->SetTitle("KLMTRG -> L1 trg [ns]");
  h_ECLL1_2nd  = new TH1F("h_ECLL1_2nd",  "ECLTRG 2nd input-> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_ECLL1_2nd->GetXaxis()->SetTitle("ECLTRG 2nd input -> L1 trg [ns]");
  h_CDC3DL1  = new TH1F("h_CDC3DL1",  "CDCTRG 3D -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_CDC3DL1->GetXaxis()->SetTitle("CDCTRG 3D -> L1 trg [ns]");
  h_CDCNNL1  = new TH1F("h_CDCNNL1",  "CDCTRG NN -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_CDCNNL1->GetXaxis()->SetTitle("CDCTRG NN -> L1 trg [ns]");
  h_TSFL1  = new TH1F("h_TSFL1",  "CDCTRG TSF -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_TSFL1->GetXaxis()->SetTitle("CDCTRG TSF -> L1 trg [ns]");
  h_B2LL1  = new TH1F("h_B2LL1",  "B2L window -> L1 trg [ns]", 320, -320 * 7.8, 0);
  h_B2LL1->GetXaxis()->SetTitle("B2L window -> L1 trg [ns]");

  h_map_ST  = new TH1F("h_map_ST",  "CDCTRG short track map [5.625 degrees]", 64, 0, 360);
  h_map_ST->GetXaxis()->SetTitle("CDCTRG short track map [5.625 degrees]");
  h_map_ST2  = new TH1F("h_map_ST2",  "CDCTRG short track map [5.625 degrees]", 64, 0, 360);
  h_map_ST2->GetXaxis()->SetTitle("CDCTRG short track map [5.625 degrees]");
  h_map_veto  = new TH1F("h_map_veto",  "CDCTRG 2D veto map [5.625 degrees]", 64, 0, 360);
  h_map_veto->GetXaxis()->SetTitle("CDCTRG 2D veto map [5.625 degrees]");
  h_map_TSF0 = new TH1F("h_map_TSF0",  "CDCTRG TSF0 map [5.625 degrees]", 64, 0, 360);
  h_map_TSF0->GetXaxis()->SetTitle("CDCTRG TSF0 map [5.625 degrees]");
  h_map_TSF2 = new TH1F("h_map_TSF2",  "CDCTRG TSF2 map [5.625 degrees]", 64, 0, 360);
  h_map_TSF2->GetXaxis()->SetTitle("CDCTRG TSF2 map [5.625 degrees]");
  h_map_TSF4 = new TH1F("h_map_TSF4",  "CDCTRG TSF4 map [5.625 degrees]", 64, 0, 360);
  h_map_TSF4->GetXaxis()->SetTitle("CDCTRG TSF4 map [5.625 degrees]");
  h_map_TSF1 = new TH1F("h_map_TSF1",  "CDCTRG TSF1 map [5.625 degrees]", 64, 0, 360);
  h_map_TSF1->GetXaxis()->SetTitle("CDCTRG TSF1 map [5.625 degrees]");
  h_map_TSF3 = new TH1F("h_map_TSF3",  "CDCTRG TSF3 map [5.625 degrees]", 64, 0, 360);
  h_map_TSF3->GetXaxis()->SetTitle("CDCTRG TSF3 map [5.625 degrees]");

  oldDir->cd();
}

void TRGGRLDQMModule::beginRun()
{

  dirDQM->cd();

  h_N_track->Reset();
  h_phi_i->Reset();
  h_phi_CDC->Reset();
  h_sector_CDC->Reset();
  h_sector_KLM->Reset();
  h_slot_CDC->Reset();
  h_slot_TOP->Reset();
  h_E_ECL->Reset();
  h_theta_ECL->Reset();
  h_phi_ECL->Reset();
  h_CDCL1->Reset();
  h_ECLL1->Reset();
  h_TOPL1->Reset();
  h_KLML1->Reset();
  h_CDC3DL1->Reset();
  h_CDCNNL1->Reset();
  h_TSFL1->Reset();
  h_B2LL1->Reset();
  h_map_ST->Reset();
  h_map_ST2->Reset();
  h_map_veto->Reset();
  h_map_TSF0->Reset();
  h_map_TSF2->Reset();
  h_map_TSF4->Reset();
  h_map_TSF1->Reset();
  h_map_TSF3->Reset();

  oldDir->cd();
}

void TRGGRLDQMModule::initialize()
{
  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM
}

void TRGGRLDQMModule::endRun()
{
}

void TRGGRLDQMModule::event()
{
  StoreObjPtr<TRGGRLUnpackerStore> evtinfo("TRGGRLUnpackerStore");
  if (!evtinfo) return;

  int N_track = evtinfo->m_N_track;
  int bin = h_N_track->GetBinContent(N_track + 1);
  h_N_track->SetBinContent(N_track + 1, bin + 1);

  for (int i = 0; i < 36; i++) {
    if (evtinfo->m_phi_i[i]) {
      h_phi_i->Fill(5 + i * 10);
    }
    if (evtinfo->m_phi_CDC[i]) {
      h_phi_CDC->Fill(5 + i * 10);
    }
  }
  for (int i = 0; i < 8; i++) {
    if (evtinfo->m_sector_CDC[i]) {
      h_sector_CDC->Fill(i * 45);
    }
    if (evtinfo->m_sector_KLM[i]) {
      h_sector_KLM->Fill(i * 45);
    }
  }
  for (int i = 0; i < 16; i++) {
    if (evtinfo->m_slot_CDC[i]) {
      h_slot_CDC->Fill(11.25 + i * 22.5);
    }
    if (evtinfo->m_slot_TOP[i]) {
      h_slot_TOP->Fill(11.25 + i * 22.5);
    }
  }

  int N_cluster = evtinfo->m_N_cluster;
  for (int i = 0; i < N_cluster; i++) {
    h_E_ECL->Fill(evtinfo->m_E_ECL[i] * 0.005 + 0.0025);
    h_theta_ECL->Fill(evtinfo->m_theta_ECL[i] * 1.40625 + 1.40625 * 0.5);
    h_phi_ECL->Fill(evtinfo->m_phi_ECL[i] * 1.40625 + 1.40625 * 0.5);
  }

  int timeL1 = evtinfo->m_coml1 - evtinfo->m_revoclk;

  if (evtinfo->m_CDCL1_count != 0) {
    h_CDCL1->Fill((evtinfo->m_CDCL1_count + timeL1 - 0.5) * (-7.8));
  }
  if (evtinfo->m_ECLL1_count != 0) {
    h_ECLL1->Fill((evtinfo->m_ECLL1_count + timeL1 - 0.5) * (-7.8));
  }
  if (evtinfo->m_ECLL1_count != 0 && evtinfo->m_N_cluster_1 != 0) {
    h_ECLL1_2nd->Fill((evtinfo->m_ECLL1_count + timeL1 - 0.5) * (-7.8) + 7.8 * 16);
  }
  if (evtinfo->m_TOPL1_count != 0) {
    h_TOPL1->Fill((evtinfo->m_TOPL1_count + timeL1 - 0.5) * (-7.8));
  }
  if (evtinfo->m_KLML1_count != 0) {
    h_KLML1->Fill((evtinfo->m_KLML1_count + timeL1 - 0.5) * (-7.8));
  }
  if (evtinfo->m_CDC3DL1_count != 0) {
    h_CDC3DL1->Fill((evtinfo->m_CDC3DL1_count + timeL1 - 0.5) * (-7.8));
  }
  if (evtinfo->m_CDCNNL1_count != 0) {
    h_CDCNNL1->Fill((evtinfo->m_CDCNNL1_count + timeL1 - 0.5) * (-7.8));
  }
  if (evtinfo->m_TSFL1_count != 0) {
    h_TSFL1->Fill((evtinfo->m_TSFL1_count + timeL1 - 0.5) * (-7.8));
  }
  if (timeL1 != 0) {
    h_B2LL1->Fill((timeL1 - 0.5) * (-7.8));
  }

  for (int i = 0; i < 64; i++) {
    if (evtinfo->m_map_ST[i]) {
      h_map_ST->Fill(2.8 + i * 360.0 / 64.0);
    }
    if (evtinfo->m_map_ST2[i]) {
      h_map_ST2->Fill(2.8 + i * 360.0 / 64.0);
    }
    if (evtinfo->m_map_veto[i]) {
      h_map_veto->Fill(2.8 + i * 360.0 / 64.0);
    }
    if (evtinfo->m_map_TSF0[i]) {
      h_map_TSF0->Fill(2.8 + i * 360.0 / 64.0);
    }
    if (evtinfo->m_map_TSF2[i]) {
      h_map_TSF2->Fill(2.8 + i * 360.0 / 64.0);
    }
    if (evtinfo->m_map_TSF4[i]) {
      h_map_TSF4->Fill(2.8 + i * 360.0 / 64.0);
    }
    if (evtinfo->m_map_TSF1[i]) {
      h_map_TSF1->Fill(2.8 + i * 360.0 / 64.0);
    }
    if (evtinfo->m_map_TSF3[i]) {
      h_map_TSF3->Fill(2.8 + i * 360.0 / 64.0);
    }
  }
}


