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
  class VxdID;

  class ModuleParamList;

  /**
   * Findlet for finging intersections of sinosoidal curves in the 2D Hough space by iteratively calling
   * fastInterceptFinder2d.
   */
  class FastInterceptFinder2D : public TrackFindingCDC::Findlet<std::pair<VxdID, std::pair<long, long>>, std::pair<double, double>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<std::pair<VxdID, std::pair<long, long>>, std::pair<double, double>>;

  public:
    /// Find intercepts in the 2D Hough space
    FastInterceptFinder2D();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the prepared hits and create tracks for extrapolation to PXD
    void apply(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits, std::vector<std::pair<double, double>>& tracks) override;

  private:

    /// convert float to long int for more similarity to the FPGA implementation
    /// @param value to be converted
    /// @param power multiply value by 10^power
    inline long convertToInt(double value, int power)
    {
      long factor = (long)pow(10, power);
      return round(factor * value);
    }

    /// layer filter, checks if at least hits from 3 layers are in a set of hits
    /// @param layer bool-vector containing information whether there as a hit in a layer
    inline unsigned short layerFilter(std::vector<bool> layer)
    {
      uint layercount = std::count(layer.begin(), layer.end(), true);
      return (layercount >= 3 ? layercount : 0);
    }

    /// find intercepts in the 2D Hough Space by recursively calling itself until no hits are assinged
    /// to a given Hough Space cell, or until the maximum number of recursions (m_maxRecursionLevel) is reached
    /// @param hits vector containing the hits that are used for track finding
    /// @param xmin minimum x-index of the sub-Hough Space in the current recursion step
    /// @param xmax maximum x-index of the sub-Hough Space in the current recursion step
    /// @param ymin minimum y-index of the sub-Hough Space in the current recursion step
    /// @param ymax maximum y-index of the sub-Hough Space in the current recursion step
    /// @param currentRecursion current recursion step, has to be < m_maxRecursionLevel
    void fastInterceptFinder2d(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits,
                               uint xmin, uint xmax, uint ymin, uint ymax, uint currentRecursion);

    /// Find Hough Space clusters. Looop over all found sectors in m_SectorArray and then calls
    /// the DepthFirstSearch function to recursively find the clusters
    void FindHoughSpaceCluster();

    /// Perform depth first search recursive algorithm to find clusters in the Hough Space
    /// @param lastIndexX x-index of the last cell checked
    /// @param lastIndexY y-index of the last cell checked
    void DepthFirstSearch(uint lastIndexX, uint lastIndexY);

    // Parameters
    /// Is this the intercept finder for the u-side hits (r-phi) or v-side (r-z)?
    bool m_param_isUFinder = true;

    /// maximum number of recursive calls of fastInterceptFinder2d
    uint m_maxRecursionLevel = 6;

    /// number of sectors of the Hough Space on the horizontal axis
    uint m_nAngleSectors = 256;

    /// number of sectors of the Hough Space on the vertical axis
    uint m_nVerticalSectors = 256;

    /// vertical size of the Hough Space, defaults to the value for u-side
    long m_verticalHoughSpaceSize = convertToInt(64, 6);

    /// minimum x value of the Hough Space, defaults to the value for u-side
    double m_minimumX = -3.168;
    /// maximum x value of the Hough Space, defaults to the value for u-side
    double m_maximumX = 3.168;

    /// HS unit size in x
    double m_unitX = 0;
    /// HS unit size in y
    double m_unitY = 0;

    /// minimum cluster size of sectors belonging to intercepts in the Hough Space
    uint m_MinimumHSClusterSize = 3;
    /// maximum cluster size of sectors belonging to intercepts in the Hough Space
    uint m_MaximumHSClusterSize = 1000;
    /// maximum cluster size in x of sectors belonging to intercepts in the Hough Space
    uint m_MaximumHSClusterSizeX = 1000;
    /// maximum cluster size in y of sectors belonging to intercepts in the Hough Space
    uint m_MaximumHSClusterSizeY = 1000;

    /// Look-Up-Tables for values as cache to speed up calculation
    /// sine values of the Hough Space sector boarder coordinates
    std::array<long, 16385> m_HSSinValuesLUT = {0};
    /// cosine values of the Hough Space sector boarder coordinates
    std::array<long, 16385> m_HSCosValuesLUT = {0};
    /// sine values of the Hough Space sector center coordinates
    std::array<long, 16384> m_HSCenterSinValuesLUT = {0};
    /// cosine values of the Hough Space sector center coordinates
    std::array<long, 16384> m_HSCenterCosValuesLUT = {0};
    /// y values of the Hough Space sector boarders
    std::array<long, 16385> m_HSYLUT = {0};
    /// y values of the Hough Space sector centers
    std::array<long, 16384> m_HSYCenterLUT = {0};
    /// x values of the Hough Space sector boarders
    std::array<double, 16385> m_HSXLUT = {0};
    /// x values of the Hough Space sector centers
    std::array<double, 16384> m_HSXCenterLUT = {0};

    /// vector containing only the 1D representation of active cells to speed up processing
    std::vector<int> m_SectorArray;
    /// vector containing information for each cell whether it contained enough hits after m_maxRecursionLevel
    /// to be "active".
    /// The value will be (- number of hits) for an active cell after fastInterceptFinder2d or 0 for an inactive cell,
    /// The value will be positive with the cluster number assigned to it after cluster finding
    std::vector<std::pair<uint, uint>> m_activeSectorArray;

    /// count the clusters
    uint m_clusterCount = 0;
    /// size of the current cluster
    uint m_clusterSize = 0;

    /// start cell of the recursive cluster finding in the Hough Space
    std::pair<int, int> m_clusterInitialPosition = std::make_pair(0, 0);
    /// center of gravity containing describing the current best track parameters in the Hough Space
    std::pair<int, int> m_clusterCoG = std::make_pair(0, 0);

    /// vector containing track candidates, consisting of the found intersection values in the Hough Space
    std::vector<std::pair<double, double>> m_trackCandidates;

  };
}
