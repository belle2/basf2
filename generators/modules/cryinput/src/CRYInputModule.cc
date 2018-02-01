/************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015  Belle II Collaboration                              *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <generators/modules/cryinput/CRYInputModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>

#include <TLorentzVector.h>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TRandom3.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>  // For Ubuntu Linux

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CRYInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CRYInputModule::CRYInputModule() : Module()
{
  //Set module properties
  setDescription("Generates cosmic showers with CRY");
  addParam("CosmicDataDir", m_cosmicdatadir, "Directory that holds the cosmic data for CRY.", std::string(""));
  addParam("SetupFile", m_setupfile, "User setup file for CRY.", std::string(""));
  addParam("acceptLength", m_acceptLength, "Length of the acceptance box [m]", 10.);
  addParam("acceptWidth", m_acceptWidth, "Width of the acceptance box [m]", 10.);
  addParam("acceptHeight", m_acceptHeight, "Height of the acceptance box [m]", 10.);
  addParam("keepLength", m_keepLength, "Length of the keepance box [m]", 10.);
  addParam("keepWidth", m_keepWidth, "Width of the keepance box [m]", 10.);
  addParam("keepHeight", m_keepHeight, "Height of the keepance box [m]", 10.);
  addParam("maxTrials", m_maxTrials, "Maximum number of trials per event", 1000);
  addParam("kineticEnergyThreshold", m_kineticEnergyThreshold, "Energy threshold [GeV]", 0.01);
  addParam("timeOffset", m_timeOffset, "Time offset [s]", 0.0);

  m_keepLength = m_keepLength * Belle2::Unit::m;
  m_keepWidth = m_keepWidth * Belle2::Unit::m;
  m_keepLength = m_keepLength * Belle2::Unit::m;
  m_acceptLength = m_acceptLength * Belle2::Unit::m;
  m_acceptWidth = m_acceptWidth * Belle2::Unit::m;
  m_acceptHeight = m_acceptHeight * Belle2::Unit::m;
  m_timeOffset = m_timeOffset * Belle2::Unit::s;
  m_kineticEnergyThreshold = m_kineticEnergyThreshold * Belle2::Unit::GeV;
}


CRYInputModule::~CRYInputModule()
{

}


void CRYInputModule::initialize()
{
  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  m_generator.setSetupFile(m_setupfile);
  m_generator.setCosmicDataDir(m_cosmicdatadir);
  m_generator.setAcceptLength(m_acceptLength);
  m_generator.setAcceptWidth(m_acceptWidth);
  m_generator.setAcceptHeight(m_acceptHeight);
  m_generator.setKeepLength(m_keepLength);
  m_generator.setKeepWidth(m_keepWidth);
  m_generator.setKeepHeight(m_keepHeight);
  m_generator.setMaxTrials(m_maxTrials);
  m_generator.setKineticEnergyThreshold(m_kineticEnergyThreshold);
  m_generator.setTimeOffset(m_timeOffset);

  m_generator.init();

}


void CRYInputModule::event()
{
  m_mcGraph.clear();

  m_generator.generateEvent(m_mcGraph);

  m_mcGraph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
}


void CRYInputModule::terminate()
{
  m_generator.term();
}
