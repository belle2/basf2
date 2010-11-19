/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#include "../include/AxialTrackFinder.h"

#include "CLHEP/Geometry/Point3D.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;

AxialTrackFinder::AxialTrackFinder()
{
}


AxialTrackFinder::~AxialTrackFinder()
{
}

double AxialTrackFinder::ShortestDistance(CDCTrack track, CDCSegment segment)
{
  TVector3 trackPoint;   //starting point of the track
  TVector3 segmentPoint;  //point of the segment
  TVector3 trackDirection = track.getDirection(); //track direction, builds together with track point a "track straight line"
  TVector3 perpDir; //direction perpendicular to track direction
  TVector3 perpPoint; //intersection from "track line" with a to it perpendicular line through segment point

  trackPoint.SetX(track.getOuterMostHit().getConformalX());
  trackPoint.SetY(track.getOuterMostHit().getConformalY());
  trackPoint.SetZ(0);

  segmentPoint.SetX(segment.getOuterMostHit().getConformalX());
  segmentPoint.SetY(segment.getOuterMostHit().getConformalY());
  segmentPoint.SetZ(0);

  //direction perpendicular to track direction
  double perpDirY = -trackDirection.x() / trackDirection.y();
  perpDir.SetX(1);
  perpDir.SetY(perpDirY);
  perpDir.SetZ(0);

  //calculate the intersection point
  double relation = trackDirection.y() / trackDirection.x();

  double enumerator = (segmentPoint.y() - trackPoint.y()) + (trackPoint.x() - segmentPoint.x()) * relation;
  double denominator = perpDir.x() * relation - perpDir.y();

  double factor = enumerator / denominator;
  perpPoint.SetX(segmentPoint.x() + factor*perpDir.x());
  perpPoint.SetY(segmentPoint.y() + factor*perpDir.y());
  perpPoint.SetZ(0);

  //distance between the segment point und the intersection point ( =shortest distance from segment to "track line")
  double distance = (segmentPoint - perpPoint).Mag();

  return distance;
}


int AxialTrackFinder::FindNextSegment(CDCTrack track, string SegmentsCDCArray, int superlayerId, double maxSimpleDistance, double maxDistance, double maxAngle)
{
  //INFO("*********Search for track: "<<track.getId());
  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str());

//start values for the search, just some big enough values
  double minDistance = 3;

  int nSegment = cdcSegmentsArray->GetEntries();
  int bestCandidate;//index of the best candidate in the cdcSegmentsArray

  for (int i = 0; i < nSegment; i++) {
    CDCSegment segmentCandidate = *cdcSegmentsArray[i];

    if (segmentCandidate.getSuperlayerId() == superlayerId) {
//INFO("        SL und ID  " << segmentCandidate.getSuperlayerId() << "  " << segmentCandidate.getId());

//Calculate the distance between the innermost hit of the track and the outermost hit of the segment
      double segment_X = segmentCandidate.getOuterMostHit().getWirePosX();
      double segment_Y = segmentCandidate.getOuterMostHit().getWirePosY();

      double track_X = track.getInnerMostHit().getWirePosX();
      double track_Y = track.getInnerMostHit().getWirePosY();

      double simpleDistance = sqrt(pow((segment_X - track_X), 2) + pow((segment_Y - track_Y), 2));

//Calculate the distance between the segment and the straight line formed by the outermost hit and the direction of the track in the conformal plane
      double distance = ShortestDistance(track, segmentCandidate);
//Calculate the angle between segment and track direction in the conformal plane
      double angle = segmentCandidate.getDirection().Angle(track.getDirection());

      //INFO("Simple Distance: " << simpleDistance);
      //INFO("Distance: " << distance);
      //INFO("Angle: " << angle);

//check if the segment SLID is lower than that of the inner segment of the track
      if (segmentCandidate.getSuperlayerId() < track.getInnerMostSegment().getSuperlayerId()) {
//consider only good and unused segments
        if (segmentCandidate.getIsGood() == true && segmentCandidate.getIsUsed() == false) {
//cut on the distance in the normal plane
          if (simpleDistance < maxSimpleDistance) {
            if (angle < maxAngle) {
//check the angle and the perendicular distance in the conformal plane
              if (distance < minDistance) {
                // INFO("new shortest distance  " << distance)
                minDistance = distance;
                bestCandidate = i;


              }
            }
          }
        }
      }
    }
  }//end of loop over all segments
//cut on the final angle
  if (minDistance < maxDistance) {
    // INFO(" Best candidate  " << cdcSegmentsArray[bestCandidate]->getSuperlayerId() << "  " << cdcSegmentsArray[bestCandidate]->getId());

    return bestCandidate;
  }


  return 9999;
}



void AxialTrackFinder::ConnectSegments(string SegmentsCDCArray, string TracksCDCArray)
{
  StoreArray<CDCTrack> cdcTracksArray(TracksCDCArray.c_str());
  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str());

  int nSegments = cdcSegmentsArray.GetEntries();
  int start = 9;

  int trackcounter = 0; //count the number of tracks

  for (int superlayerId = start; superlayerId > 1; superlayerId = superlayerId - 2) {  //loop over all axial superlayers beginning at the outermost
    for (int i = 0; i < nSegments; i++) {
      if (cdcSegmentsArray[i]->getSuperlayerId() == superlayerId && cdcSegmentsArray[i]->getIsGood() == true && cdcSegmentsArray[i]->getIsUsed() == false) {  //if a good unsused segment in this superlayer is found, a new track is started
        new(cdcTracksArray->AddrAt(trackcounter)) CDCTrack(trackcounter + 1); //create new track
        cdcTracksArray[trackcounter]->addSegment(*cdcSegmentsArray[i]); //add the segment to this new track
        cdcSegmentsArray[i]->setIsUsed(true);
        trackcounter++;
      }
    }


    int nTracks = cdcTracksArray.GetEntries();
    int bestSegment;
    for (int i = 0; i < nTracks; i++) { //loop over all tracks (direct search)

      /*search for a best segment in a superlayer superlayerId-2 for the given track. First value: cut on distance in normal plane,
        second value: cut on perpendicular distance in the conformal plane, third value: cut on angle in conformal plane*/
      bestSegment = FindNextSegment(*cdcTracksArray[i], SegmentsCDCArray.c_str(), superlayerId - 2, 28.2, 0.007, 0.35);

      if (bestSegment != 9999) { //if a convenient segment is found
        cdcTracksArray[i]->addSegment(*cdcSegmentsArray[bestSegment]); //add this segment to the track
        cdcSegmentsArray[bestSegment]->setIsUsed(true);

      }
    }//end loop over all tracks (direct search)

    //Check if there are still segments in this superlayer not successive to one of the tracks
    for (int i = 0; i < nTracks; i++) {//loop over all tracks (indirect search)
      bool direct = false;
      for (int j = 0; j < cdcTracksArray[i]->getNSegments(); j++) {
        if (cdcTracksArray[i]->getSegments().at(j).getSuperlayerId() == superlayerId) {
          direct = true;
        }
      }
      if (direct == false) {

        bestSegment = FindNextSegment(*cdcTracksArray[i], SegmentsCDCArray.c_str(), superlayerId - 2, 50, 0.01 , 0.35);

        if (bestSegment != 9999) {
          cdcTracksArray[i]->addSegment(*cdcSegmentsArray[bestSegment]);
          cdcSegmentsArray[bestSegment]->setIsUsed(true);
        }
      }
    }  //end loop over all tracks (indirect search)
  } //end loop over superlayers
}


