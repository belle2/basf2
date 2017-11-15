/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/OnlyParticleLists/OnlyParticleLists.h>
#include <framework/datastore/DataStore.h>
#include <analysis/dataobjects/ParticleList.h>

using namespace Belle2;

REG_MODULE(OnlyParticleLists)

void OnlyParticleListsModule::initialize()
{
  for (auto& entry : DataStore::Instance().getStoreEntryMap(DataStore::c_Event)) {
    entry.second.dontWriteOut = entry.second.objClass != ParticleList::Class();
  }
}




