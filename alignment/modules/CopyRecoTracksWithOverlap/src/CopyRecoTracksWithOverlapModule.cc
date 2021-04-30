/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/CopyRecoTracksWithOverlap/CopyRecoTracksWithOverlapModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CopyRecoTracksWithOverlap)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CopyRecoTracksWithOverlapModule::CopyRecoTracksWithOverlapModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  setDescription("Copy RecoTracks with overlap hits in VXD to a new StoreArray (Will need a refit).\n"
                 "If particleList is specified, take only RecoTracks associated to the particles (allows to make prior cuts at analysis level)."
                );

  addParam("overlapRecoTracksStoreArrayName", m_overlapRecoTracksArrayName, "Name of StoreArray with output RecoTracks with overlaps",
           m_overlapRecoTracksArrayName);
  addParam("particleList", m_particleListName, "Name of particle list for which associated RecoTracks should be copied",
           m_particleListName);

}

void CopyRecoTracksWithOverlapModule::initialize()
{
  m_RecoTracks.isRequired();
  m_OverlappingRecoTracks.registerInDataStore(m_overlapRecoTracksArrayName);

  // Register all the relations
  RecoTrack::registerRequiredRelations(m_OverlappingRecoTracks);

  if (!m_particleListName.empty())
    m_ParticleList.isRequired(m_particleListName);
}

void CopyRecoTracksWithOverlapModule::event()
{
  if (!m_particleListName.empty()) {
    auto nParticles = m_ParticleList->getListSize();
    for (unsigned int iParticle = 0; iParticle < nParticles; ++iParticle) {
      auto particle = m_ParticleList->getParticle(iParticle);
      auto track = particle->getTrack();
      if (!track) {
        B2ERROR("No Track for particle.");
        continue;
      }
      auto recoTrack = track->getRelatedTo<RecoTrack>();
      if (!recoTrack) {
        B2ERROR("No RecoTrack for Track");
        continue;
      }

      processRecoTrack(*recoTrack);
    }
  } else {
    for (auto& recoTrack : m_RecoTracks)
      processRecoTrack(recoTrack);
  }
}


void CopyRecoTracksWithOverlapModule::processRecoTrack(const RecoTrack& track)
{
  std::array<int, 6> nHitsInLayer = {0, 0, 0, 0, 0, 0};

  // PXD clusters
  for (auto hit : track.getPXDHitList())
    ++nHitsInLayer.at(hit->getSensorID().getLayerNumber() - 1);

  // SVD clusters
  for (auto hit : track.getSVDHitList())
    ++nHitsInLayer.at(hit->getSensorID().getLayerNumber() - 1);

  bool hasOverlap = false;

  // Let's be as explicit as possible here:
  // NOTE: for SVD, U and V hits are stored separatelly in the RecoTrack.
  //       Therefore double hit in a layer means 4 SVD clusters (at same layer)
  // NOTE: We ignore the possibility of curlers, non-2D SVD hits etc.

  if (nHitsInLayer[0] == 2) hasOverlap = true;
  if (nHitsInLayer[1] == 2) hasOverlap = true;

  if (nHitsInLayer[2] == 4) hasOverlap = true;
  if (nHitsInLayer[3] == 4) hasOverlap = true;
  if (nHitsInLayer[4] == 4) hasOverlap = true;
  if (nHitsInLayer[5] == 4) hasOverlap = true;

  if (hasOverlap) {
    // copy RecoTrack to a new array
    auto overlapTrack = track.copyToStoreArray(m_OverlappingRecoTracks);
    overlapTrack->addHitsFromRecoTrack(&track);
  }

}

