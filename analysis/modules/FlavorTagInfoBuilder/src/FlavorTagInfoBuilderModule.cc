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
}

void FlavorTagInfoBuilderModule::initialize()
{
  // input: Particles and RestOfEvent
  StoreArray<RestOfEvent> roeArray;
  StoreArray<Particle> particles;
  roeArray.isRequired();

  // output: FlavorTagInfo
  StoreArray<FlavorTagInfo> flavTagArray;
  flavTagArray.registerInDataStore();
  particles.registerRelationTo(flavTagArray);
  roeArray.registerRelationTo(flavTagArray);
}

void FlavorTagInfoBuilderModule::event()
{
  // input
  StoreArray<RestOfEvent> roeArray;

  // output
  StoreArray<FlavorTagInfo> flavTagArray;


  for (int i = 0; i < roeArray.getEntries(); i++) {
    const RestOfEvent* roe = roeArray[i];
    const Particle* particle = roe->getRelated<Particle>();

    // create FlavorTagInfo object
    FlavorTagInfo* flavTag = flavTagArray.appendNew();

    // create relations: Particle <-> FlavorTagInfo , RestOfEvent <-> FlavorTagInfo
    particle->addRelationTo(flavTag);
    roe->addRelationTo(flavTag);

  }
}