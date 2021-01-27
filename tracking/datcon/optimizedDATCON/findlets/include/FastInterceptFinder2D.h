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
  class SpacePoint;
  class VxdID;

  class ModuleParamList;

  /**
   * Findlet for finding intersections of sinosoidal curves in the 2D Hough space by iteratively calling
   * fastInterceptFinder2d. This is done 80 times for a subset of SVD sensors, one subset for each layer 6 sensor
   * to reduce combinatorics in the Hough Space and to improve the purity of the found track candidates.
   * The found track candidates are then clustered via a recursive search. Afterwards track candidates are formed
   * and stored in the output vector.
   */
  class FastInterceptFinder2D : public
    TrackFindingCDC::Findlet<std::tuple<const SpacePoint*, const VxdID, double, double, double>, std::vector<const SpacePoint*>> {
    /// Parent class
    using Super =
      TrackFindingCDC::Findlet<std::tuple<const SpacePoint*, const VxdID, double, double, double>, std::vector<const SpacePoint*>>;

    typedef std::map<VxdID, std::vector<VxdID>> friendSensorMap;

    typedef std::tuple<const SpacePoint*, const VxdID, double, double, double> hitTuple;

  public:
    /// Find intercepts in the 2D Hough space
    FastInterceptFinder2D();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the prepared hits and create tracks for extrapolation to PXD
    void apply(std::vector<hitTuple>& hits, std::vector<std::vector<const SpacePoint*>>& trackCandidates) override;

  private:
    /// fill the map of friend sensors for each L6 sensor to
    void initializeSectorFriendMap();

    /// Use the friend map to just fill the hits in the acceptance region of the current L6 sensor into
    /// the m_currentSensorsHitList which is then used the Hough trafo track finding
    void fillThisSensorsHitMap(std::vector<hitTuple>& hits, const VxdID thisLayerSixSensor);

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
    void fastInterceptFinder2d(std::vector<const hitTuple*>& hits, uint xmin, uint xmax, uint ymin, uint ymax, uint currentRecursion);

    /// Find Hough Space clusters. Looop over all found sectors in m_SectorArray and then calls
    /// the DepthFirstSearch function to recursively find the clusters
    void FindHoughSpaceCluster();

    /// Perform depth first search recursive algorithm to find clusters in the Hough Space
    /// @param lastIndexX x-index of the last cell checked
    /// @param lastIndexY y-index of the last cell checked
    void DepthFirstSearch(uint lastIndexX, uint lastIndexY);

    // Parameters
    /// maximum number of recursive calls of fastInterceptFinder2d
    uint m_param_maxRecursionLevel = 7;

    /// number of sectors of the Hough Space on the horizontal axis
    uint m_param_nAngleSectors = 128;

    /// number of sectors of the Hough Space on the vertical axis
    uint m_param_nVerticalSectors = 128;

    /// vertical size of the Hough Space, defaults to the value for u-side
    double m_param_verticalHoughSpaceSize = 0.1;

    /// minimum x value of the Hough Space, defaults to the value for u-side
    double m_param_minimumX = -3.168;
    /// maximum x value of the Hough Space, defaults to the value for u-side
    double m_param_maximumX = 3.168;

    /// minimum cluster size of sectors belonging to intercepts in the Hough Space
    uint m_param_MinimumHSClusterSize = 3;
    /// maximum cluster size of sectors belonging to intercepts in the Hough Space
    uint m_param_MaximumHSClusterSize = 100;
    /// maximum cluster size in x of sectors belonging to intercepts in the Hough Space
    uint m_param_MaximumHSClusterSizeX = 100;
    /// maximum cluster size in y of sectors belonging to intercepts in the Hough Space
    uint m_param_MaximumHSClusterSizeY = 100;

    // class variables
    /// HS unit size in x
    double m_unitX = 0;
    /// HS unit size in y
    double m_unitY = 0;

    /// friendMap for all the SVD L6 sensors
    friendSensorMap m_fullFriendMap;

    /// all possible L6 sensors to loop over
    std::vector<VxdID> m_layerSixSensors;

    /// hits that are in the acceptance region (= on friend sensors) for the current L6 senosr
    std::vector<const hitTuple*> m_currentSensorsHitList;

    /// Look-Up-Tables for values as cache to speed up calculation
    /// sine values of the Hough Space sector boarder coordinates
    std::array<double, 16385> m_HSSinValuesLUT = {0};
    /// cosine values of the Hough Space sector boarder coordinates
    std::array<double, 16385> m_HSCosValuesLUT = {0};
    /// sine values of the Hough Space sector center coordinates
    std::array<double, 16384> m_HSCenterSinValuesLUT = {0};
    /// cosine values of the Hough Space sector center coordinates
    std::array<double, 16384> m_HSCenterCosValuesLUT = {0};
    /// y values of the Hough Space sector boarders
    std::array<double, 16385> m_HSYLUT = {0};
    /// y values of the Hough Space sector centers
    std::array<double, 16384> m_HSYCenterLUT = {0};
    /// x values of the Hough Space sector boarders
    std::array<double, 16385> m_HSXLUT = {0};
    /// x values of the Hough Space sector centers
    std::array<double, 16384> m_HSXCenterLUT = {0};

    /// Vector containing only the 1D representation of active cells to speed up processing
    /// The value at each position will be (- number of hits) for an active cell after fastInterceptFinder2d or 0 for an inactive cell,
    /// The value at each position will be positive with the cluster number assigned to it after cluster finding
    std::vector<int> m_SectorArray;

    /// this sorting makes sure the clusters can be searched from bottom left of the HS to top right
    /// normally, a C++ array looks like a matrix:
    /// (0   , 0) ... (maxX, 0   )
    ///    ...            ...
    /// (0, maxY) ... (maxX, maxY)
    /// but for sorting we want it to be like regular coordinates
    /// (0, maxY) ... (maxX, maxY)
    ///    ...            ...
    /// (0, 0   ) ... (maxX, 0   )
    /// By setting the offset to the maximum allowed number of cells (2^14) and simplifying
    /// (16384 - lhs.second) * 16384 + lhs.first < (16384 - rhs.second) * 16384 + rhs.first
    /// to
    /// (rhs.second - lhs.second) * 16384 < rhs.first - lhs.first
    /// we get the formula below
    struct paircompare {
      bool operator()(const std::pair<uint, uint>& lhs, const std::pair<uint, uint>& rhs) const
      {return ((int)rhs.second - (int)lhs.second) * 16384 < (int)rhs.first - (int)lhs.first;}
    };
    /// Vector only containing active HS sectors, i.e. those with hits from enough layers contained in them.
    /// The content are the indices of the HS cell, and the hit tuples in that cell
    std::map<std::pair<uint, uint>, std::vector<const hitTuple*>, paircompare> m_activeSectors;

    /// count the clusters
    uint m_clusterCount = 0;
    /// size of the current cluster
    uint m_clusterSize = 0;

    /// start cell of the recursive cluster finding in the Hough Space
    std::pair<uint, uint> m_clusterInitialPosition = std::make_pair(0, 0);
    /// center of gravity containing describing the current best track parameters in the Hough Space
    std::pair<int, int> m_clusterCoG = std::make_pair(0, 0);

    /// the current track candidate
    std::vector<const SpacePoint*> m_currentTrackCandidate;

    /// vector containing track candidates, consisting of the found intersection values in the Hough Space
    std::vector<std::vector<const SpacePoint*>> m_trackCandidates;

  };
}
