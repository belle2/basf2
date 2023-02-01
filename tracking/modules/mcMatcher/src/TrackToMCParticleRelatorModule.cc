/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/mcMatcher/TrackToMCParticleRelatorModule.h>

#include <framework/datastore/RelationVector.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

REG_MODULE(TrackToMCParticleRelator);

TrackToMCParticleRelatorModule::TrackToMCParticleRelatorModule() : Module()
{
  setDescription("A module to set relations from Track to MCParticle via the RecoTrack the Track is related to. The functionality is: Take Track, get relation to RecoTrack, from RecoTrack get relation to MCParticle, and finally add a relation from Track to MCParticle.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("TracksName", m_TracksName,
           "Name of the Tracks StoreArray to be used.", m_TracksName);
  addParam("RecoTracksName", m_RecoTracksName,
           "Name of the RecoTracks StoreArray to be used.", m_RecoTracksName);
  addParam("MCParticlesName", m_MCParticlesName, "Name of the MCParticles StoreArray to be used.", m_MCParticlesName);
}


TrackToMCParticleRelatorModule::~TrackToMCParticleRelatorModule()
{
}


void TrackToMCParticleRelatorModule::initialize()
{
  m_Tracks.isRequired(m_TracksName);
  m_RecoTracks.isRequired(m_RecoTracksName);
  // Can't require MCParticles, as this module is also run during data reconstruction
  m_MCParticles.isOptional(m_MCParticlesName);

  m_Tracks.registerRelationTo(m_MCParticles);
}

void TrackToMCParticleRelatorModule::event()
{
  // Don't do anything if MCParticles aren't present
  if (m_MCParticles.getEntries() == 0) {
    return;
  }

  for (const auto& track : m_Tracks) {
    const RelationVector<RecoTrack>& recoTracks = track.getRelationsTo<RecoTrack>(m_RecoTracksName);
    const auto& mcParticleWithWeight = recoTracks[0]->getRelatedToWithWeight<MCParticle>(m_MCParticlesName);
    const MCParticle* mcParticle = mcParticleWithWeight.first;
    if (mcParticle) {
      B2DEBUG(28, "Relation to MCParticle set.");
      track.addRelationTo(mcParticle, mcParticleWithWeight.second);
    } else {
      B2DEBUG(28, "Relation to MCParticle not set. No related MCParticle to RecoTrack.");
    }
  }

  B2DEBUG(20, "MC matching finished.");
}
