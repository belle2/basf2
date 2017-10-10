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

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <genfit/Exception.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template <>
genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                                  const SpacePoint& spacePoint) const
{
  if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::SVD) {
    // We always use the first cluster here to create the plane. Should not make much difference?
    return getPlane(measuredStateOnPlane, *(spacePoint.getRelated<SVDCluster>()));
  } else if (spacePoint.getType() == VXD::SensorInfoBase::SensorType::PXD) {
    return getPlane(measuredStateOnPlane, *(spacePoint.getRelated<PXDCluster>()));
  } else {
    B2FATAL("Can not extrapolate unknown type " << spacePoint.getType() << "!");
  }
}

template <>
genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, SVDCluster& svdCluster) const
{
  SVDRecoHit recoHit(&svdCluster);
  return getPlane(measuredStateOnPlane, recoHit);
}

template <>
genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane, PXDCluster& pxdCluster) const
{
  PXDRecoHit recoHit(&pxdCluster);
  return getPlane(measuredStateOnPlane, recoHit);
}

template <>
genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                                                  const TrackFindingCDC::CDCRLWireHit& rlWireHit) const
{
  CDCRecoHit recoHit(rlWireHit.getHit(), nullptr);
  // TODO: need to set RL here properly
  // recoHit.setLeftRightResolution();
  return getPlane(measuredStateOnPlane, recoHit);
}

template <>
genfit::SharedPlanePtr AdvanceAlgorithm::getPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane __attribute__((unused)),
                                                  const genfit::MeasuredStateOnPlane& plane) const
{
  return plane.getPlane();
}

/// Expose the useMaterialEffects parameter.
void AdvanceAlgorithm::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useMaterialEffects"), m_param_useMaterialEffects,
                                "Use material effects during extrapolation.", m_param_useMaterialEffects);
}
