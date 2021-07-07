/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/BendFacetRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BendFacetRelationVarSet::extract(const Relation<const CDCFacet>* ptrFacetRelation)
{
  if (not ptrFacetRelation) return false;
  const Relation<const CDCFacet>& facetRelation = *ptrFacetRelation;

  const CDCFacet& fromFacet = *facetRelation.first;
  const CDCFacet& toFacet   = *facetRelation.second;

  const CDCRLWireHit& fromStartRLWireHit = fromFacet.getStartRLWireHit();
  const CDCRLWireHit& fromMiddleRLWireHit = fromFacet.getMiddleRLWireHit();
  const CDCRLWireHit& fromEndRLWireHit = fromFacet.getEndRLWireHit();

  const CDCRLWireHit& toStartRLWireHit = toFacet.getStartRLWireHit();
  const CDCRLWireHit& toMiddleRLWireHit = toFacet.getMiddleRLWireHit();
  const CDCRLWireHit& toEndRLWireHit = toFacet.getEndRLWireHit();

  const ParameterLine2D& fromStartToMiddle = fromFacet.getStartToMiddleLine();
  const ParameterLine2D& fromStartToEnd = fromFacet.getStartToEndLine();
  const ParameterLine2D& fromMiddleToEnd = fromFacet.getMiddleToEndLine();

  const ParameterLine2D& toStartToMiddle = toFacet.getStartToMiddleLine();
  const ParameterLine2D& toStartToEnd = toFacet.getStartToEndLine();
  const ParameterLine2D& toMiddleToEnd = toFacet.getMiddleToEndLine();

  const double fromStartVarL = fromStartRLWireHit.getRefDriftLengthVariance();
  const double fromMiddleVarL = fromMiddleRLWireHit.getRefDriftLengthVariance();
  const double fromEndVarL = fromEndRLWireHit.getRefDriftLengthVariance();

  const double toStartVarL = toStartRLWireHit.getRefDriftLengthVariance();
  const double toMiddleVarL = toMiddleRLWireHit.getRefDriftLengthVariance();
  const double toEndVarL = toEndRLWireHit.getRefDriftLengthVariance();

  // Lets call the four involved hits A, B, C and D.
  const double fromAB = fromStartToMiddle.tangential().norm();
  const double fromAC = fromStartToEnd.tangential().norm();
  const double fromBC = fromMiddleToEnd.tangential().norm();

  const double toBC = toStartToMiddle.tangential().norm();
  const double toBD = toStartToEnd.tangential().norm();
  const double toCD = toMiddleToEnd.tangential().norm();

  const double sAB = fromAB;
  const double sBC = (fromBC + toBC) / 2;
  const double sCD = toCD;

  const double sAC = fromAC;
  const double sBD = toBD;
  // const double sAC = sAB + sBC;
  // const double sBD = sBC + sCD;

  const double fromDeltaPhi =
    fromStartToMiddle.tangential().angleWith(fromStartToEnd.tangential());
  const double toDeltaPhi =
    toStartToMiddle.tangential().angleWith(toStartToEnd.tangential());

  const double deltaPhi = fromDeltaPhi - toDeltaPhi;

  const double fromCurv = 2 * fromDeltaPhi / sAC;
  const double toCurv = 2 * toDeltaPhi / sBD;
  const double deltaCurv = toCurv - fromCurv;

  const double dPhiA = 1 / sAB;
  const double dPhiB = 1 / sAB + 2 / sBC;
  const double dPhiC = 1 / sCD + 2 / sBC;
  const double dPhiD = 1 / sCD;

  const double dCurvA = 2 / sAC / sAB;
  const double dCurvB = 2 / sAC / sAB + (2 / sAC + 2 / sBD) / sBC;
  const double dCurvC = 2 / sBD / sCD + (2 / sAC + 2 / sBD) / sBC;
  const double dCurvD = 2 / sBD / sCD;

  const double varLA = fromStartVarL;
  const double varLB = (fromMiddleVarL + toStartVarL) / 2;
  const double varLC = (fromEndVarL + toMiddleVarL) / 2;
  const double varLD = toEndVarL;

  const double deltaPhiVar =
    varLA * dPhiA * dPhiA +
    varLB * dPhiB * dPhiB +
    varLC * dPhiC * dPhiC +
    varLD * dPhiD * dPhiD;

  const double deltaCurvVar =
    varLA * dCurvA * dCurvA +
    varLB * dCurvB * dCurvB +
    varLC * dCurvC * dCurvC +
    varLD * dCurvD * dCurvD;

  const double deltaCurvPull = deltaCurv / std::sqrt(deltaCurvVar);
  const double deltaPhiPull = deltaPhi / std::sqrt(deltaPhiVar);

  var<named("delta_phi")>() = std::fabs(deltaPhi);
  var<named("delta_phi_pull")>() = std::fabs(deltaPhiPull);
  var<named("delta_curv")>() = std::fabs(deltaCurv);
  var<named("delta_curv_pull")>() = std::fabs(deltaCurvPull);

  double r = (fromFacet.getMiddleRecoPos2D().norm() + fromFacet.getMiddleRecoPos2D().norm()) / 2;
  var<named("delta_phi_pull_per_r")>() = std::fabs(deltaPhiPull) / r;
  var<named("delta_curv_pull_per_r")>() = std::fabs(deltaCurvPull) / r;

  return true;
}
