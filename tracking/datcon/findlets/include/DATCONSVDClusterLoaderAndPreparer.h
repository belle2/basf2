/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class SVDCluster;
  class VxdID;

  /**
   * Findlet for loading SVDClusters that were created by the DATCONSVDClusterizer findlet and prepare them
   * for usage in the FastInterceptFinder2D by calculating the conformal transformed x,y coordinates and the creating pairs
   * of coordinates for finding track candidates in r-phi and r-z.
   */
  class DATCONSVDClusterLoaderAndPreparer : public
    TrackFindingCDC::Findlet<const SVDCluster, const SVDCluster, std::pair<VxdID, std::pair<long, long>>,
        std::pair<VxdID, std::pair<long, long>>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const SVDCluster, const SVDCluster, std::pair<VxdID, std::pair<long, long>>,
          std::pair<VxdID, std::pair<long, long>>>;

  public:
    /// Load clusters and prepare them for intercept finding
    DATCONSVDClusterLoaderAndPreparer();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load the SVDClusters and create two vectors containing the hits prepared for intercept finding
    /// This function takes uClusters and vClusters as distinct vectors which where created by the DATCONSVDClusterizer findlet
    void apply(const std::vector<SVDCluster>& uClusters, const std::vector<SVDCluster>& vClusters,
               std::vector<std::pair<VxdID, std::pair<long, long>>>& uHits,
               std::vector<std::pair<VxdID, std::pair<long, long>>>& vHits) override;

  private:
    // Parameters
    /// Cut value for aborting tracking if more than this number of clusters is found on one layer
    uint m_param_maxClustersPerLayer = 200;

    /// array containing the number of clusters per layer. If this exceeds a cut value, tracking is aborted
    std::array<uint, 8> m_nClusterPerLayer = {0};

    // ATTENTION: all the values below are hardcoded and taken from svd/data/SVD-Components.xml
    /// Radii of the SVD layers, in µm
    const std::array<int, 4> m_const_SVDRadii = {38990, 80000, 104000, 135150};
    /// shift in r-phi to create windmill structure, in µm
    const std::array<int, 4> m_const_RPhiShiftsOfLayers = { -4680, -10780, -11036, -19076};
    /// phi-value of the first ladder in each layer, i.e. sensors X.1.Y, in rad
    const std::array<double, 4> m_const_InitialAngle = {(342.195 - 360.) / 180. * M_PI, 8. / 180. * M_PI, -8. / 180. * M_PI, -4. / 180. * M_PI};
    /// angle difference between two consecutive ladders, in rad
    const std::array<double, 4> m_const_AngleStep = {2. * M_PI / 7., M_PI / 5., M_PI / 6., M_PI / 8.};
    /// cosine values of the slanted sensors
    const std::array<double, 4> m_const_CosSlantedAngles = {1, cos(0.207694180987), cos(0.279252680319), cos(0.368264472171)};

    /// shift along z of the L3 senosrs, in µn
    const std::array<int, 2> m_const_ZShiftL3 = {92350, -32650};
    /// shift along z of the L4 senosrs, in µn
    const std::array<int, 3> m_const_ZShiftL4 = {149042, 24760, -100240};
    /// shift along z of the L5 senosrs, in µn
    const std::array<int, 4> m_const_ZShiftL5 = {233754, 110560, -14440, -139440};
    /// shift along z of the L6 senosrs, in µn
    const std::array<int, 5> m_const_ZShiftL6 = {303471, 182060, 57060, -67940, -192940};

  };
}
