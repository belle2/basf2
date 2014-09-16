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



void TrackCreator::create(const std::vector<CDCSegmentTripleTrack>& segmentTripleTracks,
                          std::vector<CDCTrack>& tracks) const
{

  for (const CDCSegmentTripleTrack & segmentTripleTrack : segmentTripleTracks) {

    tracks.push_back(CDCTrack());
    CDCTrack& track = tracks.back();
    create(segmentTripleTrack, track);

  }

}




void TrackCreator::create(const std::vector<CDCAxialStereoSegmentPairTrack>& axialStereoSegmentPairTracks,
                          std::vector<CDCTrack>& tracks) const
{

  for (const CDCAxialStereoSegmentPairTrack & axialStereoSegmentPairTrack : axialStereoSegmentPairTracks) {

    tracks.push_back(CDCTrack());
    CDCTrack& track = tracks.back();
    create(axialStereoSegmentPairTrack, track);

  }

}





void TrackCreator::create(const CDCSegmentTripleTrack& segmentTripleTrack,
                          CDCTrack& track) const
{

  //B2DEBUG(200,"Lenght of segmentTripleTrack is " << segmentTripleTrack.size() );
  if (not segmentTripleTrack.empty()) {

    CDCSegmentTripleTrack::const_iterator itSegmentTriple = segmentTripleTrack.begin();
    const CDCSegmentTriple* firstTriple = *itSegmentTriple++;

    // Set the start fits of the track to the ones of the first segment
    track.setStartTrajectory3D(firstTriple->getTrajectory2D(), firstTriple->getTrajectorySZ());

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

    track.setEndTrajectory3D(endTrajectory2D, endTrajectorySZ);

  }

}





void TrackCreator::create(const CDCAxialStereoSegmentPairTrack& axialStereoSegmentPairTrack,
                          CDCTrack& track) const
{

  //B2DEBUG(200,"Lenght of segmentTripleTrack is " << segmentTripleTrack.size() );
  if (not  axialStereoSegmentPairTrack.empty()) {

    CDCAxialStereoSegmentPairTrack::const_iterator itSegmentPair = axialStereoSegmentPairTrack.begin();
    const CDCAxialStereoSegmentPair* ptrFirstSegmentPair = *itSegmentPair++;

    if (not ptrFirstSegmentPair) B2ERROR("Nullptr encounter in CDCAxialStereoSegmentPairTrack");
    const CDCAxialStereoSegmentPair& firstSegmentPair = *ptrFirstSegmentPair;

    // Set the start fits of the track to the ones of the first segment
    track.setStartTrajectory3D(firstSegmentPair.getTrajectory2D(), firstSegmentPair.getTrajectorySZ());

    FloatType perpSOffset = 0.0;
    appendStartRecoHits3D(firstSegmentPair, perpSOffset, track);

    while (itSegmentPair != axialStereoSegmentPairTrack.end()) {

      const CDCAxialStereoSegmentPair* ptrSecondSegmentPair = *itSegmentPair++;
      if (not ptrSecondSegmentPair) B2ERROR("Nullptr encounter in CDCAxialStereoSegmentPairTrack");


      const CDCAxialStereoSegmentPair& firstSegmentPair = *ptrFirstSegmentPair;
      const CDCAxialStereoSegmentPair& secondSegmentPair = *ptrSecondSegmentPair;

      perpSOffset = appendAverageStartEnd(firstSegmentPair,
                                          secondSegmentPair,
                                          perpSOffset,
                                          track);

      ptrFirstSegmentPair = ptrSecondSegmentPair;

    }

    const CDCAxialStereoSegmentPair& lastSegmentPair = *ptrFirstSegmentPair;
    appendEndRecoHits3D(lastSegmentPair, perpSOffset, track);

    // Set the end fits of the track to the ones of the first segment
    CDCTrajectory2D endTrajectory2D = lastSegmentPair.getTrajectory2D();
    CDCTrajectorySZ endTrajectorySZ = lastSegmentPair.getTrajectorySZ();


    // Set the reference point on the trajectories to the last reconstructed hit
    FloatType perpSShift = endTrajectory2D.setLocalOrigin(track.getEndRecoHit3D().getRecoPos2D());
    endTrajectorySZ.passiveMoveS(perpSShift);
    //Both fits now have the travel distance scale from the last hit as it should be
    track.setEndTrajectory3D(endTrajectory2D, endTrajectorySZ);

  }

}


bool TrackCreator::create(const CDCAxialRecoSegment2D& segment,
                          CDCTrack& track) const
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

    track.setStartTrajectory3D(trajectory2D, trajectorySZ);

    //Shift fit to the last reco hit and assigne the end fit of the track as well
    FloatType perpSShift = trajectory2D.setLocalOrigin(track.getEndRecoHit3D().getRecoPos2D());
    trajectorySZ.passiveMoveS(perpSShift); //this does not really do anythin for the line z = 0*s + 0
    // mentioned only for completeness

    track.setEndTrajectory3D(trajectory2D, trajectorySZ);
    return true;

  }
}




void TrackCreator::appendStartRecoHits3D(const CDCSegmentTriple& triple,
                                         FloatType perpSOffset,
                                         CDCTrack& recohits3D) const
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



void TrackCreator::appendMiddleRecoHits3D(const CDCSegmentTriple& triple,
                                          FloatType perpSOffset,
                                          CDCTrack& recohits3D) const
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



void TrackCreator::appendEndRecoHits3D(const CDCSegmentTriple& triple,
                                       FloatType perpSOffset,
                                       CDCTrack& recohits3D) const
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





void TrackCreator::appendStartRecoHits3D(const CDCAxialStereoSegmentPair& pair,
                                         FloatType perpSOffset,
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


void TrackCreator::appendEndRecoHits3D(const CDCAxialStereoSegmentPair& pair,
                                       FloatType perpSOffset,
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
                                    FloatType perpSOffset,
                                    CDCTrack& recohits3D) const
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





FloatType TrackCreator::appendAverageStartEnd(const CDCSegmentTriple& triple,
                                              const CDCSegmentTriple& followingTriple,
                                              FloatType perpSOffset,
                                              CDCTrack& recoHits3D) const
{

  const CDCRecoSegment2D* ptrEndSegment = triple.getEnd();
  const CDCRecoSegment2D* ptrFollowingStartSegment = followingTriple.getStart();

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






FloatType TrackCreator::appendAverageStartEnd(const CDCAxialStereoSegmentPair& pair,
                                              const CDCAxialStereoSegmentPair& followingPair,
                                              FloatType perpSOffset,
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


FloatType TrackCreator::appendAverage(const CDCRecoSegment2D& segment,
                                      const CDCTrajectory2D& trajectory2D,
                                      const CDCTrajectorySZ& trajectorySZ,
                                      FloatType perpSOffset,
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

  FloatType firstPerpS = firstRecoHit3D.getPerpS();


  CDCRecoHit3D followingFirstRecoHit3D =
    CDCRecoHit3D::reconstruct(firstRecoHit2D,
                              followingTrajectory2D,
                              followingTrajectorySZ);

  FloatType followingFirstPerpS = followingFirstRecoHit3D.getPerpS();

  FloatType followingPerpSOffSet = firstPerpS + perpSOffset - followingFirstPerpS;

  for (const CDCRecoHit2D & recoHit2D : segment) {

    CDCRecoHit3D recoHit3D =
      CDCRecoHit3D::reconstruct(recoHit2D,
                                trajectory2D,
                                trajectorySZ);

    recoHit3D.shiftPerpS(perpSOffset);


    CDCRecoHit3D followingRecoHit3D =
      CDCRecoHit3D::reconstruct(recoHit2D,
                                followingTrajectory2D,
                                followingTrajectorySZ);

    followingRecoHit3D.shiftPerpS(followingPerpSOffSet);

    recoHits3D.push_back(CDCRecoHit3D::average(recoHit3D, followingRecoHit3D));

  }


  const CDCRecoHit2D& lastRecoHit2D = segment.back() ;

  CDCRecoHit3D followingLastRecoHit3D =
    CDCRecoHit3D::reconstruct(lastRecoHit2D,
                              followingTrajectory2D,
                              followingTrajectorySZ);

  FloatType newPrepSOffset = recoHits3D.back().getPerpS() - followingLastRecoHit3D.getPerpS();

  return newPrepSOffset;

}
