/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Kahn, Martin Ritter                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/OnlyWriteOutParticleLists/OnlyWriteOutParticleLists.h>
#include <framework/datastore/DataStore.h>
#include <analysis/dataobjects/ParticleList.h>

using namespace Belle2;

REG_MODULE(OnlyWriteOutParticleLists)

OnlyWriteOutParticleListsModule::OnlyWriteOutParticleListsModule()
{
  setDescription("Marks all objects in DataStore except those of type ParticleList as WrtieOut=False. Intedend to run before outputting an index file to remove unecessary arrays.");
}

void OnlyWriteOutParticleListsModule::initialize()
{
  for (auto& entry : DataStore::Instance().getStoreEntryMap(DataStore::c_Event)) {
    entry.second.dontWriteOut = entry.second.objClass != ParticleList::Class();
  }
}




