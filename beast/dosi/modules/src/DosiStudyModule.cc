/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/dosi/modules/DosiStudyModule.h>
#include <beast/dosi/dataobjects/DosiSimHit.h>
#include <beast/dosi/dataobjects/DosiHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

//c++
#include <cmath>
#include <string>
#include <fstream>

// ROOT
#include <TString.h>
#include <TH1.h>
#include <TH2.h>

#include <generators/SAD/dataobjects/SADMetaHit.h>

using namespace std;
using namespace Belle2;
using namespace dosi;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DosiStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DosiStudyModule::DosiStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Dosis (BEAST)");
}

DosiStudyModule::~DosiStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void DosiStudyModule::defineHisto()
{
  for (int i = 0; i < 5; i++) {

















    h_dosi_edep0[i] = new TH1F(TString::Format("dosi_edep0_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep1[i] = new TH1F(TString::Format("dosi_edep1_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep2[i] = new TH1F(TString::Format("dosi_edep2_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep3[i] = new TH1F(TString::Format("dosi_edep3_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep4[i] = new TH1F(TString::Format("dosi_edep4_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep5[i] = new TH1F(TString::Format("dosi_edep5_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep6[i] = new TH1F(TString::Format("dosi_edep6_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep7[i] = new TH1F(TString::Format("dosi_edep7_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);
    h_dosi_edep8[i] = new TH1F(TString::Format("dosi_edep8_%d", i), "Energy deposited [MeV]", 50000, 0., 400.);

    h_dosi_edep0[i]->Sumw2();
    h_dosi_edep1[i]->Sumw2();
    h_dosi_edep2[i]->Sumw2();
    h_dosi_edep3[i]->Sumw2();
    h_dosi_edep4[i]->Sumw2();
    h_dosi_edep5[i]->Sumw2();
    h_dosi_edep6[i]->Sumw2();
    h_dosi_edep7[i]->Sumw2();
    h_dosi_edep8[i]->Sumw2();

    h_dosi_rs_edep0[i] = new TH2F(TString::Format("dosi_rs_edep0_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep1[i] = new TH2F(TString::Format("dosi_rs_edep1_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep2[i] = new TH2F(TString::Format("dosi_rs_edep2_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep3[i] = new TH2F(TString::Format("dosi_rs_edep3_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep4[i] = new TH2F(TString::Format("dosi_rs_edep4_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep5[i] = new TH2F(TString::Format("dosi_rs_edep5_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep6[i] = new TH2F(TString::Format("dosi_rs_edep6_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep7[i] = new TH2F(TString::Format("dosi_rs_edep7_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);
    h_dosi_rs_edep8[i] = new TH2F(TString::Format("dosi_rs_edep8_%d", i), "Energy deposited [MeV]", 50000, 0., 400., 12, 0., 12.);

    h_dosi_rs_edep0[i]->Sumw2();
    h_dosi_rs_edep1[i]->Sumw2();
    h_dosi_rs_edep2[i]->Sumw2();
    h_dosi_rs_edep3[i]->Sumw2();
    h_dosi_rs_edep4[i]->Sumw2();
    h_dosi_rs_edep5[i]->Sumw2();
    h_dosi_rs_edep6[i]->Sumw2();
    h_dosi_rs_edep7[i]->Sumw2();
    h_dosi_rs_edep8[i]->Sumw2();
  }

}


void DosiStudyModule::initialize()
{
  B2INFO("DosiStudyModule: Initialize");

  //read dosi xml file
  //getXMLData();
  //StoreArray<MCParticle> mcParticles;
  //StoreArray<DosiSimHit> SimHits;
  //RelationArray relMCSimHit(mcParticles, SimHits);

  REG_HISTOGRAM

}

void DosiStudyModule::beginRun()
{
}

void DosiStudyModule::event()
{
  //Here comes the actual event processing
  StoreArray<MCParticle> mcParticles;
  StoreArray<DosiSimHit> SimHits;
  StoreArray<DosiHit> Hits;
  StoreArray<SADMetaHit> MetaHits;

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

  RelationIndex<MCParticle, DosiSimHit> relMCSimHit;
  typedef RelationIndex<MCParticle, DosiSimHit>::Element RelationElement;

  for (const auto& SimHit : SimHits) {
    const int detNB = SimHit.getCellId();
    const double Edep = SimHit.getEnergyDep() * 1e3; //GeV -> MeV
    h_dosi_edep0[detNB]->Fill(Edep);
    h_dosi_rs_edep0[detNB]->Fill(Edep, ring_section);
    for (const RelationElement& rel : relMCSimHit.getElementsTo(SimHit)) {
      const MCParticle* particle = rel.from;
      const float Mass = particle->getMass();
      const float Energy = particle->getEnergy();
      const float Kinetic = (Energy - Mass) * 1e3; //GeV to MeV
      const int pdg = particle->getPDG();
      h_dosi_edep1[detNB]->Fill(Edep);
      if (0.005 <= Kinetic && Kinetic <= 10.0)
        h_dosi_edep2[detNB]->Fill(Edep);
      if (0.005 <= Kinetic && Kinetic <= 10.0 && (fabs(pdg) == Const::electron.getPDGCode()))
        h_dosi_edep3[detNB]->Fill(Edep);
      if (0.1 <= Kinetic && Kinetic <= 10.0 && (fabs(pdg) == Const::electron.getPDGCode()))
        h_dosi_edep4[detNB]->Fill(Edep);
      if (0.001 <= Kinetic && Kinetic <= 0.050)
        h_dosi_edep5[detNB]->Fill(Edep);
      if (pdg == Const::neutron.getPDGCode())
        h_dosi_edep6[detNB]->Fill(Edep);
      if (pdg == Const::neutron.getPDGCode() && (0.001 <= Kinetic && Kinetic <= 10.0))
        h_dosi_edep7[detNB]->Fill(Edep);


      h_dosi_rs_edep1[detNB]->Fill(Edep, ring_section);
      if (0.005 <= Kinetic && Kinetic <= 10.0)
        h_dosi_rs_edep2[detNB]->Fill(Edep, ring_section);
      if (0.005 <= Kinetic && Kinetic <= 10.0 && (fabs(pdg) == Const::electron.getPDGCode()))
        h_dosi_rs_edep3[detNB]->Fill(Edep, ring_section);
      if (0.1 <= Kinetic && Kinetic <= 10.0 && (fabs(pdg) == Const::electron.getPDGCode()))
        h_dosi_rs_edep4[detNB]->Fill(Edep, ring_section);
      if (0.001 <= Kinetic && Kinetic <= 0.050)
        h_dosi_rs_edep5[detNB]->Fill(Edep, ring_section);
      if (pdg == Const::neutron.getPDGCode())
        h_dosi_rs_edep6[detNB]->Fill(Edep, ring_section);
      if (pdg == Const::neutron.getPDGCode() && (0.001 <= Kinetic && Kinetic <= 10.0))
        h_dosi_rs_edep7[detNB]->Fill(Edep, ring_section);
    }
  }
}
/*
//read tube centers, impulse response, and garfield drift data filename from DOSI.xml
void DosiStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"DOSI\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("DosiStudy");

}
*/
void DosiStudyModule::endRun()
{


}

void DosiStudyModule::terminate()
{
}


