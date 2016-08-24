/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/bgo/modules/BgoStudyModule.h>
#include <beast/bgo/dataobjects/BgoSimHit.h>
#include <beast/bgo/dataobjects/BgoHit.h>
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
using namespace bgo;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BgoStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BgoStudyModule::BgoStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Bgos (BEAST)");

  addParam("Ethres", m_Ethres, "Energy threshold in MeV");
}

BgoStudyModule::~BgoStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void BgoStudyModule::defineHisto()
{
  for (int i = 0; i < 8; i++) {
    h_bgo_rate[i] = new TH1F(TString::Format("h_bgo_rate_%d", i), "Energy deposited [MeV]", 100, 0., 100.);
    h_bgo_rate[i]->Sumw2();
  }
  for (int i = 0; i < 8; i++) {
    h_bgos_Evtof1[i] = new TH2F(TString::Format("h_bgos_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                                1000, 0., 400.);
    h_bgos_Evtof2[i] = new TH2F(TString::Format("h_bgos_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                                1000., 1000, 0., 400.);
    h_bgo_Evtof1[i] = new TH2F(TString::Format("h_bgo_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                               1000, 0., 400.);
    h_bgo_Evtof2[i] = new TH2F(TString::Format("h_bgo_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_bgos_Evtof3[i] = new TH2F(TString::Format("h_bgos_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                                1000., 1000, 0., 400.);
    h_bgos_Evtof4[i] = new TH2F(TString::Format("h_bgos_Evtof4_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                                1000., 1000, 0., 400.);
    h_bgos_edep[i] = new TH1F(TString::Format("h_bgos_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgos_edep2[i] = new TH1F(TString::Format("h_bgos_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgos_edep_test[i] = new TH1F(TString::Format("h_bgos_edep_test_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);

    h_bgo_edep[i] = new TH1F(TString::Format("h_bgo_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep_test[i] = new TH1F(TString::Format("h_bgo_edep_test_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);

    h_bgo_edepWeight[i] = new TH1F(TString::Format("h_bgo_edepWeight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep_testWeight[i] = new TH1F(TString::Format("h_bgo_edep_testWeight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);

    h_bgo_edep[i]->Sumw2();
    h_bgo_edep_test[i]->Sumw2();
    h_bgo_edepWeight[i]->Sumw2();
    h_bgo_edep_testWeight[i]->Sumw2();
    h_bgos_edep[i]->Sumw2();
    h_bgos_edep2[i]->Sumw2();
    h_bgos_edep_test[i]->Sumw2();

    h_bgo_edep1[i] = new TH1F(TString::Format("h_bgo_edep1_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep2[i] = new TH1F(TString::Format("h_bgo_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep3[i] = new TH1F(TString::Format("h_bgo_edep3_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep4[i] = new TH1F(TString::Format("h_bgo_edep4_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);

    h_bgo_edep1[i]->Sumw2();
    h_bgo_edep2[i]->Sumw2();
    h_bgo_edep3[i]->Sumw2();
    h_bgo_edep4[i]->Sumw2();
  }
  h_bgo_s = new TH1F("h_bgo_s", "", 4000, -200., 200.);
  h_bgo_s_cut = new TH1F("h_bgo_s_cut", "", 4000, -200., 200.);
}


void BgoStudyModule::initialize()
{
  B2INFO("BgoStudyModule: Initialize");

  //read bgo xml file
  //getXMLData();

  REG_HISTOGRAM

}

void BgoStudyModule::beginRun()
{
}

void BgoStudyModule::event()
{
  //Here comes the actual event processing
  StoreArray<BgoSimHit> SimHits;
  StoreArray<BgoHit> Hits;
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
    h_bgo_s->Fill(-s);
    if ((-33.0 <= -s && -s <= -30.0) || (19.0 <= -s && -s <= 23.0)) {
      h_bgo_s_cut->Fill(-s);
      Reject = true;
    }
  }
  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    BgoSimHit* aHit = SimHits[i];
    int detNB = aHit->getCellId();
    //int trkID = aHit->getTrackId();
    int pdg = aHit->getPDGCode();
    double Edep = aHit->getEnergyDep() * 1e3; //GeV -> MeV
    double tof = aHit->getFlightTime(); //ns

    h_bgos_Evtof1[detNB]->Fill(tof, Edep);
    if (pdg == 22) h_bgos_Evtof2[detNB]->Fill(tof, Edep);
    else if (fabs(pdg) == 11) h_bgos_Evtof3[detNB]->Fill(tof, Edep);
    else h_bgos_Evtof4[detNB]->Fill(tof, Edep);
    h_bgos_edep2[detNB]->Fill(Edep);
    if (Edep > m_Ethres[detNB]) {
      h_bgos_edep[detNB]->Fill(Edep);
      if (!Reject)h_bgos_edep_test[detNB]->Fill(Edep);
    }
    h_bgo_rate[0]->Fill(detNB);
    h_bgo_rate[1]->Fill(detNB, rate);
  }

  for (const auto& Hit : Hits) {
    const int detNB = Hit.getCellId();
    const double edep = Hit.getEnergyDep() * 1e3; //GeV -> MeV
    const double recedep = Hit.getEnergyRecDep() * 1e3; //GeV -> MeV
    const double tof = Hit.getFlightTime(); //ns
    h_bgo_rate[2]->Fill(detNB);
    h_bgo_rate[3]->Fill(detNB, rate);
    h_bgo_edep[detNB]->Fill(recedep);
    h_bgo_edep_test[detNB]->Fill(edep);
    h_bgo_edepWeight[detNB]->Fill(recedep, rate);
    h_bgo_edep_testWeight[detNB]->Fill(edep, rate);
    h_bgo_Evtof1[detNB]->Fill(tof, recedep);
    h_bgo_Evtof2[detNB]->Fill(tof, edep);
    double ethres = m_Ethres[detNB] * 1e3;
    if (recedep > ethres) {
      h_bgo_edep1[detNB]->Fill(recedep);
      h_bgo_edep2[detNB]->Fill(recedep, rate);
      h_bgo_rate[4]->Fill(detNB);
      h_bgo_rate[5]->Fill(detNB, rate);
    }
    ethres += gRandom->Gaus(0, m_Ethres[detNB] * 1e3 * 0.3);
    if (recedep > ethres) {
      h_bgo_edep3[detNB]->Fill(recedep);
      h_bgo_edep4[detNB]->Fill(recedep, rate);
      h_bgo_rate[6]->Fill(detNB);
      h_bgo_rate[7]->Fill(detNB, rate);
    }
  }

}
/*
//read tube centers, impulse response, and garfield drift data filename from BGO.xml
void BgoStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BGO\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("BgoStudy");

}
*/
void BgoStudyModule::endRun()
{


}

void BgoStudyModule::terminate()
{
}


