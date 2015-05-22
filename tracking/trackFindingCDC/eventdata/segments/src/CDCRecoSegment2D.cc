/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/eventdata/collections/FillGenfitTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;




namespace {
  void createTangentSegment(const CDCRLWireHitSegment& rlWireHitSegment,
                            CDCTangentSegment& tangentSegment)
  {
    size_t nRLWireHits = rlWireHitSegment.size();
    if (nRLWireHits < 2) return;

    tangentSegment.reserve(nRLWireHits - 1);

    CDCRLWireHitSegment::const_iterator itRLWireHit = rlWireHitSegment.begin();

    /// Setup for the following shifting operations
    const CDCRLWireHit* firstRLWireHit = nullptr;         //recoRLWireHitSegment[-1];
    const CDCRLWireHit* secondRLWireHit = *itRLWireHit++; //recoRLWireHitSegment[0];

    const CDCRLWireHitSegment::const_iterator itEndRLWireHit = rlWireHitSegment.end();
    while (itRLWireHit != itEndRLWireHit) {

      firstRLWireHit = secondRLWireHit; //recoRLWireHitSegment[iRLWireHit];
      secondRLWireHit = *itRLWireHit++;  //recoRLWireHitSegment[iRLWireHit+1];

      tangentSegment.push_back(CDCTangent(firstRLWireHit, secondRLWireHit));
    }

    if (tangentSegment.size() + 1 != rlWireHitSegment.size()) B2ERROR("Wrong number of tangents created.");

  }



  void createFacetSegment(const CDCRLWireHitSegment& rlWireHitSegment,
                          CDCFacetSegment& facetSegment)
  {
    size_t nRLWireHits = rlWireHitSegment.size();
    if (nRLWireHits < 3) return;

    facetSegment.reserve(nRLWireHits - 2);

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

      facetSegment.push_back(CDCFacet(firstRLWireHit, secondRLWireHit, thirdRLWireHit));
    }

    if (facetSegment.size() + 2 != rlWireHitSegment.size()) B2ERROR("Wrong number of facets created.");

  }



  template<class MaybePtrTangent>
  CDCRecoSegment2D condenseTangentSegment(const std::vector<MaybePtrTangent>& tangentSegment)
  {
    CDCRecoSegment2D result;
    result.reserve(tangentSegment.size() + 1);

    size_t nTangents = tangentSegment.size();
    if (nTangents == 0) {
      //pass
    } else if (nTangents == 1) {
      const CDCTangent* tangent = tangentSegment.front();
      result.push_back(tangent->getFromRecoHit2D());
      result.push_back(tangent->getToRecoHit2D());

    } else { // nTangents > 2
      typename std::vector<MaybePtrTangent>::const_iterator itTangent = tangentSegment.begin();

      const CDCTangent* firstTangent = *itTangent++;  // tangentSegment[0];
      const CDCTangent* secondTangent = *itTangent++;  // tangentSegment[1];

      result.push_back(firstTangent->getFromRecoHit2D());

      while (itTangent != tangentSegment.end()) {

        firstTangent = secondTangent; // tangentSegment[iTangent];
        secondTangent = *itTangent++; // tangentSegment[iTangent+1];

        result.push_back(CDCRecoHit2D::average(firstTangent->getToRecoHit2D(),
                                               secondTangent->getFromRecoHit2D()));
      }

      result.push_back(secondTangent->getToRecoHit2D());

    }

    return result;

  }



  template<class MaybePtrFacet>
  CDCRecoSegment2D condenseFacetSegment(const std::vector<MaybePtrFacet>& facetSegment)
  {
    CDCRecoSegment2D result;
    size_t nFacets = facetSegment.size();
    result.reserve(nFacets + 2);

    if (nFacets == 0) {
      //pass
    } else if (nFacets == 1) {
      const CDCFacet* onlyFacet = facetSegment.front();
      result.push_back(onlyFacet->getStartRecoHit2D());
      result.push_back(onlyFacet->getMiddleRecoHit2D());
      result.push_back(onlyFacet->getEndRecoHit2D());

    } else if (nFacets == 2) {
      typename std::vector<MaybePtrFacet>::const_iterator itFacet = facetSegment.begin();
      const CDCFacet* firstFacet = *itFacet++;
      const CDCFacet* secondFacet = *itFacet;

      result.push_back(firstFacet->getStartRecoHit2D());
      result.push_back(CDCRecoHit2D::average(secondFacet->getStartRecoHit2D() ,
                                             firstFacet->getMiddleRecoHit2D()));

      result.push_back(CDCRecoHit2D::average(secondFacet->getMiddleRecoHit2D(),
                                             firstFacet->getEndRecoHit2D()));

      result.push_back(secondFacet->getEndRecoHit2D());

    } else { // nFacets > 2
      typename std::vector<MaybePtrFacet>::const_iterator itFacet = facetSegment.begin();
      const CDCFacet* firstFacet  = *itFacet++;  // facetSegment[0];
      const CDCFacet* secondFacet = *itFacet++;  // facetSegment[1];
      const CDCFacet* thirdFacet  = *itFacet++;  // facetSegment[2];

      result.push_back(firstFacet->getStartRecoHit2D());

      result.push_back(CDCRecoHit2D::average(firstFacet->getMiddleRecoHit2D(),
                                             secondFacet->getStartRecoHit2D()));

      result.push_back(CDCRecoHit2D::average(firstFacet->getEndRecoHit2D(),
                                             secondFacet->getMiddleRecoHit2D(),
                                             thirdFacet->getStartRecoHit2D()));

      while (itFacet != facetSegment.end()) {

        firstFacet = secondFacet;   //facetSegment[iFacet];
        secondFacet = thirdFacet;   //facetSegment[iFacet+1];
        thirdFacet = *itFacet++; //facetSegment[iFacet+2];

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





CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCTangentSegment& tangentSegment)
{
  return ::condenseTangentSegment(tangentSegment.items());
}


CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCTangent* >& tangentPath)
{
  return ::condenseTangentSegment(tangentPath);
}




CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCFacetSegment& facetSegment)
{
  return ::condenseFacetSegment(facetSegment.items());
}





CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCFacet* >& facetPath)
{
  return ::condenseFacetSegment(facetPath);
}





CDCRecoSegment2D CDCRecoSegment2D::reconstructUsingTangents(const CDCRLWireHitSegment& rlWireHitSegment)
{
  CDCTangentSegment tangentSegment;
  createTangentSegment(rlWireHitSegment, tangentSegment);
  return condense(tangentSegment);
}





CDCRecoSegment2D CDCRecoSegment2D::reconstructUsingFacets(const CDCRLWireHitSegment& rlWireHitSegment)
{
  CDCFacetSegment facetSegment;
  createFacetSegment(rlWireHitSegment, facetSegment);
  return condense(facetSegment);
}


bool CDCRecoSegment2D::fillInto(genfit::TrackCand& gfTrackCand) const
{
  CDCTrajectory3D trajectory3D(getTrajectory2D());

  if (not empty()) {
    const CDCRecoHit2D& startRecoHit2D = front();
    Vector3D startPos3D(startRecoHit2D.getRecoPos2D(), 0.0);
    trajectory3D.setLocalOrigin(startPos3D);
  }

  fillHitsInto(*this, gfTrackCand);
  return trajectory3D.fillInto(gfTrackCand);
}
