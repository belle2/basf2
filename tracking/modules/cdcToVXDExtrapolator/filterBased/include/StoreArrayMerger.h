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
#include <framework/core/ModuleParamList.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Findlet for using the relations between two store arrays of RecoTracks
   * to create new RecoTracks with the merged tracks and write them to a new store array.
   *
   * Additionally, this findlet has helper function to
   * * remove all tracks which have already a related partner from a std::vector of CDC tracks
   * * ... and of VXD tracks
   * * fetch the RecoTracks from a StoreArray and write their pointers into two std::vectors
   */
  class StoreArrayMerger : public TrackFindingCDC::Findlet<const std::pair<RecoTrack*, std::vector<const SpacePoint*>>> {
  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Require/register the store arrays
    void initialize() override;

    /// Fetch the CDC RecoTracks from the input Store Arrays and fill them into a vector.
    void fetch(std::vector<RecoTrack*>& cdcRecoTrackVector);

    /**
     * Use the relations between elements in the two CDC and VXD Store Arrays and
     * merge them together into a new store array.
     * During this, all hits are filled into a new reco track (in the order VXD -> CDC)
     * and the track parameters are chosen, that the position comes from the VXD track and
     * the momentum from the CDC track.
     */
    void apply(const std::vector<std::pair<RecoTrack*, std::vector<const SpacePoint*>>>& cdcTracksWithMatchedSpacePoints) override;
  private:
    // Parameters
    /** StoreArray name of the VXD Track Store Array */
    std::string m_param_vxdRecoTrackStoreArrayName = "CKFVXDRecoTracks";
    /** StoreArray name of the CDC Track Store Array */
    std::string m_param_cdcRecoTrackStoreArrayName = "CDCRecoTracks";
    /** StoreArray name of the merged Track Store Array */
    std::string m_param_mergedRecoTrackStoreArrayName = "MergedRecoTracks";
    /** Also add tracks, which could not be fitted and have no partner to the resulting array. */
    bool m_param_addUnfittableTracks = true;

    // Store Arrays
    /// CDC Reco Tracks Store Array
    StoreArray<RecoTrack> m_cdcRecoTracks;
    /// VXD Reco Tracks Store Array
    StoreArray<RecoTrack> m_vxdRecoTracks;
    /// Merged Reco Tracks Store Array
    StoreArray<RecoTrack> m_mergedRecoTracks;
  };
}
