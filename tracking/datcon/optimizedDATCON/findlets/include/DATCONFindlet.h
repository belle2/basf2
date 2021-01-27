/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
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
#include <tracking/datcon/optimizedDATCON/findlets/FastInterceptFinder1D.h>
// #include <tracking/datcon/optimizedDATCON/findlets/ToPXDExtrapolator.h>
// #include <tracking/datcon/optimizedDATCON/findlets/ROICalculator.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Findlet for performing the DATCON ROI calculation.
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

    /// Simple 1D Hough Space intercept finder
    FastInterceptFinder1D m_simpleInterceptFinder;

    // container to share data between findlets

    /// hits are a combination of:
    /// pointer to a SpacePoint
    /// the VxdID of the SpacePoint
    /// the conformal-transformed x and y coordinates of the SpacePoint
    /// and its z coordinate
    std::vector<std::tuple<const SpacePoint*, const VxdID, double, double, double>> m_hits;

    /// A track candidate is a vector of SpacePoint, and in each event multple track candidates
    /// will be created, which are stored in a vector themselves.
    std::vector<std::vector<const SpacePoint*>> m_trackCandidates;

    /// Parameter to switch between the 1D and 2D intercept finder options
    /// Instead two findlets could be created that would only differ in the intercept finder findlet
    bool m_param_use1DInterceptFinder = false;

  };
}
