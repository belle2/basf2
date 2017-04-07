/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/StoreArrayMerger.h>
#include <framework/dataobjects/Helix.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace Belle2;

namespace {
  /// Helper function to get the seed or the measured state on plane from a track
  void extractTrackState(const RecoTrack& recoTrack, TVector3& position, TVector3& momentum, int& charge)
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
}

void StoreArrayMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string&)
{
  // CDC input tracks
  moduleParamList->addParameter("CDCRecoTrackStoreArrayName", m_param_cdcRecoTrackStoreArrayName,
                                "StoreArray name of the CDC Track Store Array", m_param_cdcRecoTrackStoreArrayName);
  // VXD input tracks
  moduleParamList->addParameter("VXDRecoTrackStoreArrayName", m_param_vxdRecoTrackStoreArrayName,
                                "StoreArray name of the VXD Track Store Array", m_param_vxdRecoTrackStoreArrayName);

  // combined output tracks
  moduleParamList->addParameter("MergedRecoTrackStoreArrayName", m_param_mergedRecoTrackStoreArrayName,
                                "StoreArray name of the merged Track Store Array", m_param_mergedRecoTrackStoreArrayName);

  // parameter
  moduleParamList->addParameter("AddUnfittableTracks", m_param_addUnfittableTracks,
                                "Also add tracks, which could not be fitted and have no partner to the resulting array.",
                                m_param_addUnfittableTracks);
}

void StoreArrayMerger::initialize()
{
  m_cdcRecoTracks.isRequired(m_param_cdcRecoTrackStoreArrayName);

  m_vxdRecoTracks.registerInDataStore(m_param_vxdRecoTrackStoreArrayName);
  RecoTrack::registerRequiredRelations(m_vxdRecoTracks);

  m_mergedRecoTracks.registerInDataStore(m_param_mergedRecoTrackStoreArrayName);

  // register all required relations to be able to use RecoTrack
  // and its relations
  RecoTrack::registerRequiredRelations(m_mergedRecoTracks);

  m_cdcRecoTracks.registerRelationTo(m_vxdRecoTracks);
  m_vxdRecoTracks.registerRelationTo(m_cdcRecoTracks);

  TrackFindingCDC::Findlet<const std::pair<RecoTrack*, std::vector<const SpacePoint*>>>::initialize();
}

void StoreArrayMerger::fetch(std::vector<RecoTrack*>& cdcRecoTrackVector)
{
  cdcRecoTrackVector.reserve(cdcRecoTrackVector.size() + m_cdcRecoTracks.getEntries());

  for (RecoTrack& recoTrack : m_cdcRecoTracks) {
    cdcRecoTrackVector.push_back(&recoTrack);
  }
}

void StoreArrayMerger::apply(const std::vector<std::pair<RecoTrack*, std::vector<const SpacePoint*>>>&
                             cdcTracksWithMatchedSpacePoints)
{
  ////// CUT
  // Create new VXD tracks out of the found VXD space points and store them into a store array
  for (const auto& cdcTrackWithMatchedSpacePoints : cdcTracksWithMatchedSpacePoints) {
    RecoTrack* cdcRecoTrack = cdcTrackWithMatchedSpacePoints.first;
    const std::vector<const SpacePoint*> matchedSpacePoints = cdcTrackWithMatchedSpacePoints.second;

    // TODO: correct seeds
    RecoTrack* vxdRecoTrack = m_vxdRecoTracks.appendNew(cdcRecoTrack->getPositionSeed(),
                                                        cdcRecoTrack->getMomentumSeed(),
                                                        cdcRecoTrack->getChargeSeed());

    unsigned int hitCounter = 0;
    for (const auto& spacePoint : matchedSpacePoints) {
      if (spacePoint) {
        const auto& relatedClusters = spacePoint->getRelationsWith<SVDCluster>();
        vxdRecoTrack->addSVDHit(relatedClusters[0], hitCounter);
        vxdRecoTrack->addSVDHit(relatedClusters[1], hitCounter + 1);
        hitCounter += 2;
      }
    }

    cdcRecoTrack->addRelationTo(vxdRecoTrack);
  }
  /////// CUT

  // Combine the vxd and cdc tracks based on the relations between tracks in the Data Store
  // write all vxd tracks to StoreArray and add a CDC track, if there is one.
  for (const auto& currentVXDTrack : m_vxdRecoTracks) {

    auto relatedCDCRecoTrack = currentVXDTrack.getRelated<RecoTrack>(m_param_cdcRecoTrackStoreArrayName);

    // vxd position will be filled with seed or fitted position.
    TVector3 vxdPosition;
    TVector3 vxdMomentum;
    int vxdCharge;

    extractTrackState(currentVXDTrack, vxdPosition, vxdMomentum, vxdCharge);

    // add related RecoTracks to VXD tracks if merging was successful
    if (relatedCDCRecoTrack) {
      // We construct a helix out of the CDC track parameter and "extrapolate" it to the vxd start position to get
      // the momentum right
      TVector3 cdcPosition;
      TVector3 cdcMomentum;
      int cdcCharge;

      extractTrackState(*relatedCDCRecoTrack, cdcPosition, cdcMomentum, cdcCharge);

      const auto bField = BFieldMap::Instance().getBField(cdcPosition).Z();

      const Helix cdcHelix(cdcPosition, cdcMomentum, cdcCharge, bField);
      const double arcLengthOfVXDPosition = cdcHelix.getArcLength2DAtXY(vxdPosition.X(), vxdPosition.Y());

      const auto& extrapolatedMomentum = cdcHelix.getMomentumAtArcLength2D(arcLengthOfVXDPosition, bField);

      auto newRecoTrack = m_mergedRecoTracks.appendNew(vxdPosition,
                                                       extrapolatedMomentum,
                                                       cdcCharge);
      newRecoTrack->addHitsFromRecoTrack(&currentVXDTrack);
      newRecoTrack->addHitsFromRecoTrack(relatedCDCRecoTrack, newRecoTrack->getNumberOfTotalHits());
    } else {
      // And not if not...
      if (m_param_addUnfittableTracks or currentVXDTrack.getRepresentations().empty() or currentVXDTrack.wasFitSuccessful()) {
        auto newRecoTrack = m_mergedRecoTracks.appendNew(vxdPosition,
                                                         vxdMomentum,
                                                         vxdCharge);
        newRecoTrack->addHitsFromRecoTrack(&currentVXDTrack);
      }
    }
  }

  // add all unmatched CDCTracks to the StoreArray as well
  for (const auto& currentCDCTrack : m_cdcRecoTracks) {
    auto relatedVXDRecoTrack = currentCDCTrack.getRelated<RecoTrack>(m_param_vxdRecoTrackStoreArrayName);
    if (not relatedVXDRecoTrack and (m_param_addUnfittableTracks or currentCDCTrack.getRepresentations().empty()
                                     or currentCDCTrack.wasFitSuccessful())) {
      TVector3 cdcPosition;
      TVector3 cdcMomentum;
      int cdcCharge;

      extractTrackState(currentCDCTrack, cdcPosition, cdcMomentum, cdcCharge);

      auto newRecoTrack = m_mergedRecoTracks.appendNew(cdcPosition, cdcMomentum,
                                                       cdcCharge);
      newRecoTrack->addHitsFromRecoTrack(&currentCDCTrack);
    }
  }
}