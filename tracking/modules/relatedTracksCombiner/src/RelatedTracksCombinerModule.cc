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
#include <geometry/bfieldmap/BFieldMap.h>

using namespace Belle2;

REG_MODULE(RelatedTracksCombiner);

namespace {
  /// Helper function to get the seed or the measured state on plane from a track
  void extractTrackState(const RecoTrack& recoTrack,
                         TVector3& position, TVector3& momentum, int& charge)
  {
    if (recoTrack.getRepresentations().empty() or not recoTrack.wasFitSuccessful()) {
      position = recoTrack.getPositionSeed();
      momentum = recoTrack.getMomentumSeed();
      charge = recoTrack.getChargeSeed();
    } else {
      const auto& measuredStateOnPlane = recoTrack.getMeasuredStateOnPlaneFromFirstHit();
      position = measuredStateOnPlane.getPos();
      momentum = measuredStateOnPlane.getMom();
      charge = measuredStateOnPlane.getCharge();
    }
  }

  /// Extract a momentum and charge from a reco track at a given position.
  TVector3 extrapolateMomentum(const RecoTrack& relatedCDCRecoTrack, const TVector3& vxdPosition)
  {
    TVector3 cdcPosition;
    TVector3 cdcMomentum;
    int cdcCharge;

    extractTrackState(relatedCDCRecoTrack, cdcPosition, cdcMomentum, cdcCharge);

    const auto bField = BFieldMap::Instance().getBField(cdcPosition).Z();

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
  for (const RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
    const RecoTrack* vxdRecoTrack = cdcRecoTrack.getRelated<RecoTrack>(m_vxdRecoTracksStoreArrayName);

    TVector3 cdcPosition;
    TVector3 cdcMomentum;
    int cdcCharge;

    extractTrackState(cdcRecoTrack, cdcPosition, cdcMomentum, cdcCharge);

    if (vxdRecoTrack) {
      TVector3 vxdPosition;
      TVector3 vxdMomentum;
      int vxdCharge;

      extractTrackState(*vxdRecoTrack, vxdPosition, vxdMomentum, vxdCharge);

      const TVector3& trackMomentum = extrapolateMomentum(cdcRecoTrack, vxdPosition);

      RecoTrack* newMergedTrack = m_recoTracks.appendNew(vxdPosition, trackMomentum, cdcCharge);

      newMergedTrack->addHitsFromRecoTrack(vxdRecoTrack);
      newMergedTrack->addHitsFromRecoTrack(&cdcRecoTrack, newMergedTrack->getNumberOfTotalHits());
    } else {
      if (not m_useOnlyFittedTracksInSingles or cdcRecoTrack.wasFitSuccessful()) {
        RecoTrack* newTrack = m_recoTracks.appendNew(cdcPosition, cdcMomentum, cdcCharge);
        newTrack->addHitsFromRecoTrack(&cdcRecoTrack);
      }
    }
  }

  for (const RecoTrack& vxdRecoTrack : m_vxdRecoTracks) {
    const RecoTrack* cdcRecoTrack = vxdRecoTrack.getRelated<RecoTrack>(m_cdcRecoTracksStoreArrayName);

    TVector3 vxdPosition;
    TVector3 vxdMomentum;
    int vxdCharge;

    extractTrackState(vxdRecoTrack, vxdPosition, vxdMomentum, vxdCharge);

    if (not cdcRecoTrack) {
      if (not m_useOnlyFittedTracksInSingles or vxdRecoTrack.wasFitSuccessful()) {
        RecoTrack* newTrack = m_recoTracks.appendNew(vxdPosition, vxdMomentum, vxdCharge);
        newTrack->addHitsFromRecoTrack(&vxdRecoTrack);
      }
    }
  }
}

