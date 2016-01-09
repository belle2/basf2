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

}

BgoStudyModule::~BgoStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void BgoStudyModule::defineHisto()
{
  //Default values are set here. New values can be in BGO.xml.
  for (int i = 0; i < 8; i++) {
    h_edep[i] = new TH1F(TString::Format("h_edep_%d", i), "", 10000, 0., 1000.);
    h_dose[i] = new TH1F(TString::Format("h_dose_%d", i), "", 10000, 0., 1000.);
    h_time[i] = new TH1F(TString::Format("h_time_%d", i), "", 1000, 0., 100.);
    h_vtime[i] = new TH1F(TString::Format("h_vtime_%d", i), "", 1000, 0., 100.);
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
  double Edep[8];
  for (int i = 0; i < 8; i ++)Edep[i] = 0;
  for (int i = 0; i < nSimHits; i++) {
    BgoSimHit* aHit = SimHits[i];
    int detNb = aHit->getCellId();
    double edep = aHit->getEnergyDep();
    double time = aHit->getFlightTime();
    Edep[detNb] += edep * 1e3;
    h_dose[detNb]->Fill(edep * 1e3); //GeV to MeV
    h_time[detNb]->Fill(time);
    h_vtime[detNb]->Fill(time, edep * 1e3);
  }
  for (int i = 0; i < 8; i ++) {
    if (Edep[i] > 0)h_edep[i]->Fill(Edep[i]);
  }

}
//read tube centers, impulse response, and garfield drift data filename from BGO.xml
void BgoStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BGO\"]/Content/");

  B2INFO("BgoStudy");

}
void BgoStudyModule::endRun()
{


}

void BgoStudyModule::terminate()
{
}


