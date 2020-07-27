/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Sasha Glazov                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pruneRecoTracks/PruneRecoHitsModule.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

REG_MODULE(PruneRecoHits);

PruneRecoHitsModule::PruneRecoHitsModule() :
  Module()
{
  setDescription("Prunes RecoHits, which are marked to be pruned.");
  setPropertyFlags(c_ParallelProcessingCertified);

}

void PruneRecoHitsModule::initialize()
{
  StoreArray<RecoHitInformation> recoHitInformation;
  recoHitInformation.isRequired();

  m_subsetOfUnprunedRecoHitInformation.registerSubset(recoHitInformation, DataStore::c_DontWriteOut);
}

void PruneRecoHitsModule::event()
{
  m_subsetOfUnprunedRecoHitInformation.select([](const RecoHitInformation * recoHitInformation) {
    return recoHitInformation->getFlag() != RecoHitInformation::RecoHitFlag::c_pruned;
  });
}

