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
  for (int i = 0; i < 18; i++) {
    h_csi_Evtof1[i] = new TH2F(TString::Format("h_csi_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                               1000, 0., 400.);
    h_csi_Evtof2[i] = new TH2F(TString::Format("h_csi_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_csi_Evtof3[i] = new TH2F(TString::Format("h_csi_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_csi_Evtof4[i] = new TH2F(TString::Format("h_csi_Evtof4_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_csi_Evtof5[i] = new TH2F(TString::Format("h_csi_Evtof5_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_csi_Evtof6[i] = new TH2F(TString::Format("h_csi_Evtof6_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_csi_edep[i] = new TH1F(TString::Format("h_csi_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep1[i] = new TH1F(TString::Format("h_csi_edep1_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep2[i] = new TH1F(TString::Format("h_csi_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep1Weight[i] = new TH1F(TString::Format("h_csi_edep1Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep2Weight[i] = new TH1F(TString::Format("h_csi_edep2Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep_nocut[i] = new TH1F(TString::Format("h_csi_edep_nocut_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_csi_edep_test[i] = new TH1F(TString::Format("h_csi_edep_test_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
  }
  h_csi_s = new TH1F("h_csi_s", "", 4000, -200., 200.);
  h_csi_s_cut = new TH1F("h_csi_s_cut", "", 4000, -200., 200.);
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
  StoreArray<SADMetaHit> SADtruth;

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }
  double rate = 0;
  int nSAD = SADtruth.getEntries();
  Bool_t Reject = false;
  for (int i = 0; i < nSAD; i++) {
    SADMetaHit* aHit = SADtruth[i];
    double s = aHit->gets();
    rate = aHit->getrate();
    h_csi_s->Fill(-s);
    if ((-33.0 <= -s && -s <= -30.0) || (19.0 <= -s && -s <= 23.0)) {
      h_csi_s_cut->Fill(-s);
      Reject = true;
    }
  }
  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    CsiSimHit* aHit = SimHits[i];
    int detNB = aHit->getCellId();
    //int trkID = aHit->getTrackId();
    int pdg = aHit->getPDGCode();
    double Edep = aHit->getEnergyDep() * 1e3; //GeV -> MeV
    double tof = aHit->getFlightTime(); //ns

    h_csi_edep_nocut[detNB]->Fill(Edep);
    h_csi_Evtof1[detNB]->Fill(tof, Edep);
    if (pdg == 22) h_csi_Evtof2[detNB]->Fill(tof, Edep);
    else if (fabs(pdg) == 11) h_csi_Evtof3[detNB]->Fill(tof, Edep);
    else h_csi_Evtof4[detNB]->Fill(tof, Edep);
    if (Edep > m_Ethres) {
      h_csi_edep[detNB]->Fill(Edep);
      if (!Reject)
        h_csi_edep_test[detNB]->Fill(Edep);
    }
  }


  for (const auto& Hit : Hits) {
    int detNB = Hit.getCellId();
    double Edep = Hit.getEnergyDep() * 1e3; //GeV -> MeV
    double RecEdep = Hit.getEnergyRecDep() * 1e3; //GeV -> MeV
    double tof = Hit.getFlightTime(); //ns
    h_csi_edep1[detNB]->Fill(Edep);
    h_csi_edep2[detNB]->Fill(RecEdep);
    h_csi_edep1Weight[detNB]->Fill(Edep, rate);
    h_csi_edep2Weight[detNB]->Fill(RecEdep, rate);
    h_csi_Evtof5[detNB]->Fill(tof, Edep);
    h_csi_Evtof6[detNB]->Fill(tof, RecEdep);
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


