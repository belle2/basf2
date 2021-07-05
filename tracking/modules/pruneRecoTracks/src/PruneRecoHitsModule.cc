/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

