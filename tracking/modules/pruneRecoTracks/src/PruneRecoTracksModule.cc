/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/pruneRecoTracks/PruneRecoTracksModule.h>

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

  addParam("removeHits", m_do_remove_hits,
           "Remove hits in the module or leave it for PruneRecoHitsModule"
           , m_do_remove_hits);
}

void PruneRecoTracksModule::initialize()
{
  if (m_RecoTracks.isOptional(m_storeArrayName)) {
    m_RecoHitInformations.isRequired();

    m_subsetOfUnprunedRecoHitInformation.registerSubset(m_RecoHitInformations, DataStore::c_DontWriteOut);
  }
}

void PruneRecoTracksModule::event()
{
  if (m_RecoTracks.getEntries() > 0) {
    for (auto& t : m_RecoTracks) {
      t.prune();
    }

    if (m_do_remove_hits) {
      m_subsetOfUnprunedRecoHitInformation.select([](const RecoHitInformation * recoHitInformation) {
        return recoHitInformation->getFlag() != RecoHitInformation::RecoHitFlag::c_pruned;
      });
    }
  }
}

