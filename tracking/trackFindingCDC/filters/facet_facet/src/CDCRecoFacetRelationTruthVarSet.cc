/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCRecoFacetRelationTruthVarSet.h"
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRecoFacetRelationTruthVarSet::CDCRecoFacetRelationTruthVarSet(const std::string& prefix) :
  VarSet<CDCRecoFacetRelationTruthVarNames>(prefix)
{
}

bool CDCRecoFacetRelationTruthVarSet::extract(const Relation<CDCRecoFacet>* ptrFacetRelation)
{
  bool extracted = extractNested(ptrFacetRelation);
  if (not extracted or not ptrFacetRelation) return false;

  const CDCRecoFacet* ptrFromFacet = ptrFacetRelation->first;
  const CDCRecoFacet* ptrToFacet = ptrFacetRelation->second;

  const CDCRecoFacet& fromFacet = *ptrFromFacet;
  const CDCRecoFacet& toFacet = *ptrToFacet;

  // Despite of that two facets are neighbors if both are true facets
  // That also implies the correct tof alignment of the hits not common to both facets
  CellWeight fromFacetWeight = m_mcFacetFilter(fromFacet);
  CellWeight toFacetWeight = m_mcFacetFilter(toFacet);

  bool mcDecision = (not isNotACell(fromFacetWeight)) and (not isNotACell(toFacetWeight));
  var<named("truth")>() = mcDecision;
  return true;
}

void CDCRecoFacetRelationTruthVarSet::initialize()
{
  m_mcFacetFilter.initialize();
}

void CDCRecoFacetRelationTruthVarSet::terminate()
{
  m_mcFacetFilter.terminate();
}
