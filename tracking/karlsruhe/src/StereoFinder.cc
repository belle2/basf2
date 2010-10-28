/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/


#include "../include/StereoFinder.h"

#include "CLHEP/Geometry/Point3D.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/logging/Logger.h>

#include <tracking/karlsruhe/AxialTrackFinder.h>


using namespace std;
using namespace Belle2;

StereoFinder::StereoFinder()
{
}

StereoFinder::~StereoFinder()
{
}

int StereoFinder::FindNextStereoSegment(CDCTrack startTrack, string SegmentsCDCArray, int superlayerId, double maxDistance, double maxAngle)
{
//INFO("***** Search for Track "<<startTrack.getId());
  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str());

  double minDistance = 0.1; //start value for minimizing the distance
  double minAngle = 1;
  int nSegments = cdcSegmentsArray->GetEntries();
  int  bestCandidate;
  for (int i = 0; i < nSegments; i++) { //loop over all segments

    double angle = startTrack.getDirection().Angle(cdcSegmentsArray[i]->getDirection());
    double distance = AxialTrackFinder::ShortestDistance(startTrack, *cdcSegmentsArray[i]);

    if (cdcSegmentsArray[i]->getSuperlayerId() == superlayerId) {
      // INFO("         Sl and ID  "<< cdcSegmentsArray[i]->getSuperlayerId()<<" "<<cdcSegmentsArray[i]->getId());

      if (cdcSegmentsArray[i]->getIsGood() == true && cdcSegmentsArray[i]->getIsUsed() == false) {
        // INFO("Angle: "<<angle);
        // INFO("Distance "<<distance);
        if (angle < maxAngle && distance < maxDistance) {
          if (angle < minAngle || distance < minDistance) {
            minDistance = distance;
            minAngle = angle;
            bestCandidate = i;
          }
        }
      }
    }
  }//end loop over all segments
  if (minDistance < maxDistance) {
    // INFO("BestCandidate: "<<cdcSegmentsArray[bestCandidate]->getSuperlayerId()<<" "<<cdcSegmentsArray[bestCandidate]->getId());
    return bestCandidate;
  }

  return 9999;
}



void StereoFinder::AppendStereoSegments(string StereoSegmentsCDCArray, string TracksCDCArray)
{
  StoreArray<CDCSegment> cdcStereoSegmentsArray(StereoSegmentsCDCArray.c_str());
  StoreArray<CDCTrack> cdcTracksArray(TracksCDCArray.c_str());

  int nSegments = cdcStereoSegmentsArray.GetEntries();
  int nTracks = cdcTracksArray.GetEntries();
  int nextSegment = 0;
  int index = 0;

  for (int j = 0; j < nTracks; j++) {   //loop over all Tracks
    cdcTracksArray[j]->update();
    //search for matching stereo segments in superlayer 8
    nextSegment = FindNextStereoSegment(*cdcTracksArray[j], StereoSegmentsCDCArray.c_str(), 8, 0.05, 0.3);
    if (nextSegment != 9999) {

      //shift the position of each hit of the stereo segment, so that it fits better to the corresponding track
      for (int i = 0; i < cdcStereoSegmentsArray[nextSegment]->getNHits(); i++) {
        index = cdcStereoSegmentsArray[nextSegment]->getTrackHits().at(i).shiftAlongZ(cdcTracksArray[j]->getDirection(), cdcTracksArray[j]->getOuterMostHit());
      }
      //check is the segment if fullfilling a harder distance cut after beeing shifted, only then the segment is added to the track
      nextSegment = FindNextStereoSegment(*cdcTracksArray[j], StereoSegmentsCDCArray.c_str(), 8, 0.0016, 0.3);
      if (nextSegment != 9999) {
        cdcTracksArray[j]->addSegment(*cdcStereoSegmentsArray[nextSegment]);
        cdcStereoSegmentsArray[nextSegment]->setIsUsed(true);

//Unused segments are shifted corresponding to the shift of the first found stereo segment (e.g. its hits). In this way stereo segment belonging to this track can be found more easily
        for (int i = 0; i < nSegments; i++) {//start loop over segments to shift them along the wire
          if (cdcStereoSegmentsArray[nextSegment]->getIsUsed() == false) {
            if (cdcStereoSegmentsArray[i]->getSuperlayerId() == 4) {
              cdcStereoSegmentsArray[i]->shiftAlongZ(index);
            }
            if (cdcStereoSegmentsArray[i]->getSuperlayerId() == 6 || cdcStereoSegmentsArray[i]->getSuperlayerId() == 2) {
              cdcStereoSegmentsArray[i]->shiftAlongZ(10 - index);
            }

          }
        }//end loop over segment to shift them along the wire
      }
    }
//search for matching stereo segments in superlayers 6, 4, 2
    for (int sl = 6; sl > 1; sl = sl - 2) { //loop over stereo superlayers
      nextSegment = FindNextStereoSegment(*cdcTracksArray[j], StereoSegmentsCDCArray.c_str(), sl, 0.05, 0.3);
      if (nextSegment != 9999) {

//shift the position of each hit of the stereo segment, so that it fits better to the corresponding track
        for (int i = 0; i < cdcStereoSegmentsArray[nextSegment]->getNHits(); i++) {
          index = cdcStereoSegmentsArray[nextSegment]->getTrackHits().at(i).shiftAlongZ(cdcTracksArray[j]->getDirection(), cdcTracksArray[j]->getOuterMostHit());
        }
//define different distance cut depending on superlayer
        double maxDistance;
        if (sl == 6) maxDistance = 0.003 ;
        if (sl == 4) maxDistance = 0.007 ;
        if (sl == 2) maxDistance = 0.01 ;

        //check is the segment if fullfilling a harder distance cut after beeing shifted, only then the segment is added to the track
        nextSegment = FindNextStereoSegment(*cdcTracksArray[j], StereoSegmentsCDCArray.c_str(), sl, maxDistance, 0.3);
        if (nextSegment != 9999) {
          cdcTracksArray[j]->addSegment(*cdcStereoSegmentsArray[nextSegment]);
          cdcStereoSegmentsArray[nextSegment]->setIsUsed(true);

        }
      }
    }  //end loop over stereo superlayers
  }//end loop over all tracks

}


