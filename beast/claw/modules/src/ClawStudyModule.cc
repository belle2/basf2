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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
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

  for (int i = 0; i < 8; i++) {
    h_claws_rate1[i] = new TH1F(TString::Format("claws_rate1_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_rate2[i] = new TH1F(TString::Format("claws_rate2_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_rate1W[i] = new TH1F(TString::Format("claws_rate1W_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_rate2W[i] = new TH1F(TString::Format("claws_rate2W_%d", i), "PE distributions", 5000, 0., 5000.);
    h_claws_pe1[i] = new TH2F(TString::Format("claws_pe1_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);
    h_claws_pe2[i] = new TH2F(TString::Format("claws_pe2_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);
    h_claws_pe1W[i] = new TH2F(TString::Format("claws_pe1W_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);
    h_claws_pe2W[i] = new TH2F(TString::Format("claws_pe2W_%d", i), "PE distributions", 5000, 0., 5000., 1000, 0., 1000.);
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
  StoreArray<SADMetaHit> sadMetaHits;
  double rate = 0;
  for (const auto& sadMetaHit : sadMetaHits) {
    rate = sadMetaHit.getrate();
  }

  //number of entries in SimHits
  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    ClawSimHit* aHit = SimHits[i];
    int detNB = aHit->getCellId();
    if (detNB < 8) {
      //int trkID = aHit->getTrackId();
      int pdg = aHit->getPDGCode();
      double Edep = aHit->getEnergyDep() * 1e3; //GeV -> MeV
      double tof = aHit->getFlightTime(); //ns

      h_claws_Evtof1[detNB]->Fill(tof, Edep);
      if (pdg == 22) h_claws_Evtof2[detNB]->Fill(tof, Edep);
      else if (fabs(pdg) == 11) h_claws_Evtof3[detNB]->Fill(tof, Edep);
      else h_claws_Evtof4[detNB]->Fill(tof, Edep);
      if (Edep > m_Ethres) {
        h_claws_edep[detNB]->Fill(Edep);
        h_Wclaws_edep[detNB]->Fill(Edep, rate);
      }
    }
  }

  for (const auto& Hit : Hits) {
    const int detNb = Hit.getdetNb();
    const int timebin = Hit.gettime();
    const float edep = Hit.getedep();
    const float pe = Hit.getPE();
    h_claws_rate1[detNb]->Fill(pe);
    h_claws_rate1W[detNb]->Fill(pe, rate);
    h_claws_pe1[detNb]->Fill(timebin, pe);
    h_claws_pe1W[detNb]->Fill(timebin, pe, rate);
    if (edep > m_Ethres) {
      h_claws_rate2[detNb]->Fill(pe);
      h_claws_rate2W[detNb]->Fill(pe, rate);
      h_claws_pe2[detNb]->Fill(timebin, pe);
      h_claws_pe2W[detNb]->Fill(timebin, pe, rate);
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


