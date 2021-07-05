/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/FitTrackRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool FitTrackRelationVarSet::extract(const Relation<const CDCTrack>* ptrTrackRelation)
{
  if (not ptrTrackRelation) return false;

  const CDCTrack* fromTrack = ptrTrackRelation->first;
  const CDCTrack* toTrack   = ptrTrackRelation->second;

  CDCTrajectory3D fromFit = fromTrack->getEndTrajectory3D();
  CDCTrajectory3D toFit   = toTrack->getStartTrajectory3D();

  Vector3D commonOrigin = fromFit.getSupport();

  fromFit.setLocalOrigin(commonOrigin);
  toFit.setLocalOrigin(commonOrigin);

  const UncertainHelix& fromHelix = fromFit.getLocalHelix();
  const UncertainHelix& toHelix = toFit.getLocalHelix();
  UncertainHelix commonHelix = UncertainHelix::average(fromHelix, toHelix);

  finitevar<named("is_fitted")>() = not commonHelix->isInvalid();
  finitevar<named("curv")>() = commonHelix->curvatureXY();
  finitevar<named("tanl")>() = commonHelix->tanLambda();
  finitevar<named("z0")>() = commonOrigin.z();

  using namespace NHelixParameterIndices;
  finitevar<named("curv_var")>() = commonHelix.variance(c_Curv);
  finitevar<named("tanl_var")>() = commonHelix.variance(c_TanL);
  finitevar<named("z0_var")>() = commonHelix.variance(c_Z0);

  finitevar<named("chi2")>() = std::fabs(commonHelix.chi2());
  finitevar<named("ndf")>() = commonHelix.ndf();
  finitevar<named("chi2_per_ndf")>() = std::fabs(commonHelix.chi2()) / commonHelix.ndf();
  finitevar<named("p_value")>() = TMath::Prob(commonHelix.chi2(), commonHelix.ndf());
  finitevar<named("p_value_cut")>() = TMath::Prob(commonHelix.chi2(), commonHelix.ndf()) < 0.02;
  return true;
}
