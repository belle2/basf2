/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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




