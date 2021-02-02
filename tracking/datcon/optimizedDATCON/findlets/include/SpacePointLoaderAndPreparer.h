/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>
#include <tuple>

namespace Belle2 {
  class SpacePoint;
  class VxdID;

  class ModuleParamList;

  /**
   * Findlet for loading SVDClusters that were created by the DATCONSVDSimpleClusterizerModule and prepare them
   * for usage in the FastInterceptFinder2D by calculating the conformal transformed x,y coordinates and the creating pairs
   * of coordinates for finding track candidates in r-phi and r-z.
   */
  class SpacePointLoaderAndPreparer : public
    TrackFindingCDC::Findlet<std::tuple<const SpacePoint*, const VxdID, double, double, double>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<std::tuple<const SpacePoint*, const VxdID, double, double, double>>;

  public:
    /// Load clusters and prepare them for intercept finding
    SpacePointLoaderAndPreparer();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load the SVD SpacePoints and create a tuple containing a pointer to each SpacePoint, and its VxdID,
    /// conformal transformed x and y coordinates (x'=2x/(x^2+y^2), y'=2y/(x^2+y^2)) and its z coordinate as cache
    void apply(std::vector<std::tuple<const SpacePoint*, const VxdID, double, double, double>>& hits) override;

  private:
    // Parameters
    /// StoreArray name of the input Track Store Array
    std::string m_param_SVDSpacePointStoreArrayName = "SVDSpacePoints";

    // Store Arrays
    /// Input SpacePoints Store Array
    StoreArray<SpacePoint> m_storeSpacePoints;

  };
}
