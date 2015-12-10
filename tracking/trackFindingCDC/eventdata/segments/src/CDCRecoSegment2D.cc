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

#include <tracking/trackFindingCDC/eventdata/segments/CDCFacetSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCTangentSegment.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitSegment.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <tracking/trackFindingCDC/eventdata/collections/FillGenfitTrack.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <genfit/TrackCand.h>


#include <boost/range/adaptor/reversed.hpp>
#include <numeric>
#include <iterator>

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

    // Make tangents from pairs of hits along the segment.
    transform_adjacent_pairs(rlWireHitSegment.begin(), rlWireHitSegment.end(),
                             back_inserter(tangentSegment),
                             [](const CDCRLWireHit * firstRLWireHit,
    const CDCRLWireHit * secondRLWireHit) {
      return CDCTangent(firstRLWireHit, secondRLWireHit);
    });

    if (tangentSegment.size() + 1 != rlWireHitSegment.size()) {
      B2ERROR("Wrong number of tangents created.");
    }

  }


  void createFacetSegment(const CDCRLWireHitSegment& rlWireHitSegment,
                          CDCFacetSegment& facetSegment)
  {
    size_t nRLWireHits = rlWireHitSegment.size();
    if (nRLWireHits < 3) return;

    facetSegment.reserve(nRLWireHits - 2);

    // Make tangents from pairs of hits along the segment.
    transform_adjacent_triples(rlWireHitSegment.begin(), rlWireHitSegment.end(),
                               back_inserter(facetSegment),
                               [](const CDCRLWireHit * firstRLWireHit,
                                  const CDCRLWireHit * secondRLWireHit,
    const CDCRLWireHit * thirdRLWireHit) {
      return CDCFacet(firstRLWireHit, secondRLWireHit, thirdRLWireHit);
    });

    if (facetSegment.size() + 2 != rlWireHitSegment.size()) {
      B2ERROR("Wrong number of facets created.");
    }

  }



  template<class AMaybePtrTangent>
  CDCRecoSegment2D condenseTangentSegment(const std::vector<AMaybePtrTangent>& tangentSegment)
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
      typename std::vector<AMaybePtrTangent>::const_iterator itTangent = tangentSegment.begin();

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

    result.receiveISuperCluster();
    return result;
  }



  template<class AMaybePtrFacet>
  CDCRecoSegment2D condenseFacetSegment(const std::vector<AMaybePtrFacet>& facetSegment)
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
      typename std::vector<AMaybePtrFacet>::const_iterator itFacet = facetSegment.begin();
      const CDCFacet* firstFacet = *itFacet++;
      const CDCFacet* secondFacet = *itFacet;

      result.push_back(firstFacet->getStartRecoHit2D());
      result.push_back(CDCRecoHit2D::average(secondFacet->getStartRecoHit2D() ,
                                             firstFacet->getMiddleRecoHit2D()));

      result.push_back(CDCRecoHit2D::average(secondFacet->getMiddleRecoHit2D(),
                                             firstFacet->getEndRecoHit2D()));

      result.push_back(secondFacet->getEndRecoHit2D());

    } else { // nFacets > 2
      typename std::vector<AMaybePtrFacet>::const_iterator itFacet = facetSegment.begin();
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

    result.receiveISuperCluster();
    return result;
  }

}





CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCTangentSegment& tangentSegment)
{
  const std::vector<CDCTangent>& tangents = tangentSegment;
  return ::condenseTangentSegment(tangents);
}


CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCTangent* >& tangentPath)
{
  return ::condenseTangentSegment(tangentPath);
}




CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCFacetSegment& facetSegment)
{
  const std::vector<CDCFacet>& facets = facetSegment;
  return ::condenseFacetSegment(facets);
}

CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCFacet* >& facetPath)
{
  return ::condenseFacetSegment(facetPath);
}

CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCRecoSegment2D*>& segmentPath)
{
  CDCRecoSegment2D result;
  for (const CDCRecoSegment2D* ptrSegment : segmentPath) {
    assert(ptrSegment);
    const CDCRecoSegment2D& segment = *ptrSegment;
    for (const CDCRecoHit2D& recoHit2D : segment) {
      result.push_back(recoHit2D);
    }
  }
  result.receiveISuperCluster();
  return result;
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

vector<const CDCWire*> CDCRecoSegment2D::getWireSegment() const
{
  std::vector<const CDCWire*> wireSegment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    wireSegment.push_back(&(recoHit2D.getWire()));
  }
  return wireSegment;
}


CDCWireHitSegment CDCRecoSegment2D::getWireHitSegment() const
{
  CDCWireHitSegment wireHitSegment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    wireHitSegment.push_back(&(recoHit2D.getWireHit()));
  }
  return wireHitSegment;
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

CDCRecoSegment2D CDCRecoSegment2D::reversed() const
{
  CDCRecoSegment2D reverseSegment;
  reverseSegment.reserve(size());
  for (const CDCRecoHit2D& recohit : boost::adaptors::reverse(*this)) {
    reverseSegment.push_back(recohit.reversed());
  }

  reverseSegment.setTrajectory2D(getTrajectory2D().reversed());
  reverseSegment.m_automatonCell = m_automatonCell;
  return reverseSegment;
}

void CDCRecoSegment2D::reverse()
{
  // Reverse the trajectory
  getTrajectory2D().reverse();

  // Reverse the left right passage hypotheses
  for (CDCRecoHit2D& recoHit2D : *this) {
    recoHit2D.reverse();
  }
  // Reverse the arrangement of hits.
  std::reverse(begin(), end());
}

void CDCRecoSegment2D::unsetAndForwardMaskedFlag() const
{
  getAutomatonCell().unsetMaskedFlag();
  for (const CDCRecoHit2D& recoHit2D : *this) {
    const CDCWireHit& wireHit = recoHit2D.getWireHit();
    wireHit.getAutomatonCell().unsetMaskedFlag();
  }
}

void CDCRecoSegment2D::setAndForwardMaskedFlag() const
{
  getAutomatonCell().setMaskedFlag();
  for (const CDCRecoHit2D& recoHit2D : *this) {
    const CDCWireHit& wireHit = recoHit2D.getWireHit();
    wireHit.getAutomatonCell().setMaskedFlag();
  }
}

void CDCRecoSegment2D::receiveMaskedFlag() const
{
  for (const CDCRecoHit2D& recoHit2D : *this) {
    const CDCWireHit& wireHit = recoHit2D.getWireHit();
    if (wireHit.getAutomatonCell().hasMaskedFlag()) {
      getAutomatonCell().setMaskedFlag();
      return;
    }
  }
}

bool CDCRecoSegment2D::isFullyTaken() const
{
  for (const CDCRecoHit2D& recoHit : items()) {
    if (not recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      return false;
    }
  }

  return true;
}
