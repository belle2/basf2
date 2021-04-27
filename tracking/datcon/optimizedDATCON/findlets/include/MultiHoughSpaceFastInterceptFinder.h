/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/datcon/optimizedDATCON/findlets/HitSelector.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class VxdID;
  class HitData;

  class ModuleParamList;

  /**
   * Findlet for finding intersections of sinosoidal curves in the 2D Hough space by iteratively calling
   * fastInterceptFinder2d. This is done 80 times for a subset of SVD sensors, one subset for each layer 6 sensor
   * to reduce combinatorics in the Hough Space and to improve the purity of the found track candidates.
   * The found track candidates are then clustered via a recursive search. Afterwards track candidates are formed
   * and stored in the output vector.
   */
  class MultiHoughSpaceFastInterceptFinder : public TrackFindingCDC::Findlet<HitData, std::vector<HitData*>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<HitData, std::vector<HitData*>>;

    typedef std::map<VxdID, std::vector<VxdID>> friendSensorMap;

  public:
    /// Find intercepts in the 2D Hough space
    MultiHoughSpaceFastInterceptFinder();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the prepared hits and create track candidates for further processing like hit filtering and fitting
    void apply(std::vector<HitData>& hits, std::vector<std::vector<HitData*>>& rawTrackCandidates) override;

  private:
    // sub-findlets
    /// Use the friend map to just fill the hits in the acceptance region of the current L6 sensor into
    /// the m_currentSensorsHitList which is then used the Hough trafo track finding
    HitSelector m_HitSelector;

    /// fill the map of friend sensors for each L6 sensor to
    void initializeSectorFriendMap();

    /// layer filter, checks if at least hits from 3 layers are in a set of hits
    /// @param layer bitset containing information whether there as a hit in a layer
    inline unsigned short layerFilter(std::bitset<8>& layer)
    {
      uint layercount = layer.count();
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
    void fastInterceptFinder2d(const std::vector<HitData*>& hits, uint xmin, uint xmax, uint ymin, uint ymax,
                               uint currentRecursion);

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
    uint m_param_nAngleSectors = 256;

    /// number of sectors of the Hough Space on the vertical axis
    uint m_param_nVerticalSectors = 256;

    /// vertical size of the Hough Space, defaults to the value for u-side
    double m_param_verticalHoughSpaceSize = 0.2;

    /// minimum x value of the Hough Space, defaults to the value for u-side
    double m_param_minimumX = -3.168;
    /// maximum x value of the Hough Space, defaults to the value for u-side
    double m_param_maximumX = 3.168;

    /// minimum cluster size of sectors belonging to intercepts in the Hough Space
    uint m_param_MinimumHSClusterSize = 6;
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

    /// hits that are in the acceptance region (= on friend sensors) for the current L6 senosr
    std::vector<HitData*> m_currentSensorsHitList;

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

    /// this sorting makes sure the clusters can be searched from bottom left of the HS to top right
    /// normally, a C++ array looks like a matrix:
    /// (0, 0   ) ... (maxX, 0   )
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
    /// Map containing only active HS sectors, i.e. those with hits from enough layers contained in them.
    /// The keys are the indices of the HS cell, and the custom sort function above is used to sort the content.
    /// The value is a pair consisting of the (negative) number of layers hit in a given cell,
    /// and a vector containing the hit information of all hits that are contained in this cell.
    /// During cluster finding the first value of the value-pair will be assigned the current cluster number.
    std::map<std::pair<uint, uint>, std::pair<int, std::vector<HitData*>>, paircompare> m_activeSectors;

    /// count the clusters
    uint m_clusterCount = 0;
    /// size of the current cluster
    uint m_clusterSize = 0;

    /// start cell of the recursive cluster finding in the Hough Space
    std::pair<uint, uint> m_clusterInitialPosition = std::make_pair(0, 0);
    /// center of gravity containing describing the current best track parameters in the Hough Space
    std::pair<int, int> m_clusterCoG = std::make_pair(0, 0);

    /// the current track candidate
    std::vector<HitData*> m_currentTrackCandidate;

    /// vector containing track candidates, consisting of the found intersection values in the Hough Space
    std::vector<std::vector<HitData*>> m_trackCandidates;
  };
}
