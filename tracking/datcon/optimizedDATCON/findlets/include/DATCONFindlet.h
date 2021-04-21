/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/datcon/optimizedDATCON/findlets/SpacePointLoaderAndPreparer.h>
#include <tracking/datcon/optimizedDATCON/findlets/FastInterceptFinder2D.h>
#include <tracking/datcon/optimizedDATCON/findlets/FastInterceptFinder2DSimple.h>
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.dcl.h>
#include <tracking/datcon/optimizedDATCON/findlets/TrackCandidateOverlapResolver.h>
#include <tracking/datcon/optimizedDATCON/findlets/RecoTrackStorer.h>
#include <tracking/datcon/optimizedDATCON/findlets/ROIFinder.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class HitData;
  class SpacePoint;
  class SpacePointTrackCand;

  /**
   * Main Findlet for DATCON.
   */
  class DATCONFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor for adding the subfindlets
    DATCONFindlet();

    /// Default desctructor
    ~DATCONFindlet();

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
    FastInterceptFinder2D m_interceptFinder;
    /// Simple Hough Space intercept finder
    FastInterceptFinder2DSimple m_interceptFinderSimple;

    /// Raw track candidate cleaner
    RawTrackCandCleaner<HitData> m_rawTCCleaner;

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
    std::vector<HitData> m_hitDataVector;

    /// Vector containint raw track candidates
    std::vector<std::vector<HitData*>> m_rawTrackCandidates;

    /// A track candidate is a vector of SpacePoint, and in each event multple track candidates
    /// will be created, which are stored in a vector themselves.
    std::vector<SpacePointTrackCand> m_trackCandidates;

    /// Use the elaborate FastInterceptFinder2D with multiple Hough spaces (true)
    /// or the simple one with just one Hough space (false)
    bool m_param_useSubHoughSpaces = false;

  };
}
