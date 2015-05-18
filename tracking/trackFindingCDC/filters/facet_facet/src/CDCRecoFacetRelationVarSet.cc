/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCRecoFacetRelationVarSet.h"
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRecoFacetRelationVarSet::CDCRecoFacetRelationVarSet(const std::string& prefix) :
  VarSet<CDCRecoFacetRelationVarNames>(prefix)
{
}

bool CDCRecoFacetRelationVarSet::extract(const Relation<CDCRecoFacet>* ptrFacetRelation)
{
  bool extracted = extractNested(ptrFacetRelation);
  if (not extracted or not ptrFacetRelation) return false;

  const CDCRecoFacet* ptrFromFacet = ptrFacetRelation->first;
  const CDCRecoFacet* ptrToFacet = ptrFacetRelation->second;

  const CDCRecoFacet& fromFacet = *ptrFromFacet;
  const CDCRecoFacet& toFacet = *ptrToFacet;


  const ParameterLine2D& fromStartToMiddle = fromFacet.getStartToMiddleLine();
  const ParameterLine2D& fromStartToEnd    = fromFacet.getStartToEndLine();

  const ParameterLine2D& toStartToEnd   = toFacet.getStartToEndLine();
  const ParameterLine2D& toMiddleToEnd  = toFacet.getMiddleToEndLine();

  const double fromMiddleCos = fromStartToMiddle.tangential().cosWith(toStartToEnd.tangential());
  const double toMiddleCos = fromStartToEnd.tangential().cosWith(toMiddleToEnd.tangential());

  const double fromMiddlePhi = acos(fromMiddleCos);
  const double toMiddlePhi = acos(toMiddleCos);

  var<named("from_middle_phi")>() = fromMiddlePhi;
  var<named("to_middle_phi")>() = toMiddlePhi;


  return true;
}
