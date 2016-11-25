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
    h_dosi_edep0[i] = new TH1F(TString::Format("dosi_edep0_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_dosi_edep1[i] = new TH1F(TString::Format("dosi_edep1_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_dosi_edep2[i] = new TH1F(TString::Format("dosi_edep2_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);
    h_dosi_edep3[i] = new TH1F(TString::Format("dosi_edep3_%d", i), "Energy deposited [MeV]", 5000, 0., 400.);

    h_dosi_edep0[i]->Sumw2();
    h_dosi_edep1[i]->Sumw2();
    h_dosi_edep2[i]->Sumw2();
    h_dosi_edep3[i]->Sumw2();
  }

}


void DosiStudyModule::initialize()
{
  B2INFO("DosiStudyModule: Initialize");

  //read dosi xml file
  //getXMLData();

  REG_HISTOGRAM

}

void DosiStudyModule::beginRun()
{
}

void DosiStudyModule::event()
{
  //Here comes the actual event processing
  StoreArray<DosiSimHit> SimHits;
  StoreArray<DosiHit> Hits;
  StoreArray<SADMetaHit> MetaHits;

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

  //Loop over SimHit
  for (const auto& SimHit : SimHits) {
    int detNB = SimHit.getCellId();
    double Edep = SimHit.getEnergyDep() * 1e3; //GeV -> MeV
    h_dosi_edep0[detNB]->Fill(Edep);
    int pdg = SimHit.getPDGCode();
    h_dosi_edep1[detNB]->Fill(Edep);
    if (0.01 < Edep && Edep < 10.0 && (fabs(pdg) == 11 || pdg == 22)) h_dosi_edep2[detNB]->Fill(Edep);
    if (0.01 < Edep && Edep < 10.0 && pdg == 2112) h_dosi_edep3[detNB]->Fill(Edep);
  }
  /*
  //Loop over DigiHit
  for (const auto& Hit : Hits) {
    int detNB = Hit.getCellId();
    double Edep = Hit.getEnergyDep() * 1e3; //GeV -> MeV

  }
  */
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


