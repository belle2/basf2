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

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/FileSystem.h>

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
  addParam("acceptance", m_acceptance, R"DOC(Size of the acceptance volume. This can be either be:
1. one value being the radius of a sphere
2. two values for the radius (in xy) and the half-length (in z) of a cylinder
3. three values for x,y,z half-length of a box

All values are in cm)DOC", m_acceptance);
  addParam("maxTrials", m_maxTrials, "Maximum number of trials per event", 1000);
  addParam("kineticEnergyThreshold", m_kineticEnergyThreshold, "Energy threshold [GeV]", 0.01);
  addParam("timeOffset", m_timeOffset, "Time offset [s]", 0.0);
}

void CRYInputModule::initialize()
{
  if (m_cosmicdatadir.empty()) {
    m_cosmicdatadir = FileSystem::findFile("data/generators/modules/cryinput/");
  }

  StoreArray<MCParticle> mcparticle;
  mcparticle.registerInDataStore();

  m_generator.setCosmicDataDir(m_cosmicdatadir);
  m_generator.setAcceptance(m_acceptance);
  m_generator.setMaxTrials(m_maxTrials);
  m_generator.setKineticEnergyThreshold(m_kineticEnergyThreshold);
  m_generator.setTimeOffset(m_timeOffset);
  m_generator.init();
}

void CRYInputModule::event()
{
  m_mcGraph.clear();
  m_generator.generateEvent(m_mcGraph);
  m_mcGraph.generateList();
}

void CRYInputModule::terminate()
{
  m_generator.term();
}
