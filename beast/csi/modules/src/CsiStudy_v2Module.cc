/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/csi/modules/CsiStudy_v2Module.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Const.h>
#include <cmath>

#include <fstream>
#include <string>

// ROOT
#include <TH1.h>
#include <TH2.h>

using namespace std;

using namespace Belle2;
using namespace csi;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CsiStudy_v2)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CsiStudy_v2Module::CsiStudy_v2Module() : HistoModule()
{
  // Set module properties
  setDescription("Study_v2 module for Csis (BEAST)");

  addParam("Ethres", m_Ethres, "Energy threshold in MeV", 0.0);
}

CsiStudy_v2Module::~CsiStudy_v2Module()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void CsiStudy_v2Module::defineHisto()
{
  for (int i = 0; i < 153; i++) {
    h_csi_drate[i] = new TH1F(TString::Format("csi_rate_%d", i), "count", 18, 0., 18.);
    h_csi_drate[i]->Sumw2();

    h_csi_rs_drate[i] = new TH2F(TString::Format("csi_rs_drate_%d", i), "count v. ring section", 18, 0., 18., 12, 0., 12.);
    h_csi_rs_drate[i]->Sumw2();

    for (int j = 0; j < 18; j++) {

      h_csi_dedep[j][i] = new TH1F(TString::Format("csi_dedep_%d_%d", j, i), "Energy deposited [MeV]", 5000, 0., 400.);
      h_csi_rs_dedep[j][i] = new TH2F(TString::Format("csi_rs_dedep_%d_%d", j, i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0., 12.);

      h_csi_denergy[j][i] = new TH1F(TString::Format("csi_denergy_%d_%d", j, i), "Energy deposited [MeV]", 200, 0.1, 3.);
      h_csi_rs_denergy[j][i] = new TH2F(TString::Format("csi_rs_denergy_%d_%d", j, i), "Energy deposited [MeV] per section", 200, 0.1, 3.,
                                        12, 0.,
                                        12.);

      h_csi_dedep[j][i]->Sumw2();
      h_csi_rs_dedep[j][i]->Sumw2();
      h_csi_denergy[j][i]->Sumw2();
      h_csi_rs_denergy[j][i]->Sumw2();
    }
  }
  /*
  for (int i = 0; i < 2; i++) {
    h_csi_rate[i] = new TH1F(TString::Format("csi_rate_%d", i), "count", 18, 0., 18.);
    h_csi_rate[i]->Sumw2();

    h_csi_rs_rate[i] = new TH2F(TString::Format("csi_rs_rate_%d", i), "count v. ring section", 18, 0., 18., 12, 0., 12.);
    h_csi_rs_rate[i]->Sumw2();
  }
  for (int i = 0; i < 18; i++) {
    h_csi_energyVrs1[i] = new TH2F(TString::Format("csi_energyVrs1_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0.,
                                   12.);
    h_csi_energyVrs2[i] = new TH2F(TString::Format("csi_energyVrs2_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0.,
                                   12.);
    h_csi_energyVrs3[i] = new TH2F(TString::Format("csi_energyVrs3_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0.,
                                   12.);

    h_csi_energyVrs1W[i] = new TH2F(TString::Format("csi_energyVrs1W_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12,
                                    0.,
                                    12.);
    h_csi_energyVrs2W[i] = new TH2F(TString::Format("csi_energyVrs2W_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12,
                                    0.,
                                    12.);
    h_csi_energyVrs3W[i] = new TH2F(TString::Format("csi_energyVrs3W_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12,
                                    0.,
                                    12.);

    h_csi_energy1[i] = new TH1F(TString::Format("csi_energy1_%d", i), "Energy deposited [MeV]", 200, 0.1, 3.);
    h_csi_energy2[i] = new TH1F(TString::Format("csi_energy2_%d", i), "Energy deposited [MeV]", 200, 0.1, 3.);
    h_csi_energy3[i] = new TH1F(TString::Format("csi_energy3_%d", i), "Energy deposited [MeV]", 200, 0.1, 3.);

    h_csi_energy1W[i] = new TH1F(TString::Format("csi_energy1W_%d", i), "Energy deposited [MeV]", 200, 0.1, 3.);
    h_csi_energy2W[i] = new TH1F(TString::Format("csi_energy2W_%d", i), "Energy deposited [MeV]", 200, 0.1, 3.);
    h_csi_energy3W[i] = new TH1F(TString::Format("csi_energy3W_%d", i), "Energy deposited [MeV]", 200, 0.1, 3.);

    h_csi_Evtof[i] = new TH2F(TString::Format("csi_Evtof_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                              1000, 0., 400.);
    h_csi_Evtof1[i] = new TH2F(TString::Format("csi_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                               1000, 0., 400.);
    h_csi_Evtof2[i] = new TH2F(TString::Format("csi_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_csi_Evtof3[i] = new TH2F(TString::Format("csi_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                               1000., 1000, 0., 400.);

    h_csi_edep[i] = new TH1F(TString::Format("csi_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep1[i] = new TH1F(TString::Format("csi_edep1_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep2[i] = new TH1F(TString::Format("csi_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep1Weight[i] = new TH1F(TString::Format("csi_edep1Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep2Weight[i] = new TH1F(TString::Format("csi_edep2Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);

    h_csi_rs_edep1[i] = new TH2F(TString::Format("csi_rs_edep1_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0., 12.);
    h_csi_rs_edep2[i] = new TH2F(TString::Format("csi_rs_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0., 12.);
    h_csi_rs_edep1Weight[i] = new TH2F(TString::Format("csi_rs_edep1Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0.,
                                       12.);
    h_csi_rs_edep2Weight[i] = new TH2F(TString::Format("csi_rs_edep2Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0.,
                                       12.);

    h_csi_energy1[i]->Sumw2();
    h_csi_energy2[i]->Sumw2();
    h_csi_energy3[i]->Sumw2();
    h_csi_energy1W[i]->Sumw2();
    h_csi_energy2W[i]->Sumw2();
    h_csi_energy3W[i]->Sumw2();
    h_csi_energyVrs1[i]->Sumw2();
    h_csi_energyVrs2[i]->Sumw2();
    h_csi_energyVrs3[i]->Sumw2();
    h_csi_energyVrs1W[i]->Sumw2();
    h_csi_energyVrs2W[i]->Sumw2();
    h_csi_energyVrs3W[i]->Sumw2();
    h_csi_edep[i]->Sumw2();
    h_csi_edep1[i]->Sumw2();
    h_csi_edep2[i]->Sumw2();
    h_csi_edep1Weight[i]->Sumw2();
    h_csi_edep2Weight[i]->Sumw2();
    h_csi_rs_edep1[i]->Sumw2();
    h_csi_rs_edep2[i]->Sumw2();
    h_csi_rs_edep1Weight[i]->Sumw2();
    h_csi_rs_edep2Weight[i]->Sumw2();
  }
  */
}


void CsiStudy_v2Module::initialize()
{
  B2INFO("CsiStudy_v2Module: Initialize");

  //load
  MetaHits.isRequired();
  SimHits.isOptional();
  Hits.isOptional();

  //read csi xml file
  getXMLData();

  REG_HISTOGRAM

}

void CsiStudy_v2Module::beginRun()
{
}

void CsiStudy_v2Module::event()
{
  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

  //Look at the meta data to extract IR rate and scattering ring section
  //double rate = 0;
  int ring_section = -1;
  int section_ordering[12] = {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  for (const auto& MetaHit : MetaHits) {
    //rate = MetaHit.getrate();
    double sad_ssraw = MetaHit.getssraw();
    double ssraw = 0;
    if (sad_ssraw >= 0) ssraw = sad_ssraw / 100.;
    else ssraw = 3000. + sad_ssraw / 100.;
    //else if (sad_ssraw < 0) ssraw = 3000. + sad_ssraw / 100.;
    ring_section = section_ordering[(int)((ssraw) / 250.)] - 1;
    //ring_section = MetaHit.getring_section() - 1;
  }
  /*
  //Loop over SimHit
  for (const auto& SimHit : SimHits) {
      int detNB = SimHit.getCellId();
    int pdg = SimHit.getPDGCode();
    double Edep = SimHit.getEnergyDep() * 1e3; //GeV -> MeV
    double tof = SimHit.getFlightTime(); //ns

    h_csi_edep[detNB]->Fill(Edep);
    h_csi_Evtof[detNB]->Fill(tof, Edep);
    if (pdg == Const::photon.getPDGCode()) h_csi_Evtof1[detNB]->Fill(tof, Edep);
    else if (fabs(pdg) == Const::electron.getPDGCode()) h_csi_Evtof2[detNB]->Fill(tof, Edep);
    h_csi_energy1[detNB]->Fill(log10(Edep));
    h_csi_energy1W[detNB]->Fill(log10(Edep), rate);
    h_csi_energyVrs1[detNB]->Fill(log10(Edep), ring_section);
    h_csi_energyVrs1W[detNB]->Fill(log10(Edep), ring_section, rate);
  }
  */
  //Loop over DigiHit
  for (const auto& Hit : Hits) {
    int detNB = Hit.getCellId();
    double Edep = Hit.getEnergyDep() * 1e3; //GeV -> MeV
    double RecEdep = Edep;//Hit.getEnergyRecDep() * 1e3; //GeV -> MeV
    //double tof = Hit.getFlightTime(); //ns
    /*
    h_csi_rate[0]->Fill(detNB);
    h_csi_rate[1]->Fill(detNB, rate);
    h_csi_edep1[detNB]->Fill(Edep);
    h_csi_edep2[detNB]->Fill(RecEdep);
    h_csi_edep1Weight[detNB]->Fill(Edep, rate);
    h_csi_edep2Weight[detNB]->Fill(RecEdep, rate);
    h_csi_Evtof3[detNB]->Fill(tof, RecEdep);
    h_csi_rs_rate[0]->Fill(detNB, ring_section);
    h_csi_rs_rate[1]->Fill(detNB, ring_section, rate);
    h_csi_rs_edep1[detNB]->Fill(Edep, ring_section);
    h_csi_rs_edep2[detNB]->Fill(RecEdep, ring_section);
    h_csi_rs_edep1Weight[detNB]->Fill(Edep, ring_section, rate);
    h_csi_rs_edep2Weight[detNB]->Fill(RecEdep, ring_section, rate);
    h_csi_energy2[detNB]->Fill(log10(Edep));
    h_csi_energy3[detNB]->Fill(log10(RecEdep));
    h_csi_energy2W[detNB]->Fill(log10(Edep), rate);
    h_csi_energy3W[detNB]->Fill(log10(RecEdep), rate);
    h_csi_energyVrs2[detNB]->Fill(log10(Edep), ring_section);
    h_csi_energyVrs3[detNB]->Fill(log10(RecEdep), ring_section);
    h_csi_energyVrs2W[detNB]->Fill(log10(Edep), ring_section, rate);
    h_csi_energyVrs3W[detNB]->Fill(log10(RecEdep), ring_section, rate);
    */
    for (int i = 0; i < 153; i ++) {
      /*cout << " thres1 " << m_Thres_hitRate[detNB][i]
      << " thres2 " << m_Thres_sumE[detNB][i]
      << " edep " << RecEdep << " true Edep " << Edep << endl;*/
      if (RecEdep >= m_Thres_hitRate[detNB][i] && m_Thres_hitRate[detNB][i] > 0) {
        h_csi_drate[i]->Fill(detNB);
        h_csi_rs_drate[i]->Fill(detNB, ring_section);
      }
      if (RecEdep >= m_Thres_sumE[detNB][i] && m_Thres_sumE[detNB][i] > 0) {
        h_csi_dedep[detNB][i]->Fill(RecEdep);
        h_csi_rs_dedep[detNB][i]->Fill(RecEdep, ring_section);

        h_csi_denergy[detNB][i]->Fill(log10(RecEdep));
        h_csi_rs_denergy[detNB][i]->Fill(log10(RecEdep), ring_section);
      }
    }
  }
}
//read tube centers, impulse response, and garfield drift data filename from CSI.xml
void CsiStudy_v2Module::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CSI\"]/Content/");
  m_Ethres = content.getDouble("Ethres");


  for (int i = 0; i < 18; i ++) {
    int iThres_hitRate = 0;
    for (double Thres : content.getArray(TString::Format("thres_hitRate_det%d", i).Data(), {0})) {
      m_Thres_hitRate[i][iThres_hitRate] = Thres;
      iThres_hitRate++;
    }
    int iThres_sumE = 0;
    for (double Thres : content.getArray(TString::Format("thres_sumE_det%d", i).Data(), {0})) {
      m_Thres_sumE[i][iThres_sumE] = Thres;
      iThres_sumE++;
    }
  }

  B2INFO("CsiStudy_v2");

}
void CsiStudy_v2Module::endRun()
{


}

void CsiStudy_v2Module::terminate()
{
}


