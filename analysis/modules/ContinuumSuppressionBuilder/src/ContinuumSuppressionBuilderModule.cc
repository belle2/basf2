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

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ContinuumSuppression.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>


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
  addParam("particleList", m_particleList, "Name of the ParticleList", std::string(""));

  // TODO: Add a parameter to select use_finalstate_for_sig==0 or ==1.
  // addParam("FinalStateForSignalSelection",     m_finalStateForSignalSelection,     "Final state for signal selection criteria",      defaultSelection);
}

void ContinuumSuppressionBuilderModule::initialize()
{
  // Input
  StoreObjPtr<ParticleList>::required(m_particleList);
  StoreArray<Particle> particles;
  particles.isRequired();

  // Output
  StoreArray<ContinuumSuppression> csArray;
  csArray.registerInDataStore();
  particles.registerRelationTo(csArray);
}

void ContinuumSuppressionBuilderModule::event()
{
  // Input Particle
  StoreObjPtr<ParticleList> plist(m_particleList);

  for (unsigned i = 0; i < plist->getListSize(); i++) {
    addContinuumSuppression(plist->getParticle(i));
  }
}

