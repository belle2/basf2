/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/TangentSegmentCreator.h"

#include <boost/foreach.hpp>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;





TangentSegmentCreator::TangentSegmentCreator()
{
}





TangentSegmentCreator::~TangentSegmentCreator()
{
}





void TangentSegmentCreator::create(
  const std::vector< std::vector<const CDCRecoFacet*> >& facetPaths,
  std::vector< CDCRecoTangentVector >& recoTangentSegments
) const
{

  recoTangentSegments.reserve(recoTangentSegments.size() + facetPaths.size());

  for (const std::vector<const CDCRecoFacet*>& facetPath : facetPaths) {

    //create a new recoSegment2D
    recoTangentSegments.push_back(CDCRecoTangentVector());
    CDCRecoTangentVector& recoTangentSegment = recoTangentSegments.back();

    create(facetPath, recoTangentSegment);

    recoTangentSegment.ensureUnique();

  }

}





void TangentSegmentCreator::create(
  const std::vector<const CDCRecoFacet*>& facetPath,
  CDCRecoTangentVector& recoTangentSegment
) const
{

  recoTangentSegment.reserve(facetPath.size() * 3);

  for (const CDCRecoFacet * ptrFacet : facetPath) {
    if (not ptrFacet) continue;

    const CDCRecoFacet& facet = *ptrFacet;

    //the alignement of the tangents does not play a major role here
    recoTangentSegment.push_back(facet.getStartToMiddle());
    recoTangentSegment.push_back(facet.getStartToEnd());
    recoTangentSegment.push_back(facet.getMiddleToEnd());

  }
}
