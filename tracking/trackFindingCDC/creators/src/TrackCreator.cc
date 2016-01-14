/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/creators/TrackCreator.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TrackCreator::create(const std::vector<Path<const CDCSegmentTriple> >& segmentTriplePaths,
                          std::vector<CDCTrack>& tracks) const
{
  for (const Path<const CDCSegmentTriple>& segmentTriplePath : segmentTriplePaths) {
    tracks.push_back(CDCTrack());
    CDCTrack& track = tracks.back();
    create(segmentTriplePath, track);
  }
}

void TrackCreator::create(const std::vector<Path<const CDCSegmentPair> >& segmentPairPaths,
                          std::vector<CDCTrack>& tracks) const
{
  for (const Path<const CDCSegmentPair>& segmentPairPath : segmentPairPaths) {
    tracks.push_back(CDCTrack());
    CDCTrack& track = tracks.back();
    create(segmentPairPath, track);
  }
}

void TrackCreator::create(const Path<const CDCSegmentTriple>& segmentTripleTrack,
                          CDCTrack& track) const
{
  //B2DEBUG(200,"Lenght of segmentTripleTrack is " << segmentTripleTrack.size() );
  if (not segmentTripleTrack.empty()) {

    Path<const CDCSegmentTriple>::const_iterator itSegmentTriple = segmentTripleTrack.begin();
    const CDCSegmentTriple* firstTriple = *itSegmentTriple++;

    // Set the start fits of the track to the ones of the first segment
    CDCTrajectory3D startTrajectory3D(firstTriple->getTrajectory2D(), firstTriple->getTrajectorySZ());


    double perpSOffset = 0.0;
    appendStartRecoHits3D(*firstTriple, perpSOffset, track);
    appendMiddleRecoHits3D(*firstTriple, perpSOffset, track);

    while (itSegmentTriple != segmentTripleTrack.end()) {

      const CDCSegmentTriple* secondTriple = *itSegmentTriple++;
      perpSOffset = appendAverageStartEnd(*firstTriple,
                                          *secondTriple,
                                          perpSOffset,
                                          track);

      appendMiddleRecoHits3D(*secondTriple, perpSOffset, track);

      firstTriple = secondTriple;

    }

    appendEndRecoHits3D(*firstTriple, perpSOffset, track);

    // Set the end fits of the track to the ones of the first segment
    CDCTrajectory2D endTrajectory2D = firstTriple->getTrajectory2D();
    CDCTrajectorySZ endTrajectorySZ = firstTriple->getTrajectorySZ();
    CDCTrajectory3D endTrajectory3D(endTrajectory2D, endTrajectorySZ);

    // Set the reference point on the trajectories to the last reconstructed hit
    startTrajectory3D.setLocalOrigin(track.getStartRecoHit3D().getRecoPos3D());
    endTrajectory3D.setLocalOrigin(track.getEndRecoHit3D().getRecoPos3D());

    track.setStartTrajectory3D(startTrajectory3D);
    track.setEndTrajectory3D(endTrajectory3D);

  }

}





void TrackCreator::create(const Path<const CDCSegmentPair>& segmentPairTrack,
                          CDCTrack& track) const
{

  //B2DEBUG(200,"Lenght of segmentTripleTrack is " << segmentTripleTrack.size() );
  if (not  segmentPairTrack.empty()) {

    Path<const CDCSegmentPair>::const_iterator itSegmentPair = segmentPairTrack.begin();
    const CDCSegmentPair* ptrFirstSegmentPair = *itSegmentPair++;

    if (not ptrFirstSegmentPair) {
      B2ERROR("Nullptr encounter in Path<const CDCSegmentPair>");
      return;
    }
    const CDCSegmentPair& firstSegmentPair = *ptrFirstSegmentPair;

    // Keep the fit of the first segment pair to set it as the fit at the start of the track
    CDCTrajectory3D startTrajectory3D = firstSegmentPair.getTrajectory3D();

    double perpSOffset = 0.0;
    appendStartRecoHits3D(firstSegmentPair, perpSOffset, track);

    while (itSegmentPair != segmentPairTrack.end()) {

      const CDCSegmentPair* ptrSecondSegmentPair = *itSegmentPair++;
      if (not ptrSecondSegmentPair) {
        B2ERROR("Nullptr encounter in Path<const CDCSegmentPair>");
        return;
      }


      const CDCSegmentPair& firstSegmentPair = *ptrFirstSegmentPair;
      const CDCSegmentPair& secondSegmentPair = *ptrSecondSegmentPair;

      perpSOffset = appendAverageStartEnd(firstSegmentPair,
                                          secondSegmentPair,
                                          perpSOffset,
                                          track);

      ptrFirstSegmentPair = ptrSecondSegmentPair;

    }

    const CDCSegmentPair& lastSegmentPair = *ptrFirstSegmentPair;
    appendEndRecoHits3D(lastSegmentPair, perpSOffset, track);

    // Keep the fit of the last segment pair to set it as the fit at the end of the track
    CDCTrajectory3D endTrajectory3D = lastSegmentPair.getTrajectory3D();

    // Move the reference point of the start fit to the first observered position
    startTrajectory3D.setLocalOrigin(track.getStartRecoHit3D().getRecoPos3D());
    track.setStartTrajectory3D(startTrajectory3D);

    // Move the reference point of the end fit to the last observered position
    endTrajectory3D.setLocalOrigin(track.getEndRecoHit3D().getRecoPos3D());
    track.setEndTrajectory3D(endTrajectory3D);
  }

}

void TrackCreator::create(std::vector<const CDCWireHit*>& hits, CDCTrack& track) const
{
  if (hits.size() == 0) return;

  const CDCTrajectory2D& trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  for (const CDCWireHit* item : hits) {
    if (item->getAutomatonCell().hasTakenFlag() || item->getAutomatonCell().hasMaskedFlag()) continue;

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstructNearest(item, trackTrajectory2D);
    track.push_back(std::move(cdcRecoHit3D));
    cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
  }
}


void TrackCreator::appendStartRecoHits3D(const CDCSegmentTriple& triple,
                                         double perpSOffset,
                                         CDCTrack& recohits3D) const
{
  const CDCRecoSegment2D* startSegment = triple.getStartSegment();
  if (startSegment != nullptr) {
    appendRecoHits3D(*startSegment,
                     triple.getTrajectory2D(),
                     triple.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }
}



void TrackCreator::appendMiddleRecoHits3D(const CDCSegmentTriple& triple,
                                          double perpSOffset,
                                          CDCTrack& recohits3D) const
{
  const CDCRecoSegment2D* middleSegment = triple.getMiddleSegment();
  if (middleSegment != nullptr) {
    appendRecoHits3D(*middleSegment,
                     triple.getTrajectory2D(),
                     triple.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }
}



void TrackCreator::appendEndRecoHits3D(const CDCSegmentTriple& triple,
                                       double perpSOffset,
                                       CDCTrack& recohits3D) const
{
  const CDCRecoSegment2D* endSegment = triple.getEndSegment();
  if (endSegment != nullptr) {
    appendRecoHits3D(*endSegment,
                     triple.getTrajectory2D(),
                     triple.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }
}





void TrackCreator::appendStartRecoHits3D(const CDCSegmentPair& pair,
                                         double perpSOffset,
                                         CDCTrack& recohits3D) const
{

  const CDCRecoSegment2D* ptrStartSegment = pair.getStartSegment();
  if (ptrStartSegment != nullptr) {
    const CDCRecoSegment2D& startSegment = *ptrStartSegment;
    appendRecoHits3D(startSegment,
                     pair.getTrajectory2D(),
                     pair.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }
}


void TrackCreator::appendEndRecoHits3D(const CDCSegmentPair& pair,
                                       double perpSOffset,
                                       CDCTrack& recohits3D) const
{
  const CDCRecoSegment2D* ptrEndSegment = pair.getEndSegment();
  if (ptrEndSegment != nullptr) {
    const CDCRecoSegment2D& endSegment = *ptrEndSegment;
    appendRecoHits3D(endSegment,
                     pair.getTrajectory2D(),
                     pair.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }




}









void TrackCreator::appendRecoHits3D(const CDCRecoSegment2D& segment,
                                    const CDCTrajectory2D& trajectory2D,
                                    const CDCTrajectorySZ& trajectorySZ,
                                    double perpSOffset,
                                    CDCTrack& recohits3D) const
{
  for (const CDCRecoHit2D& recohit2D : segment) {
    recohits3D.push_back(CDCRecoHit3D::reconstruct(recohit2D,
                                                   trajectory2D,
                                                   trajectorySZ));
    recohits3D.back().shiftArcLength2D(perpSOffset);
  }
}





double TrackCreator::appendAverageStartEnd(const CDCSegmentTriple& triple,
                                           const CDCSegmentTriple& followingTriple,
                                           double perpSOffset,
                                           CDCTrack& recoHits3D) const
{

  const CDCRecoSegment2D* ptrEndSegment = triple.getEndSegment();
  const CDCRecoSegment2D* ptrFollowingStartSegment = followingTriple.getStartSegment();

  if (ptrEndSegment == ptrFollowingStartSegment and
      ptrFollowingStartSegment != nullptr) {

    //followingStartSegment and endSegment point to the same object
    //hence they have the same two dimensional recohits
    //make RecoHits3D from the fits stored in the triples and average them
    const CDCRecoSegment2D& commonSegment = *ptrEndSegment;

    return appendAverage(commonSegment,
                         triple.getTrajectory2D(),
                         triple.getTrajectorySZ(),
                         perpSOffset,
                         followingTriple.getTrajectory2D(),
                         followingTriple.getTrajectorySZ(),
                         recoHits3D);

  } else {
    return perpSOffset;
  }

}






double TrackCreator::appendAverageStartEnd(const CDCSegmentPair& pair,
                                           const CDCSegmentPair& followingPair,
                                           double perpSOffset,
                                           CDCTrack& recoHits3D) const
{

  const CDCRecoSegment2D* ptrEndSegment = pair.getEndSegment();
  const CDCRecoSegment2D* ptrFollowingStartSegment = followingPair.getStartSegment();

  if (ptrEndSegment == ptrFollowingStartSegment and
      ptrFollowingStartSegment != nullptr) {

    //followingStartSegment and endSegment point to the same object
    //hence they have the same two dimensional recohits
    //make RecoHits3D from the fits stored in the triples and average them
    const CDCRecoSegment2D& commonSegment = *ptrEndSegment;

    return appendAverage(commonSegment,
                         pair.getTrajectory2D(),
                         pair.getTrajectorySZ(),
                         perpSOffset,
                         followingPair.getTrajectory2D(),
                         followingPair.getTrajectorySZ(),
                         recoHits3D);

  } else {
    return perpSOffset;
  }

}


double TrackCreator::appendAverage(const CDCRecoSegment2D& segment,
                                   const CDCTrajectory2D& trajectory2D,
                                   const CDCTrajectorySZ& trajectorySZ,
                                   double perpSOffset,
                                   const CDCTrajectory2D& followingTrajectory2D,
                                   const CDCTrajectorySZ& followingTrajectorySZ,
                                   CDCTrack& recoHits3D) const
{
  if (segment.empty()) return perpSOffset;

  const CDCRecoHit2D& firstRecoHit2D = segment.front();

  CDCRecoHit3D firstRecoHit3D =
    CDCRecoHit3D::reconstruct(firstRecoHit2D,
                              trajectory2D,
                              trajectorySZ);

  double firstPerpS = firstRecoHit3D.getArcLength2D();


  CDCRecoHit3D followingFirstRecoHit3D =
    CDCRecoHit3D::reconstruct(firstRecoHit2D,
                              followingTrajectory2D,
                              followingTrajectorySZ);

  double followingFirstPerpS = followingFirstRecoHit3D.getArcLength2D();

  double followingPerpSOffSet = firstPerpS + perpSOffset - followingFirstPerpS;

  for (const CDCRecoHit2D& recoHit2D : segment) {

    CDCRecoHit3D recoHit3D =
      CDCRecoHit3D::reconstruct(recoHit2D,
                                trajectory2D,
                                trajectorySZ);

    recoHit3D.shiftArcLength2D(perpSOffset);


    CDCRecoHit3D followingRecoHit3D =
      CDCRecoHit3D::reconstruct(recoHit2D,
                                followingTrajectory2D,
                                followingTrajectorySZ);

    followingRecoHit3D.shiftArcLength2D(followingPerpSOffSet);

    recoHits3D.push_back(CDCRecoHit3D::average(recoHit3D, followingRecoHit3D));

  }


  const CDCRecoHit2D& lastRecoHit2D = segment.back() ;

  CDCRecoHit3D followingLastRecoHit3D =
    CDCRecoHit3D::reconstruct(lastRecoHit2D,
                              followingTrajectory2D,
                              followingTrajectorySZ);

  double newPrepSOffset = recoHits3D.back().getArcLength2D() - followingLastRecoHit3D.getArcLength2D();

  return newPrepSOffset;

}
