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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>

// ROOT
#include <TVector3.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>
#include <TMath.h>

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

  RelationIndex<MCParticle, DosiSimHit> relMCSimHit;
  typedef RelationIndex<MCParticle, DosiSimHit>::Element RelationElement;

  for (const auto& SimHit : SimHits) {
    const int detNB = SimHit.getCellId();
    const double Edep = SimHit.getEnergyDep() * 1e3; //GeV -> MeV
    h_dosi_edep0[detNB]->Fill(Edep);
    for (const RelationElement& rel : relMCSimHit.getElementsTo(SimHit)) {
      const MCParticle* particle = rel.from;
      const float Mass = particle->getMass();
      const float Energy = particle->getEnergy();
      const float Kinetic = (Energy - Mass) * 1e3; //GeV to MeV
      const int pdg = particle->getPDG();
      h_dosi_edep1[detNB]->Fill(Edep);
      if (0.005 <= Kinetic && Kinetic <= 10.0)
        h_dosi_edep2[detNB]->Fill(Edep);
      if (0.005 <= Kinetic && Kinetic <= 10.0 && (fabs(pdg) == 11))
        h_dosi_edep3[detNB]->Fill(Edep);
      if (0.1 <= Kinetic && Kinetic <= 10.0 && (fabs(pdg) == 11))
        h_dosi_edep4[detNB]->Fill(Edep);
      if (0.001 <= Kinetic && Kinetic <= 0.050)
        h_dosi_edep5[detNB]->Fill(Edep);
      if (pdg == 2112)
        h_dosi_edep6[detNB]->Fill(Edep);
      if (pdg == 2112 && (0.001 <= Kinetic && Kinetic <= 10.0))
        h_dosi_edep7[detNB]->Fill(Edep);
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


