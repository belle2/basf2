/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/claw/modules/ClawStudyModule.h>
#include <beast/claw/dataobjects/ClawSimHit.h>
#include <beast/claw/dataobjects/ClawHit.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <cmath>

#include <fstream>
#include <string>

// ROOT
#include <TH1.h>
#include <TH2.h>

int eventNum = 0;

using namespace std;

using namespace Belle2;
using namespace claw;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ClawStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ClawStudyModule::ClawStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Claws (BEAST)");

  addParam("Ethres", m_Ethres, "Energy threshold in MeV", 0.0);
}

ClawStudyModule::~ClawStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void ClawStudyModule::defineHisto()
{
  for (int i = 0; i < 8; i++) {
    h_claws_Evtof1[i] = new TH2F(TString::Format("claws_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                                 1000, 0., 10.);
    h_claws_Evtof2[i] = new TH2F(TString::Format("claws_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                                 1000., 1000, 0., 10.);
    h_claws_Evtof3[i] = new TH2F(TString::Format("claws_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                                 1000., 1000, 0., 10.);
    h_claws_Evtof4[i] = new TH2F(TString::Format("claws_Evtof4_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                                 1000., 1000, 0., 10.);
    h_claws_edep[i] = new TH1F(TString::Format("claws_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
    h_Wclaws_edep[i] = new TH1F(TString::Format("Wclaws_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
  }

  h_claws_hitrate1 = new TH1F("claws_hitrate1", "Hit distributions", 8, 0., 8.);
  h_claws_hitrate2 = new TH1F("claws_hitrate2", "Hit distributions", 8, 0., 8.);
  h_claws_hitrate1W = new TH1F("claws_hitrate1W", "Hit distributions", 8, 0., 8.);
  h_claws_hitrate2W = new TH1F("claws_hitrate2W", "Hit distributions", 8, 0., 8.);

  h_claws_hitrate1->Sumw2();
  h_claws_hitrate1W->Sumw2();
  h_claws_hitrate2->Sumw2();
  h_claws_hitrate2W->Sumw2();

  h_claws_rs_hitrate1 = new TH2F("claws_rs_hitrate1", "Hit distributions vs rs", 8, 0., 8., 12, 0., 12.);
  h_claws_rs_hitrate2 = new TH2F("claws_rs_hitrate2", "Hit distributions vs rs", 8, 0., 8., 12, 0., 12.);
  h_claws_rs_hitrate1W = new TH2F("claws_rs_hitrate1W", "Hit distributions vs rs", 8, 0., 8., 12, 0., 12.);
  h_claws_rs_hitrate2W = new TH2F("claws_rs_hitrate2W", "Hit distributions vs rs", 8, 0., 8., 12, 0., 12.);

  h_claws_rs_hitrate1->Sumw2();
  h_claws_rs_hitrate1W->Sumw2();
  h_claws_rs_hitrate2->Sumw2();
  h_claws_rs_hitrate2W->Sumw2();

  for (int i = 0; i < 8; i++) {
    h_claws_rate1[i] = new TH1F(TString::Format("claws_rate1_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_rate2[i] = new TH1F(TString::Format("claws_rate2_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_rate1W[i] = new TH1F(TString::Format("claws_rate1W_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_rate2W[i] = new TH1F(TString::Format("claws_rate2W_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_pe1[i] = new TH2F(TString::Format("claws_pe1_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);
    h_claws_pe2[i] = new TH2F(TString::Format("claws_pe2_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);
    h_claws_pe1W[i] = new TH2F(TString::Format("claws_pe1W_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);
    h_claws_pe2W[i] = new TH2F(TString::Format("claws_pe2W_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);

    h_claws_rs_rate1[i] = new TH2F(TString::Format("claws_rs_rate1_%d", i), "PE distributions", 5000, 0., 5000., 12, 0., 12.);
    h_claws_rs_rate2[i] = new TH2F(TString::Format("claws_rs_rate2_%d", i), "PE distributions", 5000, 0., 5000., 12, 0., 12.);
    h_claws_rs_rate1W[i] = new TH2F(TString::Format("claws_rs_rate1W_%d", i), "PE distributions", 5000, 0., 5000., 12, 0., 12.);
    h_claws_rs_rate2W[i] = new TH2F(TString::Format("claws_rs_rate2W_%d", i), "PE distributions", 5000, 0., 5000., 12, 0., 12.);

    h_claws_rate1[i]->Sumw2();
    h_claws_rate2[i]->Sumw2();
    h_claws_rate1W[i]->Sumw2();
    h_claws_rate2W[i]->Sumw2();
    h_claws_rs_rate1[i]->Sumw2();
    h_claws_rs_rate2[i]->Sumw2();
    h_claws_rs_rate1W[i]->Sumw2();
    h_claws_rs_rate2W[i]->Sumw2();
    h_claws_pe1[i]->Sumw2();
    h_claws_pe2[i]->Sumw2();
    h_claws_pe1W[i]->Sumw2();
    h_claws_pe2W[i]->Sumw2();
  }
}


void ClawStudyModule::initialize()
{
  B2INFO("ClawStudyModule: Initialize");

  REG_HISTOGRAM

  //get CLAWS paramters ie energy threshold
  getXMLData();
}

void ClawStudyModule::beginRun()
{
}

void ClawStudyModule::event()
{
  //Here comes the actual event processing
  StoreArray<ClawSimHit>  SimHits;
  StoreArray<ClawHit> Hits;
  StoreArray<SADMetaHit> MetaHits;

  double rate = 0;
  int ring_section = -1;
  for (const auto& MetaHit : MetaHits) {
    rate = MetaHit.getrate();
    ring_section = MetaHit.getring_section() - 1;
  }

  //number of entries in SimHits
  //int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < (int) SimHits.getEntries(); i++) {
    ClawSimHit* aHit = SimHits[i];
    int detNB = aHit->getCellId();
    if (detNB < 8) {
      //int trkID = aHit->getTrackId();
      int pdg = aHit->getPDGCode();
      double Edep = aHit->getEnergyDep() * 1e3; //GeV -> MeV
      double tof = aHit->getFlightTime(); //ns

      h_claws_Evtof1[detNB]->Fill(tof, Edep);
      if (pdg == Const::photon.getPDGCode()) h_claws_Evtof2[detNB]->Fill(tof, Edep);
      else if (fabs(pdg) == Const::electron.getPDGCode()) h_claws_Evtof3[detNB]->Fill(tof, Edep);
      else h_claws_Evtof4[detNB]->Fill(tof, Edep);
      if (Edep > m_Ethres) {
        h_claws_edep[detNB]->Fill(Edep);
        h_Wclaws_edep[detNB]->Fill(Edep, rate);
      }
    }
  }

  for (const auto& Hit : Hits) {
    const int detNb = Hit.getdetNb();
    if (detNb < 8) {
      const int timebin = Hit.gettime();
      const float edep = Hit.getedep();
      const float pe = Hit.getPE();
      h_claws_hitrate1->Fill(detNb);
      h_claws_hitrate1W->Fill(detNb, rate);
      h_claws_rate1[detNb]->Fill(pe);
      h_claws_rate1W[detNb]->Fill(pe, rate);
      h_claws_rs_rate1[detNb]->Fill(pe, ring_section);
      h_claws_rs_rate1W[detNb]->Fill(pe, ring_section, rate);
      h_claws_rs_hitrate1->Fill(detNb, ring_section);
      h_claws_rs_hitrate1W->Fill(detNb, ring_section, rate);
      h_claws_pe1[detNb]->Fill(timebin, pe);
      h_claws_pe1W[detNb]->Fill(timebin, pe, rate);
      if (edep > m_Ethres) {
        h_claws_hitrate2->Fill(detNb);
        h_claws_hitrate2W->Fill(detNb, rate);
        h_claws_rate2[detNb]->Fill(pe);
        h_claws_rate2W[detNb]->Fill(pe, rate);
        h_claws_rs_rate2[detNb]->Fill(pe, ring_section);
        h_claws_rs_rate2W[detNb]->Fill(pe, ring_section, rate);
        h_claws_rs_hitrate2->Fill(detNb, ring_section);
        h_claws_rs_hitrate2W->Fill(detNb, ring_section, rate);
        h_claws_pe2[detNb]->Fill(timebin, pe);
        h_claws_pe2W[detNb]->Fill(timebin, pe, rate);
      }
    }
  }

}

//read energy threshold from CLAW.xml
void ClawStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CLAW\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("ClawStudy");
}


void ClawStudyModule::endRun()
{



}

void ClawStudyModule::terminate()
{
}


