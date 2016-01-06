/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/pindiode/modules/PindiodeStudyModule.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <beast/pindiode/dataobjects/PindiodeHit.h>
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
using namespace pindiode;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PindiodeStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PindiodeStudyModule::PindiodeStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Pindiodes (BEAST)");

}

PindiodeStudyModule::~PindiodeStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void PindiodeStudyModule::defineHisto()
{
  //Default values are set here. New values can be in PINDIODE.xml.
  for (int i = 0; i < 100; i++) {
    h_dose[i] = new TH1F(TString::Format("h_dose_%d", i), "", 10000, 0., 10.);
    h_volt[i] = new TH1F(TString::Format("h_volt_%d", i), "", 10000, 0., 10.);
    h_time[i] = new TH1F(TString::Format("h_time_%d", i), "", 1000, 0., 1000.);
    h_vtime[i] = new TH1F(TString::Format("h_vtime_%d", i), "", 1000, 0., 1000.);
  }

}


void PindiodeStudyModule::initialize()
{
  B2INFO("PindiodeStudyModule: Initialize");

  //read pindiode xml file
  getXMLData();

  REG_HISTOGRAM

}

void PindiodeStudyModule::beginRun()
{
}

void PindiodeStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<PindiodeSimHit>  SimHits;
  StoreArray<PindiodeHit> Hits;

  //Skip events with no Hits
  if (Hits.getEntries() == 0) {
    return;
  }

  //int nSimHits = SimHits.getEntries();
  int nHits = Hits.getEntries();
  cout << "nHits " << nHits << endl;


  for (int i = 0; i < nHits; i++) {
    PindiodeHit* aHit = Hits[i];
    int detNb = aHit->getdetNb();
    double edep = aHit->getedep();
    double volt = aHit->getV();
    double time = aHit->gettime();
    h_dose[detNb]->Fill(edep);
    h_volt[detNb]->Fill(volt);
    h_time[detNb]->Fill(time);
    h_vtime[detNb]->Fill(time, volt);
  }


}
//read tube centers, impulse response, and garfield drift data filename from PINDIODE.xml
void PindiodeStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"PINDIODE\"]/Content/");
  /*
  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    PINCenter.push_back(TVector3(activeParams.getLength("z_pindiode"), activeParams.getLength("r_pindiode"),
                                 activeParams.getLength("Phi")));
    nPIN++;
  }
  */
  B2INFO("PinDigitizer");

}
void PindiodeStudyModule::endRun()
{


}

void PindiodeStudyModule::terminate()
{
}


