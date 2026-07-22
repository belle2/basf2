/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/findlets/SingleHoughSpaceFastInterceptFinder.h>
#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackingUtilities;
using namespace vxdHoughTracking;

SingleHoughSpaceFastInterceptFinder::SingleHoughSpaceFastInterceptFinder() : Super()
{
}

void SingleHoughSpaceFastInterceptFinder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "maximumRecursionLevel"), m_maxRecursionLevel,
                                "Maximum recursion level for the fast Hough trafo algorithm.",  m_maxRecursionLevel);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "nAngleSectors"), m_nAngleSectors,
                                "Number of angle sectors (= x-axis) dividing the Hough space.", m_nAngleSectors);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "nVerticalSectors"), m_nVerticalSectors,
                                "Number of vertical sectors (= y-axis) dividing the Hough space.", m_nVerticalSectors);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "verticalHoughSpaceSize"), m_verticalHoughSpaceSize,
                                "Vertical size of the Hough space.", m_verticalHoughSpaceSize);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "HoughSpaceMinimumX"), m_minimumX,
                                "Minimum x value of the Hough space.", m_minimumX);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "HoughSpaceMaximumX"), m_maximumX,
                                "Maximum x value of the Hough space.", m_maximumX);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "minimumHSClusterSize"), m_MinimumHSClusterSize,
                                "Maximum x value of the Hough space.", m_MinimumHSClusterSize);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "maximumHSClusterSize"), m_MaximumHSClusterSize,
                                "Maximum x value of the Hough space.", m_MaximumHSClusterSize);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "maximumHSClusterSizeX"), m_MaximumHSClusterSizeX,
                                "Maximum x value of the Hough space.", m_MaximumHSClusterSizeX);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "maximumHSClusterSizeY"), m_MaximumHSClusterSizeY,
                                "Maximum x value of the Hough space.", m_MaximumHSClusterSizeY);

}

void SingleHoughSpaceFastInterceptFinder::initialize()
{
  Super::initialize();

  const ushort maxRecursionLevelFromSectors = ceil(log2(std::max(m_nAngleSectors, m_nVerticalSectors))) - 1;
  m_maxRecursionLevel = std::max(maxRecursionLevelFromSectors, m_maxRecursionLevel);
  B2ASSERT("The maximum number of recursions (maximumRecursionLevel) must not be larger than " << c_maxAllowedRecusionLevel <<
           ", but it is " << m_maxRecursionLevel <<
           ", please choose a smaller value for maximumRecursionLevel, and / or for nAngleSectors and / or nVerticalSectors.",
           m_maxRecursionLevel <= c_maxAllowedRecusionLevel);
  B2ASSERT("The maximum number of angleSectors must not be larger than " << c_maxHSSectorNumber <<
           ", but it is " << m_nAngleSectors << ", please choose a smaller value for nAngleSectors.",
           m_nAngleSectors <= c_maxHSSectorNumber);
  B2ASSERT("The maximum number of verticalSectors must not be larger than " << c_maxHSSectorNumber <<
           ", but it is " << m_nVerticalSectors << ", please choose a smaller value for nAngleSectors.",
           m_nVerticalSectors <= c_maxHSSectorNumber);

  m_unitX = (m_maximumX - m_minimumX) / (float)m_nAngleSectors;
  m_unitY = 2. * m_verticalHoughSpaceSize / m_nVerticalSectors;

  for (ushort i = 0; i < m_nAngleSectors; i++) {
    float x = m_minimumX + m_unitX * (float)i;
    float xc = x + 0.5 * m_unitX;

    m_HSSinValuesLUT[i] = sin(x);
    m_HSCosValuesLUT[i] = cos(x);
    m_HSCenterSinValuesLUT[i] = sin(xc);
    m_HSCenterCosValuesLUT[i] = cos(xc);
  }
  m_HSSinValuesLUT[m_nAngleSectors] = sin(m_maximumX);
  m_HSCosValuesLUT[m_nAngleSectors] = cos(m_maximumX);

  B2DEBUG(29, "HS size x: " << (m_maximumX - m_minimumX) << " HS size y: " << m_verticalHoughSpaceSize <<
          " unitX: " << m_unitX << " unitY: " << m_unitY);
}


void SingleHoughSpaceFastInterceptFinder::apply(std::vector<VXDHoughState>& hits,
                                                std::vector<std::vector<VXDHoughState*>>& rawTrackCandidates)
{
  m_trackCandidates.clear();
  m_activeSectorsMap.clear();
  m_activeSectorsIndices.clear();

  const std::vector<VXDHoughState*> currentEventHitList = TrackingUtilities::as_pointers<VXDHoughState>(hits);

  fastInterceptFinder2d(currentEventHitList, 0, m_nAngleSectors, 0, m_nVerticalSectors, 0);

  FindHoughSpaceCluster();

  for (auto& trackCand : m_trackCandidates) {
    // sort for layer, and 2D radius in case of same layer before storing as SpacePointTrackCand
    // outer hit goes first, as later on tracks are build from outside to inside
    std::sort(trackCand.begin(), trackCand.end(),
    [](const VXDHoughState * a, const VXDHoughState * b) {
      return
        (a->getDataCache().layer > b->getDataCache().layer) or
        (a->getDataCache().layer == b->getDataCache().layer
         and a->getHit()->getPosition().Perp() > b->getHit()->getPosition().Perp());
    });

    rawTrackCandidates.emplace_back(trackCand);
  }

  B2DEBUG(29, "m_trackCandidates.size: " << m_trackCandidates.size());

}


void SingleHoughSpaceFastInterceptFinder::fastInterceptFinder2d(const std::vector<VXDHoughState*>& hits,
    const ushort xmin, const ushort xmax, const ushort ymin, const ushort ymax, const ushort currentRecursion)
{
  std::vector<VXDHoughState*> containedHits;
  containedHits.reserve(hits.size());
  std::bitset<8> layerHits; /* For layer filter */

  if (currentRecursion == m_maxRecursionLevel + 1) return;

  // these int-divisions can cause {min, center} or {center, max} to be the same, which is a desired behaviour
  const ushort centerx = xmin + (ushort)((xmax - xmin) >> 1);
  const ushort centery = ymin + (ushort)((ymax - ymin) >> 1);
  const ushort xIndexCache[3] = {xmin, centerx, xmax};
  const ushort yIndexCache[3] = {ymin, centery, ymax};

  for (int i = 0; i < 2 ; ++i) {
    const ushort left  = xIndexCache[i];
    const ushort right = xIndexCache[i + 1];
    const ushort localIndexX = left;

    if (left == right) continue;

    const float& sinLeft     = m_HSSinValuesLUT[left];
    const float& cosLeft     = m_HSCosValuesLUT[left];
    const float& sinRight    = m_HSSinValuesLUT[right];
    const float& cosRight    = m_HSCosValuesLUT[right];

    // the sin and cos of the current center can't be stored in a LUT, as the number of possible centers
    // is quite large and the logic would become rather complex
    const float sinCenter   = m_HSCenterSinValuesLUT[(left + right) >> 1];
    const float cosCenter   = m_HSCenterCosValuesLUT[(left + right) >> 1];

    for (int j = 0; j < 2; ++j) {
      const ushort lowerIndex = yIndexCache[j];
      const ushort upperIndex = yIndexCache[j + 1];

      if (lowerIndex == upperIndex) continue;

      const ushort localIndexY = lowerIndex;
      // Sector counting for y starts at positive values, i.e. the topmost sector has index 0,
      // and the bottommost sector has the highest index
      const float& localUpperCoordinate = m_verticalHoughSpaceSize - m_unitY * lowerIndex;
      const float& localLowerCoordinate = m_verticalHoughSpaceSize - m_unitY * upperIndex;

      // reset layerHits and containedHits
      layerHits = 0;
      containedHits.clear();
      for (VXDHoughState* hit : hits) {

        const VXDHoughState::DataCache& hitData = hit->getDataCache();
        const float& m = hitData.xConformal;
        const float& a = hitData.yConformal;

        const float derivativeyLeft   = m * -sinLeft   + a * cosLeft;
        const float derivativeyRight  = m * -sinRight  + a * cosRight;
        const float derivativeyCenter = m * -sinCenter + a * cosCenter;

        // Only interested in the rising arm of the sinosoidal curves.
        // Thus if derivative on both sides of the cell is negative, ignore and continue.
        if (derivativeyLeft < 0 and derivativeyRight < 0 and derivativeyCenter < 0) continue;

        const float yLeft   = m * cosLeft   + a * sinLeft;
        const float yRight  = m * cosRight  + a * sinRight;
        const float yCenter = m * cosCenter + a * sinCenter;

        /* Check if HS-parameter curve is inside (or outside) actual sub-HS */
        if ((yLeft <= localUpperCoordinate and yRight >= localLowerCoordinate) or
            (yCenter <= localUpperCoordinate and yLeft >= localLowerCoordinate and yRight >= localLowerCoordinate) or
            (yCenter >= localLowerCoordinate and yLeft <= localUpperCoordinate and yRight <= localUpperCoordinate)) {
          layerHits[hitData.layer] = true;
          containedHits.emplace_back(hit);
        }
      }

      if (layerFilter(layerHits) > 0) {
        // recursive call of fastInterceptFinder2d, until currentRecursion == m_maxRecursionLevel
        if (currentRecursion < m_maxRecursionLevel) {
          fastInterceptFinder2d(containedHits, left, right, lowerIndex, upperIndex, currentRecursion + 1);
        } else {
          // As usual in creating a 2D-array as 1D, calculate the global index as
          // xIndex + yIndex * ySize
          // A bit more complicated here though, since the y-axis is inverted on the fly.
          // Instead of starting the coordinate system in the top-left corner, it has to start in the bottom-left corner
          // for creating HS clusters from bottom to top and from left to right, so this becomes
          // xIndex + ySize * (ySize - yIndex)   (eq. 0)
          const uint globalIndex = localIndexX + c_maxHSSectorNumber * (c_maxHSSectorNumber - localIndexY);
          m_activeSectorsMap.insert({globalIndex, containedHits});
          m_activeSectorsIndices.push_back(globalIndex);
        }
      }
    }
  }
}


void SingleHoughSpaceFastInterceptFinder::FindHoughSpaceCluster()
{
  m_clusterCount = 1;

  // Sort vector to create HS clusters from bottom left to top right
  std::sort(m_activeSectorsIndices.begin(), m_activeSectorsIndices.end());

  for (const uint& currentGlobalSectorIndex : m_activeSectorsIndices) {

    const auto currentCellHits = m_activeSectorsMap.find(currentGlobalSectorIndex);
    if (currentCellHits == m_activeSectorsMap.end()) {
      continue;
    }

    // Get local (x, y) indices out of the globalSectorIndex by reverting (eq. 0)
    m_clusterInitialPosition = std::make_pair((currentGlobalSectorIndex & c_xIndexBitMask),
                                              c_maxHSSectorNumber - (currentGlobalSectorIndex >> c_maxAllowedRecusionLevel));
    m_clusterSize = 1;

    m_currentTrackCandidate.clear();
    for (VXDHoughState* hit : currentCellHits->second) {
      m_currentTrackCandidate.emplace_back(hit);
    }
    // this sector now has been used and the hits have been processed, so it can be removed from the map
    m_activeSectorsMap.erase(currentGlobalSectorIndex);

    // Check for HS sectors connected to each other which could form a cluster
    DepthFirstSearch(currentGlobalSectorIndex);
    // if cluster valid (i.e. not too small and not too big): finalize!
    if (m_clusterSize >= m_MinimumHSClusterSize and m_clusterSize <= m_MaximumHSClusterSize) {
      m_trackCandidates.emplace_back(m_currentTrackCandidate);
      m_currentTrackCandidate.clear();
    }
    m_clusterCount++;
  }
}

void SingleHoughSpaceFastInterceptFinder::DepthFirstSearch(const uint lastGlobalSectorIndex)
{
  if (m_clusterSize >= m_MaximumHSClusterSize) return;

  // Get local (x, y) indices out of the globalSectorIndex by reverting (eq. 0)
  const ushort lastLocalIndexX = (lastGlobalSectorIndex & c_xIndexBitMask);
  const ushort lastLocalIndexY = c_maxHSSectorNumber - (lastGlobalSectorIndex >> c_maxAllowedRecusionLevel);

  // For the iterative / recursive serach, just check the direct neighbours in x and y direction
  for (ushort currentLocalIndexY = lastLocalIndexY; currentLocalIndexY >= lastLocalIndexY - 1; currentLocalIndexY--) {
    if (std::abs(static_cast<short>(m_clusterInitialPosition.second) - static_cast<short>(currentLocalIndexY)) >=
        m_MaximumHSClusterSizeY
        or m_clusterSize >= m_MaximumHSClusterSize or currentLocalIndexY > m_nVerticalSectors) {
      return;
    }

    for (ushort currentLocalIndexX = lastLocalIndexX; currentLocalIndexX <= lastLocalIndexX + 1; currentLocalIndexX++) {
      if (std::abs(static_cast<short>(m_clusterInitialPosition.first) - static_cast<short>(currentLocalIndexX)) >= m_MaximumHSClusterSizeX
          or m_clusterSize >= m_MaximumHSClusterSize or currentLocalIndexX > m_nAngleSectors) {
        return;
      }

      // Calculate the global index for this sector by applying (eq. 0)
      const uint currentGlobalSectorIndex = currentLocalIndexX + c_maxHSSectorNumber * (c_maxHSSectorNumber - currentLocalIndexY);
      // The currentGlobalSectorIndex sector is the current one has already been checked, so continue
      if (currentGlobalSectorIndex == lastGlobalSectorIndex) {
        continue;
      }

      // first check bounds to avoid out-of-bound array access
      // as they are uints, they are always >= 0, and in case of an overflow they would be too large
      if (currentLocalIndexX < m_nAngleSectors and currentLocalIndexY < m_nVerticalSectors) {

        const auto activeSectorHits = m_activeSectorsMap.find(currentGlobalSectorIndex);
        // Only continue searching if the current cluster is smaller than the maximum cluster size
        if (activeSectorHits != m_activeSectorsMap.end()) {
          m_clusterSize++;

          // No need to check whether currentGlobalSectorIndex exists as a key in m_activeSectorsMap as they were
          // created at the same time so it's certain the key exists.
          for (VXDHoughState* hit : activeSectorHits->second) {
            if (not TrackingUtilities::is_in(hit, m_currentTrackCandidate)) {
              m_currentTrackCandidate.emplace_back(hit);
            }
          }
          // this sector now has been used and the hits have been processed, so it can be removed from the map
          m_activeSectorsMap.erase(currentGlobalSectorIndex);

          // search in the next Hough Space cells...
          DepthFirstSearch(currentGlobalSectorIndex);
        }
      }
    }
  }
}
