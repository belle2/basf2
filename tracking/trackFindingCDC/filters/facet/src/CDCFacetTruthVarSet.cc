/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/CDCFacetTruthVarSet.h>


#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCFacetTruthVarSet::CDCFacetTruthVarSet(const std::string& prefix) :
  VarSet<CDCFacetTruthVarNames>(prefix)
{
}

bool CDCFacetTruthVarSet::extract(const CDCFacet* ptrFacet)
{
  bool extracted = extractNested(ptrFacet);
  if (not extracted or not ptrFacet) return false;
  const CDCFacet& facet = *ptrFacet;

  const Weight mcWeight = m_mcFacetFilter(facet);
  var<named("truth")>() =  not std::isnan(mcWeight);
  return true;
}

void CDCFacetTruthVarSet::initialize()
{
  m_mcFacetFilter.initialize();
}

void CDCFacetTruthVarSet::terminate()
{
  m_mcFacetFilter.terminate();
}
