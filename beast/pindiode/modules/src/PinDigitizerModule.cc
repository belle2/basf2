/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/pindiode/modules/PinDigitizerModule.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <beast/pindiode/dataobjects/PindiodeHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


using namespace std;
using namespace Belle2;
using namespace pindiode;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PinDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PinDigitizerModule::PinDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Pindiode digitizer module");

  //Default values are set here. New values can be in PINDIODE.xml.

}

PinDigitizerModule::~PinDigitizerModule()
{
}

void PinDigitizerModule::initialize()
{
  B2INFO("PinDigitizer: Initializing");
  StoreArray<PindiodeHit>::registerPersistent();

  //get the garfield drift data, gas, and PIN paramters
  getXMLData();

}

void PinDigitizerModule::beginRun()
{
}

void PinDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<PindiodeSimHit> PinSimHits;

  //Skip events with no PinSimHits, but continue the event counter
  if (PinSimHits.getEntries() == 0) {
    Event++;
    return;
  }


  Event++;

}
//read tube centers, impulse response, and garfield drift data filename from PINDIODE.xml
void PinDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"PINDIODE\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    PINCenter.push_back(TVector3(activeParams.getLength("PINpos_x"), activeParams.getLength("PINpos_y"),
                                 activeParams.getLength("PINpos_z")));
    nPIN++;
  }

  B2INFO("PinDigitizer: Aquired pin locations and gas parameters");
  B2INFO("              from PINDIODE.xml. There are " << nPIN << " PINs implemented");

}

void PinDigitizerModule::endRun()
{
}

void PinDigitizerModule::terminate()
{
}


