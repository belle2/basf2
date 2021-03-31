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
  // input
  m_roes.isRequired();

  // output: FlavorTaggerInfo
  m_flavorTaggerInfos.registerInDataStore();
  m_flavorTaggerInfoMaps.registerInDataStore();
  StoreArray<Particle>().registerRelationTo(m_flavorTaggerInfos);
  m_roes.registerRelationTo(m_flavorTaggerInfos);
}

void FlavorTaggerInfoBuilderModule::event()
{
  for (int i = 0; i < m_roes.getEntries(); i++) {
    const RestOfEvent* roe = m_roes[i];
    if (!roe->isBuiltWithMostLikely()) continue;
    const Particle* particle = roe->getRelated<Particle>();

    // create FlavorTaggerInfo object
    FlavorTaggerInfo* flavTag = m_flavorTaggerInfos.appendNew();

    // create relations: Particle <-> FlavorTaggerInfo , RestOfEvent <-> FlavorTaggerInfo
    particle->addRelationTo(flavTag);
    roe->addRelationTo(flavTag);

  }
}
