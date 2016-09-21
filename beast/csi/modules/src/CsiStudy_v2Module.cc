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
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/dataobjects/CsiHit_v2.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <cmath>
#include <boost/foreach.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>
#include <TMath.h>

#include <generators/SAD/dataobjects/SADMetaHit.h>

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
  for (int i = 0; i < 2; i++) {
    h_csi_rate[i] = new TH1F(TString::Format("csi_rate_%d", i), "count", 18, 0., 18.);
    h_csi_rate[i]->Sumw2();

    h_csi_rs_rate[i] = new TH2F(TString::Format("csi_rs_rate_%d", i), "count v. ring section", 18, 0., 18., 12, 0., 12.);
    h_csi_rs_rate[i]->Sumw2();
  }
  for (int i = 0; i < 18; i++) {
    h_csi_energy1[i] = new TH2F(TString::Format("csi_energy1_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0., 12.);
    h_csi_energy2[i] = new TH2F(TString::Format("csi_energy2_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0., 12.);
    h_csi_energy3[i] = new TH2F(TString::Format("csi_energy3_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0., 12.);

    h_csi_energy1W[i] = new TH2F(TString::Format("csi_energy1W_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0.,
                                 12.);
    h_csi_energy2W[i] = new TH2F(TString::Format("csi_energy2W_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0.,
                                 12.);
    h_csi_energy3W[i] = new TH2F(TString::Format("csi_energy3W_%d", i), "Energy deposited [MeV] per section", 200, 0.1, 3., 12, 0.,
                                 12.);

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

}


void CsiStudy_v2Module::initialize()
{
  B2INFO("CsiStudy_v2Module: Initialize");

  //read csi xml file
  getXMLData();

  REG_HISTOGRAM

}

void CsiStudy_v2Module::beginRun()
{
}

void CsiStudy_v2Module::event()
{
  //Here comes the actual event processing
  StoreArray<CsiSimHit>  SimHits;
  StoreArray<CsiHit_v2>  Hits;
  StoreArray<SADMetaHit> MetaHits;

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

  //Look at the meta data to extract IR rate and scattering ring section
  double rate = 0;
  int ring_section = -1;
  int section_ordering[12] = {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  for (const auto& MetaHit : MetaHits) {
    rate = MetaHit.getrate();
    double sad_ssraw = MetaHit.getssraw();
    double ssraw = 0;
    if (sad_ssraw >= 0) ssraw = sad_ssraw / 100.;
    else if (sad_ssraw < 0) ssraw = 3000. + sad_ssraw / 100.;
    ring_section = section_ordering[(int)((ssraw) / 250.)] - 1;
    //ring_section = MetaHit.getring_section() - 1;
  }

  //Loop over SimHit
  for (const auto& SimHit : SimHits) {
    int detNB = SimHit.getCellId();
    int pdg = SimHit.getPDGCode();
    double Edep = SimHit.getEnergyDep() * 1e3; //GeV -> MeV
    double tof = SimHit.getFlightTime(); //ns

    h_csi_edep[detNB]->Fill(Edep);
    h_csi_Evtof[detNB]->Fill(tof, Edep);
    if (pdg == 22) h_csi_Evtof1[detNB]->Fill(tof, Edep);
    else if (fabs(pdg) == 11) h_csi_Evtof2[detNB]->Fill(tof, Edep);
    h_csi_energy1[detNB]->Fill(log10(Edep), ring_section);
    h_csi_energy1W[detNB]->Fill(log10(Edep), ring_section, rate);
  }

  //Loop over DigiHit
  for (const auto& Hit : Hits) {
    int detNB = Hit.getCellId();
    double Edep = Hit.getEnergyDep() * 1e3; //GeV -> MeV
    double RecEdep = Hit.getEnergyRecDep() * 1e3; //GeV -> MeV
    double tof = Hit.getFlightTime(); //ns
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
    h_csi_energy2[detNB]->Fill(log10(Edep), ring_section);
    h_csi_energy3[detNB]->Fill(log10(RecEdep), ring_section);
    h_csi_energy2W[detNB]->Fill(log10(Edep), ring_section, rate);
    h_csi_energy3W[detNB]->Fill(log10(RecEdep), ring_section, rate);
  }
}
//read tube centers, impulse response, and garfield drift data filename from CSI.xml
void CsiStudy_v2Module::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CSI\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("CsiStudy_v2");

}
void CsiStudy_v2Module::endRun()
{


}

void CsiStudy_v2Module::terminate()
{
}


