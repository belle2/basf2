/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/RecoSegmentCreator.h"

#include <boost/foreach.hpp>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;





RecoSegmentCreator::RecoSegmentCreator()
{
}





RecoSegmentCreator::~RecoSegmentCreator()
{
}





void RecoSegmentCreator::create(
  const std::vector< std::vector<const CDCRecoFacet* > >& facetPaths,
  std::vector<CDCRecoSegment2D>& recoSegments
) const
{

  recoSegments.reserve(recoSegments.size() + facetPaths.size());

  for (const std::vector<const CDCRecoFacet* >& facetPath : facetPaths) {

    //create a new recoSegment2D
    recoSegments.push_back(CDCRecoSegment2D());
    CDCRecoSegment2D& recoSegment = recoSegments.back();

    create(facetPath, recoSegment);

  }
}





void RecoSegmentCreator::create(
  const std::vector<const CDCRecoFacet* >& facetPath,
  CDCRecoSegment2D& recoSegment
) const
{

  size_t nFacets = facetPath.size();

  recoSegment.reserve(nFacets + 2);

  if (nFacets == 0) return;

  else if (nFacets == 1) {
    const CDCRecoFacet* onlyFacet = *(facetPath.begin());
    recoSegment.push_back(onlyFacet->getStartRecoHit2D());
    recoSegment.push_back(onlyFacet->getMiddleRecoHit2D());
    recoSegment.push_back(onlyFacet->getEndRecoHit2D());

  } else if (nFacets == 2) {
    vector<const CDCRecoFacet*>::const_iterator itFacets = facetPath.begin();
    const CDCRecoFacet* firstFacet = *itFacets++;
    const CDCRecoFacet* secondFacet =  *itFacets;

    recoSegment.push_back(firstFacet->getStartRecoHit2D());
    recoSegment.push_back(CDCRecoHit2D::average(secondFacet->getStartRecoHit2D() ,
                                                firstFacet->getMiddleRecoHit2D()));

    recoSegment.push_back(CDCRecoHit2D::average(secondFacet->getMiddleRecoHit2D(),
                                                firstFacet->getEndRecoHit2D()));

    recoSegment.push_back(secondFacet->getEndRecoHit2D());


  } else { // nFacets > 2
    vector<const CDCRecoFacet*>::const_iterator itFacets = facetPath.begin();
    const CDCRecoFacet* firstFacet  = *itFacets++;  // facetPath[0];
    const CDCRecoFacet* secondFacet = *itFacets++;  // facetPath[1];
    const CDCRecoFacet* thirdFacet  = *itFacets++;  // facetPath[2];

    recoSegment.push_back(firstFacet->getStartRecoHit2D());
    recoSegment.push_back(CDCRecoHit2D::average(firstFacet->getMiddleRecoHit2D(),
                                                secondFacet->getStartRecoHit2D()));
    recoSegment.push_back(CDCRecoHit2D::average(firstFacet->getEndRecoHit2D(),
                                                secondFacet->getMiddleRecoHit2D(),
                                                thirdFacet->getStartRecoHit2D()));

    while (itFacets != facetPath.end()) {

      firstFacet = secondFacet; //facetPath[iFacet];
      secondFacet = thirdFacet; //facetPath[iFacet+1];
      thirdFacet = *itFacets++; //facetPath[iFacet+2];

      recoSegment.push_back(CDCRecoHit2D::average(firstFacet->getEndRecoHit2D(),
                                                  secondFacet->getMiddleRecoHit2D(),
                                                  thirdFacet->getStartRecoHit2D()));

      // if ( isNAN(recoSegment.back().getRefDisp2D().x()) or isNAN(recoSegment.back().getRefDisp2D().y()) ){
      //  cout << recoSegment.back().getRefDisp2D() << endl;

      //  cout << firstFacet->getEndRecoHit2D() << endl;
      //  cout << firstFacet->getEndRefPos2D() << endl;
      //  cout << firstFacet->getStartToEndLine() << endl;
      //  cout << firstFacet->getMiddleToEndLine() << endl;

      //  cout << secondFacet->getMiddleRecoHit2D() << endl;
      //  cout << secondFacet->getMiddleRefPos2D() << endl;
      //  cout << secondFacet->getStartToMiddleLine() << endl;
      //  cout << secondFacet->getMiddleToEndLine() << endl;


      //  cout << thirdFacet->getStartRecoHit2D() << endl;
      //  cout << thirdFacet->getStartRefPos2D() << endl;
      //  cout << thirdFacet->getStartToMiddleLine() << endl;
      //  cout << thirdFacet->getStartToEndLine() << endl;


      // }

    }

    recoSegment.push_back(CDCRecoHit2D::average(secondFacet->getEndRecoHit2D(),
                                                thirdFacet->getMiddleRecoHit2D()));
    recoSegment.push_back(thirdFacet->getEndRecoHit2D());

  }

}


