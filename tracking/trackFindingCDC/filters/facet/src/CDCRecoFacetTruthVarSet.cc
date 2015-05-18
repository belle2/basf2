/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/CDCRecoFacetTruthVarSet.h"


#include <assert.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRecoFacetTruthVarSet::CDCRecoFacetTruthVarSet(const std::string& prefix) :
  VarSet<CDCRecoFacetTruthVarNames>(prefix)
{
}

bool CDCRecoFacetTruthVarSet::extract(const CDCRecoFacet* ptrFacet)
{
  bool extracted = extractNested(ptrFacet);
  if (not extracted or not ptrFacet) return false;
  const CDCRecoFacet& facet = *ptrFacet;

  const CellWeight mcWeight = m_mcFacetFilter(facet);
  var<named("truth")>() =  not isNotACell(mcWeight);
  return true;
}

void CDCRecoFacetTruthVarSet::initialize()
{
  m_mcFacetFilter.initialize();
}

void CDCRecoFacetTruthVarSet::terminate()
{
  m_mcFacetFilter.terminate();
}
