/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/base/AdvanceAlgorithm.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <genfit/Exception.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template <>
bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint& spacePoint) const
{
  if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::SVD) {
    // We always use the first cluster here to create the plane. Should not make much difference?
    return extrapolate(measuredStateOnPlane, *(spacePoint.getRelated<SVDCluster>()));
  } else if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::PXD) {
    return extrapolate(measuredStateOnPlane, *(spacePoint.getRelated<PXDCluster>()));
  } else {
    B2FATAL("Can not extrapolate unknown type " << spacePoint.getType() << "!");
  }
}

template <>
bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, SVDCluster& svdCluster) const
{
  SVDRecoHit recoHit(&svdCluster);
  return extrapolate(measuredStateOnPlane, recoHit);
}

template <>
bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, PXDCluster& pxdCluster) const
{
  PXDRecoHit recoHit(&pxdCluster);
  return extrapolate(measuredStateOnPlane, recoHit);
}

template <>
bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                   const TrackFindingCDC::CDCRLWireHit& rlWireHit) const
{
  // TODO: do something here
  return false;
}

template <>
bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                   const genfit::MeasuredStateOnPlane& plane) const
{
  return extrapolate(measuredStateOnPlane, plane.getPlane());
}

template <>
bool AdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const genfit::SharedPlanePtr& plane) const
{
  try {
    genfit::MaterialEffects::getInstance()->setNoEffects(not m_param_useMaterialEffects);
    measuredStateOnPlane.extrapolateToPlane(plane);
    genfit::MaterialEffects::getInstance()->setNoEffects(false);
  } catch (genfit::Exception e) {
    B2WARNING(e.what());
    return false;
  }

  return true;
}

/// Expose the useMaterialEffects parameter.
void AdvanceAlgorithm::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter("useMaterialEffects", m_param_useMaterialEffects,
                                "Use material effects during extrapolation.", m_param_useMaterialEffects);
}