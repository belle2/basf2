/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/relatedTracksCombiner/RelatedTracksCombinerModule.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

using namespace Belle2;

REG_MODULE(RelatedTracksCombiner);

RelatedTracksCombinerModule::RelatedTracksCombinerModule() :
  Module()
{
  setDescription("Combine related tracks from CDC and VXD into a single track by copying the hit "
                 "information and combining the seed information. The sign of the weight defines, "
                 "if the hits go before (-1) or after (+1) the CDC track.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CDCRecoTracksStoreArrayName", m_cdcRecoTracksStoreArrayName, "Name of the input CDC StoreArray.",
           m_cdcRecoTracksStoreArrayName);
  addParam("VXDRecoTracksStoreArrayName", m_vxdRecoTracksStoreArrayName, "Name of the input VXD StoreArray.",
           m_vxdRecoTracksStoreArrayName);
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the output StoreArray.", m_recoTracksStoreArrayName);
}

void RelatedTracksCombinerModule::initialize()
{
  m_vxdRecoTracks.isRequired(m_vxdRecoTracksStoreArrayName);
  m_cdcRecoTracks.isRequired(m_cdcRecoTracksStoreArrayName);

  m_recoTracks.registerInDataStore(m_recoTracksStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_recoTracks);

  m_recoTracks.registerRelationTo(m_vxdRecoTracks);
  m_recoTracks.registerRelationTo(m_cdcRecoTracks);
}

void RelatedTracksCombinerModule::event()
{
  TrackFitter trackFitter;

  // Loop over all CDC reco tracks and add them to the store array of they do not have a match or combined them with
  // their VXD partner if they do.
  // For this, the fitted or seed state of the tracks is used - if they are already fitted or not.
  for (RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
    const RelationVector<RecoTrack>& relatedVXDRecoTracks = cdcRecoTrack.getRelationsWith<RecoTrack>(m_vxdRecoTracksStoreArrayName);

    B2ASSERT("Can not handle more than 2 relations!", relatedVXDRecoTracks.size() <= 2);

    RecoTrack* vxdTrackBefore = nullptr;
    RecoTrack* vxdTrackAfter = nullptr;

    for (unsigned int index = 0; index < relatedVXDRecoTracks.size(); ++index) {
      const double weight = relatedVXDRecoTracks.weight(index);
      if (weight < 0) {
        vxdTrackBefore = relatedVXDRecoTracks[index];
      } else if (weight > 0) {
        vxdTrackAfter = relatedVXDRecoTracks[index];
      }
    }

    // Do not output non-fittable tracks
    if (not vxdTrackAfter and not vxdTrackBefore and not trackFitter.fit(cdcRecoTrack)) {
      continue;
    }

    RecoTrack* newMergedTrack = nullptr;

    if (vxdTrackBefore) {
      newMergedTrack = vxdTrackBefore->copyToStoreArray(m_recoTracks);
      newMergedTrack->addHitsFromRecoTrack(vxdTrackBefore, newMergedTrack->getNumberOfTotalHits());
      newMergedTrack->addRelationTo(vxdTrackBefore);
    } else {
      newMergedTrack = cdcRecoTrack.copyToStoreArray(m_recoTracks);
    }

    newMergedTrack->addHitsFromRecoTrack(&cdcRecoTrack, newMergedTrack->getNumberOfTotalHits());
    newMergedTrack->addRelationTo(&cdcRecoTrack);

    if (vxdTrackAfter) {
      newMergedTrack->addHitsFromRecoTrack(vxdTrackAfter, newMergedTrack->getNumberOfTotalHits(), true);
      newMergedTrack->addRelationTo(vxdTrackAfter);
    }
  }

  // Now we only have to add the VXD tracks without a match
  for (RecoTrack& vxdRecoTrack : m_vxdRecoTracks) {
    const RecoTrack* cdcRecoTrack = vxdRecoTrack.getRelated<RecoTrack>(m_cdcRecoTracksStoreArrayName);
    if (not cdcRecoTrack and trackFitter.fit(vxdRecoTrack)) {
      RecoTrack* newTrack = vxdRecoTrack.copyToStoreArray(m_recoTracks);
      newTrack->addHitsFromRecoTrack(&vxdRecoTrack);
      newTrack->addRelationTo(&vxdRecoTrack);
    }
  }
}

