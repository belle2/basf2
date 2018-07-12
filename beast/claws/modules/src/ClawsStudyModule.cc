/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/claws/modules/ClawsStudyModule.h>
#include <beast/claws/dataobjects/CLAWSSimHit.h>
#include <beast/claws/dataobjects/ClawsHit.h>
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

using namespace std;

using namespace Belle2;
using namespace claws;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ClawsStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ClawsStudyModule::ClawsStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Clawss (BEAST)");

  addParam("Ethres", m_Ethres, "Energy threshold in MeV", 0.0);
}

ClawsStudyModule::~ClawsStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void ClawsStudyModule::defineHisto()
{
  /*
  for (int i = 0; i < 16; i++) {
    h_clawss_Evtof1[i] = new TH2F(TString::Format("clawss_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 500, 0., 1000.,
                                 100, 0., 10.);
    h_clawss_Evtof2[i] = new TH2F(TString::Format("clawss_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 500, 0.,
                                 100., 1000, 0., 10.);
    h_clawss_Evtof3[i] = new TH2F(TString::Format("clawss_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 500, 0.,
                                 100., 1000, 0., 10.);
    h_clawss_Evtof4[i] = new TH2F(TString::Format("clawss_Evtof4_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 500, 0.,
                                 100., 1000, 0., 10.);
    h_clawss_edep[i] = new TH1F(TString::Format("clawss_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
    h_Wclawss_edep[i] = new TH1F(TString::Format("Wclawss_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
  }
  */
  h_clawss_hitrate1 = new TH1F("clawss_hitrate1", "Hit distributions", 16, 0., 16.);
  h_clawss_hitrate2 = new TH1F("clawss_hitrate2", "Hit distributions", 16, 0., 16.);
  h_clawss_hitrate1W = new TH1F("clawss_hitrate1W", "Hit distributions", 16, 0., 16.);
  h_clawss_hitrate2W = new TH1F("clawss_hitrate2W", "Hit distributions", 16, 0., 16.);

  h_clawss_hitrate1->Sumw2();
  h_clawss_hitrate1W->Sumw2();
  h_clawss_hitrate2->Sumw2();
  h_clawss_hitrate2W->Sumw2();

  h_clawss_rs_hitrate1 = new TH2F("clawss_rs_hitrate1", "Hit distributions vs rs", 16, 0., 16., 12, 0., 12.);
  h_clawss_rs_hitrate2 = new TH2F("clawss_rs_hitrate2", "Hit distributions vs rs", 16, 0., 16., 12, 0., 12.);
  h_clawss_rs_hitrate1W = new TH2F("clawss_rs_hitrate1W", "Hit distributions vs rs", 16, 0., 16., 12, 0., 12.);
  h_clawss_rs_hitrate2W = new TH2F("clawss_rs_hitrate2W", "Hit distributions vs rs", 16, 0., 16., 12, 0., 12.);

  h_clawss_rs_hitrate1->Sumw2();
  h_clawss_rs_hitrate1W->Sumw2();
  h_clawss_rs_hitrate2->Sumw2();
  h_clawss_rs_hitrate2W->Sumw2();

  for (int i = 0; i < 16; i++) {
    h_clawss_rate1[i] = new TH1F(TString::Format("clawss_rate1_%d", i), "PE distributions", 500, 0., 5000.);
    h_clawss_rate2[i] = new TH1F(TString::Format("clawss_rate2_%d", i), "PE distributions", 500, 0., 5000.);
    h_clawss_rate1W[i] = new TH1F(TString::Format("clawss_rate1W_%d", i), "PE distributions", 500, 0., 5000.);
    h_clawss_rate2W[i] = new TH1F(TString::Format("clawss_rate2W_%d", i), "PE distributions", 500, 0., 5000.);
    h_clawss_pe1[i] = new TH2F(TString::Format("clawss_pe1_%d", i), "PE distributions", 500, 0., 5000., 100, 0., 1000.);
    h_clawss_pe2[i] = new TH2F(TString::Format("clawss_pe2_%d", i), "PE distributions", 500, 0., 5000., 100, 0., 1000.);
    h_clawss_pe1W[i] = new TH2F(TString::Format("clawss_pe1W_%d", i), "PE distributions", 500, 0., 5000., 100, 0., 1000.);
    h_clawss_pe2W[i] = new TH2F(TString::Format("clawss_pe2W_%d", i), "PE distributions", 500, 0., 5000., 100, 0., 1000.);

    h_clawss_rs_rate1[i] = new TH2F(TString::Format("clawss_rs_rate1_%d", i), "PE distributions", 500, 0., 5000., 12, 0., 12.);
    h_clawss_rs_rate2[i] = new TH2F(TString::Format("clawss_rs_rate2_%d", i), "PE distributions", 500, 0., 5000., 12, 0., 12.);
    h_clawss_rs_rate1W[i] = new TH2F(TString::Format("clawss_rs_rate1W_%d", i), "PE distributions", 500, 0., 5000., 12, 0., 12.);
    h_clawss_rs_rate2W[i] = new TH2F(TString::Format("clawss_rs_rate2W_%d", i), "PE distributions", 500, 0., 5000., 12, 0., 12.);

    h_clawss_rate1[i]->Sumw2();
    h_clawss_rate2[i]->Sumw2();
    h_clawss_rate1W[i]->Sumw2();
    h_clawss_rate2W[i]->Sumw2();
    h_clawss_rs_rate1[i]->Sumw2();
    h_clawss_rs_rate2[i]->Sumw2();
    h_clawss_rs_rate1W[i]->Sumw2();
    h_clawss_rs_rate2W[i]->Sumw2();
    h_clawss_pe1[i]->Sumw2();
    h_clawss_pe2[i]->Sumw2();
    h_clawss_pe1W[i]->Sumw2();
    h_clawss_pe2W[i]->Sumw2();
  }
}


void ClawsStudyModule::initialize()
{
  B2INFO("ClawsStudyModule: Initialize");

  REG_HISTOGRAM

  //get CLAWSS paramters ie energy threshold
  getXMLData();
}

void ClawsStudyModule::beginRun()
{
}

void ClawsStudyModule::event()
{
  //Here comes the actual event processing
  StoreArray<CLAWSSimHit>  SimHits;
  StoreArray<ClawsHit> Hits;
  StoreArray<SADMetaHit> MetaHits;

  double rate = 0;
  int ring_section = -1;
  for (const auto& MetaHit : MetaHits) {
    rate = MetaHit.getrate();
    ring_section = MetaHit.getring_section() - 1;
  }

  /*
  //number of entries in SimHits
  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    CLAWSSimHit* aHit = SimHits[i];
        int lad = aHit->getLadder();
  int sen = aHit->getSensor();
    //const int detNb = SimHit.getCellId();
    //int pdg = SimHit.getPDGCode();
    int detNB = (lad - 1) * 8 + sen - 1;
    //int detNB = aHit->getCellId();
    if (detNB < 16) {
      //int trkID = aHit->getTrackId();
      int pdg = aHit->getPDG();
      double Edep = aHit->getEnergyVisible() * 1e3; //GeV -> MeV
      double tof = aHit->getTime(); //ns

      //h_clawss_Evtof1[detNB]->Fill(tof, Edep);
      //if (pdg == 22) h_clawss_Evtof2[detNB]->Fill(tof, Edep);
      //else if (fabs(pdg) == 11) h_clawss_Evtof3[detNB]->Fill(tof, Edep);
      //else h_clawss_Evtof4[detNB]->Fill(tof, Edep);
      if (Edep > m_Ethres) {
        //h_clawss_edep[detNB]->Fill(Edep);
        //h_Wclawss_edep[detNB]->Fill(Edep, rate);
      }

    }
  }
  */
  for (const auto& Hit : Hits) {
    const int detNb = Hit.getdetNb();
    if (detNb < 16) {
      const int timebin = Hit.gettime();
      const float edep = Hit.getedep();
      const float pe = Hit.getPE();
      h_clawss_hitrate1->Fill(detNb);
      h_clawss_hitrate1W->Fill(detNb, rate);
      h_clawss_rate1[detNb]->Fill(pe);
      h_clawss_rate1W[detNb]->Fill(pe, rate);
      h_clawss_rs_rate1[detNb]->Fill(pe, ring_section);
      h_clawss_rs_rate1W[detNb]->Fill(pe, ring_section, rate);
      h_clawss_rs_hitrate1->Fill(detNb, ring_section);
      h_clawss_rs_hitrate1W->Fill(detNb, ring_section, rate);
      h_clawss_pe1[detNb]->Fill(timebin, pe);
      h_clawss_pe1W[detNb]->Fill(timebin, pe, rate);
      if (edep > m_Ethres) {
        h_clawss_hitrate2->Fill(detNb);
        h_clawss_hitrate2W->Fill(detNb, rate);
        h_clawss_rate2[detNb]->Fill(pe);
        h_clawss_rate2W[detNb]->Fill(pe, rate);
        h_clawss_rs_rate2[detNb]->Fill(pe, ring_section);
        h_clawss_rs_rate2W[detNb]->Fill(pe, ring_section, rate);
        h_clawss_rs_hitrate2->Fill(detNb, ring_section);
        h_clawss_rs_hitrate2W->Fill(detNb, ring_section, rate);
        h_clawss_pe2[detNb]->Fill(timebin, pe);
        h_clawss_pe2W[detNb]->Fill(timebin, pe, rate);
      }
    }
  }

}

//read energy threshold from CLAWS.xml
void ClawsStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CLAWS\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("ClawsStudy");
}


void ClawsStudyModule::endRun()
{



}

void ClawsStudyModule::terminate()
{
}


