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
  std::tuple<const TVector3&, const TVector3&, short> extractTrackState(const RecoTrack& recoTrack)
  {
    if (not recoTrack.wasFitSuccessful()) {
      return std::make_tuple(recoTrack.getPositionSeed(), recoTrack.getMomentumSeed(), recoTrack.getChargeSeed());
    } else {
      const auto& measuredStateOnPlane = recoTrack.getMeasuredStateOnPlaneFromFirstHit();
      return std::make_tuple(measuredStateOnPlane.getPos(), measuredStateOnPlane.getMom(), measuredStateOnPlane.getCharge());
    }
  }

  /// Extract a momentum and charge from a reco track at a given position.
  TVector3 extrapolateMomentum(const RecoTrack& relatedCDCRecoTrack, const TVector3& vxdPosition)
  {
    TVector3 cdcPosition;
    TVector3 cdcMomentum;
    int cdcCharge;

    std::tie(cdcPosition, cdcMomentum, cdcCharge) = extractTrackState(relatedCDCRecoTrack);

    const auto bField = BFieldMap::Instance().getBField(cdcPosition).Z();

    const Helix cdcHelix(cdcPosition, cdcMomentum, cdcCharge, bField);
    const double arcLengthOfVXDPosition = cdcHelix.getArcLength2DAtXY(vxdPosition.X(), vxdPosition.Y());

    return cdcHelix.getMomentumAtArcLength2D(arcLengthOfVXDPosition, bField);
  }

  /// Helper function to add a new track to the store array with the given properties.
  RecoTrack* addNewTrackFromSeed(const RecoTrack& recoTrack, StoreArray<RecoTrack>& recoTracks,
                                 const TVector3& position, const TVector3& momentum, const int& charge)
  {
    RecoTrack* newTrack = recoTracks.appendNew(position, momentum, charge);
    newTrack->setSeedCovariance(recoTrack.getSeedCovariance());
    newTrack->setTimeSeed(recoTrack.getTimeSeed());
    return newTrack;
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
    short cdcCharge;

    std::tie(cdcPosition, cdcMomentum, cdcCharge) = extractTrackState(cdcRecoTrack);

    if (vxdRecoTrack) {
      TVector3 vxdPosition;
      TVector3 vxdMomentum;
      short vxdCharge;

      std::tie(vxdPosition, vxdMomentum, vxdCharge) = extractTrackState(*vxdRecoTrack);

      const TVector3& trackMomentum = extrapolateMomentum(cdcRecoTrack, vxdPosition);

      RecoTrack* newMergedTrack = addNewTrackFromSeed(*vxdRecoTrack, m_recoTracks, vxdPosition, trackMomentum, cdcCharge);
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
    short vxdCharge;

    std::tie(vxdPosition, vxdMomentum, vxdCharge) = extractTrackState(vxdRecoTrack);

    if (not cdcRecoTrack) {
      if (not m_useOnlyFittedTracksInSingles or vxdRecoTrack.wasFitSuccessful()) {
        RecoTrack* newTrack = addNewTrackFromSeed(vxdRecoTrack, m_recoTracks, vxdPosition, vxdMomentum, vxdCharge);
        newTrack->addHitsFromRecoTrack(&vxdRecoTrack);
      }
    }
  }
}

