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

#include <cmath>

#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/logging/Logger.h>

#include <tracking/karlsruhe/AxialTrackFinder.h>

#define pi 3.141592654

using namespace std;
using namespace Belle2;

StereoFinder::StereoFinder()
{
}

StereoFinder::~StereoFinder()
{
}

double StereoFinder::ShortestDistance(CDCTrack track, CDCSegment segment)
{
  return AxialTrackFinder::ShortestDistance(track.getOuterMostSegment(),
                                            segment);
}

double StereoFinder::SimpleDistance(CDCTrack track, CDCSegment segment)
{
  return AxialTrackFinder::SimpleDistance(track.getOuterMostSegment(),
                                          segment);
}

void StereoFinder::FindStereoSegments(CDCTrack startTrack,
                                      string SegmentsCDCArray, double SimpleDistanceCut,
                                      double ShortDistanceCut, int SLId)
{

  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str()); //Input segments
  int nSegments = cdcSegmentsArray.GetEntries();

  for (int i = 0; i < nSegments; i++) { //loop over all segments
    //B2INFO("SL: "<<cdcSegmentsArray[i]->getSuperlayerId()<<"  Simple Distance: "<<SimpleDistance(startTrack, *cdcSegmentsArray[i]));
    //first necessary neighbouring condition and check the correct superlayer
    if (cdcSegmentsArray[i]->getSuperlayerId() == SLId && SimpleDistance(
          startTrack, *cdcSegmentsArray[i]) < SimpleDistanceCut
        && cdcSegmentsArray[i]->getIsGood() == true) {
      //the decisive neighbouring condition
      if (ShortestDistance(startTrack, *cdcSegmentsArray[i])
          < ShortDistanceCut) {
        //add the Id of the track candidate to the segment
        cdcSegmentsArray[i]->setTrackCandId(startTrack.getId());
        //B2INFO("Possible stereo Candidate for track "<<startTrack.getId()<<"  in SL "<<SLId);

      }
    }

  }//end loop over all segments
}

bool StereoFinder::SegmentFromOvercrowdedSL(CDCTrack track, int SegmentIndex)
{

  bool overcrowded = false;
  int counter = 0;
  int SL = track.getSegments().at(SegmentIndex).getSuperlayerId();

  for (int i = 0; i < track.getNSegments(); i++) { //count if there is more the one segment with the same SLId in this candidate
    if (track.getSegments().at(i).getSuperlayerId() == SL) {
      counter++;
    }
  }
  if (counter > 1)
    overcrowded = true;

  return overcrowded;
}

void StereoFinder::FitCandidates(CDCTrack & candidate)
{
  if (candidate.getNSegments() > 0) {
    TGraph * graph;
    double x[100];
    double y[100];
    int nHits = candidate.getNHits(); //Nr of hits in a track
    bool refit = true;

    while (refit == true) { //while loop to refit the candidate if one segment is removed
      refit = false;
      //loop over all hits and get their conformal coordinates
      for (int j = 0; j < nHits; j++) {
        x[j] = candidate.getTrackHits().at(j).getConformalX();
        y[j] = candidate.getTrackHits().at(j).getConformalY();
      }

      graph = new TGraph(nHits, x, y);

      double min = graph->GetXaxis()->GetXmin();
      double max = graph->GetXaxis()->GetXmax();

      //Fit the conformal coordinates with simple linear fit
      graph->Fit("pol1", "Q", "", min, max);
      TF1 *fit = graph->GetFunction("pol1");

      double Chi = fit->GetChisquare();

      //candidate.setChiSquare(Chi);
      //Check the Chi2 of the fit, if it is too bad search for segment to remove
      if (Chi > 0.0000001) {
        //B2INFO("Search for bad stereo segments to remove");
        TVector3 fitPoint;   //some point on the fit line
        fitPoint.SetX(x[0]);
        fitPoint.SetY(fit->GetParameter(1) * x[0]);
        fitPoint.SetZ(0);

        for (int j = 0; j < candidate.getNSegments(); j++) { //loop over all segments and check their distance to the fit line

          TVector3 segmentPoint;
          segmentPoint.SetX(
            candidate.getSegments().at(j).getOuterMostHit().getConformalX());
          segmentPoint.SetY(
            candidate.getSegments().at(j).getOuterMostHit().getConformalY());
          segmentPoint.SetZ(0);
          TVector3 segmentDirection =
            candidate.getSegments().at(j).getDirection();

          double distance = AxialTrackFinder::ShortestDistance(
                              segmentPoint, segmentDirection, fitPoint);

          //B2INFO("Distance: "<<distance);
          //Check the distance. If there are several segments from this superlayer assigned to this candidate, more strict cut is applied.
          if ((distance > 0.02 || (SegmentFromOvercrowdedSL(
                                     candidate, j) == true && distance > 0.008))
              && candidate.getNSegments() > 1) {

            int badId = candidate.getSegments().at(j).getId();
            //remove bad segment
            candidate.removeSegment(badId);

            //track to be fit again
            refit = true;

          }

        }
      }
    }  //end while

  }
}

void StereoFinder::AppendStereoSegments(string StereoSegmentsCDCArray,
                                        string TracksCDCArray)
{

  StoreArray<CDCSegment> cdcStereoSegmentsArray(
    StereoSegmentsCDCArray.c_str()); //input segments
  StoreArray<CDCTrack> cdcTracksArray(TracksCDCArray.c_str()); //output track candidates

  int nSegments = cdcStereoSegmentsArray.GetEntries();
  int nTracks = cdcTracksArray.GetEntries();

  vector<CDCTrack> StereoCandidates; //vector to hold track candidates with only the stereo segments

  //create a TrackCandidate with the same Id as in the given StoreArray and place it in vector
  for (int i = 0; i < nTracks; i++) {
    CDCTrack track(i);
    StereoCandidates.push_back(track);
  }

  int SL = 8; //start superlayer
  double simpleCut = 30; //cut on the simple distance
  double shortDistanceCut = 0.05 ; //cut on the 'short' distance for the first search loop
  double strictCut = 0.005 ; //cut on the 'short' distance for the final decision (after shifting the stereo segments)
  double angleCut = 0.3 ; //cut on the angle between the track candidate and the segment
  //int index = 0;

  while (SL > 1) { //loop over all stereo superlayers

    for (int i = 0; i < nTracks; i++) { //loop over all Tracks

      //recalculate the cut depending on the superlayer difference
      int SLDiff = abs(cdcTracksArray[i]->getOuterMostSegment().getSuperlayerId() - SL);
      simpleCut = 30 + (SLDiff * 10 - 10);

      //search for matching stereo segments in the given superlayer
      FindStereoSegments(*cdcTracksArray[i],
                         StereoSegmentsCDCArray.c_str(), simpleCut, shortDistanceCut, SL);

    } //end loop over all tracks

    for (int j = 0; j < nSegments; j++) { //loop over all segments

      CDCSegment segment = *cdcStereoSegmentsArray[j];


      for (unsigned int k = 0; k < segment.getTrackCandId().size(); k++) { //loop over all track candidate to which this segment may belong
        int trackId = segment.getTrackCandId().at(k);

        //shift all hits in this segment according to the given track candidate
        for (int i = 0; i < segment.getNHits(); i++) {

          segment.getTrackHits().at(i).shiftAlongZ(
            cdcTracksArray[segment.getTrackCandId().at(
                             k)]->getDirection(),
            cdcTracksArray[segment.getTrackCandId().at(
                             k)]->getOuterMostHit());

        }
        //check if now the segment can pass more strict cuts

        double angle = cdcTracksArray[trackId]->getOuterMostSegment().getDirection().Angle(segment.getDirection());
        if (angle > pi / 2) angle = angle - pi; // -90 < angle < 90

        if (ShortestDistance(*cdcTracksArray[trackId], segment) < strictCut
            && abs(angle) < angleCut) {
          //B2INFO("    stereo segment added")
          StereoCandidates.at(trackId).addSegment(segment);
        }

      } //end loop over track candidates
      cdcStereoSegmentsArray[j]->clearTrackCandId();

    } //end loop over all segments

    SL = SL - 2;

  } //end while loop


  //additional quality check: fit the temporarily created candidates consisting only from stereo segments
  for (int i = 0; i < nTracks; i++) {

    FitCandidates(StereoCandidates.at(i));
    //B2INFO("Chi2: "<<StereoCandidates.at(i).getChiSquare());
  }

  //Add the final stereo segments to the corresponding track candidates
  //B2INFO("Add stereo segments to the track candidates");
  for (int i = 0; i < nTracks; i++) {
    for (int j = 0; j < StereoCandidates.at(i).getNSegments(); j++) {
      cdcTracksArray[i]->addSegment(
        StereoCandidates.at(i).getSegments().at(j));
    }
  }

}


