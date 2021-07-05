/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/pruneGenfitTracks/PruneGenfitTracksModule.h>
#include <framework/datastore/StoreArray.h>
#include <genfit/Track.h>

using namespace Belle2;

REG_MODULE(PruneGenfitTracks);

PruneGenfitTracksModule::PruneGenfitTracksModule() :
  Module()
{
  setDescription("Prunes genfit tracks");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("pruneFlags", m_pruneFlags,
           "Pruning options passed on to genfit. See genfit::Track class for a detailed description",
           std::string("FL"));

  addParam("storeArrayName", m_storeArrayName,
           "Name of the StoreArray which is pruned",
           std::string("GF2Tracks"));

}

void PruneGenfitTracksModule::event()
{
  auto tracks = Belle2::StoreArray<genfit::Track>(m_storeArrayName);
  for (auto& t : tracks) {
    t.prune(m_pruneFlags.c_str());
  }
}

