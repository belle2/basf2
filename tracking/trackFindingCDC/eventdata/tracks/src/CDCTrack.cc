/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <tracking/trackFindingCDC/numerics/FloatComparing.h>

#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <framework/logging/Logger.h>

#include <algorithm>
#include <utility>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRLWireHit;
  }
}

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Reconstruct a segment with the two fits and append it to the track
  void appendReconstructed(const CDCSegment2D* segment,
                           const CDCTrajectory3D& trajectory3D,
                           double perpSOffset,
                           CDCTrack& track)
  {
    B2ASSERT("Did not expect segment == nullptr", segment);

    for (const CDCRecoHit2D& recohit2D : *segment) {
      track.push_back(CDCRecoHit3D::reconstruct(recohit2D,
                                                trajectory3D));
      track.back().shiftArcLength2D(perpSOffset);
    }
  }


  /**
   *  Append the three dimensional reconstructed hits from the given segments averaged over two possible trajectories.
   *  In case of overlapping segments in segment triple or segment pairs segment usually participate in two trajectory fits.
   *  In principle we end up with two reconstructed positions from each of the fits which we have to average out. Also there
   *  is a mismatch of the start point of the travel distance scale since the reference points of the two trajectories differ.
   *  Hence we shift the travel distance scale of the following trajectory such that both match on the first hit of the given segment.
   *  For the other hits the average of the travel distance is assumed.
   *  Furthermore an carry over offset of the travel distance can be introduced which might arise from earlier travel distance shifts
   *  on other previous parts of the whole track.
   *  Return value is the travel distance by which the following trajectories have to be shifted to match the overall track travel distance scale
   *  on the last hit of the given segment.
   */
  double appendReconstructedAverage(const CDCSegment2D* segment,
                                    const CDCTrajectory3D& trajectory3D,
                                    double perpSOffset,
                                    const CDCTrajectory3D& parallelTrajectory3D,
                                    CDCTrack& track)
  {
    B2ASSERT("Did not expect segment == nullptr", segment);

    const CDCRecoHit2D& firstRecoHit2D = segment->front();

    CDCRecoHit3D firstRecoHit3D =
      CDCRecoHit3D::reconstruct(firstRecoHit2D, trajectory3D);

    double firstPerpS = firstRecoHit3D.getArcLength2D();

    CDCRecoHit3D parallelFirstRecoHit3D =
      CDCRecoHit3D::reconstruct(firstRecoHit2D, parallelTrajectory3D);

    double parallelFirstPerpS = parallelFirstRecoHit3D.getArcLength2D();

    double parallelPerpSOffSet = firstPerpS + perpSOffset - parallelFirstPerpS;

    for (const CDCRecoHit2D& recoHit2D : *segment) {

      CDCRecoHit3D recoHit3D =
        CDCRecoHit3D::reconstruct(recoHit2D, trajectory3D);

      recoHit3D.shiftArcLength2D(perpSOffset);


      CDCRecoHit3D parallelRecoHit3D =
        CDCRecoHit3D::reconstruct(recoHit2D, parallelTrajectory3D);

      parallelRecoHit3D.shiftArcLength2D(parallelPerpSOffSet);

      track.push_back(CDCRecoHit3D::average(recoHit3D, parallelRecoHit3D));

    }

    const CDCRecoHit2D& lastRecoHit2D = segment->back() ;

    CDCRecoHit3D parallelLastRecoHit3D =
      CDCRecoHit3D::reconstruct(lastRecoHit2D, parallelTrajectory3D);

    double newPrepSOffset = track.back().getArcLength2D() - parallelLastRecoHit3D.getArcLength2D();

    return newPrepSOffset;
  }
}

CDCTrack::CDCTrack(const std::vector<CDCRecoHit3D>& recoHits3D)
  : std::vector<CDCRecoHit3D>(recoHits3D)
{
}

CDCTrack::CDCTrack(const CDCSegment2D& segment) :
  m_startTrajectory3D(segment.getTrajectory2D()),
  m_endTrajectory3D(segment.getTrajectory2D())
{
  if (segment.empty()) return;

  // Adjust the start point
  const CDCRecoHit2D& startRecoHit2D = segment.front();
  const CDCRecoHit2D& endRecoHit2D = segment.back();

  Vector3D startPos3D(startRecoHit2D.getRecoPos2D(), 0.0);
  Vector3D endPos3D(endRecoHit2D.getRecoPos2D(), 0.0);

  m_startTrajectory3D.setLocalOrigin(startPos3D);
  m_endTrajectory3D.setLocalOrigin(endPos3D);

  for (const CDCRecoHit2D& recoHit2D : segment) {
    const CDCRLWireHit& rlWireHit = recoHit2D.getRLWireHit();
    Vector3D recoPos3D(recoHit2D.getRecoPos2D(), 0.0);
    double perpS = m_startTrajectory3D.calcArcLength2D(recoPos3D);
    push_back(CDCRecoHit3D(rlWireHit, recoPos3D, perpS));
  }

  // TODO: Maybe enhance the estimation of the z coordinate with the superlayer slopes.
}

CDCTrack CDCTrack::condense(const std::vector<const CDCTrack*>& trackPath)
{
  if (trackPath.empty()) {
    return CDCTrack();
  } else if (trackPath.size() == 1) {
    return CDCTrack(*(trackPath[0]));
  } else {
    CDCTrack result;
    for (const CDCTrack* track :  trackPath) {
      for (const CDCRecoHit3D& recoHit3D : *track) {
        result.push_back(recoHit3D);
        /// FIXME :  arc lengths are not set properly
      }
    }

    CDCTrajectory3D startTrajectory3D = trackPath.front()->getStartTrajectory3D();
    CDCTrajectory3D endTrajectory3D = trackPath.back()->getStartTrajectory3D();

    double resetPerpSOffset =
      startTrajectory3D.setLocalOrigin(result.front().getRecoPos3D());
    result.setStartTrajectory3D(startTrajectory3D);

    endTrajectory3D.setLocalOrigin(result.back().getRecoPos3D());
    result.setEndTrajectory3D(endTrajectory3D);

    for (CDCRecoHit3D& recoHit3D : result) {
      recoHit3D.shiftArcLength2D(-resetPerpSOffset);
    }

    return result;
  }
}

CDCTrack CDCTrack::condense(const Path<const CDCSegmentTriple>& segmentTriplePath)
{
  CDCTrack track;
  // B2DEBUG(200,"Lenght of segmentTripleTrack is " << segmentTripleTrack.size() );
  if (segmentTriplePath.empty()) return track;

  Path<const CDCSegmentTriple>::const_iterator itSegmentTriple = segmentTriplePath.begin();
  const CDCSegmentTriple* firstSegmentTriple = *itSegmentTriple++;

  // Set the start fits of the track to the ones of the first segment
  CDCTrajectory3D startTrajectory3D = firstSegmentTriple->getTrajectory3D();


  double perpSOffset = 0.0;
  appendReconstructed(firstSegmentTriple->getStartSegment(),
                      firstSegmentTriple->getTrajectory3D(),
                      perpSOffset,
                      track);

  appendReconstructed(firstSegmentTriple->getMiddleSegment(),
                      firstSegmentTriple->getTrajectory3D(),
                      perpSOffset, track);

  while (itSegmentTriple != segmentTriplePath.end()) {

    const CDCSegmentTriple* secondSegmentTriple = *itSegmentTriple++;
    B2ASSERT("Two segement triples do not overlap in their axial segments",
             firstSegmentTriple->getEndSegment() == secondSegmentTriple->getStartSegment());

    perpSOffset = appendReconstructedAverage(firstSegmentTriple->getEndSegment(),
                                             firstSegmentTriple->getTrajectory3D(),
                                             perpSOffset,
                                             secondSegmentTriple->getTrajectory3D(),
                                             track);

    appendReconstructed(secondSegmentTriple->getMiddleSegment(),
                        secondSegmentTriple->getTrajectory3D(),
                        perpSOffset, track);

    firstSegmentTriple = secondSegmentTriple;

  }

  const CDCSegmentTriple* lastSegmentTriple = firstSegmentTriple;

  appendReconstructed(lastSegmentTriple->getEndSegment(),
                      lastSegmentTriple->getTrajectory3D(),
                      perpSOffset, track);

  // Set the end fits of the track to the ones of the last segment
  CDCTrajectory3D endTrajectory3D = lastSegmentTriple->getTrajectory3D();

  // Set the reference point on the trajectories to the last reconstructed hit
  double resetPerpSOffset = startTrajectory3D.setLocalOrigin(track.front().getRecoPos3D());
  track.setStartTrajectory3D(startTrajectory3D);

  endTrajectory3D.setLocalOrigin(track.back().getRecoPos3D());
  track.setEndTrajectory3D(endTrajectory3D);

  for (CDCRecoHit3D& recoHit3D : track) {
    recoHit3D.shiftArcLength2D(-resetPerpSOffset);
  }

  return track;
}

CDCTrack CDCTrack::condense(const Path<const CDCSegmentPair>& segmentPairPath)
{
  CDCTrack track;

  //B2DEBUG(200,"Lenght of segmentTripleTrack is " << segmentTripleTrack.size() );
  if (segmentPairPath.empty()) return track;

  Path<const CDCSegmentPair>::const_iterator itSegmentPair = segmentPairPath.begin();
  const CDCSegmentPair* firstSegmentPair = *itSegmentPair++;


  // Keep the fit of the first segment pair to set it as the fit at the start of the track
  CDCTrajectory3D startTrajectory3D = firstSegmentPair->getTrajectory3D();

  double perpSOffset = 0.0;
  appendReconstructed(firstSegmentPair->getFromSegment(),
                      firstSegmentPair->getTrajectory3D(),
                      perpSOffset, track);

  while (itSegmentPair != segmentPairPath.end()) {

    const CDCSegmentPair* secondSegmentPair = *itSegmentPair++;

    B2ASSERT("Two segement pairs do not overlap in their segments",
             firstSegmentPair->getToSegment() == secondSegmentPair->getFromSegment());

    perpSOffset = appendReconstructedAverage(firstSegmentPair->getToSegment(),
                                             firstSegmentPair->getTrajectory3D(),
                                             perpSOffset,
                                             secondSegmentPair->getTrajectory3D(),
                                             track);

    firstSegmentPair = secondSegmentPair;
  }

  const CDCSegmentPair* lastSegmentPair = firstSegmentPair;
  appendReconstructed(lastSegmentPair->getToSegment(),
                      lastSegmentPair->getTrajectory3D(),
                      perpSOffset, track);

  // Keep the fit of the last segment pair to set it as the fit at the end of the track
  CDCTrajectory3D endTrajectory3D = lastSegmentPair->getTrajectory3D();

  // Move the reference point of the start fit to the first observered position
  double resetPerpSOffset = startTrajectory3D.setLocalOrigin(track.front().getRecoPos3D());
  track.setStartTrajectory3D(startTrajectory3D);

  // Move the reference point of the end fit to the last observered position
  endTrajectory3D.setLocalOrigin(track.back().getRecoPos3D());
  track.setEndTrajectory3D(endTrajectory3D);

  for (CDCRecoHit3D& recoHit3D : track) {
    recoHit3D.shiftArcLength2D(-resetPerpSOffset);
  }

  return track;
}

std::vector<CDCSegment3D> CDCTrack::splitIntoSegments() const
{
  vector<CDCSegment3D> result;
  ISuperLayer lastISuperLayer = -1;
  for (const CDCRecoHit3D& recoHit3D : *this) {
    ISuperLayer iSuperLayer = recoHit3D.getISuperLayer();
    if (result.empty() or lastISuperLayer != iSuperLayer) {
      result.emplace_back();
    }
    result.back().push_back(recoHit3D);
    lastISuperLayer = iSuperLayer;
  }
  return result;
}

void CDCTrack::reverse()
{
  if (empty()) return;

  // Exchange the forward and backward trajectory and reverse them
  std::swap(m_startTrajectory3D, m_endTrajectory3D);
  m_startTrajectory3D.reverse();
  m_endTrajectory3D.reverse();

  const CDCRecoHit3D& lastRecoHit3D = back();
  double lastPerpS = lastRecoHit3D.getArcLength2D();
  double newLastPerpS = m_startTrajectory3D.calcArcLength2D(lastRecoHit3D.getRecoPos3D());

  // Reverse the left right passage hypotheses and reverse the measured travel distance
  for (CDCRecoHit3D& recoHit3D : *this) {
    recoHit3D.reverse();
    double perpS = recoHit3D.getArcLength2D();
    recoHit3D.setArcLength2D(newLastPerpS + lastPerpS - perpS);
  }

  // Reverse the arrangement of hits.
  std::reverse(begin(), end());
}

CDCTrack CDCTrack::reversed() const
{
  CDCTrack reversedTrack(*this);
  reversedTrack.reverse();
  return reversedTrack;
}

MayBePtr<const CDCRecoHit3D> CDCTrack::find(const CDCWireHit& wireHit) const
{
  auto hasWireHit = [&wireHit](const CDCRecoHit3D & recoHit3D) {
    return recoHit3D.hasWireHit(wireHit);
  };
  auto itRecoHit3D = std::find_if(this->begin(), this->end(), hasWireHit);
  return itRecoHit3D == this->end() ? nullptr : &*itRecoHit3D;
}

void CDCTrack::unsetAndForwardMaskedFlag() const
{
  getAutomatonCell().unsetMaskedFlag();
  for (const CDCRecoHit3D& recoHit3D : *this) {
    const CDCWireHit& wireHit = recoHit3D.getWireHit();
    wireHit.getAutomatonCell().unsetMaskedFlag();
  }
}

void CDCTrack::setAndForwardMaskedFlag() const
{
  getAutomatonCell().setMaskedFlag();
  for (const CDCRecoHit3D& recoHit3D : *this) {
    const CDCWireHit& wireHit = recoHit3D.getWireHit();
    wireHit.getAutomatonCell().setMaskedFlag();
  }
}

void CDCTrack::receiveMaskedFlag() const
{
  for (const CDCRecoHit3D& recoHit3D : *this) {
    const CDCWireHit& wireHit = recoHit3D.getWireHit();
    if (wireHit.getAutomatonCell().hasMaskedFlag()) {
      getAutomatonCell().setMaskedFlag();
      return;
    }
  }
}

void CDCTrack::forwardTakenFlag(bool takenFlag) const
{
  for (const CDCRecoHit3D& recoHit3D : *this) {
    recoHit3D.getWireHit().getAutomatonCell().setTakenFlag(takenFlag);
  }
}

void CDCTrack::sortByArcLength2D()
{
  std::stable_sort(begin(),
                   end(),
  [](const CDCRecoHit3D & recoHit, const CDCRecoHit3D & otherRecoHit) {
    double arcLength = recoHit.getArcLength2D();
    double otherArcLength = otherRecoHit.getArcLength2D();
    return lessFloatHighNaN(arcLength, otherArcLength);
  });
}

void CDCTrack::shiftToPositiveArcLengths2D(bool doForAllTracks)
{
  const CDCTrajectory2D& startTrajectory2D = getStartTrajectory3D().getTrajectory2D();
  if (doForAllTracks or startTrajectory2D.isCurler(1.1)) {
    const double shiftValue = startTrajectory2D.getLocalCircle()->arcLengthPeriod();
    if (std::isfinite(shiftValue)) {
      for (CDCRecoHit3D& recoHit : *this) {
        if (recoHit.getArcLength2D() < 0) {
          recoHit.shiftArcLength2D(shiftValue);
        }
      }
    }
  }
}
