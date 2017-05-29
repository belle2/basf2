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

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <framework/core/ModuleParamList.h>

#include <framework/dataobjects/Helix.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Findlet to handle export to the StoreArray.
   *
   * Its functions can be used in derived classes, to export the result objects into a store array and merge it
   * together with other store arrays.
   */
  class StoreArrayHandler : public TrackFindingCDC::ProcessingSignalListener {
    using Super = TrackFindingCDC::ProcessingSignalListener;
  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
    {
      // VXD tracks
      moduleParamList->addParameter("VXDRecoTrackStoreArrayName", m_param_vxdRecoTrackStoreArrayName,
                                    "StoreArray name of the VXD Track Store Array", m_param_vxdRecoTrackStoreArrayName);

      // combined output tracks
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

    template <class AResult>
    void combine(const std::vector<AResult>& seedsWithHits)
    {
      for (const auto& seedWithHits : seedsWithHits) {
        const auto& seedTrack = seedWithHits.first;
        const auto& vxdPosition = seedWithHits.second.first;
        const auto& hits = seedWithHits.second.second;

        TVector3 trackMomentum;
        int trackCharge;

        // Extrapolate the seed's track parameters onto the vxdPosition
        extrapolateMomentum(*seedTrack, vxdPosition, trackMomentum, trackCharge);

        RecoTrack* newVXDOnlyTrack = addNewTrack(vxdPosition, trackMomentum, trackCharge, hits, m_vxdRecoTracks);
        seedTrack->addRelationTo(newVXDOnlyTrack);

        if (m_param_exportAlsoMergedTracks) {
          // Add merged VXD-CDC-track
          RecoTrack* newMergedTrack = addNewTrack(vxdPosition, trackMomentum, trackCharge, hits, m_mergedRecoTracks);
          newMergedTrack->addHitsFromRecoTrack(seedTrack, newMergedTrack->getNumberOfTotalHits());
        }
      }
    }

  private:
    // Parameters
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
    void extractTrackState(const RecoTrack& recoTrack, TVector3& position, TVector3& momentum, int& charge) const
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
    void extrapolateMomentum(const RecoTrack& relatedCDCRecoTrack, const TVector3& vxdPosition, TVector3& extrapolatedMomentum,
                             int& cdcCharge)
    {
      TVector3 cdcPosition;
      TVector3 cdcMomentum;

      extractTrackState(relatedCDCRecoTrack, cdcPosition, cdcMomentum, cdcCharge);

      const auto bField = BFieldMap::Instance().getBField(cdcPosition).Z();

      const Helix cdcHelix(cdcPosition, cdcMomentum, cdcCharge, bField);
      const double arcLengthOfVXDPosition = cdcHelix.getArcLength2DAtXY(vxdPosition.X(), vxdPosition.Y());

      extrapolatedMomentum = cdcHelix.getMomentumAtArcLength2D(arcLengthOfVXDPosition, bField);
    }

    /// Add a new track to the given store array with the SVD hits and the given track parameters
    template <class AHitType>
    RecoTrack* addNewTrack(const TVector3& trackPosition, const TVector3& trackMomentum,
                           int trackCharge, std::vector<const AHitType*> hits,
                           StoreArray<RecoTrack>& storeArray) const
    {
      // Add vxd-only track for reference
      RecoTrack* newRecoTrack = storeArray.appendNew(trackPosition, trackMomentum, trackCharge);

      unsigned int hitCounter = 0;
      for (const auto& hit : hits) {
        newRecoTrack->addSVDHit(hit, hitCounter);
        hitCounter++;
      }

      return newRecoTrack;
    }

  };
}
