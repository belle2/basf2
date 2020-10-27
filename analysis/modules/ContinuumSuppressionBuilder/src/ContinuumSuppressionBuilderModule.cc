/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/ContinuumSuppressionBuilder/ContinuumSuppressionBuilderModule.h>

#include <analysis/ContinuumSuppression/ContinuumSuppression.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ContinuumSuppressionBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ContinuumSuppressionBuilderModule::ContinuumSuppressionBuilderModule() : Module()
{
  // Set module properties
  setDescription("Creates for each Particle in the given ParticleLists a ContinuumSuppression dataobject and makes BASF2 relation between them.");

  // Parameter definitions
  addParam("particleList", m_particleListName, "Name of the ParticleList", std::string(""));

  addParam("ROEMask", m_ROEMask, "ROE mask", std::string(""));

}

void ContinuumSuppressionBuilderModule::initialize()
{
  // Input
  m_plist.isRequired(m_particleListName);
  StoreArray<Particle>().isRequired();

  // Output
  m_csarray.registerInDataStore();
  StoreArray<Particle>().registerRelationTo(m_csarray);
}

void ContinuumSuppressionBuilderModule::event()
{
  for (unsigned i = 0; i < m_plist->getListSize(); i++) {
    addContinuumSuppression(m_plist->getParticle(i), m_ROEMask);      // pass the ROEMask to cs.cc here as a second argument.
  }
}

