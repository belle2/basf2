/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    const Particle* particle = roe->getRelated<Particle>();

    // create FlavorTaggerInfo object if it does not exist
    FlavorTaggerInfo* flavTag = roe->getRelatedTo<FlavorTaggerInfo>() ? roe->getRelatedTo<FlavorTaggerInfo>() :
                                m_flavorTaggerInfos.appendNew();

    flavTag -> addMethodMap("FBDT");
    flavTag -> addMethodMap("FANN");
    flavTag -> addMethodMap("DNN");

    if (!roe->getRelatedTo<FlavorTaggerInfo>()) {
      // create relations: Particle <-> FlavorTaggerInfo , RestOfEvent <-> FlavorTaggerInfo
      particle->addRelationTo(flavTag);
      roe->addRelationTo(flavTag);
    }

  }
}
