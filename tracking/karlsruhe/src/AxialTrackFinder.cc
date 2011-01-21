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


#include <cmath>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/logging/Logger.h>


#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"

#define pi 3.141592654

using namespace std;
using namespace Belle2;


AxialTrackFinder::AxialTrackFinder()
{
}

AxialTrackFinder::~AxialTrackFinder()
{
}


double AxialTrackFinder::ShortestDistance(TVector3 firstPoint, TVector3 firstDirection, TVector3 secondPoint)
{

  TVector3 perpDir; //direction perpendicular to firstDirection
  TVector3 perpPoint; //intersection from "first line" with a to it perpendicular line through second point

  //direction perpendicular to first line direction
  double perpDirY = -firstDirection.x() / firstDirection.y();
  perpDir.SetX(1);
  perpDir.SetY(perpDirY);
  perpDir.SetZ(0);

  //calculate the intersection point
  double relation = firstDirection.y() / firstDirection.x();

  double enumerator = (secondPoint.y() - firstPoint.y())
                      + (firstPoint.x() - secondPoint.x()) * relation;
  double denominator = perpDir.x() * relation - perpDir.y();

  double factor = enumerator / denominator;
  perpPoint.SetX(secondPoint.x() + factor * perpDir.x());
  perpPoint.SetY(secondPoint.y() + factor * perpDir.y());
  perpPoint.SetZ(0);

  //B2INFO("Intersection Point: "<<perpPoint.X()<<"  "<<perpPoint.Y()<<"  "<<perpPoint.Z());

  //distance between the second point und the intersection point ( = shortest distance from second point to "first line")
  double distance = (secondPoint - perpPoint).Mag();

  return distance;
}


double AxialTrackFinder::ShortestDistance(CDCSegment segment1,
                                          CDCSegment segment2)
{
  TVector3 FirstSegmentPoint; //starting point of the one segment
  TVector3 SecondSegmentPoint; //point of the  other segment
  TVector3 FirstSegmentDirection = segment1.getDirection(); //direction of the first segment, builds together with starting point a "segment straight line" in the conformal plane


  FirstSegmentPoint.SetX(segment1.getOuterMostHit().getConformalX());
  FirstSegmentPoint.SetY(segment1.getOuterMostHit().getConformalY());
  FirstSegmentPoint.SetZ(0);


  SecondSegmentPoint.SetX(segment2.getOuterMostHit().getConformalX());
  SecondSegmentPoint.SetY(segment2.getOuterMostHit().getConformalY());
  SecondSegmentPoint.SetZ(0);

  double distance = ShortestDistance(FirstSegmentPoint, FirstSegmentDirection, SecondSegmentPoint);

  return distance ;

}



double AxialTrackFinder::SimpleDistance(CDCSegment segment1,
                                        CDCSegment segment2)
{

  double segment1_X = (segment1.getOuterMostHit().getWirePosX()
                       + segment1.getInnerMostHit().getWirePosX()) / 2;
  double segment1_Y = (segment1.getOuterMostHit().getWirePosY()
                       + segment1.getInnerMostHit().getWirePosY()) / 2;

  double segment2_X = (segment2.getOuterMostHit().getWirePosX()
                       + segment2.getInnerMostHit().getWirePosX()) / 2;
  double segment2_Y = (segment2.getOuterMostHit().getWirePosY()
                       + segment2.getInnerMostHit().getWirePosY()) / 2;

  double simpleDistance = sqrt(pow((segment1_X - segment2_X), 2) + pow(
                                 (segment1_Y - segment2_Y), 2));

  return simpleDistance;
}


float AxialTrackFinder::UsedSegmentsFraction(CDCTrack track,
                                             vector<int> & UsedSegmentId)
{

  int used = 0;

  for (int i = 0; i < track.getNSegments(); i++) { //loop over alle segment and compare their Ids to those which were already used
    for (unsigned int j = 0; j < UsedSegmentId.size(); j++) {

      if (track.getSegments().at(i).getId() == UsedSegmentId.at(j)) {
        used++;
      }
    }
  }

  float fraction = float(used / track.getNSegments());

  return fraction;

}


vector<CDCTrack> AxialTrackFinder:: FindTrackCandidates(string SegmentsCDCArray, double SimpleDistanceCut, double ShortDistanceCut, int StartSLId)
{

  vector<CDCTrack> TrackCandidates;

  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str()); //Input segments

  int nSegments = cdcSegmentsArray.GetEntries();
  int NTracks = 0;
  int SLId = StartSLId; //start value for the superlayerId
  int trackcounter = -1; //count the number of track candidates

  //Step 1: Count all good segments in the start superlayer, create the same number of track candidates and assign the segment from the start superlayer to them

  //loop over all segment and count good segments in the given superlayer ( = starting points for the track candidates)
  for (int i = 0; i < nSegments; i++) {
    CDCSegment* segmentCandidate = cdcSegmentsArray[i];
    if (segmentCandidate->getSuperlayerId() == StartSLId && segmentCandidate->getIsGood() == true) {

      trackcounter++;
      segmentCandidate->setTrackCandId(trackcounter);

    }
  }//end loop over all segments

  //B2INFO("****Create Tracks****");
  for (int i = 0; i <= trackcounter; i++) {
    //create a TrackCandidate and place it in the corresponding vector
    CDCTrack track(i);
    TrackCandidates.push_back(track);

    //loop over all segments, add segments with correct SLId (= StartSlId) and corresponding TrackCandId to created track
    for (int j = 0; j < nSegments; j++) {
      for (unsigned k = 0; k  < cdcSegmentsArray[j]->getTrackCandId().size(); k++) {

        if (cdcSegmentsArray[j]->getSuperlayerId() == StartSLId
            && cdcSegmentsArray[j]->getTrackCandId().at(k) == i) {

          TrackCandidates.at(i).addSegment(*cdcSegmentsArray[j]);
        }
      }
    }//end loop over all segments
  } //end loop over over all track candidates


  //Step 2: search for further segments for already created tracks in the superlayers below

  while (SLId > 0) {  //while loop over all axial superlayers

    //loop over all Segments, if two neighbouring segments are found, the TrackCandId from the 'higher' one is assigned to the 'lower' one
    for (int i = 0; i < nSegments; i++) {
      CDCSegment* segmentCandidate = cdcSegmentsArray[i];

      for (int j = 0; j < nSegments; j++) {
        //first necessary neighbouring conditions
        if (cdcSegmentsArray[j]->getSuperlayerId()
            == segmentCandidate->getSuperlayerId() - 2
            && SimpleDistance(*segmentCandidate, *cdcSegmentsArray[j]) < SimpleDistanceCut
            && cdcSegmentsArray[j]->getIsGood() == true) {
          //the decisive neighbouring condition
          if (ShortestDistance(*segmentCandidate,
                               *cdcSegmentsArray[j]) < ShortDistanceCut) {

            cdcSegmentsArray[j]->setTrackCandId(segmentCandidate->getTrackCandId()); //copy the TrackCandId from one segment to another
          }
        }

      }

    }//end loop over all segments

    NTracks = TrackCandidates.size();

    //Now after the correct TrackCandIds are assigned, the corresponding segments can be added to the tracks

    for (int i = 0; i < NTracks; i++) { //loop over all track candidates
      CDCTrack candidate = TrackCandidates.at(i);

      for (int j = 0; j < nSegments; j++) { //loop over all segments

        //boolean to mark the case, when two different segments from the same superlayer are matching the same track candidate
        //if already == true, the track candidate should be split in two different candidates
        bool already = false;
        for (unsigned k = 0; k
             < cdcSegmentsArray[j]->getTrackCandId().size(); k++) { //loop over all TrackCandIds assigned to this segment
          if (cdcSegmentsArray[j]->getSuperlayerId() == SLId - 2
              && cdcSegmentsArray[j]->getTrackCandId().at(k) == i) {

            for (int sl = 0; sl
                 < TrackCandidates.at(i).getNSegments(); sl++) { //check, if there were already another segment from the same superlayer assigned to this candidate
              if (cdcSegmentsArray[j]->getSuperlayerId()
                  == TrackCandidates.at(i).getSegments().at(
                    sl).getSuperlayerId()) {

                already = true;
              }
            }

            if (already == false) {
              TrackCandidates.at(i).addSegment(
                *cdcSegmentsArray[j]);

            } else { // Split the track candidate

              //create a new candidate by copying the old one (new Id) and adding to it the new segment
              trackcounter++;

              CDCTrack SplitTrack(candidate, trackcounter);
              TrackCandidates.push_back(SplitTrack);

              TrackCandidates.at(trackcounter).addSegment(
                *cdcSegmentsArray[j]);
              cdcSegmentsArray[j]->setTrackCandId(trackcounter);

            }
          } //endif

        }  //end for loop over all TrackCandIds
      }

    } //end for loop over all track candidates

    SLId = SLId - 2; //go one axial supelayer lower

  } //end while loop over all axial superlayers


  return TrackCandidates;
}


void AxialTrackFinder::FitTrackCandidate(CDCTrack & candidate)
{

  TGraph * graph;
  double x[100];
  double y[100];
  int nHits = candidate.getNHits(); //Nr of hits in a track
  int temp_nHits; //Nr of hit in a track *excluding* the hits in the first superlayer
  temp_nHits = 0;

  //loop over all hits and get their conformal coordinates (exclude first layer!)
  for (int j = 0; j < nHits; j++) {

    if (candidate.getTrackHits().at(j).getSuperlayerId() != 1) {
      temp_nHits++;
      x[j] = candidate.getTrackHits().at(j).getConformalX();
      y[j] = candidate.getTrackHits().at(j).getConformalY();

    }
  }

  graph = new TGraph(temp_nHits, x, y);

  double min = graph->GetXaxis()->GetXmin();
  double max = graph->GetXaxis()->GetXmax();

  //Fit the conformal coordinates with simple linear fit
  graph->Fit("pol1", "Q", "", min, max);
  TF1 *fit = graph->GetFunction("pol1");

  double Chi = fit->GetChisquare();

  candidate.setChiSquare(Chi);

  //First estimation of the absolut momentum value
  double x0 = -fit->GetParameter(0) / fit->GetParameter(1);  // x- and y - axis section
  double y0 = fit->GetParameter(0);
  double R = sqrt(1 / x0 * 1 / x0 + 1 / y0 * 1 / y0); //Radius of the track circle in the normal plane
  double p = R * 1.5 / 299.792458;  //Preliminary(!!!) calculation of momentum in GeV, magnetic field and c wont be hardcoded in the future
  //B2INFO("x: "<<x0<<"  y: "<<y0<<"  R: "<<R<<"  p: "<<p);
  candidate.setMomentumValue(p);

}



void AxialTrackFinder::FitTrackCandidates(vector<CDCTrack> & candidates)
{

  for (unsigned int i = 0; i < candidates.size(); i++) {
    FitTrackCandidate(candidates.at(i));
  }
}

void AxialTrackFinder::CollectTrackCandidates(string SegmentsCDCArray,
                                              string TracksCDCArray)
{

  StoreArray<CDCSegment> cdcSegmentsArray(SegmentsCDCArray.c_str()); //Input Segments
  StoreArray<CDCTrack> cdcTracksArray(TracksCDCArray.c_str()); //Output TrackCandidates

  vector<CDCTrack> TrackCandidates;  //vector to hold Track Candidates, is emptied and refilled for each start superlayer
  vector<CDCTrack> FinalTrackCandidates;  //vector to gather Track Candidates (who passed the Chi^2 test) from all start superlayers

  int nSegments = cdcSegmentsArray.GetEntries();
  int NTracks = 0;

  vector<int> UsedSegmentId; //vector to store the Ids of segments, which were already used to reconstruct a track candidate

  int startSLId = 9;

  //int trackcounter = -1; //count the number of track candidates
  int counter = -1;

  B2INFO("Find possible track candidates...");

  //loop over all possible starting superlayers, starting with the outermost superlayer
  //first all track candidates are found, after a simple fit those with too large Chi2 are discarded, all the other candidates are collected in FinalTrackCandidates
  while (startSLId > 1) {

    TrackCandidates = FindTrackCandidates("AxialSegmentsCDCArray", 28, 0.005,
                                          startSLId);

    NTracks = TrackCandidates.size();
    //B2INFO("Number of TrackCandidates starting in SL "<<startSLId<<" : "<<NTracks);

    FitTrackCandidates(TrackCandidates);

    //Discard bad chi2 and tracks with unrealistic momentum
    for (int i = 0; i < NTracks; i++) {
      //B2INFO("Track Nr "<<i<<"  Chi2: "<<TrackCandidates.at(i).getChiSquare());
      if (TrackCandidates.at(i).getChiSquare() < 0.00009 && TrackCandidates.at(i).getMomentumValue() < 15) { //what value???
        FinalTrackCandidates.push_back(TrackCandidates.at(i));
      }

    }
    //B2INFO("Number of TrackCandidates after Chi2 test: "<<FinalTrackCandidates.size());

    startSLId = startSLId - 2;  //go one axial supelayer lower

    //cleanup
    TrackCandidates.erase(TrackCandidates.begin(), TrackCandidates.end());
    for (int i = 0; i < nSegments; i++) {
      cdcSegmentsArray[i]->clearTrackCandId();
    }

  }//end while loop over superlayers


  B2INFO("Select the best track candidates...");
  //Now the best ( = longest) TrackCandidates are chosen from the FinalTrackCandidates

  int NFinalTracks = FinalTrackCandidates.size();

  int NumberOfSegments = 5; //maximum possible track candidate length

  while (NumberOfSegments > 1) {  //loop over possible track lengths
    for (int i = 0; i < NFinalTracks; i++) {  //loop over all track candidates

      //check for correct number of segments and the fraction of already used segments
      if (FinalTrackCandidates.at(i).getNSegments() == NumberOfSegments
          && UsedSegmentsFraction(FinalTrackCandidates.at(i),
                                  UsedSegmentId) < 0.3) {
        counter++;
        //B2INFO("Nr "<<counter<<"   Final track with "<<NumberOfSegments<<" found");

        //Add the Ids of all used segments to the UsedSegmentId vector
        for (int j = 0; j < FinalTrackCandidates.at(i).getNSegments(); j++) {
          UsedSegmentId.push_back(
            FinalTrackCandidates.at(i).getSegments().at(j).getId());
        }

        //extension for special cases, where short tracks found separately can be combined to one track

        if (NumberOfSegments == 3 || NumberOfSegments == 2) {
          int additionalSegments = 2;

          while (additionalSegments > 0) { //while loop, to be performed two times, first searching for 2 segment tracks, then 1 segment tracks
            for (int test = 0; test < NFinalTracks; test++) {  //loop over all candidates
              //check for right segments number and used fraction
              if (test != i
                  && FinalTrackCandidates.at(test).getNSegments()
                  == additionalSegments
                  && UsedSegmentsFraction(
                    FinalTrackCandidates.at(test),
                    UsedSegmentId) < 0.3) {

                //additional variable to check if the short tracks can be combined to one
                double angle = FinalTrackCandidates.at(test).getSegments().at(0).getDirection().Angle(FinalTrackCandidates.at(i).getSegments().at(0).getDirection());
                if (angle > pi / 2) angle = angle - pi ; // -90 < angle < 90

                //check for shortest distance and angle
                if (ShortestDistance(
                      FinalTrackCandidates.at(test).getSegments().at(
                        0),
                      FinalTrackCandidates.at(i).getSegments().at(
                        0)) < 0.005 && abs(angle) < 0.3) {
                  //B2INFO("One short track added to the candididate");

                  //Add all segment from one short track to the other and mark segment Ids as used
                  for (int s = 0; s
                       < FinalTrackCandidates.at(test).getNSegments(); s++) {

                    FinalTrackCandidates.at(i).addSegment(FinalTrackCandidates.at(test).getSegments().at(s));
                    UsedSegmentId.push_back(
                      FinalTrackCandidates.at(test).getSegments().at(
                        s).getId());

                  }
                  //Refit the candidate
                  FitTrackCandidate(FinalTrackCandidates.at(i));

                }
              }
            }  //loop over all candidates
            additionalSegments--;
          }
        } //end extension

        //Finally add the candidate to the StoreArray
        new(cdcTracksArray->AddrAt(counter)) CDCTrack(
          FinalTrackCandidates.at(i), counter);
        B2INFO("Candidate " << counter << " added to final array ...");
      }
    } //end for loop over all candidates

    NumberOfSegments--; //decrease required track length

  } //end while

}





