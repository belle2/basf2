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
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

SingleHoughSpaceFastInterceptFinder::SingleHoughSpaceFastInterceptFinder() : Super()
{
}

void SingleHoughSpaceFastInterceptFinder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumRecursionLevel"), m_maxRecursionLevel,
                                "Maximum recursion level for the fast Hough trafo algorithm.",  m_maxRecursionLevel);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "nAngleSectors"), m_nAngleSectors,
                                "Number of angle sectors (= x-axis) dividing the Hough space.", m_nAngleSectors);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "nVerticalSectors"), m_nVerticalSectors,
                                "Number of vertical sectors (= y-axis) dividing the Hough space.", m_nVerticalSectors);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "verticalHoughSpaceSize"), m_verticalHoughSpaceSize,
                                "Vertical size of the Hough space.", m_verticalHoughSpaceSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "HoughSpaceMinimumX"), m_minimumX,
                                "Minimum x value of the Hough space.", m_minimumX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "HoughSpaceMaximumX"), m_maximumX,
                                "Maximum x value of the Hough space.", m_maximumX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumHSClusterSize"), m_MinimumHSClusterSize,
                                "Maximum x value of the Hough space.", m_MinimumHSClusterSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSize"), m_MaximumHSClusterSize,
                                "Maximum x value of the Hough space.", m_MaximumHSClusterSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSizeX"), m_MaximumHSClusterSizeX,
                                "Maximum x value of the Hough space.", m_MaximumHSClusterSizeX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSizeY"), m_MaximumHSClusterSizeY,
                                "Maximum x value of the Hough space.", m_MaximumHSClusterSizeY);

}

void SingleHoughSpaceFastInterceptFinder::initialize()
{
  Super::initialize();

  m_maxRecursionLevel = ceil(log2(std::max(m_nAngleSectors, m_nVerticalSectors))) - 1;
  if (m_maxRecursionLevel > 14) {
    B2ERROR("The maximum number of recursions (maximumRecursionLevel) must not be larger than 14, but it is " <<
            m_maxRecursionLevel <<
            ", please choose a smaller value for maximumRecursionLevel, and / or for nAngleSectors and / or nVerticalSectors.");
  }
  m_unitX = (m_maximumX - m_minimumX) / (double)m_nAngleSectors;
  for (uint i = 0; i < m_nAngleSectors; i++) {
    double x = m_minimumX + m_unitX * (double)i;
    double xc = x + 0.5 * m_unitX;

    m_HSXLUT[i] = x;
    m_HSSinValuesLUT[i] = sin(x);
    m_HSCosValuesLUT[i] = cos(x);
    m_HSCenterSinValuesLUT[i] = sin(xc);
    m_HSCenterCosValuesLUT[i] = cos(xc);
    m_HSXCenterLUT[i] = xc;
  }
  m_HSXLUT[m_nAngleSectors] = m_maximumX;
  m_HSSinValuesLUT[m_nAngleSectors] = sin(m_maximumX);
  m_HSCosValuesLUT[m_nAngleSectors] = cos(m_maximumX);

  m_unitY = 2. * m_verticalHoughSpaceSize / m_nVerticalSectors;
  for (uint i = 0; i <= m_nVerticalSectors; i++) {
    m_HSYLUT[i] = m_verticalHoughSpaceSize - m_unitY * i;
    m_HSYCenterLUT[i] = m_verticalHoughSpaceSize - m_unitY * i - 0.5 * m_unitY;
  }
  B2DEBUG(29, "HS size x: " << (m_maximumX - m_minimumX) << " HS size y: " << m_verticalHoughSpaceSize <<
          " unitX: " << m_unitX << " unitY: " << m_unitY);
}


void SingleHoughSpaceFastInterceptFinder::apply(std::vector<VXDHoughState>& hits,
                                                std::vector<std::vector<VXDHoughState*>>& rawTrackCandidates)
{
  m_trackCandidates.clear();
  m_activeSectors.clear();

  const std::vector<VXDHoughState*> currentEventHitList = TrackFindingCDC::as_pointers<VXDHoughState>(hits);

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


void SingleHoughSpaceFastInterceptFinder::fastInterceptFinder2d(const std::vector<VXDHoughState*>& hits, uint xmin, uint xmax,
    uint ymin,
    uint ymax, uint currentRecursion)
{
  std::vector<VXDHoughState*> containedHits;
  containedHits.reserve(hits.size());
  std::bitset<8> layerHits; /* For layer filter */

  if (currentRecursion == m_maxRecursionLevel + 1) return;

  // these int-divisions can cause {min, center} or {center, max} to be the same, which is a desired behaviour
  const uint centerx = xmin + (uint)((xmax - xmin) / 2);
  const uint centery = ymin + (uint)((ymax - ymin) / 2);
  const uint xIndexCache[3] = {xmin, centerx, xmax};
  const uint yIndexCache[3] = {ymin, centery, ymax};

  for (int i = 0; i < 2 ; ++i) {
    const uint left  = xIndexCache[i];
    const uint right = xIndexCache[i + 1];
    const uint localIndexX = left;

    if (left == right) continue;

    const double& sinLeft     = m_HSSinValuesLUT[left];
    const double& cosLeft     = m_HSCosValuesLUT[left];
    const double& sinRight    = m_HSSinValuesLUT[right];
    const double& cosRight    = m_HSCosValuesLUT[right];

    // the sin and cos of the current center can't be stored in a LUT, as the number of possible centers
    // is quite large and the logic would become rather complex
    const double sinCenter   = m_HSCenterSinValuesLUT[(left + right) / 2];
    const double cosCenter   = m_HSCenterCosValuesLUT[(left + right) / 2];

    for (int j = 0; j < 2; ++j) {
      const uint lowerIndex = yIndexCache[j];
      const uint upperIndex = yIndexCache[j + 1];

      if (lowerIndex == upperIndex) continue;

      const uint localIndexY = lowerIndex;
      const double& localUpperCoordinate = m_HSYLUT[lowerIndex];
      const double& localLowerCoordinate = m_HSYLUT[upperIndex];

      // reset layerHits and containedHits
      layerHits = 0;
      containedHits.clear();
      for (VXDHoughState* hit : hits) {

        const VXDHoughState::DataCache& hitData = hit->getDataCache();
        const double& m = hitData.xConformal;
        const double& a = hitData.yConformal;

        const double derivativeyLeft   = m * -sinLeft   + a * cosLeft;
        const double derivativeyRight  = m * -sinRight  + a * cosRight;
        const double derivativeyCenter = m * -sinCenter + a * cosCenter;

        // Only interested in the rising arm of the sinosoidal curves.
        // Thus if derivative on both sides of the cell is negative, ignore and continue.
        if (derivativeyLeft < 0 and derivativeyRight < 0 and derivativeyCenter < 0) continue;

        const double yLeft   = m * cosLeft   + a * sinLeft;
        const double yRight  = m * cosRight  + a * sinRight;
        const double yCenter = m * cosCenter + a * sinCenter;

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
          m_activeSectors.insert({std::make_pair(localIndexX, localIndexY), std::make_pair(-layerFilter(layerHits), containedHits) });
        }
      }
    }
  }
}


void SingleHoughSpaceFastInterceptFinder::FindHoughSpaceCluster()
{
  m_clusterCount = 1;

  for (auto& currentCell : m_activeSectors) {

    // cell content meanings:
    // -3, -4  : active sector, not yet visited
    // 0       : non-active sector (will never be visited, only checked)
    // 1,2,3...: index of the clusters
    if (currentCell.second.first > -1) continue;

    m_clusterInitialPosition = std::make_pair(currentCell.first.first, currentCell.first.second);
    m_clusterSize = 1;
    currentCell.second.first = m_clusterCount;

    m_currentTrackCandidate.clear();
    for (VXDHoughState* hit : currentCell.second.second) {
      m_currentTrackCandidate.emplace_back(hit);
    }

    // Check for HS sectors connected to each other which could form a cluster
    DepthFirstSearch(currentCell.first.first, currentCell.first.second);
    // if cluster valid (i.e. not too small and not too big): finalize!
    if (m_clusterSize >= m_MinimumHSClusterSize and m_clusterSize <= m_MaximumHSClusterSize) {

      m_trackCandidates.emplace_back(m_currentTrackCandidate);
      m_currentTrackCandidate.clear();
    }
    m_clusterCount++;
  }
}

void SingleHoughSpaceFastInterceptFinder::DepthFirstSearch(uint lastIndexX, uint lastIndexY)
{
  if (m_clusterSize >= m_MaximumHSClusterSize) return;

  for (uint currentIndexY = lastIndexY; currentIndexY >= lastIndexY - 1; currentIndexY--) {
    if (abs((int)m_clusterInitialPosition.second - (int)currentIndexY) >= m_MaximumHSClusterSizeY or
        m_clusterSize >= m_MaximumHSClusterSize or currentIndexY > m_nVerticalSectors) return;
    for (uint currentIndexX = lastIndexX; currentIndexX <= lastIndexX + 1; currentIndexX++) {
      if (abs((int)m_clusterInitialPosition.first - (int)currentIndexX) >= m_MaximumHSClusterSizeX or
          m_clusterSize >= m_MaximumHSClusterSize or currentIndexX > m_nAngleSectors) return;

      // The cell (currentIndexX, currentIndexY) is the current one has already been checked, so continue
      if (lastIndexX == currentIndexX && lastIndexY == currentIndexY) continue;

      // first check bounds to avoid out-of-bound array access
      // as they are uints, they are always >= 0, and in case of an overflow they would be too large
      if (currentIndexX < m_nAngleSectors and currentIndexY < m_nVerticalSectors) {

        auto activeSector = m_activeSectors.find({currentIndexX, currentIndexY});
        // Only continue searching if the current cluster is smaller than the maximum cluster size
        if (activeSector != m_activeSectors.end() and activeSector->second.first < 0 /*and m_clusterSize < m_MaximumHSClusterSize*/) {
          activeSector->second.first = m_clusterCount;
          m_clusterSize++;

          // No need to check whether currentIndex exists as a key in m_activeSectors as they were created at the same time
          // so it's certain the key exists.
          for (VXDHoughState* hit : activeSector->second.second) {
            if (not TrackFindingCDC::is_in(hit, m_currentTrackCandidate)) {
              m_currentTrackCandidate.emplace_back(hit);
            }
          }

          // search in the next Hough Space cells...
          DepthFirstSearch(currentIndexX, currentIndexY);
        }
      }
    }
  }
}
