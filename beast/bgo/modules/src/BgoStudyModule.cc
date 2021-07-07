/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/bgo/modules/BgoStudyModule.h>
#include <beast/bgo/dataobjects/BgoSimHit.h>
#include <beast/bgo/dataobjects/BgoHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <cmath>

#include <fstream>
#include <string>

// ROOT
#include <TH1.h>
#include <TH2.h>

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
  for (int i = 0; i < 2; i++) {
    h_bgo_rate[i] = new TH1F(TString::Format("bgo_rate_%d", i), "count", 18, 0., 18.);
    h_bgo_rate[i]->Sumw2();

    h_bgo_rs_rate[i] = new TH2F(TString::Format("bgo_rs_rate_%d", i), "count v. ring section", 18, 0., 18., 12, 0., 12.);
    h_bgo_rs_rate[i]->Sumw2();
  }
  for (int i = 0; i < 18; i++) {
    h_bgo_Evtof[i] = new TH2F(TString::Format("bgo_Evtof_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                              1000, 0., 400.);
    h_bgo_Evtof1[i] = new TH2F(TString::Format("bgo_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                               1000, 0., 400.);
    h_bgo_Evtof2[i] = new TH2F(TString::Format("bgo_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                               1000., 1000, 0., 400.);
    h_bgo_Evtof3[i] = new TH2F(TString::Format("bgo_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                               1000., 1000, 0., 400.);

    h_bgo_edep[i] = new TH1F(TString::Format("bgo_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep1[i] = new TH1F(TString::Format("bgo_edep1_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep2[i] = new TH1F(TString::Format("bgo_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep1Weight[i] = new TH1F(TString::Format("bgo_edep1Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_bgo_edep2Weight[i] = new TH1F(TString::Format("bgo_edep2Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);

    h_bgo_rs_edep1[i] = new TH2F(TString::Format("bgo_rs_edep1_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0., 12.);
    h_bgo_rs_edep2[i] = new TH2F(TString::Format("bgo_rs_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0., 12.);
    h_bgo_rs_edep1Weight[i] = new TH2F(TString::Format("bgo_rs_edep1Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0.,
                                       12.);
    h_bgo_rs_edep2Weight[i] = new TH2F(TString::Format("bgo_rs_edep2Weight_%d", i), "Energy deposited [MeV]", 5000, 0., 400., 12, 0.,
                                       12.);

    h_bgo_edep[i]->Sumw2();
    h_bgo_edep1[i]->Sumw2();
    h_bgo_edep2[i]->Sumw2();
    h_bgo_edep1Weight[i]->Sumw2();
    h_bgo_edep2Weight[i]->Sumw2();
    h_bgo_rs_edep1[i]->Sumw2();
    h_bgo_rs_edep2[i]->Sumw2();
    h_bgo_rs_edep1Weight[i]->Sumw2();
    h_bgo_rs_edep2Weight[i]->Sumw2();
  }

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
    else ssraw = 3000. + sad_ssraw / 100.;
    //else if (sad_ssraw < 0) ssraw = 3000. + sad_ssraw / 100.;
    ring_section = section_ordering[(int)((ssraw) / 250.)] - 1;
    //ring_section = MetaHit.getring_section() - 1;
  }
  //Loop over SimHit
  for (const auto& SimHit : SimHits) {
    int detNB = SimHit.getCellId();
    int pdg = SimHit.getPDGCode();
    double Edep = SimHit.getEnergyDep() * 1e3; //GeV -> MeV
    double tof = SimHit.getFlightTime(); //ns

    h_bgo_edep[detNB]->Fill(Edep);
    h_bgo_Evtof[detNB]->Fill(tof, Edep);
    if (pdg == Const::photon.getPDGCode()) h_bgo_Evtof1[detNB]->Fill(tof, Edep);
    else if (fabs(pdg) == Const::electron.getPDGCode()) h_bgo_Evtof2[detNB]->Fill(tof, Edep);
    double RecEdep = Edep;
    h_bgo_rate[0]->Fill(detNB);
    h_bgo_rate[1]->Fill(detNB, rate);
    h_bgo_edep1[detNB]->Fill(Edep);
    h_bgo_edep2[detNB]->Fill(RecEdep);
    h_bgo_edep1Weight[detNB]->Fill(Edep, rate);
    h_bgo_edep2Weight[detNB]->Fill(RecEdep, rate);
    h_bgo_Evtof3[detNB]->Fill(tof, RecEdep);
    h_bgo_rs_rate[0]->Fill(detNB, ring_section);
    h_bgo_rs_rate[1]->Fill(detNB, ring_section, rate);
    h_bgo_rs_edep1[detNB]->Fill(Edep, ring_section);
    h_bgo_rs_edep2[detNB]->Fill(RecEdep, ring_section);
    h_bgo_rs_edep1Weight[detNB]->Fill(Edep, ring_section, rate);
    h_bgo_rs_edep2Weight[detNB]->Fill(RecEdep, ring_section, rate);
  }
  /*
  //Loop over DigiHit
  for (const auto& Hit : Hits) {
    int detNB = Hit.getCellId();
    double Edep = Hit.getEnergyDep() * 1e3; //GeV -> MeV
    double RecEdep = Hit.getEnergyRecDep() * 1e3; //GeV -> MeV
    double tof = Hit.getFlightTime(); //ns
    h_bgo_rate[0]->Fill(detNB);
    h_bgo_rate[1]->Fill(detNB, rate);
    h_bgo_edep1[detNB]->Fill(Edep);
    h_bgo_edep2[detNB]->Fill(RecEdep);
    h_bgo_edep1Weight[detNB]->Fill(Edep, rate);
    h_bgo_edep2Weight[detNB]->Fill(RecEdep, rate);
    h_bgo_Evtof3[detNB]->Fill(tof, RecEdep);
    h_bgo_rs_rate[0]->Fill(detNB, ring_section);
    h_bgo_rs_rate[1]->Fill(detNB, ring_section, rate);
    h_bgo_rs_edep1[detNB]->Fill(Edep, ring_section);
    h_bgo_rs_edep2[detNB]->Fill(RecEdep, ring_section);
    h_bgo_rs_edep1Weight[detNB]->Fill(Edep, ring_section, rate);
    h_bgo_rs_edep2Weight[detNB]->Fill(RecEdep, ring_section, rate);
  }
  */
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


