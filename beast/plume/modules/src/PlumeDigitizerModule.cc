/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/plume/modules/PlumeDigitizerModule.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>
#include <beast/plume/dataobjects/PlumeHit.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

//c++
#include <string>
#include <fstream>

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
  m_plumeHits.registerInDataStore();

  m_plumeSimHits.isRequired();
  m_particles.isOptional();

  //get the sensor parameters and set the parameters of the response function
  getXMLData();
  setResponseModel(); // currently empty

}

void PlumeDigitizerModule::beginRun()
{

}

void PlumeDigitizerModule::event()
{

  //skip events with no plumeSimHits, but continue the event counter
  if (m_plumeSimHits.getEntries() == 0) {
    Event++;
    return;
  }


  for (int i = 0; i < m_plumeSimHits.getEntries(); i++) {
    PlumeSimHit* aHit = m_plumeSimHits[i];

    // This is where the detailed response function shall be called
    m_nofPixels = 3;
    m_posmm_u = aHit->getposOUT_u();
    m_posmm_v = aHit->getposOUT_v();
    m_posmm_x = aHit->getposIN_x();
    m_posmm_y = aHit->getposIN_y();
    m_posmm_z = aHit->getposIN_z();

    //create PlumeHit
    m_plumeHits.appendNew(PlumeHit(aHit->getsensorID(), m_nofPixels, m_posmm_u, m_posmm_v, m_posmm_x, m_posmm_y,
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
  //GearDir content = GearDir("/Detector/DetectorComponent[@name=\"PLUME\"]/Content/");

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


