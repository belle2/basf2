/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/FlavorTagInfoBuilder/FlavorTagInfoBuilderModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/FlavorTagInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>

#include <iostream>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FlavorTagInfoBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FlavorTagInfoBuilderModule::FlavorTagInfoBuilderModule() : Module()
{
  // Set module properties
  setDescription("Initializes the FlavorTagInfo DataObject that will be used during the Flavor Tagging. Filling is done in the FlavorTagger.py script");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("particleList", m_particleList, "Name of the ParticleList");

}

void FlavorTagInfoBuilderModule::initialize()
{
  // input: Particles and RestOfEvent
  StoreObjPtr<ParticleList>::required(m_particleList);
  StoreArray<Particle> particles;
  StoreArray<RestOfEvent> roeArray;
  particles.isRequired();

  // output: FlavorTagInfo
  StoreArray<FlavorTagInfo> flavTagArray;
  flavTagArray.registerInDataStore();
  particles.registerRelationTo(flavTagArray);
  roeArray.registerRelationTo(flavTagArray);
}

void FlavorTagInfoBuilderModule::event()
{
  // input Particle
  StoreObjPtr<ParticleList> plist(m_particleList);

  // output
  StoreArray<RestOfEvent> roeArray;
  StoreArray<FlavorTagInfo> flavTagArray;


  for (unsigned i = 0; i < plist->getListSize(); i++) {
    const Particle* particle = plist->getParticle(i);
    const RestOfEvent* roe = roeArray[i];

    // create FlavorTagInfo object
    FlavorTagInfo* flavTag = flavTagArray.appendNew();

    // create relations: Particle <-> FlavorTagInfo , RestOfEvent <-> FlavorTagInfo
    particle->addRelationTo(flavTag);
    roe->addRelationTo(flavTag);

  }
}