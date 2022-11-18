/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimationTools.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;

/** We only need a calibration for the PXD Clusters */
template<>
double VXDMomentumEstimationTools<PXDCluster>::getCalibration() const
{
  return 0.5658682413283911;
}

/** We have to handle PXD and SVD differently here */
template <>
ROOT::Math::XYZVector VXDMomentumEstimationTools<PXDCluster>::getEntryMomentumOfMCParticle(const PXDCluster& cluster) const
{
  PXDTrueHit* trueHit = cluster.getRelated<PXDTrueHit>("PXDTrueHits");
  if (trueHit == nullptr) {
    return ROOT::Math::XYZVector();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const ROOT::Math::XYZVector& momentum = sensorInfoBase.vectorToGlobal(trueHit->getEntryMomentum(), true);

    return momentum;
  }
}

/** We have to handle PXD and SVD differently here */
template <>
ROOT::Math::XYZVector VXDMomentumEstimationTools<SVDCluster>::getEntryMomentumOfMCParticle(const SVDCluster& cluster) const
{
  SVDTrueHit* trueHit = cluster.getRelated<SVDTrueHit>("SVDTrueHits");
  if (trueHit == nullptr) {
    return ROOT::Math::XYZVector();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const ROOT::Math::XYZVector& momentum = sensorInfoBase.vectorToGlobal(trueHit->getEntryMomentum(), true);

    return momentum;
  }
}

/** We have to handle PXD and SVD differently here */
template <>
ROOT::Math::XYZVector VXDMomentumEstimationTools<PXDCluster>::getEntryPositionOfMCParticle(const PXDCluster& cluster) const
{
  PXDTrueHit* trueHit = cluster.getRelated<PXDTrueHit>("PXDTrueHits");
  if (trueHit == nullptr) {
    return ROOT::Math::XYZVector();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const ROOT::Math::XYZVector& momentum =
      sensorInfoBase.pointToGlobal(ROOT::Math::XYZVector(trueHit->getEntryU(), trueHit->getEntryV(), trueHit->getEntryW()), true);

    return momentum;
  }
}

/** We have to handle PXD and SVD differently here */
template <>
ROOT::Math::XYZVector VXDMomentumEstimationTools<SVDCluster>::getEntryPositionOfMCParticle(const SVDCluster& cluster) const
{
  PXDTrueHit* trueHit = cluster.getRelated<PXDTrueHit>("SVDTrueHits");
  if (trueHit == nullptr) {
    return ROOT::Math::XYZVector();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const ROOT::Math::XYZVector& momentum =
      sensorInfoBase.pointToGlobal(ROOT::Math::XYZVector(trueHit->getEntryU(), trueHit->getEntryV(), trueHit->getEntryW()), true);

    return momentum;
  }
}
