/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCFacetSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCTangentSegment.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitSegment.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/GetIterator.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <numeric>
#include <iterator>

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
  CDCSegment2D condenseTangentSegment(const ATangentRange& tangentSegment)
  {
    using TangentIt = GetIterator<const ATangentRange&>;
    TangentIt tangentIt{tangentSegment.begin()};
    TangentIt endTangentIt{tangentSegment.end()};
    int nTangents = std::distance(tangentIt, endTangentIt);

    CDCSegment2D result;
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
  CDCSegment2D condenseFacetSegment(const AFacetRange& facetSegment)
  {
    using FacetIt = GetIterator<const AFacetRange&>;
    FacetIt facetIt = facetSegment.begin();
    FacetIt endFacetIt = facetSegment.end();
    int nFacets = std::distance(facetIt, endFacetIt);

    CDCSegment2D result;
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

CDCSegment2D CDCSegment2D::condense(const CDCTangentSegment& tangentSegment)
{
  const std::vector<CDCTangent>& tangents = tangentSegment;
  CDCSegment2D segment2D = ::condenseTangentSegment(tangents);
  segment2D.setTrajectory2D(tangentSegment.getTrajectory2D());
  segment2D.receiveISuperCluster();
  segment2D.setAliasScore(tangentSegment.getAliasScore());
  return segment2D;
}

CDCSegment2D CDCSegment2D::condense(const std::vector<const CDCTangent* >& tangentPath)
{
  return ::condenseTangentSegment(tangentPath | boost::adaptors::indirected);
}

CDCSegment2D CDCSegment2D::condense(const CDCFacetSegment& facetSegment)
{
  const std::vector<CDCFacet>& facets = facetSegment;
  CDCSegment2D segment2D = ::condenseFacetSegment(facets);
  segment2D.setTrajectory2D(facetSegment.getTrajectory2D());
  segment2D.receiveISuperCluster();
  segment2D.setAliasScore(facetSegment.getAliasScore());
  return segment2D;
}

CDCSegment2D CDCSegment2D::condense(const std::vector<const CDCFacet* >& facetPath)
{
  return ::condenseFacetSegment(facetPath | boost::adaptors::indirected);
}

CDCSegment2D CDCSegment2D::condense(const std::vector<const CDCSegment2D*>& segmentPath)
{
  CDCSegment2D result;
  double aliasScore = 0;
  for (const CDCSegment2D* ptrSegment2D : segmentPath) {
    assert(ptrSegment2D);
    const CDCSegment2D& segment2D = *ptrSegment2D;
    for (const CDCRecoHit2D& recoHit2D : segment2D) {
      result.push_back(recoHit2D);
    }
    aliasScore = aliasScore + segment2D.getAliasScore();
  }
  result.receiveISuperCluster();
  result.setAliasScore(aliasScore);
  return result;
}

CDCSegment2D CDCSegment2D::reconstructUsingTangents(const CDCRLWireHitSegment& rlWireHitSegment)
{
  if (rlWireHitSegment.size() == 1) {
    CDCSegment2D segment2D;
    Vector2D zeroDisp2D(0.0, 0.0);
    segment2D.emplace_back(rlWireHitSegment.front(), zeroDisp2D);
    segment2D.setTrajectory2D(rlWireHitSegment.getTrajectory2D());
    segment2D.receiveISuperCluster();
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

CDCSegment2D CDCSegment2D::reconstructUsingFacets(const CDCRLWireHitSegment& rlWireHitSegment)
{
  if (rlWireHitSegment.size() < 3) {
    return reconstructUsingTangents(rlWireHitSegment);
  } else {
    CDCFacetSegment facetSegment = CDCFacetSegment::create(rlWireHitSegment);
    return condense(facetSegment);
  }
}

std::vector<const CDCWire*> CDCSegment2D::getWireSegment() const
{
  std::vector<const CDCWire*> wireSegment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    wireSegment.push_back(&(recoHit2D.getWire()));
  }
  return wireSegment;
}

CDCWireHitSegment CDCSegment2D::getWireHitSegment() const
{
  CDCWireHitSegment wireHitSegment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    wireHitSegment.push_back(&(recoHit2D.getWireHit()));
  }
  wireHitSegment.setTrajectory2D(getTrajectory2D());
  wireHitSegment.setAliasScore(getAliasScore());
  return wireHitSegment;
}

CDCSegment2D CDCSegment2D::getAlias() const
{
  CDCSegment2D segment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    segment.push_back(recoHit2D.getAlias());
  }
  segment.setISuperCluster(getISuperCluster());
  segment.setAliasScore(getAliasScore());
  return segment;
}

int CDCSegment2D::getNRLSwitches() const
{
  ERightLeft rlInfo = ERightLeft::c_Invalid;
  int nRLSwitches = -1;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    if (rlInfo != recoHit2D.getRLInfo()) {
      rlInfo = recoHit2D.getRLInfo();
      ++nRLSwitches;
    }
  }
  return nRLSwitches;
}

double CDCSegment2D::getRLAsymmetry() const
{
  int rlSum = 0;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    if (isValid(recoHit2D.getRLInfo())) {
      rlSum += recoHit2D.getRLInfo();
    }
  }
  return static_cast<double>(rlSum) / size();
}

CDCRLWireHitSegment CDCSegment2D::getRLWireHitSegment() const
{
  CDCRLWireHitSegment rlWireHitSegment;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    rlWireHitSegment.push_back(recoHit2D.getRLWireHit());
  }
  rlWireHitSegment.setTrajectory2D(getTrajectory2D());
  rlWireHitSegment.setAliasScore(getAliasScore());
  return rlWireHitSegment;
}

CDCSegment2D CDCSegment2D::reversed() const
{
  CDCSegment2D reverseSegment;
  reverseSegment.reserve(size());
  for (const CDCRecoHit2D& recohit : boost::adaptors::reverse(*this)) {
    reverseSegment.push_back(recohit.reversed());
  }

  reverseSegment.setTrajectory2D(getTrajectory2D().reversed());
  reverseSegment.m_automatonCell = m_automatonCell;
  reverseSegment.setISuperCluster(getISuperCluster());
  reverseSegment.setAliasScore(getAliasScore());
  return reverseSegment;
}

void CDCSegment2D::reverse()
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

void CDCSegment2D::unsetAndForwardMaskedFlag(bool toHits) const
{
  getAutomatonCell().unsetMaskedFlag();
  if (not toHits) return;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    const CDCWireHit& wireHit = recoHit2D.getWireHit();
    wireHit.getAutomatonCell().unsetMaskedFlag();
  }
}

void CDCSegment2D::setAndForwardMaskedFlag(bool toHits) const
{
  getAutomatonCell().setMaskedFlag();
  if (not toHits) return;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    const CDCWireHit& wireHit = recoHit2D.getWireHit();
    wireHit.getAutomatonCell().setMaskedFlag();
  }
}

void CDCSegment2D::receiveMaskedFlag(bool fromHits) const
{
  if (not fromHits) return;
  for (const CDCRecoHit2D& recoHit2D : *this) {
    const CDCWireHit& wireHit = recoHit2D.getWireHit();
    if (wireHit.getAutomatonCell().hasMaskedFlag()) {
      getAutomatonCell().setMaskedFlag();
      return;
    }
  }
}

bool CDCSegment2D::isFullyTaken(unsigned int maxNotTaken) const
{
  unsigned int notTakenCounter = 0;
  for (const CDCRecoHit2D& recoHit : *this) {
    if (not recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      notTakenCounter++;
      if (notTakenCounter > maxNotTaken) {
        return false;
      }
    }
  }

  return true;
}
