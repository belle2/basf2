/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <bitset>
#include <array>
#include <unordered_map>

namespace Belle2 {
  class ModuleParamList;

  namespace vxdHoughTracking {
    class VXDHoughState;

    /**
    * Findlet for finding intersections of sinusoidal curves in the 2D Hough space by iteratively calling FastInterceptFinder2d.
    * The found track candidates are then clustered via a recursive search. Afterwards track candidates are formed
    * and stored in the output vector.
    */
    class SingleHoughSpaceFastInterceptFinder : public TrackingUtilities::Findlet<VXDHoughState, std::vector<VXDHoughState*>> {
      /// Parent class
      using Super = TrackingUtilities::Findlet<VXDHoughState, std::vector<VXDHoughState*>>;

    public:
      /// Find intercepts in the 2D Hough space
      SingleHoughSpaceFastInterceptFinder();

      /// Expose the parameters of the sub findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Create the store arrays
      void initialize() override;

      /// Load in the prepared hits and create track candidates for further processing like hit filtering and fitting
      void apply(std::vector<VXDHoughState>& hits, std::vector<std::vector<VXDHoughState*>>& rawTrackCandidates) override;

    private:

      /// layer filter, checks if at least hits from 3 layers are in a set of hits
      /// @param layer bitset containing information whether there as a hit in a layer
      static inline unsigned short layerFilter(const std::bitset<8>& layer)
      {
        ushort layercount = static_cast<ushort>(layer.count());
        return (layercount >= 3 ? layercount : 0);
      }

      /// find intercepts in the 2D Hough Space by recursively calling itself until no hits are assigned
      /// to a given Hough Space cell, or until the maximum number of recursions (m_maxRecursionLevel) is reached
      /// @param hits vector containing the hits that are used for track finding
      /// @param xmin minimum x-index of the sub-Hough Space in the current recursion step
      /// @param xmax maximum x-index of the sub-Hough Space in the current recursion step
      /// @param ymin minimum y-index of the sub-Hough Space in the current recursion step
      /// @param ymax maximum y-index of the sub-Hough Space in the current recursion step
      /// @param currentRecursion current recursion step, has to be < m_maxRecursionLevel
      void fastInterceptFinder2d(const std::vector<VXDHoughState*>& hits, const ushort xmin, const ushort xmax,
                                 const ushort ymin, const ushort ymax, const ushort currentRecursion);

      /// Find Hough Space clusters. Loop over all found sectors in m_SectorArray and then calls
      /// the DepthFirstSearch function to recursively find the clusters
      void FindHoughSpaceCluster();

      /// Perform depth first search recursive algorithm to find clusters in the Hough Space
      /// @param lastGlobalSectorIndex global index of the last sector checked
      void DepthFirstSearch(const uint lastGlobalSectorIndex);

      // Parameters
      /// maximum number of recursive calls of FastInterceptFinder2d
      ushort m_maxRecursionLevel = 9;

      /// number of sectors of the Hough Space on the horizontal axis
      ushort m_nAngleSectors = 512;

      /// number of sectors of the Hough Space on the vertical axis
      ushort m_nVerticalSectors = 512;

      /// vertical size of the Hough Space, defaults to the value for u-side
      float m_verticalHoughSpaceSize = 0.16;

      /// minimum x value of the Hough Space, defaults to the value for u-side
      float m_minimumX = -3.168;
      /// maximum x value of the Hough Space, defaults to the value for u-side
      float m_maximumX = 3.168;

      /// minimum cluster size of sectors belonging to intercepts in the Hough Space
      ushort m_MinimumHSClusterSize = 1;
      /// maximum cluster size of sectors belonging to intercepts in the Hough Space
      ushort m_MaximumHSClusterSize = 12;
      /// maximum cluster size in x of sectors belonging to intercepts in the Hough Space
      ushort m_MaximumHSClusterSizeX = 2;
      /// maximum cluster size in y of sectors belonging to intercepts in the Hough Space
      ushort m_MaximumHSClusterSizeY = 12;

      // class variables
      /// HS unit size in x
      float m_unitX = 0;
      /// HS unit size in y
      float m_unitY = 0;

      /// Define some magic numbers
      /// Maximum allowed recursion level
      static const ushort c_maxAllowedRecusionLevel = 10;
      /// Maximum number of HS sectors in x and y, also the size of some of the the Look-Up-Tables (LUTs) below
      static constexpr ushort c_maxHSSectorNumber = (1 << c_maxAllowedRecusionLevel);
      /// Bit mask for coordinate transformation, creates all-ones for the coordinate system conversion
      static constexpr ushort c_xIndexBitMask = c_maxHSSectorNumber - 1;
      /// Look-Up-Table size including the right-most value
      static constexpr ushort c_maxLUTSize = c_maxHSSectorNumber + 1;

      /// Look-Up-Tables for values as cache to speed up calculation
      /// sine values of the Hough Space sector boarder coordinates
      std::array<float, c_maxLUTSize> m_HSSinValuesLUT = {0};
      /// cosine values of the Hough Space sector boarder coordinates
      std::array<float, c_maxLUTSize> m_HSCosValuesLUT = {0};
      /// sine values of the Hough Space sector center coordinates
      std::array<float, c_maxHSSectorNumber> m_HSCenterSinValuesLUT = {0};
      /// cosine values of the Hough Space sector center coordinates
      std::array<float, c_maxHSSectorNumber> m_HSCenterCosValuesLUT = {0};

      /// Map containing only active HS sectors, i.e. those with hits from enough layers contained in them.
      /// The keys are the global indices of the HS cell, the values are lists of pointers to the contained hits
      std::unordered_map<uint, std::vector<VXDHoughState*>> m_activeSectorsMap;
      /// Cache the global indices of the active sectors for sorted access
      std::vector<uint> m_activeSectorsIndices;

      /// count the clusters
      ushort m_clusterCount = 0;
      /// size of the current cluster
      ushort m_clusterSize = 0;

      /// start cell of the recursive cluster finding in the Hough Space
      std::pair<ushort, ushort> m_clusterInitialPosition = std::make_pair(0, 0);

      /// the current track candidate
      std::vector<VXDHoughState*> m_currentTrackCandidate;

      /// vector containing track candidates, consisting of the found intersection values in the Hough Space
      std::vector<std::vector<VXDHoughState*>> m_trackCandidates;

    };

  }
}
