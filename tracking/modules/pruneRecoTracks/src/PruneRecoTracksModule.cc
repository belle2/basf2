/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pruneRecoTracks/PruneRecoTracksModule.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

REG_MODULE(PruneRecoTracks);

PruneRecoTracksModule::PruneRecoTracksModule() :
  Module()
{
  setDescription("Prunes RecoTracks.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("storeArrayName", m_storeArrayName,
           "Name of the StoreArray which is pruned",
           m_storeArrayName);

}

void PruneRecoTracksModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_storeArrayName);

  if (recoTracks.isOptional()) {
    StoreArray<RecoHitInformation> recoHitInformation;
    recoHitInformation.isRequired();

    m_subsetOfUnprunedRecoHitInformation.registerSubset(recoHitInformation, DataStore::c_DontWriteOut);
  }
}

void PruneRecoTracksModule::event()
{
  Belle2::StoreArray<RecoTrack> tracks(m_storeArrayName);
  if (tracks.isValid()) {
    for (auto& t : tracks) {
      t.prune();
    }

    m_subsetOfUnprunedRecoHitInformation.select([](const RecoHitInformation * recoHitInformation) {
      return recoHitInformation->getFlag() != RecoHitInformation::RecoHitFlag::c_pruned;
    });
  }
}

