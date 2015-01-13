/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRecoSegment2D.h"

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCRecoSegment2D)



namespace {
  void createRecoTangentSegment(const CDCRLWireHitSegment& rlWireHitSegment,
                                CDCRecoTangentSegment& recoTangentSegment)
  {
    size_t nRLWireHits = rlWireHitSegment.size();
    if (nRLWireHits < 2) return;

    recoTangentSegment.reserve(nRLWireHits - 1);

    CDCRLWireHitSegment::const_iterator itRLWireHit = rlWireHitSegment.begin();

    /// Setup for the following shifting operations
    const CDCRLWireHit* firstRLWireHit = nullptr;         //recoRLWireHitSegment[-1];
    const CDCRLWireHit* secondRLWireHit = *itRLWireHit++; //recoRLWireHitSegment[0];

    const CDCRLWireHitSegment::const_iterator itEndRLWireHit = rlWireHitSegment.end();
    while (itRLWireHit != itEndRLWireHit) {

      firstRLWireHit = secondRLWireHit; //recoRLWireHitSegment[iRLWireHit];
      secondRLWireHit = *itRLWireHit++;  //recoRLWireHitSegment[iRLWireHit+1];

      recoTangentSegment.push_back(CDCRecoTangent(firstRLWireHit, secondRLWireHit));
    }

    if (recoTangentSegment.size() + 1 != rlWireHitSegment.size()) B2ERROR("Wrong number of tangents created.");

  }



  void createRecoFacetSegment(const CDCRLWireHitSegment& rlWireHitSegment,
                              CDCRecoFacetSegment& recoFacetSegment)
  {
    size_t nRLWireHits = rlWireHitSegment.size();
    if (nRLWireHits < 3) return;

    recoFacetSegment.reserve(nRLWireHits - 2);

    CDCRLWireHitSegment::const_iterator itRLWireHit = rlWireHitSegment.begin();

    /// Setup for the following shifting operations
    const CDCRLWireHit* firstRLWireHit = nullptr;         //recoRLWireHitSegment[-1];
    const CDCRLWireHit* secondRLWireHit = *itRLWireHit++; //recoRLWireHitSegment[0];
    const CDCRLWireHit* thirdRLWireHit = *itRLWireHit++;  //recoRLWireHitSegment[1];

    const CDCRLWireHitSegment::const_iterator itEndRLWireHit = rlWireHitSegment.end();
    while (itRLWireHit != itEndRLWireHit) {

      firstRLWireHit = secondRLWireHit; //recoRLWireHitSegment[iRLWireHit];
      secondRLWireHit = thirdRLWireHit; //recoRLWireHitSegment[iRLWireHit+1];
      thirdRLWireHit = *itRLWireHit++;  //recoRLWireHitSegment[iRLWireHit+2];

      recoFacetSegment.push_back(CDCRecoFacet(firstRLWireHit, secondRLWireHit, thirdRLWireHit));
    }

    if (recoFacetSegment.size() + 2 != rlWireHitSegment.size()) B2ERROR("Wrong number of facets created.");

  }



  template<class MaybePtrRecoTangent>
  CDCRecoSegment2D condenseRecoTangentSegment(const std::vector<MaybePtrRecoTangent>& recoTangentSegment)
  {
    CDCRecoSegment2D result;
    result.reserve(recoTangentSegment.size() + 1);

    size_t nTangents = recoTangentSegment.size();
    if (nTangents == 0) {
      //pass
    } else if (nTangents == 1) {
      const CDCRecoTangent* recoTangent = recoTangentSegment.front();
      result.push_back(recoTangent->getFromRecoHit2D());
      result.push_back(recoTangent->getToRecoHit2D());

    } else { // nTangents > 2
      typename std::vector<MaybePtrRecoTangent>::const_iterator itTangent = recoTangentSegment.begin();

      const CDCRecoTangent* firstTangent = *itTangent++;  // tangentSegment[0];
      const CDCRecoTangent* secondTangent = *itTangent++;  // tangentSegment[1];

      result.push_back(firstTangent->getFromRecoHit2D());

      while (itTangent != recoTangentSegment.end()) {

        firstTangent = secondTangent; // tangentSegment[iTangent];
        secondTangent = *itTangent++; // tangentSegment[iTangent+1];

        result.push_back(CDCRecoHit2D::average(firstTangent->getToRecoHit2D(),
                                               secondTangent->getFromRecoHit2D()));
      }

      result.push_back(secondTangent->getToRecoHit2D());

    }

    return result;

  }



  template<class MaybePtrRecoFacet>
  CDCRecoSegment2D condenseRecoFacetSegment(const std::vector<MaybePtrRecoFacet>& recoFacetSegment)
  {
    CDCRecoSegment2D result;
    size_t nFacets = recoFacetSegment.size();
    result.reserve(nFacets + 2);

    if (nFacets == 0) {
      //pass
    } else if (nFacets == 1) {
      const CDCRecoFacet* onlyFacet = recoFacetSegment.front();
      result.push_back(onlyFacet->getStartRecoHit2D());
      result.push_back(onlyFacet->getMiddleRecoHit2D());
      result.push_back(onlyFacet->getEndRecoHit2D());

    } else if (nFacets == 2) {
      typename std::vector<MaybePtrRecoFacet>::const_iterator itFacet = recoFacetSegment.begin();
      const CDCRecoFacet* firstFacet = *itFacet++;
      const CDCRecoFacet* secondFacet = *itFacet;

      result.push_back(firstFacet->getStartRecoHit2D());
      result.push_back(CDCRecoHit2D::average(secondFacet->getStartRecoHit2D() ,
                                             firstFacet->getMiddleRecoHit2D()));

      result.push_back(CDCRecoHit2D::average(secondFacet->getMiddleRecoHit2D(),
                                             firstFacet->getEndRecoHit2D()));

      result.push_back(secondFacet->getEndRecoHit2D());

    } else { // nFacets > 2
      typename std::vector<MaybePtrRecoFacet>::const_iterator itFacet = recoFacetSegment.begin();
      const CDCRecoFacet* firstFacet  = *itFacet++;  // recoFacetSegment[0];
      const CDCRecoFacet* secondFacet = *itFacet++;  // recoFacetSegment[1];
      const CDCRecoFacet* thirdFacet  = *itFacet++;  // recoFacetSegment[2];

      result.push_back(firstFacet->getStartRecoHit2D());

      result.push_back(CDCRecoHit2D::average(firstFacet->getMiddleRecoHit2D(),
                                             secondFacet->getStartRecoHit2D()));

      result.push_back(CDCRecoHit2D::average(firstFacet->getEndRecoHit2D(),
                                             secondFacet->getMiddleRecoHit2D(),
                                             thirdFacet->getStartRecoHit2D()));

      while (itFacet != recoFacetSegment.end()) {

        firstFacet = secondFacet;   //recoFacetSegment[iFacet];
        secondFacet = thirdFacet;   //recoFacetSegment[iFacet+1];
        thirdFacet = *itFacet++; //recoFacetSegment[iFacet+2];

        result.push_back(CDCRecoHit2D::average(firstFacet->getEndRecoHit2D(),
                                               secondFacet->getMiddleRecoHit2D(),
                                               thirdFacet->getStartRecoHit2D()));
      }

      result.push_back(CDCRecoHit2D::average(secondFacet->getEndRecoHit2D(),
                                             thirdFacet->getMiddleRecoHit2D()));

      result.push_back(thirdFacet->getEndRecoHit2D());
    }

    return result;
  }


}





CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCRecoTangentSegment& recoTangentSegment)
{
  return ::condenseRecoTangentSegment(recoTangentSegment.items());
}


CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCRecoTangent* >& recoTangentPath)
{
  return ::condenseRecoTangentSegment(recoTangentPath);
}




CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCRecoFacetSegment& recoFacetSegment)
{
  return ::condenseRecoFacetSegment(recoFacetSegment.items());
}





CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCRecoFacet* >& recoFacetPath)
{
  return ::condenseRecoFacetSegment(recoFacetPath);
}





CDCRecoSegment2D CDCRecoSegment2D::reconstructUsingTangents(const CDCRLWireHitSegment& rlWireHitSegment)
{
  CDCRecoTangentSegment recoTangentSegment;
  createRecoTangentSegment(rlWireHitSegment, recoTangentSegment);
  return condense(recoTangentSegment);
}





CDCRecoSegment2D CDCRecoSegment2D::reconstructUsingFacets(const CDCRLWireHitSegment& rlWireHitSegment)
{
  CDCRecoFacetSegment recoFacetSegment;
  createRecoFacetSegment(rlWireHitSegment, recoFacetSegment);
  return condense(recoFacetSegment);
}
