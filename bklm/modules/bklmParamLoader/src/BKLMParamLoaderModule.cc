/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/modules/bklmParamLoader/BKLMParamLoaderModule.h>

#include <framework/core/Environment.h>
#include <framework/core/utilities.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <bklm/simulation/SimulationPar.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(BKLMParamLoader)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMParamLoaderModule::BKLMParamLoaderModule() : Module()
{
  setDescription("Loads the BKLM simulation-control parameters.");

  addParam("ParamCheck",
           m_ParamCheck,
           "Turns on/off parameter validation",
           false);

  addParam("InputFileXML",
           m_Pathname,
           "Pathname of the XML input file",
           Environment::Instance().getDataSearchPath() + "/bklm/BKLMSimulationPar.xml");

  addParam("RandomSeed",
           m_RandomSeed,
           "Random number seed for RPC strip-multiplicity algorithm",
           54321);

  addParam("DoBackgroundStudy",
           m_DoBackgroundStudy,
           "Enables/disables recording of neutron-background hits",
           false);

}

BKLMParamLoaderModule::~BKLMParamLoaderModule()
{
}

void BKLMParamLoaderModule::initialize()
{

  bklm::SimulationPar* simPar = bklm::SimulationPar::instance();

  if (!FileSystem::fileExists(m_Pathname)) {
    B2ERROR("BKLMParamLoader: file " << m_Pathname << " does not exist.")
  }
  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.open(m_Pathname);
  GearDir content = GearDir("/ParamSet[@type=\"BKLM\"]/Content");
  simPar->read(content, (unsigned int) m_RandomSeed, m_DoBackgroundStudy);
  gearbox.close();

}
