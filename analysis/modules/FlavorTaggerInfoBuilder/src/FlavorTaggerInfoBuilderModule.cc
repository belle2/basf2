/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca and Fernando Abudinen                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/FlavorTaggerInfoBuilder/FlavorTaggerInfoBuilderModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/FlavorTaggerInfoMap.h>

#include <framework/datastore/StoreArray.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FlavorTaggerInfoBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FlavorTaggerInfoBuilderModule::FlavorTaggerInfoBuilderModule() : Module()
{
  // Set module properties
  setDescription("Initializes the FlavorTaggerInfo DataObject that will be used during the Flavor Tagging. Filling is done in the FlavorTagger.py script");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void FlavorTaggerInfoBuilderModule::initialize()
{
  // input: Particles and RestOfEvent
  StoreArray<RestOfEvent> roeArray;
  StoreArray<Particle> particles;
  roeArray.isRequired();

  // output: FlavorTaggerInfo
  StoreArray<FlavorTaggerInfo> flavTagArray;
  flavTagArray.registerInDataStore();
  StoreArray<FlavorTaggerInfoMap> flavTagMap;
  flavTagMap.registerInDataStore();
  particles.registerRelationTo(flavTagArray);
  roeArray.registerRelationTo(flavTagArray);
}

void FlavorTaggerInfoBuilderModule::event()
{
  // input
  StoreArray<RestOfEvent> roeArray;

  // output
  StoreArray<FlavorTaggerInfo> flavTagArray;


  for (int i = 0; i < roeArray.getEntries(); i++) {
    const RestOfEvent* roe = roeArray[i];
    const Particle* particle = roe->getRelated<Particle>();

    // create FlavorTaggerInfo object
    FlavorTaggerInfo* flavTag = flavTagArray.appendNew();

    // create relations: Particle <-> FlavorTaggerInfo , RestOfEvent <-> FlavorTaggerInfo
    particle->addRelationTo(flavTag);
    roe->addRelationTo(flavTag);

  }
}
