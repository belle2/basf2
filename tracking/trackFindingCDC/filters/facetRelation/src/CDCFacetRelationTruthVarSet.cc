/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/CDCFacetRelationTruthVarSet.h>
#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCFacetRelationTruthVarSet::CDCFacetRelationTruthVarSet(const std::string& prefix) :
  VarSet<CDCFacetRelationTruthVarNames>(prefix)
{
}

bool CDCFacetRelationTruthVarSet::extract(const Relation<const CDCFacet>* ptrFacetRelation)
{
  const CDCFacet* ptrFromFacet(ptrFacetRelation->getFrom());
  const CDCFacet* ptrToFacet(ptrFacetRelation->getTo());

  const CDCFacet& fromFacet = *ptrFromFacet;
  const CDCFacet& toFacet = *ptrToFacet;

  // Despite of that two facets are neighbors if both are true facets
  // That also implies the correct tof alignment of the hits not common to both facets
  Weight fromFacetWeight = m_mcFacetFilter(fromFacet);
  Weight toFacetWeight = m_mcFacetFilter(toFacet);

  bool mcDecision = (not std::isnan(fromFacetWeight)) and (not std::isnan(toFacetWeight));
  var<named("truth")>() = mcDecision;
  return true;
}

void CDCFacetRelationTruthVarSet::initialize()
{
  m_mcFacetFilter.initialize();
}

void CDCFacetRelationTruthVarSet::terminate()
{
  m_mcFacetFilter.terminate();
}
