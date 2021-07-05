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
TVector3 VXDMomentumEstimationTools<PXDCluster>::getEntryMomentumOfMCParticle(const PXDCluster& cluster) const
{
  PXDTrueHit* trueHit = cluster.getRelated<PXDTrueHit>("PXDTrueHits");
  if (trueHit == nullptr) {
    return TVector3();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const TVector3& momentum = sensorInfoBase.vectorToGlobal(trueHit->getEntryMomentum(), true);

    return momentum;
  }
}

/** We have to handle PXD and SVD differently here */
template <>
TVector3 VXDMomentumEstimationTools<SVDCluster>::getEntryMomentumOfMCParticle(const SVDCluster& cluster) const
{
  SVDTrueHit* trueHit = cluster.getRelated<SVDTrueHit>("SVDTrueHits");
  if (trueHit == nullptr) {
    return TVector3();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const TVector3& momentum = sensorInfoBase.vectorToGlobal(trueHit->getEntryMomentum(), true);

    return momentum;
  }
}

/** We have to handle PXD and SVD differently here */
template <>
TVector3 VXDMomentumEstimationTools<PXDCluster>::getEntryPositionOfMCParticle(const PXDCluster& cluster) const
{
  PXDTrueHit* trueHit = cluster.getRelated<PXDTrueHit>("PXDTrueHits");
  if (trueHit == nullptr) {
    return TVector3();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const TVector3& momentum = sensorInfoBase.pointToGlobal(TVector3(trueHit->getEntryU(), trueHit->getEntryV(), trueHit->getEntryW()),
                                                            true);

    return momentum;
  }
}

/** We have to handle PXD and SVD differently here */
template <>
TVector3 VXDMomentumEstimationTools<SVDCluster>::getEntryPositionOfMCParticle(const SVDCluster& cluster) const
{
  PXDTrueHit* trueHit = cluster.getRelated<PXDTrueHit>("SVDTrueHits");
  if (trueHit == nullptr) {
    return TVector3();
  } else {
    const VxdID& vxdID = cluster.getSensorID();
    const VXD::SensorInfoBase& sensorInfoBase = VXD::GeoCache::getInstance().getSensorInfo(vxdID);
    const TVector3& momentum = sensorInfoBase.pointToGlobal(TVector3(trueHit->getEntryU(), trueHit->getEntryV(), trueHit->getEntryW()),
                                                            true);

    return momentum;
  }
}
