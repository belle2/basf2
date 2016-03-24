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
  setDescription("Prunes RecoTracks");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("pruneFlags", m_pruneFlags,
           "Pruning options passed on to genfit. See genfit::Track class for a detailed description",
           std::string("FL"));

  addParam("storeArrayName", m_storeArrayName,
           "Name of the StoreArray which is pruned",
           std::string("RecoTracks"));

}

void PruneRecoTracksModule::initialize()
{
  StoreArray<RecoTrack>::required(m_storeArrayName);
}

void PruneRecoTracksModule::event()
{
  auto tracks = Belle2::StoreArray<RecoTrack>(m_storeArrayName);
  for (auto& t : tracks) {
    t.prune(m_pruneFlags.c_str());
  }
}

