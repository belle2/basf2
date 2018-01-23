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

#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  class ModuleParamList;

  /**
   * This findlet has helper function to
   * * remove all tracks which have already a related partner from a std::vector of CDC tracks
   * * ... and of VXD tracks
   * * fetch the RecoTracks from a StoreArray and write their pointers into two std::vectors
   */
  class StoreArrayMerger : public TrackFindingCDC::Findlet<RecoTrack*, RecoTrack*> {
    /// The parent class
    using Super = TrackFindingCDC::Findlet<RecoTrack*, RecoTrack*>;
  public:
    /// Expose the parameters of the findlet
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Require/register the store arrays
    void initialize() override;

    /// Fetch the RecoTracks from the two input Store Arrays and fill them into two vectors.
    void apply(std::vector<RecoTrack*>& cdcRecoTrackVector, std::vector<RecoTrack*>& vxdRecoTrackVector) override;

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

    // Store Arrays
    /// CDC Reco Tracks Store Array
    StoreArray<RecoTrack> m_cdcRecoTracks;
    /// VXD Reco Tracks Store Array
    StoreArray<RecoTrack> m_vxdRecoTracks;

    /// Helper function to remove all element in a std::vector, which have already a relation to the given store array
    void removeRecoTracksWithPartner(std::vector<RecoTrack*>& tracks, const std::string& partnerStoreArrayName);
  };
}
