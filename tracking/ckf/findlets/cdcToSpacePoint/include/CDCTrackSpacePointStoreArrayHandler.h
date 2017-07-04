/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/Helix.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  /**
   * Derived store array handler for CDC RecoTrack seeds and SpacePoints.
   *
   * This findlet is responsible for the interface between the DataStore and the CKF modules:
   *  * to write back the found VXD tracks only and the merged tracks (CDC + VXD) (apply)
   */
  template <class AResultObject>
  class CDCTrackSpacePointStoreArrayHandler : public TrackFindingCDC::Findlet<const AResultObject> {
    using Super = TrackFindingCDC::Findlet<const AResultObject>;

  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      moduleParamList->addParameter("exportTracks", m_param_exportTracks, "Export the result tracks into a StoreArray.",
                                    m_param_exportTracks);

      moduleParamList->addParameter("VXDRecoTrackStoreArrayName", m_param_vxdRecoTrackStoreArrayName,
                                    "StoreArray name of the VXD Track Store Array", m_param_vxdRecoTrackStoreArrayName);

      moduleParamList->addParameter("MergedRecoTrackStoreArrayName", m_param_mergedRecoTrackStoreArrayName,
                                    "StoreArray name of the merged Track Store Array", m_param_mergedRecoTrackStoreArrayName);

      moduleParamList->addParameter("exportAlsoMergedTracks", m_param_exportAlsoMergedTracks,
                                    "Export also the merged tracks into a StoreArray.",
                                    m_param_exportAlsoMergedTracks);
    }

    /// Require/register the store arrays
    void initialize() override
    {
      Super::initialize();

      m_vxdRecoTracks.registerInDataStore(m_param_vxdRecoTrackStoreArrayName);
      RecoTrack::registerRequiredRelations(m_vxdRecoTracks);

      m_mergedRecoTracks.registerInDataStore(m_param_mergedRecoTrackStoreArrayName);
      RecoTrack::registerRequiredRelations(m_mergedRecoTracks);

      // TODO: Make this general enough
      StoreArray<RecoTrack> cdcRecoTracks("CDCRecoTracks");
      cdcRecoTracks.registerRelationTo(m_vxdRecoTracks);
    }


    /**
     * Write back the found tracks (VXD only and the merged ones).
     */
    void apply(const std::vector<AResultObject>& cdcTracksWithMatchedSpacePoints) override
    {
      // Create a list of cdc-track - svd cluster list pairs
      std::vector<std::pair<const RecoTrack*, std::pair<TVector3, std::vector<const SpacePoint*>>>> seedsWithPositionAndHits;
      seedsWithPositionAndHits.reserve(cdcTracksWithMatchedSpacePoints.size());

      // Create new VXD tracks out of the found VXD space points and store them into a store array
      for (const auto& cdcTrackWithMatchedSpacePoints : cdcTracksWithMatchedSpacePoints) {
        RecoTrack* cdcRecoTrack = cdcTrackWithMatchedSpacePoints.first;
        if (not cdcRecoTrack) {
          continue;
        }

        const std::vector<const SpacePoint*> matchedSpacePoints = cdcTrackWithMatchedSpacePoints.second;
        if (matchedSpacePoints.empty()) {
          continue;
        }

        const TVector3& vxdPosition = matchedSpacePoints.front()->getPosition();

        TVector3 trackMomentum;
        int trackCharge;

        // Extrapolate the seed's track parameters onto the vxdPosition
        extrapolateMomentum(*cdcRecoTrack, vxdPosition, trackMomentum, trackCharge);

        RecoTrack* newVXDOnlyTrack = addNewTrack(vxdPosition, trackMomentum, trackCharge, matchedSpacePoints, m_vxdRecoTracks);
        cdcRecoTrack->addRelationTo(newVXDOnlyTrack);

        if (m_param_exportAlsoMergedTracks) {
          // Add merged VXD-CDC-track
          RecoTrack* newMergedTrack = addNewTrack(vxdPosition, trackMomentum, trackCharge, matchedSpacePoints, m_mergedRecoTracks);
          newMergedTrack->addHitsFromRecoTrack(cdcRecoTrack, newMergedTrack->getNumberOfTotalHits());
        }
      }
    }

  private:
    // Parameters
    /** Export the tracks or not */
    bool m_param_exportTracks = true;
    /** StoreArray name of the VXD Track Store Array */
    std::string m_param_vxdRecoTrackStoreArrayName = "CKFVXDRecoTracks";
    /** StoreArray name of the merged Track Store Array */
    std::string m_param_mergedRecoTrackStoreArrayName = "MergedRecoTracks";
    /** Export also the merged tracks */
    bool m_param_exportAlsoMergedTracks = true;

    // Store Arrays
    /// VXD Reco Tracks Store Array
    StoreArray<RecoTrack> m_vxdRecoTracks;
    /// Merged Reco Tracks Store Array
    StoreArray<RecoTrack> m_mergedRecoTracks;

    /// Helper function to get the seed or the measured state on plane from a track
    void extractTrackState(const RecoTrack& recoTrack,
                           TVector3& position, TVector3& momentum, int& charge) const
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
    void extrapolateMomentum(const RecoTrack& relatedCDCRecoTrack, const TVector3& vxdPosition,
                             TVector3& extrapolatedMomentum, int& cdcCharge)
    {
      TVector3 cdcPosition;
      TVector3 cdcMomentum;

      extractTrackState(relatedCDCRecoTrack, cdcPosition, cdcMomentum, cdcCharge);

      const auto bField = BFieldMap::Instance().getBField(cdcPosition).Z();

      const Helix cdcHelix(cdcPosition, cdcMomentum, cdcCharge, bField);
      const double arcLengthOfVXDPosition = cdcHelix.getArcLength2DAtXY(vxdPosition.X(), vxdPosition.Y());

      extrapolatedMomentum = cdcHelix.getMomentumAtArcLength2D(arcLengthOfVXDPosition, bField);
    }

    /// Add a new track to the given store array with the SVD/PXD hits and the given track parameters
    RecoTrack* addNewTrack(const TVector3& trackPosition, const TVector3& trackMomentum,
                           int trackCharge, std::vector<const SpacePoint*> hits,
                           StoreArray<RecoTrack>& storeArray) const
    {
      // Add vxd-only track for reference
      RecoTrack* newRecoTrack = storeArray.appendNew(trackPosition, trackMomentum, trackCharge);

      unsigned int sortingParameter = 0;
      for (const auto& spacePoint : hits) {

        int detID = spacePoint->getType();

        if (detID == VXD::SensorInfoBase::PXD) {
          RelationVector<PXDCluster> relatedClusters = spacePoint->getRelationsTo<PXDCluster>();
          B2DEBUG(10, "SPTC2RTConverter::event: Number of related PXD Clusters: " << relatedClusters.size());
          // relatedClusters should only contain 1 cluster for pxdHits. Loop over them to be robust against missing relations.
          for (const PXDCluster& cluster : relatedClusters) {
            newRecoTrack->addPXDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
            sortingParameter++;
          }
        } else if (detID == VXD::SensorInfoBase::SVD) {
          RelationVector<SVDCluster> relatedClusters = spacePoint->getRelationsTo<SVDCluster>();
          B2DEBUG(10, "SPTC2RTConverter::event: Number of related SVD Clusters: " << relatedClusters.size());
          // relatedClusters should contain 2 clusters for svdHits. Loop over them to be robust against missing relations.
          for (const SVDCluster& cluster : relatedClusters) {
            newRecoTrack->addSVDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
            sortingParameter++;
          }
        } else {
          B2WARNING("SPTC2RTConverter::event: SpacePointTrackCandidate containing SpacePoint of unrecognised detector ID: " << detID <<
                    ". Created RecoTrack doesn't contain these SpacePoints!");
        }

        spacePoint->setAssignmentState(true);
        sortingParameter++;
      }

      return newRecoTrack;
    }
  };
}
