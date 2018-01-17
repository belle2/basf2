/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/relatedTracksCombiner/RelatedTracksCombinerModule.h>

#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

REG_MODULE(RelatedTracksCombiner);

namespace {
  /// Extract a momentum and charge from a reco track at a given position.
  TVector3 extrapolateMomentum(const RecoTrack& relatedCDCRecoTrack, const TVector3& vxdPosition)
  {
    TVector3 cdcPosition;
    TVector3 cdcMomentum;
    int cdcCharge;

    std::tie(cdcPosition, cdcMomentum, cdcCharge) = relatedCDCRecoTrack.extractTrackState();

    const auto bField = BFieldManager::getFieldInTesla(cdcPosition).Z();

    const Helix cdcHelix(cdcPosition, cdcMomentum, cdcCharge, bField);
    const double arcLengthOfVXDPosition = cdcHelix.getArcLength2DAtXY(vxdPosition.X(), vxdPosition.Y());

    return cdcHelix.getMomentumAtArcLength2D(arcLengthOfVXDPosition, bField);
  }
}

RelatedTracksCombinerModule::RelatedTracksCombinerModule() :
  Module()
{
  setDescription("Combine related tracks from CDC and VXD into a sinle track by copying the hit "
                 "information and combining the seed information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CDCRecoTracksStoreArrayName", m_cdcRecoTracksStoreArrayName , "Name of the input CDC StoreArray.",
           m_cdcRecoTracksStoreArrayName);
  addParam("VXDRecoTracksStoreArrayName", m_vxdRecoTracksStoreArrayName , "Name of the input VXD StoreArray.",
           m_vxdRecoTracksStoreArrayName);
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the output StoreArray.", m_recoTracksStoreArrayName);
  addParam("useOnlyFittedTracksInSingles", m_useOnlyFittedTracksInSingles, "Do only use not fitted tracks, when no match is found.",
           m_useOnlyFittedTracksInSingles);
}

void RelatedTracksCombinerModule::initialize()
{
  m_vxdRecoTracks.isRequired(m_vxdRecoTracksStoreArrayName);
  m_cdcRecoTracks.isRequired(m_cdcRecoTracksStoreArrayName);

  m_recoTracks.registerInDataStore(m_recoTracksStoreArrayName);
  RecoTrack::registerRequiredRelations(m_recoTracks);

  m_recoTracks.registerRelationTo(m_vxdRecoTracks);
  m_recoTracks.registerRelationTo(m_cdcRecoTracks);
}

void RelatedTracksCombinerModule::event()
{
  // Loop over all CDC reco tracks and add them to the store array of they do not have a match or combined them with
  // their VXD partner if they do.
  // For this, the fitted or seed state of the tracks is used - if they are already fitted or not.
  for (const RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
    bool hasPartner = false;
    for (const RecoTrack& vxdRecoTrack : cdcRecoTrack.getRelationsWith<RecoTrack>(m_vxdRecoTracksStoreArrayName)) {
      try {
        hasPartner = true;
        TVector3 vxdPosition;
        std::tie(vxdPosition, std::ignore, std::ignore) = vxdRecoTrack.extractTrackState();

        short cdcCharge = cdcRecoTrack.getChargeSeed();

//        float vxdQI = vxdRecoTrack.getQualityIndicator();

        // For the combined track, we use the momentum of the CDC track
        // helix-extrapolated to the start position of the VXD track.
        const TVector3& trackMomentum = extrapolateMomentum(cdcRecoTrack, vxdPosition);

        // We are using the basic information of the VXD track here, but copying the momentum and the charge from the
        // cdc track.
        // TODO: we should handle the covariance matrix properly here!
        RecoTrack* newMergedTrack = vxdRecoTrack.copyToStoreArray(m_recoTracks);
        newMergedTrack->setPositionAndMomentum(vxdPosition, trackMomentum);
        newMergedTrack->setChargeSeed(cdcCharge);
        newMergedTrack->addHitsFromRecoTrack(&vxdRecoTrack);
        newMergedTrack->addHitsFromRecoTrack(&cdcRecoTrack, newMergedTrack->getNumberOfTotalHits());

//        newMergedTrack->setQualityIndicator(vxdQI);

        newMergedTrack->addRelationTo(&vxdRecoTrack);
        newMergedTrack->addRelationTo(&cdcRecoTrack);
      } catch (genfit::Exception& e) {
        B2WARNING("Could not combine tracks, because of: " << e.what());
      }
    }

    if (not hasPartner and (not m_useOnlyFittedTracksInSingles or cdcRecoTrack.wasFitSuccessful())) {
      RecoTrack* newTrack = cdcRecoTrack.copyToStoreArray(m_recoTracks);
      newTrack->addHitsFromRecoTrack(&cdcRecoTrack);
      newTrack->addRelationTo(&cdcRecoTrack);
    }
  }

  // Now we only have to add the VXD tracks without a match
  for (const RecoTrack& vxdRecoTrack : m_vxdRecoTracks) {
    const RecoTrack* cdcRecoTrack = vxdRecoTrack.getRelated<RecoTrack>(m_cdcRecoTracksStoreArrayName);
    if (not cdcRecoTrack) {
      if (not m_useOnlyFittedTracksInSingles or vxdRecoTrack.wasFitSuccessful()) {
        RecoTrack* newTrack = vxdRecoTrack.copyToStoreArray(m_recoTracks);
        newTrack->addHitsFromRecoTrack(&vxdRecoTrack);
        newTrack->addRelationTo(&vxdRecoTrack);
      }
    }
  }
}

