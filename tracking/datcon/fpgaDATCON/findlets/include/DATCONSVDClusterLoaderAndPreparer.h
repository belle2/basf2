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

namespace Belle2 {
  class SVDCluster;
  class VxdID;

  class ModuleParamList;

  /**
   * Findlet for loading SVDClusters that were created by the DATCONSVDSimpleClusterizerModule and prepare them
   * for usage in the FastInterceptFinder2D by calculating the conformal transformed x,y coordinates and the creating pairs
   * of coordinates for finding track candidates in r-phi and r-z.
   */
  class DATCONSVDClusterLoaderAndPreparer : public TrackFindingCDC::Findlet<std::pair<VxdID, std::pair<long, long>>,
        std::pair<VxdID, std::pair<long, long>>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<std::pair<VxdID, std::pair<long, long>>,
          std::pair<VxdID, std::pair<long, long>>>;

  public:
    /// Load clusters and prepare them for intercept finding
    DATCONSVDClusterLoaderAndPreparer();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load the SVDClusters and create two vectors containing the hits prepared for intercept finding
    void apply(std::vector<std::pair<VxdID, std::pair<long, long>>>& uHits,
               std::vector<std::pair<VxdID, std::pair<long, long>>>& vHits) override;

  private:
    // Parameters
    /// StoreArray name of the input Track Store Array
    std::string m_param_SVDClusterStoreArrayName = "DATCONSVDClusters";

    /// Cut value for aborting tracking if more than this number of clusters is found on one layer
    uint m_param_maxClustersPerLayer = 50;

    // Store Arrays
    /// Input SVDCluster Store Array
    StoreArray<SVDCluster> m_SVDClusters;

    /// array containing the number of clusters per layer. If this exceeds a cut value, tracking is aborted
    std::array<uint, 8> nClusterPerLayer = {0};

    // ATTENTION: all the values below are hardcoded and taken from svd/data/SVD-Components.xml
    /// Radii of the SVD layers, in µm
    const std::array<int, 4> svdRadii = {38990, 80000, 104000, 135150};
    /// shift in r-phi to create windmill structure, in µm
    const std::array<int, 4> rPhiShiftsOfLayers = { -4680, -10780, -11036, -19076};
    /// phi-value of the first ladder in each layer, i.e. sensors X.1.Y, in rad
    const std::array<double, 4> initialAngle = {(342.195 - 360.) / 180. * M_PI, 8. / 180. * M_PI, -8. / 180. * M_PI, -4. / 180. * M_PI};
    /// angle difference between two consecutive ladders, in rad
    const std::array<double, 4> angleStep = {2. * M_PI / 7., M_PI / 5., M_PI / 6., M_PI / 8.};    // in rad
    /// cosine values of the slanted sensors
    const std::array<double, 4> cosSlantedAngles = {1, cos(0.207694180987), cos(0.279252680319), cos(0.368264472171)};
    /// sine values of the slanted sensors
    const std::array<double, 4> sinSlantedAngles = {1, sin(0.207694180987), sin(0.279252680319), sin(0.368264472171)};

    /// shift along z of the L3 senosrs, in µn
    const std::array<int, 2> zShiftL3 = {92350, -32650};
    /// shift along z of the L4 senosrs, in µn
    const std::array<int, 3> zShiftL4 = {149042, 24760, -100240};
    /// shift along z of the L5 senosrs, in µn
    const std::array<int, 4> zShiftL5 = {233754, 110560, -14440, -139440};
    /// shift along z of the L6 senosrs, in µn
    const std::array<int, 5> zShiftL6 = {303471, 182060, 57060, -67940, -192940};

    /// convert double to long int for more similarity to the FPGA implementation
    /// @param value to be converted
    /// @param power multiply value by 10^power
    inline long convertToInt(double value, int power)
    {
      long factor = (long)pow(10, power);
      return round(factor * value);
    };

  };
}
