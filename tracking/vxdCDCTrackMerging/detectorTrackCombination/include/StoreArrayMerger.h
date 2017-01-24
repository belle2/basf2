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
  class StoreArrayMerger : public TrackFindingCDC::Findlet<> {
  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Require/register the store arrays
    void initialize() override;

    /// Fetch the RecoTracks from the two input Store Arrays and fill them into two vectors.
    void fetch(std::vector<RecoTrack*>& cdcRecoTrackVector, std::vector<RecoTrack*>& vxdRecoTrackVector);

    /**
     * Use the relations between elements in the two CDC and VXD Store Arrays and
     * merge them together into a new store array.
     * During this, all hits are filled into a new reco track (in the order VXD -> CDC)
     * and the track parameters are chosen, that the position comes from the VXD track and
     * the momentum from the CDC track.
     */
    void apply() override;

    /// Helper function to remove all tracks, which have a related VXD track from the vector.
    void removeCDCRecoTracksWithPartner(std::vector<RecoTrack*>& tracks);

    /// Helper function to remove all tracks, which have a related CDC track from the vector.
    void removeVXDRecoTracksWithPartner(std::vector<RecoTrack*>& tracks);

  private:
    // Parameters
    /** StoreArray name of the VXD Track Store Array */
    std::string m_param_vxdRecoTrackStoreArrayName;
    /** StoreArray name of the CDC Track Store Array */
    std::string m_param_cdcRecoTrackStoreArrayName;
    /** StoreArray name of the merged Track Store Array */
    std::string m_param_mergedRecoTrackStoreArrayName;

    // Store Arrays
    /// CDC Reco Tracks Store Array
    StoreArray<RecoTrack> m_cdcRecoTracks;
    /// VXD Reco Tracks Store Array
    StoreArray<RecoTrack> m_vxdRecoTracks;
    /// Merged Reco Tracks Store Array
    StoreArray<RecoTrack> m_mergedRecoTracks;

    /// Helper function to remove all element in a std::vector, which have already a relation to the given store array
    void removeRecoTracksWithPartner(std::vector<RecoTrack*>& tracks, const std::string& partnerStoreArrayName);
  };
}