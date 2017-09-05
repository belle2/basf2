/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/spacePoint/SpacePointMatcher.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Functor to extract the vxdID of a space point.
  struct HitIDGetter {
    operator FunctorTag();

    VxdID operator()(const SpacePoint& spacePoint) const
    {
      return spacePoint.getVxdID();
    }
  };

  bool isConnected(const VxdID& currentSensor, const VxdID& nextSensor)
  {
    const auto& sensorNumberDifference = static_cast<int>(currentSensor.getSensorNumber()) - static_cast<int>
                                         (nextSensor.getSensorNumber());
    if (abs(sensorNumberDifference) > 1) {
      return false;
    }
    auto& geoCache = VXD::GeoCache::getInstance();
    const auto& currentSensorInfo = geoCache.getSensorInfo(currentSensor);
    const auto& nextSensorInfo = geoCache.getSensorInfo(nextSensor);

    const auto& currentCenter = Vector3D(currentSensorInfo.pointToGlobal(TVector3(-0.5 * currentSensorInfo.getWidth(),
                                         -0.5 * currentSensorInfo.getLength(), 0))).xy();
    const auto& nextCenter = Vector3D(nextSensorInfo.pointToGlobal(TVector3(-0.5 * nextSensorInfo.getWidth(),
                                      -0.5 * nextSensorInfo.getLength(), 0))).xy();

    const auto& angle = std::acos(currentCenter.dot(nextCenter) / (currentCenter.norm() * nextCenter.norm()));
    return TMath::Pi() - angle > 2;
  }

  bool isConnected(const TVector3& position __attribute__((unused)), const VxdID& nextSensor __attribute__((unused)))
  {
    // TODO: It may be possible to include a much better condition here, then just returning everything from that layer
    return true;
  }
}

void SpacePointMatcher::initializeEventCache(std::vector<RecoTrack*>& seedsVector __attribute__((unused)),
                                             std::vector<const SpacePoint*>& filteredHitVector)
{
  m_cachedHitMap.clear();

  std::sort(filteredHitVector.begin(), filteredHitVector.end(), LessOf<MayIndirectTo<HitIDGetter>>());

  MayIndirectTo<HitIDGetter> hitIdGetter;
  const auto& groupedByID = adjacent_groupby(filteredHitVector.begin(), filteredHitVector.end(), hitIdGetter);
  for (const auto& group : groupedByID) {
    const auto& commonID = hitIdGetter(group.front());
    m_cachedHitMap.emplace(commonID, group);
  }
}

void SpacePointMatcher::fillInAllRanges(std::vector<RangeType>& ranges,
                                        unsigned short layer, unsigned short ladder)
{
  for (const auto& keyValuePair : m_cachedHitMap) {
    const auto& vxdID = keyValuePair.first;
    const auto& range = keyValuePair.second;
    if (vxdID.getLayerNumber() == layer and (ladder == 0 or vxdID.getLadderNumber() == ladder)) {
      ranges.push_back(range);
    }
  }
}

void SpacePointMatcher::fillInAllRanges(std::vector<RangeType>& ranges,
                                        unsigned short layer, const TVector3& position)
{
  for (const auto& keyValuePair : m_cachedHitMap) {
    const auto& vxdID = keyValuePair.first;
    const auto& range = keyValuePair.second;
    if (vxdID.getLayerNumber() == layer and isConnected(position, vxdID)) {
      ranges.push_back(range);
    }
  }
}

void SpacePointMatcher::beginRun()
{
  auto& geoCache = VXD::GeoCache::getInstance();
  const auto& layers = geoCache.getLayers();

  for (const auto& currentLayer : layers) {
    const auto& currentLayerNumber = currentLayer.getLayerNumber();
    if (currentLayerNumber <= 1) {
      continue;
    }
    auto nextLayer = currentLayer;
    nextLayer.setLayerNumber(static_cast<unsigned short>(currentLayerNumber - 1));

    const auto& currentLadders = geoCache.getLadders(currentLayer);
    const auto& nextLadders = geoCache.getLadders(nextLayer);

    for (const auto& currentLadder : currentLadders) {
      for (const auto& currentSensor : geoCache.getSensors(currentLadder)) {
        for (const auto& nextLadder : nextLadders) {
          for (const auto& nextSensor : geoCache.getSensors(nextLadder)) {
            if (isConnected(currentSensor, nextSensor)) {
              m_sensorMapping.emplace(currentSensor, nextSensor);
            }
          }
        }
      }
    }
  }
}