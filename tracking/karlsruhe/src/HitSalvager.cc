/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                           *
 * Copyright(C) 2010 - Belle II Collaboration                           *
 *                                                *
 * Author: The Belle II Collaboration                           *
 * Contributors: Oksana Brovchenko                                *
 *                                                      *
 * This software is provided "as is" without any warranty.              *
**************************************************************************/

#include "../include/HitSalvager.h"

#include <cmath>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/logging/Logger.h>

#include <tracking/karlsruhe/AxialTrackFinder.h>


using namespace std;
using namespace Belle2;

HitSalvager::HitSalvager()
{
}


HitSalvager::~HitSalvager()
{
}

double HitSalvager::ShortestDistance(CDCTrack track, CDCTrackHit hit)
{
  TVector3 trackPoint;   //starting point of the track
  TVector3 hitPoint;  //hit position
  TVector3 trackDirection = track.getDirection(); //track direction, builds together with track point a "track straight line"

  trackPoint.SetX(track.getOuterMostHit().getConformalX());
  trackPoint.SetY(track.getOuterMostHit().getConformalY());
  trackPoint.SetZ(0);

  hitPoint.SetX(hit.getConformalX());
  hitPoint.SetY(hit.getConformalY());
  hitPoint.SetZ(0);

  double distance = AxialTrackFinder::ShortestDistance(trackPoint, trackDirection, hitPoint);

  return distance;
}

bool HitSalvager::CheckSuperlayer(CDCTrack track, CDCTrackHit hit)
{
  bool already = false;
  int hit_sl = hit.getSuperlayerId();
  for (int i = 0; i < track.getNSegments(); i++) {
    if (track.getSegments().at(i).getSuperlayerId() == hit_sl) {
      already = true;
    }
  }

  return already;
}


void HitSalvager::SalvageHits(string SegmentsCDCArray, string TracksCDCArray, double maxDistance)
{

  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str());
  StoreArray<CDCTrack> cdcTracksArray(TracksCDCArray.c_str());

  int count = 0;   //variable to count the number of Tracks below the maxDistance gut
  int bestCandidate;  //index of the best candidate in the cdcTracksArray

  int NTracks = cdcTracksArray->GetEntries();

  for (int j = 0; j < cdcSegmentsArray->GetEntries(); j++) { //loop over all Segments
    if (cdcSegmentsArray[j]->getIsUsed() == false) {  //consider only unused Segments
      for (int hit = 0; hit < cdcSegmentsArray[j]->getNHits(); hit++) {  //loop over all hits
        CDCTrackHit aHit = cdcSegmentsArray[j]->getTrackHits().at(hit);
        if (aHit.getIsUsed() == false) {//consider only unused Hits
          count = 0; //reset the counter for each hit

          for (int i = 0; i < NTracks; i++) {  //loop over all tracks
            if (CheckSuperlayer(*cdcTracksArray[i], aHit) == false) {
              //Calculate the distance between the segment and the straight line formed by the outermost hit and the direction of the track in the conformal plane
              double distance = ShortestDistance(*cdcTracksArray[i], aHit);
              //check the distance
              if (distance < maxDistance) {
                count ++;  //count how many tracks fullfilled the cut
                bestCandidate = i;
              }
            }
          }//end loop over all tracks
        } //endif unused hits

//Add the hit to the track if only exact one corresponding track was found
        if (count == 1) {

//shift the position of the stereo hit, so that it fits better to the corresponding track
          if (aHit.getIsAxial() == false) {
            aHit.shiftAlongZ(cdcTracksArray[bestCandidate]->getDirection(), cdcTracksArray[bestCandidate]->getOuterMostHit());
          }

          cdcTracksArray[bestCandidate]->addTrackHit(aHit);
          cdcSegmentsArray[j]->getTrackHits()[hit].setIsUsed(true);
        }  //endif count == 1


      }  //end loop over all hits
    } //endif unused segments
  } //end loop over all segments


}



