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

#include <tracking/trackFindingCDC/eventdata/utils/GenfitUtil.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/GetIterator.h>
#include <genfit/TrackCand.h>


#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/indirected.hpp>
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
                             [](const CDCRLWireHit & firstRLWireHit,
    const CDCRLWireHit & secondRLWireHit) {
      return CDCTangent(firstRLWireHit, secondRLWireHit);
    });

    if (tangentSegment.size() + 1 != rlWireHitSegment.size()) {
      B2ERROR("Wrong number of tangents created.");
    }

  }

  template<class ATangentRange>
  CDCRecoSegment2D condenseTangentSegment(const ATangentRange& tangentSegment)
  {
    using TangentIt = GetIterator<const ATangentRange&>;
    TangentIt tangentIt{tangentSegment.begin()};
    TangentIt endTangentIt{tangentSegment.end()};
    int nTangents = std::distance(tangentIt, endTangentIt);

    CDCRecoSegment2D result;
    result.reserve(nTangents + 1);

    if (nTangents == 0) {
      //pass
    } else if (nTangents == 1) {
      // Only one tangent no averaging necesssary
      result.push_back(tangentIt->getFromRecoHit2D());
      result.push_back(tangentIt->getToRecoHit2D());

    } else { // nTangents > 2
      TangentIt firstTangentIt = tangentIt++;
      TangentIt secondTangentIt = tangentIt++;

      result.push_back(firstTangentIt->getFromRecoHit2D());

      while (tangentIt != endTangentIt) {

        firstTangentIt = secondTangentIt; // tangentSegment[iTangent];
        secondTangentIt = tangentIt++; // tangentSegment[iTangent+1];

        result.push_back(CDCRecoHit2D::average(firstTangentIt->getToRecoHit2D(),
                                               secondTangentIt->getFromRecoHit2D()));
      }

      result.push_back(secondTangentIt->getToRecoHit2D());

    }

    result.receiveISuperCluster();
    return result;
  }

  template<class AFacetRange>
  CDCRecoSegment2D condenseFacetSegment(const AFacetRange& facetSegment)
  {
    using FacetIt = GetIterator<const AFacetRange&>;
    FacetIt facetIt = facetSegment.begin();
    FacetIt endFacetIt = facetSegment.end();
    int nFacets = std::distance(facetIt, endFacetIt);

    CDCRecoSegment2D result;
    result.reserve(nFacets + 2);

    if (nFacets == 0) {
      //pass
    } else if (nFacets == 1) {
      FacetIt onlyFacetIt = facetIt;
      result.push_back(onlyFacetIt->getStartRecoHit2D());
      result.push_back(onlyFacetIt->getMiddleRecoHit2D());
      result.push_back(onlyFacetIt->getEndRecoHit2D());

    } else if (nFacets == 2) {
      FacetIt firstFacetIt = facetIt++;
      FacetIt secondFacetIt = facetIt;

      result.push_back(firstFacetIt->getStartRecoHit2D());
      result.push_back(CDCRecoHit2D::average(secondFacetIt->getStartRecoHit2D() ,
                                             firstFacetIt->getMiddleRecoHit2D()));

      result.push_back(CDCRecoHit2D::average(secondFacetIt->getMiddleRecoHit2D(),
                                             firstFacetIt->getEndRecoHit2D()));

      result.push_back(secondFacetIt->getEndRecoHit2D());

    } else { // nFacets > 2
      FacetIt firstFacetIt = facetIt++;   // facetSegment[0];
      FacetIt secondFacetIt = facetIt++;  // facetSegment[1];
      FacetIt thirdFacetIt = facetIt++;  // facetSegment[2];

      result.push_back(firstFacetIt->getStartRecoHit2D());

      result.push_back(CDCRecoHit2D::average(firstFacetIt->getMiddleRecoHit2D(),
                                             secondFacetIt->getStartRecoHit2D()));

      result.push_back(CDCRecoHit2D::average(firstFacetIt->getEndRecoHit2D(),
                                             secondFacetIt->getMiddleRecoHit2D(),
                                             thirdFacetIt->getStartRecoHit2D()));

      while (facetIt != endFacetIt) {

        firstFacetIt = secondFacetIt;                   // facetSegment[iFacet];
        secondFacetIt = thirdFacetIt;                   // facetSegment[iFacet+1];
        thirdFacetIt = facetIt++;                     // facetSegment[iFacet+2];

        result.push_back(CDCRecoHit2D::average(firstFacetIt->getEndRecoHit2D(),
                                               secondFacetIt->getMiddleRecoHit2D(),
                                               thirdFacetIt->getStartRecoHit2D()));
      }

      result.push_back(CDCRecoHit2D::average(secondFacetIt->getEndRecoHit2D(),
                                             thirdFacetIt->getMiddleRecoHit2D()));

      result.push_back(thirdFacetIt->getEndRecoHit2D());
    }

    result.receiveISuperCluster();
    return result;
  }

}

CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCTangentSegment& tangentSegment)
{
  const std::vector<CDCTangent>& tangents = tangentSegment;
  CDCRecoSegment2D segment2D = ::condenseTangentSegment(tangents);
  segment2D.setTrajectory2D(tangentSegment.getTrajectory2D());
  segment2D.setAliasScore(tangentSegment.getAliasScore());
  return segment2D;
}

CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCTangent* >& tangentPath)
{
  return ::condenseTangentSegment(tangentPath | boost::adaptors::indirected);
}

CDCRecoSegment2D CDCRecoSegment2D::condense(const CDCFacetSegment& facetSegment)
{
  const std::vector<CDCFacet>& facets = facetSegment;
  CDCRecoSegment2D segment2D = ::condenseFacetSegment(facets);
  segment2D.setTrajectory2D(facetSegment.getTrajectory2D());
  segment2D.setAliasScore(facetSegment.getAliasScore());
  return segment2D;
}

CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCFacet* >& facetPath)
{
  return ::condenseFacetSegment(facetPath | boost::adaptors::indirected);
}

CDCRecoSegment2D CDCRecoSegment2D::condense(const std::vector<const CDCRecoSegment2D*>& segmentPath)
{
  CDCRecoSegment2D result;
  double aliasScore = 0;
  for (const CDCRecoSegment2D* ptrSegment2D : segmentPath) {
    assert(ptrSegment2D);
    const CDCRecoSegment2D& segment2D = *ptrSegment2D;
    for (const CDCRecoHit2D& recoHit2D : segment2D) {
      result.push_back(recoHit2D);
    }
    aliasScore = aliasScore + segment2D.getAliasScore();
  }
  result.receiveISuperCluster();
  result.setAliasScore(aliasScore);
  return result;
}

CDCRecoSegment2D CDCRecoSegment2D::reconstructUsingTangents(const CDCRLWireHitSegment& rlWireHitSegment)
{
  if (rlWireHitSegment.size() == 1) {
    CDCRecoSegment2D segment2D;
    Vector2D zeroDisp2D(0.0, 0.0);
    segment2D.emplace_back(rlWireHitSegment.front(), zeroDisp2D);
    segment2D.setTrajectory2D(rlWireHitSegment.getTrajectory2D());
    segment2D.setAliasScore(rlWireHitSegment.getAliasScore());
    return segment2D;
  } else {
    CDCTangentSegment tangentSegment;
    createTangentSegment(rlWireHitSegment, tangentSegment);
    tangentSegment.setTrajectory2D(rlWireHitSegment.getTrajectory2D());
    tangentSegment.setAliasScore(rlWireHitSegment.getAliasScore());
    return condense(tangentSegment);
  }
}

CDCRecoSegment2D CDCRecoSegment2D::reconstructUsingFacets(const CDCRLWireHitSegment& rlWireHitSegment)
{
  if (rlWireHitSegment.size() < 3) {
    return reconstructUsingTangents(rlWireHitSegment);
  } else {
    CDCFacetSegment facetSegment = CDCFacetSegment::create(rlWireHitSegment);
    return condense(facetSegment);
  }
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
  wireHitSegment.setTrajectory2D(getTrajectory2D());
  wireHitSegment.setAliasScore(getAliasScore());
  return wireHitSegment;
}

CDCRecoSegment2D CDCRecoSegment2D::getAlias() const
{
  CDCRecoSegment2D segment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    segment.push_back(recoHit2D.getAlias());
  }
  segment.setAliasScore(getAliasScore());
  return segment;
}


CDCRLWireHitSegment CDCRecoSegment2D::getRLWireHitSegment() const
{
  CDCRLWireHitSegment rlWireHitSegment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    rlWireHitSegment.push_back(recoHit2D.getRLWireHit());
  }
  rlWireHitSegment.setTrajectory2D(getTrajectory2D());
  rlWireHitSegment.setAliasScore(getAliasScore());
  return rlWireHitSegment;
}


bool CDCRecoSegment2D::fillInto(genfit::TrackCand& gfTrackCand) const
{
  CDCTrajectory3D trajectory3D(getTrajectory2D());

  if (not empty()) {
    const CDCRecoHit2D& startRecoHit2D = front();
    Vector3D startPos3D(startRecoHit2D.getRecoPos2D(), 0.0);
    trajectory3D.setLocalOrigin(startPos3D);
  }

  GenfitUtil::fill(gfTrackCand, *this);
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
  reverseSegment.setAliasScore(getAliasScore());
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

bool CDCRecoSegment2D::isFullyTaken(unsigned int maxNotTaken) const
{
  unsigned int notTakenCounter = 0;
  for (const CDCRecoHit2D& recoHit : items()) {
    if (not recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      notTakenCounter++;
      if (notTakenCounter > maxNotTaken) {
        return false;
      }
    }
  }

  return true;
}
