/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/beamabort/modules/BeamDigitizerModule.h>
#include <beast/beamabort/dataobjects/BeamabortSimHit.h>
#include <beast/beamabort/dataobjects/BeamabortHit.h>

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
using namespace beamabort;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamDigitizerModule::BeamDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Beamabort digitizer module");

  //Default values are set here. New values can be in BEAMABORT.xml.

}

BeamDigitizerModule::~BeamDigitizerModule()
{
}

void BeamDigitizerModule::initialize()
{
  B2INFO("BeamDigitizer: Initializing");
  StoreArray<BeamabortHit>::registerPersistent();

  //get the garfield drift data, gas, and BEAM paramters
  getXMLData();

}

void BeamDigitizerModule::beginRun()
{
}

void BeamDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<BeamabortSimHit> BeamSimHits;

  //Skip events with no BeamSimHits, but continue the event counter
  if (BeamSimHits.getEntries() == 0) {
    Event++;
    return;
  }

  Event++;

}

//read tube centers, impulse response, and garfield drift data filename from BEAMABORT.xml
void BeamDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BEAMABORT\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    BEAMCenter.push_back(TVector3(activeParams.getLength("BEAMpos_x"), activeParams.getLength("BEAMpos_y"),
                                  activeParams.getLength("BEAMpos_z")));
    nBEAM++;
  }

  B2INFO("BeamDigitizer: Aquired beam locations and gas parameters");
  B2INFO("              from BEAMABORT.xml. There are " << nBEAM << " BEAMs implemented");

}

void BeamDigitizerModule::endRun()
{
}

void BeamDigitizerModule::terminate()
{
}


