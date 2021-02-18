/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/FastInterceptFinder2D.h>

#include <tracking/datcon/optimizedDATCON/entities/HitDataCache.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <iostream>

#include <fstream>

using namespace Belle2;
using namespace TrackFindingCDC;

FastInterceptFinder2D::FastInterceptFinder2D() : Super()
{
  addProcessingSignalListener(&m_HitSelector);
}

void FastInterceptFinder2D::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumRecursionLevel"),
                                m_param_maxRecursionLevel,
                                "Maximum recursion level for the fast Hough trafo algorithm.",
                                m_param_maxRecursionLevel);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "nAngleSectors"),
                                m_param_nAngleSectors,
                                "Number of angle sectors (= x-axis) dividing the Hough space.",
                                m_param_nAngleSectors);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "nVerticalSectors"),
                                m_param_nVerticalSectors,
                                "Number of vertical sectors (= y-axis) dividing the Hough space.",
                                m_param_nVerticalSectors);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "verticalHoughSpaceSize"),
                                m_param_verticalHoughSpaceSize,
                                "Vertical size of the Hough space.",
                                m_param_verticalHoughSpaceSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "HoughSpaceMinimumX"),
                                m_param_minimumX,
                                "Minimum x value of the Hough space.",
                                m_param_minimumX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "HoughSpaceMaximumX"),
                                m_param_maximumX,
                                "Maximum x value of the Hough space.",
                                m_param_maximumX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumHSClusterSize"),
                                m_param_MinimumHSClusterSize,
                                "Maximum x value of the Hough space.",
                                m_param_MinimumHSClusterSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSize"),
                                m_param_MaximumHSClusterSize,
                                "Maximum x value of the Hough space.",
                                m_param_MaximumHSClusterSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSizeX"),
                                m_param_MaximumHSClusterSizeX,
                                "Maximum x value of the Hough space.",
                                m_param_MaximumHSClusterSizeX);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHSClusterSizeY"),
                                m_param_MaximumHSClusterSizeY,
                                "Maximum x value of the Hough space.",
                                m_param_MaximumHSClusterSizeY);

}

void FastInterceptFinder2D::initialize()
{
  Super::initialize();

  // TODO: fix value for max recursion level, should be the maximum of the parameter given, and the calculation below
  m_param_maxRecursionLevel = ceil(log2(std::max(m_param_nAngleSectors, m_param_nVerticalSectors))) - 1;
  B2ASSERT("The maximum number of recursions (maximumRecursionLevel) must not be larger than 14, but it is " <<
           m_param_maxRecursionLevel <<
           ", please choose a smaller value for maximumRecursionLevel, and / or for nAngleSectors and / or nVerticalSectors.",
           m_param_maxRecursionLevel <= 14);
  m_unitX = (m_param_maximumX - m_param_minimumX) / (double)m_param_nAngleSectors;
  for (uint i = 0; i < m_param_nAngleSectors; i++) {
    double x = m_param_minimumX + m_unitX * (double)i;
    double xc = x + 0.5 * m_unitX;

    m_HSXLUT[i] = x;
    m_HSSinValuesLUT[i] = sin(x);
    m_HSCosValuesLUT[i] = cos(x);
    m_HSCenterSinValuesLUT[i] = sin(xc);
    m_HSCenterCosValuesLUT[i] = cos(xc);
    m_HSXCenterLUT[i] = xc;
  }
  m_HSXLUT[m_param_nAngleSectors] = m_param_maximumX;
  m_HSSinValuesLUT[m_param_nAngleSectors] = sin(m_param_maximumX);
  m_HSCosValuesLUT[m_param_nAngleSectors] = cos(m_param_maximumX);

  m_unitY = 2. * m_param_verticalHoughSpaceSize / m_param_nVerticalSectors;
  for (uint i = 0; i <= m_param_nVerticalSectors; i++) {
    m_HSYLUT[i] = m_param_verticalHoughSpaceSize - m_unitY * i;
    m_HSYCenterLUT[i] = m_param_verticalHoughSpaceSize - m_unitY * i - 0.5 * m_unitY;
  }
  B2DEBUG(29, "HS size x: " << (m_param_maximumX - m_param_minimumX) << " HS size y: " << m_param_verticalHoughSpaceSize << " unitX: "
          << m_unitX <<
          " unitY: " << m_unitY);

  initializeSectorFriendMap();
}


void FastInterceptFinder2D::apply(std::vector<HitDataCache>& hits, std::vector<std::vector<HitDataCache*>>& rawTrackCandidates)
{
  m_trackCandidates.clear();

  for (auto& friends : m_fullFriendMap) {
    m_activeSectors.clear();
    m_currentSensorsHitList.clear();

    m_HitSelector.apply(hits, friends.second, m_currentSensorsHitList);

    fastInterceptFinder2d(m_currentSensorsHitList, 0, m_param_nAngleSectors, 0, m_param_nVerticalSectors, 0);

    FindHoughSpaceCluster();

    if (m_breakFlag) {
      gnuplotoutput(m_currentSensorsHitList);
      uint count = 0;
      for (auto& hit : hits) {
        double X = hit.x;
        double Y = hit.y;
        double Z = hit.z;
        B2INFO("hit " << count << ":  " << X << "  " << Y << "  " << Z << "  on sensor:   " << hit.sensorID);
        count++;
      }
      B2FATAL("Too many SPs in a SPTC for sensor  " << friends.first << " ,  aborting DATCON!");
    }
  }

  for (auto& trackCand : m_trackCandidates) {
    // sort for layer, and 2D radius in case of same layer before storing as SpacePointTrackCand
    // outer hit goes first, as later on tracks are build from outside to inside
    std::sort(trackCand.begin(), trackCand.end(),
    [](const HitDataCache * a, const HitDataCache * b) {
      return
        (a->sensorID.getLayerNumber() > b->sensorID.getLayerNumber()) or
        (a->sensorID.getLayerNumber() == b->sensorID.getLayerNumber()
         and a->spacePoint->getPosition().Perp() > b->spacePoint->getPosition().Perp());
    });

    rawTrackCandidates.emplace_back(trackCand);
  }

  B2DEBUG(29, "m_trackCandidates.size: " << m_trackCandidates.size());

}


void FastInterceptFinder2D::initializeSectorFriendMap()
{
  const std::vector<VxdID> friends6XX1 = {VxdID(3, 0, 1), VxdID(4, 0, 1), VxdID(5, 0, 1)};
  const std::vector<VxdID> friends6XX2 = {VxdID(3, 0, 1), VxdID(4, 0, 1), VxdID(4, 0, 2), VxdID(5, 0, 1), VxdID(5, 0, 2)};
  const std::vector<VxdID> friends6XX3 = {VxdID(3, 0, 1), VxdID(3, 0, 2), VxdID(4, 0, 2), VxdID(5, 0, 2), VxdID(5, 0, 3)};
  const std::vector<VxdID> friends6XX4 = {VxdID(3, 0, 2), VxdID(4, 0, 2), VxdID(4, 0, 3), VxdID(5, 0, 3), VxdID(5, 0, 4)};
  const std::vector<VxdID> friends6XX5 = {VxdID(3, 0, 2), VxdID(4, 0, 3), VxdID(5, 0, 4)};

  friendSensorMap thetaFriends;
  thetaFriends.insert(std::make_pair(VxdID(6, 0, 1), friends6XX1));
  thetaFriends.insert(std::make_pair(VxdID(6, 0, 2), friends6XX2));
  thetaFriends.insert(std::make_pair(VxdID(6, 0, 3), friends6XX3));
  thetaFriends.insert(std::make_pair(VxdID(6, 0, 4), friends6XX4));
  thetaFriends.insert(std::make_pair(VxdID(6, 0, 5), friends6XX5));

  const std::vector<VxdID> friends601X = {VxdID(3, 1, 0), VxdID(3, 6, 0), VxdID(3, 7, 0), VxdID(4, 1, 0), VxdID(4, 2, 0), VxdID(4, 10, 0), VxdID(5, 1, 0), VxdID(5, 2, 0), VxdID(5, 12, 0)};
  const std::vector<VxdID> friends602X = {VxdID(3, 1, 0), VxdID(3, 2, 0), VxdID(3, 7, 0), VxdID(4, 1, 0), VxdID(4, 2, 0), VxdID(4, 10, 0), VxdID(5, 1, 0), VxdID(5, 2, 0), VxdID(5, 3, 0)};
  const std::vector<VxdID> friends603X = {VxdID(3, 1, 0), VxdID(3, 2, 0), VxdID(3, 7, 0), VxdID(4, 1, 0), VxdID(4, 2, 0), VxdID(4, 3, 0), VxdID(5, 2, 0), VxdID(5, 3, 0)};
  const std::vector<VxdID> friends604X = {VxdID(3, 1, 0), VxdID(3, 2, 0), VxdID(4, 2, 0), VxdID(4, 3, 0), VxdID(5, 3, 0), VxdID(5, 4, 0)};
  const std::vector<VxdID> friends605X = {VxdID(3, 1, 0), VxdID(3, 2, 0), VxdID(3, 3, 0), VxdID(4, 2, 0), VxdID(4, 3, 0), VxdID(4, 4, 0), VxdID(5, 3, 0), VxdID(5, 4, 0), VxdID(5, 5, 0)};
  const std::vector<VxdID> friends606X = {VxdID(3, 2, 0), VxdID(3, 3, 0), VxdID(4, 3, 0), VxdID(4, 4, 0), VxdID(4, 5, 0), VxdID(5, 4, 0), VxdID(5, 5, 0), VxdID(5, 6, 0)};
  const std::vector<VxdID> friends607X = {VxdID(3, 2, 0), VxdID(3, 3, 0), VxdID(3, 4, 0), VxdID(4, 4, 0), VxdID(4, 5, 0), VxdID(5, 5, 0), VxdID(5, 6, 0)};
  const std::vector<VxdID> friends608X = {VxdID(3, 2, 0), VxdID(3, 3, 0), VxdID(3, 4, 0), VxdID(4, 4, 0), VxdID(4, 5, 0), VxdID(4, 6, 0), VxdID(5, 6, 0), VxdID(5, 7, 0)};
  const std::vector<VxdID> friends609X = {VxdID(3, 3, 0), VxdID(3, 4, 0), VxdID(3, 5, 0), VxdID(4, 5, 0), VxdID(4, 6, 0), VxdID(4, 7, 0), VxdID(5, 6, 0), VxdID(5, 7, 0), VxdID(5, 8, 0)};
  const std::vector<VxdID> friends610X = {VxdID(3, 3, 0), VxdID(3, 4, 0), VxdID(3, 5, 0), VxdID(4, 5, 0), VxdID(4, 6, 0), VxdID(4, 7, 0), VxdID(5, 7, 0), VxdID(5, 8, 0), VxdID(5, 9, 0)};
  const std::vector<VxdID> friends611X = {VxdID(3, 4, 0), VxdID(3, 5, 0), VxdID(4, 6, 0), VxdID(4, 7, 0), VxdID(4, 8, 0), VxdID(5, 8, 0), VxdID(5, 9, 0)};
  const std::vector<VxdID> friends612X = {VxdID(3, 4, 0), VxdID(3, 5, 0), VxdID(3, 6, 0), VxdID(4, 7, 0), VxdID(4, 8, 0), VxdID(5, 9, 0), VxdID(5, 10, 0)};
  const std::vector<VxdID> friends613X = {VxdID(3, 5, 0), VxdID(3, 6, 0), VxdID(4, 7, 0), VxdID(4, 8, 0), VxdID(4, 9, 0), VxdID(5, 9, 0), VxdID(5, 10, 0), VxdID(5, 11, 0)};
  const std::vector<VxdID> friends614X = {VxdID(3, 5, 0), VxdID(3, 6, 0), VxdID(3, 7, 0), VxdID(4, 8, 0), VxdID(4, 9, 0), VxdID(4, 10, 0), VxdID(5, 10, 0), VxdID(5, 11, 0), VxdID(5, 12, 0)};
  const std::vector<VxdID> friends615X = {VxdID(3, 6, 0), VxdID(3, 7, 0), VxdID(4, 9, 0), VxdID(4, 10, 0), VxdID(5, 11, 0), VxdID(5, 12, 0)};
  const std::vector<VxdID> friends616X = {VxdID(3, 1, 0), VxdID(3, 7, 0), VxdID(4, 1, 0), VxdID(4, 9, 0), VxdID(4, 10, 0), VxdID(5, 1, 0), VxdID(5, 12, 0)};

  friendSensorMap phiFriends;
  phiFriends.insert(std::make_pair(VxdID(6, 1, 0), friends601X));
  phiFriends.insert(std::make_pair(VxdID(6, 2, 0), friends602X));
  phiFriends.insert(std::make_pair(VxdID(6, 3, 0), friends603X));
  phiFriends.insert(std::make_pair(VxdID(6, 4, 0), friends604X));
  phiFriends.insert(std::make_pair(VxdID(6, 5, 0), friends605X));
  phiFriends.insert(std::make_pair(VxdID(6, 6, 0), friends606X));
  phiFriends.insert(std::make_pair(VxdID(6, 7, 0), friends607X));
  phiFriends.insert(std::make_pair(VxdID(6, 8, 0), friends608X));
  phiFriends.insert(std::make_pair(VxdID(6, 9, 0), friends609X));
  phiFriends.insert(std::make_pair(VxdID(6, 10, 0), friends610X));
  phiFriends.insert(std::make_pair(VxdID(6, 11, 0), friends611X));
  phiFriends.insert(std::make_pair(VxdID(6, 12, 0), friends612X));
  phiFriends.insert(std::make_pair(VxdID(6, 13, 0), friends613X));
  phiFriends.insert(std::make_pair(VxdID(6, 14, 0), friends614X));
  phiFriends.insert(std::make_pair(VxdID(6, 15, 0), friends615X));
  phiFriends.insert(std::make_pair(VxdID(6, 16, 0), friends616X));

  std::vector<VxdID> friendSensors;

  // loop over all phiFriends containing layer 6 ladders
  for (auto& phiFriendPair : phiFriends) {
    // get the according vector friends6XX0 for this phiFriendPair
    std::vector<VxdID> phiFriendLadders = phiFriendPair.second;
    unsigned short layer6Ladder = phiFriendPair.first.getLadderNumber();
    // loop over all thetafriends containing layer 6 sensors
    for (auto& thetaFriendPair : thetaFriends) {
      friendSensors.clear();
      // get the according vector friends600Y
      std::vector<VxdID> thetaFriendSensors = thetaFriendPair.second;
      unsigned short layer6Sensor = thetaFriendPair.first.getSensorNumber();

      // loop over all the layers/ladders in this phifriends vector, one specific friends6XX0
      for (auto& phiFriend : phiFriendLadders) {
        // loop over all sensor number in the different layers 3-5, one specific friends600Y
        for (auto& thetaFriend : thetaFriendSensors) {
          if (phiFriend.getLayerNumber() == thetaFriend.getLayerNumber()) {
            // get layer number of either phiFriend or thetaFriend, ladder number from phiFriend, and the sensor number from the thetaFriend
            friendSensors.emplace_back(VxdID(phiFriend.getLayerNumber(), phiFriend.getLadderNumber(), thetaFriend.getSensorNumber()));
          }
        }
      }
      // add the layer 6 sensor to the list of its own friends to check the vector with std::find in fillThisSensorsHitMap
      friendSensors.emplace_back(VxdID(6, layer6Ladder, layer6Sensor));
      m_fullFriendMap.insert(std::make_pair(VxdID(6, layer6Ladder, layer6Sensor), friendSensors));
    }
  }
}


void FastInterceptFinder2D::fastInterceptFinder2d(const std::vector<HitDataCache*>& hits, uint xmin, uint xmax, uint ymin,
                                                  uint ymax, uint currentRecursion)
{
  std::vector<HitDataCache*> containedHits;
  containedHits.reserve(hits.size());
  std::bitset<8> layerHits; /* For layer filter */

  if (currentRecursion == m_param_maxRecursionLevel + 1) return;

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
      for (HitDataCache* hit : hits) {

        const double& m = hit->xConformal;
        const double& a = hit->yConformal;

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
        if ((yLeft <= localUpperCoordinate && yRight >= localLowerCoordinate) ||
            (yCenter <= localUpperCoordinate && yCenter >= localLowerCoordinate /*&& derivativeyCenter >= 0*/)) {
          layerHits[hit->sensorID.getLayerNumber()] = true;
          containedHits.emplace_back(hit);
        }
      }

      if (layerFilter(layerHits) > 0) {
        // recursive call of fastInterceptFinder2d, until currentRecursion == m_param_maxRecursionLevel
        if (currentRecursion < m_param_maxRecursionLevel) {
          fastInterceptFinder2d(containedHits, left, right, lowerIndex, upperIndex, currentRecursion + 1);
        } else {
          m_activeSectors.insert({std::make_pair(localIndexX, localIndexY), std::make_pair(-layerFilter(layerHits), containedHits) });
        }
      }
    }
  }
}


void FastInterceptFinder2D::FindHoughSpaceCluster()
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
    for (HitDataCache* hit : currentCell.second.second) {
      m_currentTrackCandidate.emplace_back(hit);
    }

    // Check for HS sectors connected to each other which could form a cluster
    DepthFirstSearch(currentCell.first.first, currentCell.first.second);
    // if cluster valid (i.e. not too small and not too big): finalize!
    if (m_clusterSize >= m_param_MinimumHSClusterSize and m_clusterSize <= m_param_MaximumHSClusterSize) {

      m_trackCandidates.emplace_back(m_currentTrackCandidate);
      if (m_currentTrackCandidate.size() > 200) {
//         m_breakFlag = true;
        gnuplotoutput(m_currentTrackCandidate);
      }
      m_currentTrackCandidate.clear();
    }
    m_clusterCount++;
  }
}

void FastInterceptFinder2D::DepthFirstSearch(uint lastIndexX, uint lastIndexY)
{
  if (m_clusterSize >= m_param_MaximumHSClusterSize) return;

  for (uint currentIndexY = lastIndexY; currentIndexY >= lastIndexY - 1; currentIndexY--) {
    if (abs((int)m_clusterInitialPosition.second - (int)currentIndexY) >= m_param_MaximumHSClusterSizeY or
        m_clusterSize >= m_param_MaximumHSClusterSize or currentIndexY > m_param_nVerticalSectors) return;
    for (uint currentIndexX = lastIndexX; currentIndexX <= lastIndexX + 1; currentIndexX++) {
      if (abs((int)m_clusterInitialPosition.first - (int)currentIndexX) >= m_param_MaximumHSClusterSizeX or
          m_clusterSize >= m_param_MaximumHSClusterSize or currentIndexX > m_param_nAngleSectors) return;

      // The cell (currentIndexX, currentIndexY) is the current one has already been checked, so continue
      if (lastIndexX == currentIndexX && lastIndexY == currentIndexY) continue;

      // first check bounds to avoid out-of-bound array access
      // as they are uints, they are always >= 0, and in case of an overflow they would be too large
      if (currentIndexX < m_param_nAngleSectors and currentIndexY < m_param_nVerticalSectors) {

        auto activeSector = m_activeSectors.find({currentIndexX, currentIndexY});
        // Only continue searching if the current cluster is smaller than the maximum cluster size
        if (activeSector != m_activeSectors.end() and activeSector->second.first < 0 /*and m_clusterSize < m_param_MaximumHSClusterSize*/) {
          activeSector->second.first = m_clusterCount;
          m_clusterSize++;

          // No need to check whether currentIndex exists as a key in m_activeSectors as they were created at the same time
          // so it's certain the key exists.
          for (HitDataCache* hit : activeSector->second.second) {
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


void FastInterceptFinder2D::gnuplotoutput(const std::vector<HitDataCache*>& hits)
{
  std::ofstream outstream;
  outstream.open("gnuplotlog.plt", std::ios::trunc);
  outstream << "set style line 3 lt rgb 'black' lw 1 pt 6" << std::endl;
  outstream << "set style line 4 lt rgb 'blue' lw 1 pt 6" << std::endl;
  outstream << "set style line 5 lt rgb 'green' lw 1 pt 6" << std::endl;
  outstream << "set style line 6 lt rgb 'red' lw 1 pt 6" << std::endl;
  outstream << std::endl;

  uint count = 0;
  for (auto& hit : hits) {
    double xc = hit->xConformal;
    double yc = hit->yConformal;
    VxdID id = hit->sensorID;
    int layer = id.getLayerNumber();
    double X = hit->spacePoint->X();
    double Y = hit->spacePoint->Y();
    double Z = hit->spacePoint->Z();

    outstream << "plot " << xc << " * -sin(x) + " << yc << " * cos(x) > 0 ? " << xc << " * cos(x) + " << yc <<
              " * sin(x) : 1/0 notitle linestyle " << layer << " # " << id << "    " << X << "   " << Y << "   " << Z << std::endl;
    if (count < hits.size() - 1) outstream << "re";
    count++;
  }

  outstream << std::endl << "pause -1" << std::endl;
  outstream.close();
}
