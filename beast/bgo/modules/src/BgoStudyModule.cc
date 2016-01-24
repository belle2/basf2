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

  addParam("Ethres", m_Ethres, "Energy threshold in MeV", 0.0);
}

BgoStudyModule::~BgoStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void BgoStudyModule::defineHisto()
{
  for (int i = 0; i < 8; i++) {
    h_bgos_Evtof1[i] = new TH2F(TString::Format("h_bgos_Evtof1_%d", i), "Energy deposited [MeV] vs TOF [ns] - all", 5000, 0., 1000.,
                                1000, 0., 10.);
    h_bgos_Evtof2[i] = new TH2F(TString::Format("h_bgos_Evtof2_%d", i), "Energy deposited [MeV] vs TOF [ns] - only photons", 5000, 0.,
                                1000., 1000, 0., 10.);
    h_bgos_Evtof3[i] = new TH2F(TString::Format("h_bgos_Evtof3_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                                1000., 1000, 0., 10.);
    h_bgos_Evtof4[i] = new TH2F(TString::Format("h_bgos_Evtof4_%d", i), "Energy deposited [MeV] vs TOF [ns] - only e+/e-", 5000, 0.,
                                1000., 1000, 0., 10.);
    h_bgos_edep[i] = new TH1F(TString::Format("h_bgos_edep_%d", i), "Energy deposited [MeV]", 5000, 0., 10.);
  }

}


void BgoStudyModule::initialize()
{
  B2INFO("BgoStudyModule: Initialize");

  //read bgo xml file
  getXMLData();

  REG_HISTOGRAM

}

void BgoStudyModule::beginRun()
{
}

void BgoStudyModule::event()
{
  //Here comes the actual event processing
  StoreArray<BgoSimHit>  SimHits;

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

  int nSimHits = SimHits.getEntries();

  //loop over all SimHit entries
  for (int i = 0; i < nSimHits; i++) {
    BgoSimHit* aHit = SimHits[i];
    int detNB = aHit->getCellId();
    //int trkID = aHit->getTrackId();
    int pdg = aHit->getPDGCode();
    double Edep = aHit->getEnergyDep(); //GeV
    double tof = aHit->getFlightTime(); //ns

    h_bgos_Evtof1[detNB]->Fill(tof, Edep);
    if (pdg == 22) h_bgos_Evtof2[detNB]->Fill(tof, Edep);
    else if (fabs(pdg) == 11) h_bgos_Evtof3[detNB]->Fill(tof, Edep);
    else h_bgos_Evtof4[detNB]->Fill(tof, Edep);
    if (Edep > m_Ethres)h_bgos_edep[detNB]->Fill(Edep);
  }


}
//read tube centers, impulse response, and garfield drift data filename from BGO.xml
void BgoStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BGO\"]/Content/");
  m_Ethres = content.getDouble("Ethres");

  B2INFO("BgoStudy");

}
void BgoStudyModule::endRun()
{


}

void BgoStudyModule::terminate()
{
}


