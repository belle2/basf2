/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/EvtGenInputModule.h>
#include <generators/evtgen/EvtGenInterface.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <generators/utilities/cm2LabBoost.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvtGenInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EvtGenInputModule::EvtGenInputModule() : Module()
{
  //Set module properties
  setDescription("EvtGen input");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("userDECFile", m_userDECFileName, "user DECfile name", string(""));
  addParam("DECFile", m_DECFileName, "standard DECfile name should be provided: default file is in release/data/generators/DECAY.DEC");
  addParam("pdlFile", m_pdlFileName, "standard pdlfile name should be provided: default file is in release/data/generators/evt.pdl");
  addParam("boost2LAB", m_boost2LAB, "Boolean to indicate whether the particles should be boosted to LAB frame", true);
}


void EvtGenInputModule::initialize()
{
  B2INFO("starting initialisation of EvtGen Input Module. ");

  //Do we need to boost?
  if (m_boost2LAB) {
    // this is hard coded!!!!!! should be provided somewhere -> run meta data
    double Eher = 7.0 * Unit::GeV;
    double Eler = 4.0 * Unit::GeV;
    double cross_angle = 83 * Unit::mrad;
    double angle = 41.5 * Unit::mrad;
    m_Ievtgen.m_labboost = getBoost(Eher, Eler, cross_angle, angle);
  }

  //setup the DECAY files:
  if (!m_userDECFileName.empty())
    m_Ievtgen.setup(m_DECFileName, m_pdlFileName, m_userDECFileName);
  else
    m_Ievtgen.setup(m_DECFileName, m_pdlFileName);


  //Initialize MCParticle collection
  StoreArray<MCParticle> MCParticles;

  B2INFO("finished initialising the EvtGen Input Module. ");

}


void EvtGenInputModule::event()
{
  B2INFO("Starting event simulation.");
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  mpg.clear();
  int nPart =  m_Ievtgen.simulateEvent(mpg);
  B2INFO("Simulated event " << eventMetaDataPtr->getEvent() << " with " << nPart << " particles.");

}

