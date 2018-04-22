/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentPair/FitSegmentPairVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/geometry/HelixParameters.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

FitSegmentPairVarSet::FitSegmentPairVarSet(bool preliminaryFit)
  : Super()
  , m_preliminaryFit(preliminaryFit)
{
}

bool FitSegmentPairVarSet::extract(const CDCSegmentPair* ptrSegmentPair)
{
  if (not ptrSegmentPair) return false;

  const CDCSegmentPair& segmentPair = *ptrSegmentPair;

  CDCAxialStereoFusion axialStereoFusion;
  if (m_preliminaryFit) {
    axialStereoFusion.fusePreliminary(segmentPair);
  } else {
    axialStereoFusion.reconstructFuseTrajectories(segmentPair);
  }
  CDCTrajectory3D fit = segmentPair.getTrajectory3D();
  Vector3D support3D = fit.getSupport();

  finitevar<named("ndf")>() = fit.isFitted() ? fit.getNDF() : NAN;
  finitevar<named("chi2")>() = fit.getChi2();
  finitevar<named("p_value")>() = fit.getPValue();

  finitevar<named("curv")>() = fit.isFitted() ? fit.getCurvatureXY() : NAN;
  finitevar<named("z0")>() = support3D.z();
  finitevar<named("tanl")>() = fit.getTanLambda();

  using namespace NHelixParameterIndices;
  finitevar<named("curv_var")>() = fit.getLocalVariance(c_Curv);
  finitevar<named("z0_var")>() = fit.getLocalVariance(c_Z0);
  finitevar<named("tanl_var")>() = fit.getLocalVariance(c_TanL);

  return true;
}

std::vector<Named<Float_t*> > FitSegmentPairVarSet::getNamedVariables(const std::string& prefix)
{
  return m_preliminaryFit ?
         Super::getNamedVariables(prefix + "pre_") :
         Super::getNamedVariables(prefix);
}
