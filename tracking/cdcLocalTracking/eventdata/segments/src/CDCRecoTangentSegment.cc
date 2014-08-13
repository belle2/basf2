/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRecoTangentSegment.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRecoTangentSegment)



CDCRecoTangentSegment CDCRecoTangentSegment::condense(const std::vector<const CDCRecoFacet* >& recoFacetPath)
{
  CDCRecoTangentSegment recoTangentSegment;

  recoTangentSegment.reserve(recoFacetPath.size() * 3);

  for (const CDCRecoFacet * ptrFacet : recoFacetPath) {
    if (not ptrFacet) continue;

    const CDCRecoFacet& facet = *ptrFacet;

    //the alignement of the tangents does not play a major role here
    recoTangentSegment.push_back(facet.getStartToMiddle());
    recoTangentSegment.push_back(facet.getStartToEnd());
    recoTangentSegment.push_back(facet.getMiddleToEnd());

  }
  recoTangentSegment.ensureUnique();

  return recoTangentSegment;

}





