/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong, Jerome Baudot                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/plume/modules/PlumeDigitizerModule.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>

#include <mdst/dataobjects/MCParticle.h>
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
#include <sstream>
#include <iostream>
#include <fstream>

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;
using namespace plume;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PlumeDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PlumeDigitizerModule::PlumeDigitizerModule() : Module()
{
  // Set module properties
  setDescription("PLUME digitizer module");

  //All parameters needed are found in PLUME.xml.


}

PlumeDigitizerModule::~PlumeDigitizerModule()
{
}

void PlumeDigitizerModule::initialize()
{
  B2INFO("Initializing PlumeDigitizer");
  m_plumeHit.registerInDataStore();

  //get the sensor parameters and set the parameters of the response function
  getXMLData();
  setResponseModel(); // currently empty

}

void PlumeDigitizerModule::beginRun()
{
}

void PlumeDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<PlumeSimHit> plumeSimHits;
  StoreArray<PlumeHit> plumeHits;

  //skip events with no plumeSimHits, but continue the event counter
  if (plumeSimHits.getEntries() == 0) {
    Event++;
    return;
  }


  for (int i = 0; i < plumeSimHits.getEntries(); i++) {
    PlumeSimHit* aHit = plumeSimHits[i];

    // This is where the detailed response function shall be called
    m_nofPixels = 3;
    m_posmm_u = aHit->getposOUT_u();
    m_posmm_v = aHit->getposOUT_v();
    m_posmm_x = aHit->getposIN_x();
    m_posmm_y = aHit->getposIN_y();
    m_posmm_z = aHit->getposIN_z();

    //create PlumeHit
    plumeHits.appendNew(PlumeHit(aHit->getsensorID(), m_nofPixels, m_posmm_u, m_posmm_v, m_posmm_x, m_posmm_y,
                                 m_posmm_z));

  }

  Event++;
}


void PlumeDigitizerModule::setResponseModel()
{


  B2INFO("PLUME response model is so far empty, which means constant cluster size.");

}


void PlumeDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"PLUME\"]/Content/");

  //get the location of the sensors
//  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {
//  }

  B2INFO("Parameters gotten from PLUME.xml: ");

}


void PlumeDigitizerModule::endRun()
{
}

void PlumeDigitizerModule::terminate()
{
}


