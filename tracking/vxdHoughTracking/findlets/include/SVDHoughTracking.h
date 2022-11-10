/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/vxdHoughTracking/findlets/SpacePointLoaderAndPreparer.h>
#include <tracking/vxdHoughTracking/findlets/MultiHoughSpaceFastInterceptFinder.h>
#include <tracking/vxdHoughTracking/findlets/SingleHoughSpaceFastInterceptFinder.h>
#include <tracking/vxdHoughTracking/findlets/RawTrackCandCleaner.dcl.h>
#include <tracking/vxdHoughTracking/findlets/TrackCandidateOverlapResolver.h>
#include <tracking/vxdHoughTracking/findlets/RecoTrackStorer.h>
#include <tracking/vxdHoughTracking/findlets/ROIFinder.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class SpacePoint;
  class SpacePointTrackCand;

  namespace vxdHoughTracking {
    class VXDHoughState;

    /**
    * Main Findlet for the SVDHoughTracking.
    */
    class SVDHoughTracking : public TrackFindingCDC::Findlet<> {
      /// Parent class
      using Super = TrackFindingCDC::Findlet<>;

    public:
      /// Constructor for adding the subfindlets
      SVDHoughTracking();

      /// Default desctructor
      ~SVDHoughTracking();

      /// Expose the parameters of the sub findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Function to call all the sub-findlets
      void apply() override;

      /// Clear the object pools
      void beginEvent() override;

    private:
      /// Findlets:

      /// Load SVDSpacePoints and prepare them for Hough-based tracking
      /// by calculating the conformal mapped x and y values of the 3D SpacePoint
      SpacePointLoaderAndPreparer m_spacePointLoaderAndPreparer;

      /// Hough Space intercept finder
      MultiHoughSpaceFastInterceptFinder m_multiHouthSpaceInterceptFinder;
      /// Simple Hough Space intercept finder
      SingleHoughSpaceFastInterceptFinder m_singleHouthSpaceInterceptFinder;

      /// Raw track candidate cleaner
      RawTrackCandCleaner<VXDHoughState> m_rawTCCleaner;

      /// Resolve hit overlaps in track candidates
      TrackCandidateOverlapResolver m_overlapResolver;

      /// Store tracks as RecoTracks
      RecoTrackStorer m_recoTrackStorer;

      /// ROIFinder findlet, calculates PXD intercepts and ROIs
      ROIFinder m_roiFinder;


      /// Container to share data between findlets

      /// Pointers to the (const) SpacePoints as a vector
      std::vector<const SpacePoint*> m_spacePointVector;

      /// Vector containing the cached hit information
      std::vector<VXDHoughState> m_vxdHoughStates;

      /// Vector containint raw track candidates
      std::vector<std::vector<VXDHoughState*>> m_rawTrackCandidates;

      /// A track candidate is a vector of SpacePoint, and in each event multple track candidates
      /// will be created, which are stored in a vector themselves.
      std::vector<SpacePointTrackCand> m_trackCandidates;

      /// Use the elaborate FastInterceptFinder2D with multiple Hough spaces (true)
      /// or the simple one with just one Hough space (false)
      bool m_useMultiHoughSpaceInterceptFinding = false;

    };

  }
}
