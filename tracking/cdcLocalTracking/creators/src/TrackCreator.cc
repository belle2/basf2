/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/TrackCreator.h"

#include <boost/foreach.hpp>
#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;



void
TrackCreator::create(
  const std::vector<CDCSegmentTripleTrack>& segmentTripleTracks,
  std::vector<CDCTrack>& tracks
) const
{

  BOOST_FOREACH(const CDCSegmentTripleTrack & segmentTripleTrack, segmentTripleTracks) {

    tracks.push_back(CDCTrack());
    CDCTrack& track = tracks.back();
    create(segmentTripleTrack, track);

  }

}



void
TrackCreator::create(
  const CDCSegmentTripleTrack& segmentTripleTrack,
  CDCTrack& track
) const
{

  //B2DEBUG(200,"Lenght of segmentTripleTrack is " << segmentTripleTrack.size() );
  if (not segmentTripleTrack.empty()) {

    CDCSegmentTripleTrack::const_iterator itSegmentTriple = segmentTripleTrack.begin();
    const CDCSegmentTriple* firstTriple = *itSegmentTriple++;

    // Set the start fits of the track to the ones of the first segment
    track.setStartTrajectory2D(firstTriple->getTrajectory2D());
    track.setStartTrajectorySZ(firstTriple->getTrajectorySZ());

    FloatType perpSOffset = 0.0;
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

    // Set the reference point on the trajectories to the last reconstructed hit
    FloatType perpSShift = endTrajectory2D.setLocalOrigin(track.getEndRecoHit3D().getRecoPos2D());
    endTrajectorySZ.passiveMoveS(perpSShift);
    //Both fits now have the travel distance scale from the last hit as it should be

    track.setEndTrajectory2D(endTrajectory2D);
    track.setEndTrajectorySZ(endTrajectorySZ);



  }

}


bool TrackCreator::create(
  const CDCAxialRecoSegment2D& segment,
  CDCTrack& track
) const
{

  CDCTrajectorySZ trajectorySZ(0, 0); // line with z = 0*s + 0 best we can assume for axial segments only

  CDCTrajectory2D trajectory2D;
  CDCRiemannFitter fitter2D;
  fitter2D.useOnlyOrientation(); // do be adjusted for best results
  fitter2D.update(trajectory2D, segment);

  if (not segment.isForwardTrajectory(trajectory2D)) {

    //B2WARNING("Fit is not oriented correctly");
    return false;

  } else {

    appendRecoHits3D(segment, trajectory2D, trajectorySZ, 0.0, track);


    track.setStartTrajectorySZ(trajectorySZ);


    track.setStartTrajectory2D(trajectory2D);

    //Shift fit to the last reco hit and assigne the end fit of the track as well
    FloatType perpSShift = trajectory2D.setLocalOrigin(track.getEndRecoHit3D().getRecoPos2D());
    trajectorySZ.passiveMoveS(perpSShift); //this does not really do anythin for the line z = 0*s + 0
    // mentioned only for completeness

    track.setEndTrajectory2D(trajectory2D);
    track.setEndTrajectorySZ(trajectorySZ);
    return true;

  }
}




void
TrackCreator::appendStartRecoHits3D(
  const CDCSegmentTriple& triple,
  FloatType perpSOffset,
  CDCRecoSegment3D& recohits3D
) const
{
  const CDCRecoSegment2D* startSegment = triple.getStart();
  if (startSegment != nullptr) {
    appendRecoHits3D(*startSegment,
                     triple.getTrajectory2D(),
                     triple.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }
}



void
TrackCreator::appendMiddleRecoHits3D(
  const CDCSegmentTriple& triple,
  FloatType perpSOffset,
  CDCRecoSegment3D& recohits3D
) const
{
  const CDCRecoSegment2D* middleSegment = triple.getMiddle();
  if (middleSegment != nullptr) {
    appendRecoHits3D(*middleSegment,
                     triple.getTrajectory2D(),
                     triple.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }
}



void
TrackCreator::appendEndRecoHits3D(
  const CDCSegmentTriple& triple,
  FloatType perpSOffset,
  CDCRecoSegment3D& recohits3D
) const
{
  const CDCRecoSegment2D* endSegment = triple.getEnd();
  if (endSegment != nullptr) {
    appendRecoHits3D(*endSegment,
                     triple.getTrajectory2D(),
                     triple.getTrajectorySZ(),
                     perpSOffset,
                     recohits3D);
  }
}





void
TrackCreator::appendRecoHits3D(
  const CDCRecoSegment2D& segment,
  const CDCTrajectory2D& trajectory2D,
  const CDCTrajectorySZ& trajectorySZ,
  FloatType perpSOffset,
  CDCRecoSegment3D& recohits3D
) const
{
  BOOST_FOREACH(const CDCRecoHit2D & recohit2D, segment) {
    //for ( CDCRecoSegment2D::const_iterator itRecoHit2D = segment->begin();
    //      itRecoHit2D  != segment->end(); ++itRecoHit2D ){

    recohits3D.push_back(CDCRecoHit3D::reconstruct(recohit2D,
                                                   trajectory2D,
                                                   trajectorySZ));
    recohits3D.back().shiftPerpS(perpSOffset);

  }
}





FloatType
TrackCreator::appendAverageStartEnd(
  const CDCSegmentTriple& triple,
  const CDCSegmentTriple& followingTriple,
  FloatType perpSOffset,
  CDCRecoSegment3D& recohits3D
) const
{

  //if the end segment of this and the following segment match
  const CDCAxialRecoSegment2D* endSegment = triple.getEnd();
  const CDCAxialRecoSegment2D* followingStartSegment = followingTriple.getStart();

  if (endSegment == followingStartSegment and
      followingStartSegment != nullptr and
      not followingStartSegment->empty()) {

    //followingStartSegment and endSegment point to the same object
    //hence they have the same recohits
    //make RecoHits3D from the fits stored in the triples and average them

    //get the start point of the perpS scale from the getEnd() segment
    const CDCRecoHit2D& firstRecoHit2D = endSegment->front() ;
    CDCRecoHit3D firstRecoHit3DFromEnd  =
      CDCRecoHit3D::reconstruct(firstRecoHit2D,
                                triple.getTrajectory2D(),
                                triple.getTrajectorySZ());

    FloatType startPerpSOfEnd = firstRecoHit3DFromEnd.getPerpS();

    BOOST_FOREACH(const CDCRecoHit2D & recoHit2D, *endSegment) {

      CDCRecoHit3D recoHit3DFromEnd   =
        CDCRecoHit3D::reconstruct(recoHit2D,
                                  triple.getTrajectory2D(),
                                  triple.getTrajectorySZ());

      CDCRecoHit3D recoHit3DFromNext =
        CDCRecoHit3D::reconstruct(recoHit2D,
                                  followingTriple.getTrajectory2D(),
                                  followingTriple.getTrajectorySZ());

      // make an offset that the both perpS scales of the getEnd() segment
      // and followingSegmentTriple
      // match on the first recohit
      recoHit3DFromNext.shiftPerpS(startPerpSOfEnd);

      recohits3D.push_back(CDCRecoHit3D::average(recoHit3DFromEnd , recoHit3DFromNext));
      CDCRecoHit3D& recoHit3DAverage = recohits3D.back();

      // we have to take special care about the travel distance
      // that the following segments can be aligned with
      // (at least) increasing order
      recoHit3DAverage.shiftPerpS(perpSOffset);

    }


    const CDCRecoHit2D& lastRecoHit2D = followingStartSegment->back() ;
    CDCRecoHit3D lastRecoHit3DFromNext =
      CDCRecoHit3D::reconstruct(lastRecoHit2D,
                                followingTriple.getTrajectory2D(),
                                followingTriple.getTrajectorySZ());

    FloatType newPrepSOffset =
      recohits3D.back().getPerpS() - lastRecoHit3DFromNext.getPerpS();

    return newPrepSOffset;

  } else {
    return perpSOffset;
  }
}



