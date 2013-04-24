/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/AxialTrackFinder.h"

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <cmath>

#include "TMath.h"
#include "TGraph.h"
#include "TFile.h"
#include "TAxis.h"
#include "TF1.h"

using namespace std;
using namespace Belle2;


AxialTrackFinder::AxialTrackFinder()
{
}

AxialTrackFinder::~AxialTrackFinder()
{
}


double AxialTrackFinder::ShortestDistance(TVector3& firstPoint, TVector3& firstDirection, TVector3& secondPoint)
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


  //distance between the second point und the intersection point ( = shortest distance from second point to "first line")
  double distance = (secondPoint - perpPoint).Mag();

  return distance;
}


double AxialTrackFinder::ShortestDistance(CDCSegment& segment1,
                                          CDCSegment& segment2)
{
  TVector3 FirstSegmentPoint; //starting point of the one segment
  TVector3 SecondSegmentPoint; //point of the  other segment
  TVector3 FirstSegmentDirection = segment1.getDirection(); //direction of the first segment, builds together with starting point a "segment straight line" in the conformal plane


  FirstSegmentPoint.SetX(segment1.getInnerMostHit().getConformalX());
  FirstSegmentPoint.SetY(segment1.getInnerMostHit().getConformalY());
  FirstSegmentPoint.SetZ(0);


  SecondSegmentPoint.SetX(segment2.getOuterMostHit().getConformalX());
  SecondSegmentPoint.SetY(segment2.getOuterMostHit().getConformalY());
  SecondSegmentPoint.SetZ(0);

  double distance = ShortestDistance(FirstSegmentPoint, FirstSegmentDirection, SecondSegmentPoint);

  return distance ;

}



double AxialTrackFinder::SimpleDistance(CDCSegment& segment1,
                                        CDCSegment& segment2)
{
  //simple distance is calculated as a difference between the innermost hits of one segment and the outermost hit of the other segment
  //as it is not known which segment is closer to the origin, both possibilities are calculated and the shortest one is taken

  //get the xy position of the innermost and outermost hits of the first segment
  double segment1_oX = segment1.getOuterMostHit().getWirePosition().x();
  double segment1_oY = segment1.getOuterMostHit().getWirePosition().y();
  double segment1_iX = segment1.getInnerMostHit().getWirePosition().x();
  double segment1_iY = segment1.getInnerMostHit().getWirePosition().y();

  //get the xy position of the innermost and outermost hits of the second segment
  double segment2_oX = segment2.getOuterMostHit().getWirePosition().x();
  double segment2_oY = segment2.getOuterMostHit().getWirePosition().y();
  double segment2_iX = segment2.getInnerMostHit().getWirePosition().x();
  double segment2_iY = segment2.getInnerMostHit().getWirePosition().y();


  double simpleDistance1 = sqrt(pow((segment1_oX - segment2_iX), 2) + pow(
                                  (segment1_oY - segment2_iY), 2));

  double simpleDistance2 = sqrt(pow((segment1_iX - segment2_oX), 2) + pow(
                                  (segment1_iY - segment2_oY), 2));

  if (simpleDistance1 < simpleDistance2) return simpleDistance1;
  else return simpleDistance2;

}


float AxialTrackFinder::UsedSegmentsFraction(CDCTrackCandidate& track,
                                             vector<int> & UsedSegmentId)
{

  int used = 0;

  for (int i = 0; i < track.getNSegments(); i++) { //loop over all segments and compare their Id's to those which were already used

    for (unsigned int j = 0; j < UsedSegmentId.size(); j++) {

      if (track.getSegments().at(i).getId() == UsedSegmentId.at(j)) { //if the given segment is found in the usedSegments vector, increase used an stop
        used++;
        break;
      }
    }
  }
  //calculate the fraction of used segments compared to all segments in this track candidate
  float fraction = float(used) / float(track.getNSegments());

  return fraction;

}

float AxialTrackFinder::UsedHitsFraction(CDCTrackCandidate& track,
                                         vector<int> & UsedTrackHitId)
{

  int used = 0;

  for (int i = 0; i < track.getNHits(); i++) { //loop over all hits and compare their Id's to those which were already used

    for (unsigned int j = 0; j < UsedTrackHitId.size(); j++) {

      if (track.getTrackHits().at(i).getStoreIndex() == UsedTrackHitId.at(j)) {//if the given hit is found in the usedHits vector, increase used an stop
        used++;
        break;
      }
    }
  }

  float fraction = float(used) / float(track.getNHits());

  return fraction;

}

int AxialTrackFinder::EvaluateBestCandidate(const vector<CDCTrackCandidate>& candidates, const vector <int>& indices)
{
  double minChi2 = 10;
  int minIndex = 999;
  //select the candidate with the lowest chi2
  for (unsigned int i = 0; i < indices.size(); i ++) {
    if (candidates.at(indices.at(i)).getChiSquare() < minChi2) {
      minChi2 = candidates.at(indices.at(i)).getChiSquare();
      minIndex = indices.at(i);
    }
  }
  B2DEBUG(150, "EvaluateBestCandidate: Minimal chi2 " << minChi2 << "  for CDCTrackCandidate " << minIndex);

  return minIndex;
}



vector<CDCTrackCandidate> AxialTrackFinder:: FindTrackCandidates(vector<CDCSegment> & cdcAxialSegments, double SimpleDistanceCut, double AngleCut, double ShortDistanceCut, int StartSLId)
{

  double myAngleCut = AngleCut;
  double myShortDistanceCut = ShortDistanceCut;
  double mySimpleDistanceCut = SimpleDistanceCut;
  vector<CDCTrackCandidate> TrackCandidates;

  int nSegments = cdcAxialSegments.size();
  int NTracks = 0;
  int SLId = StartSLId; //start value for the superlayerId
  int trackcounter = -1; //count the number of track candidates

  vector<CDCSegment> segmentArray [9]; //array to store TrackHit vectors, with track hits belonging to only one superlayer
  for (int sl = 0; sl < 9 ; ++sl) {
    for (int i = 0; i < nSegments; ++i) {
      if (cdcAxialSegments.at(i).getSuperlayerId() == sl) {
        segmentArray[sl].push_back(cdcAxialSegments.at(i));
      }
    }
  }


  //Step 1: Count all segments in the start superlayer, create the same number of track candidates and assign the segment from the start superlayer to them


  for (unsigned int i = 0; i < segmentArray[StartSLId].size(); i++) {
    trackcounter++;
    segmentArray[StartSLId].at(i).setTrackCandId(trackcounter);

  }//end loop over all segments

  TrackCandidates.reserve(2 * (trackcounter > 0 ? trackcounter : 1));
  for (int i = 0; i <= trackcounter; i++) {

    //create a TrackCandidate and place it in the corresponding vector
    TrackCandidates.push_back(CDCTrackCandidate(i));

    for (unsigned int j = 0; j < segmentArray[StartSLId].size(); j++) {
      for (unsigned k = 0; k  < segmentArray[StartSLId].at(j).getTrackCandId().size(); k++) {
        if (segmentArray[StartSLId].at(j).getTrackCandId().at(k) == i) {
          TrackCandidates.at(i).addSegment(segmentArray[StartSLId].at(j));
          B2DEBUG(200, "Create Track " << i << "  with Segment " << cdcAxialSegments[j].getId() << "  from SL " << cdcAxialSegments[j].getSuperlayerId());
        }
      }
    }//end loop over all segments

  } //end loop over over all track candidates


  //Step 2: search for further segments for already created tracks in the superlayers below

  while (SLId > 1) {  //while loop over all axial superlayers

    //loop over all Segments, if two neighbouring segments are found, the TrackCandId from the 'higher' one is assigned to the 'lower' one
    for (unsigned int i = 0; i < segmentArray[SLId].size(); i++) {
      //while searching the segment in the innermost superlayer it seems to be better to use different cuts
      //it should be done in a more general way in the future, but for now here are just some hardcoded values
      if (segmentArray[SLId].at(i).getSuperlayerId() == 2) {
        myShortDistanceCut = 0.015;
        myAngleCut = 0.5;
      } else {
        myShortDistanceCut = ShortDistanceCut;
        myAngleCut = AngleCut;
      }

      for (unsigned int j = 0; j < segmentArray[SLId - 2].size(); j++) {
        //B2INFO("Check Segment "<<segmentArray[SLId-2].at(j).getId()<<" from SL "<<SLId-2<<" with WireId "<<segmentArray[SLId-2].at(j).getInnerMostHit().getWireId());
        double angle = segmentArray[SLId].at(i).getDirection().Angle(segmentArray[SLId - 2].at(j).getDirection());
        if (angle > TMath::Pi() / 2)  angle = angle - TMath::Pi(); // -90 < angle < 90
        //first necessary neighbouring conditions
        //B2INFO("SimpleDistance: "<<SimpleDistance(*segmentCandidate, cdcAxialSegments[j])<<"  Angle: "<<abs(angle)<<"  ShortestDistance: "<<ShortestDistance(*segmentCandidate,cdcAxialSegments[j]));
        if (SimpleDistance(segmentArray[SLId].at(i), segmentArray[SLId - 2].at(j)) < mySimpleDistanceCut && abs(angle) < myAngleCut) {
          //the decisive neighbouring condition
          if (ShortestDistance(segmentArray[SLId].at(i),
                               segmentArray[SLId - 2].at(j)) < myShortDistanceCut) {
            segmentArray[SLId - 2].at(j).setTrackCandId(segmentArray[SLId].at(i).getTrackCandId()); //copy the TrackCandId from one segment to another
            //B2INFO("Is a neighbour");
          }
        }
        //B2INFO("  Is not a neighbour");
      }

    }//end loop over all segments

    NTracks = TrackCandidates.size();
    //B2INFO("NTtracks: "<<NTracks);
    //Now after the correct TrackCandIds are assigned, the corresponding segments can be added to the tracks

    for (int i = 0; i < NTracks; i++) { //loop over all track candidates
      CDCTrackCandidate candidate = TrackCandidates.at(i);
      for (unsigned int j = 0; j < segmentArray[SLId - 2].size(); j++) { //loop over all segments

        //boolean to mark the case, when two different segments from the same superlayer are matching the same track candidate
        //if already == true, the track candidate should be split in two different candidates
        bool already = false;
        for (unsigned k = 0; k < segmentArray[SLId - 2].at(j).getTrackCandId().size(); k++) { //loop over all TrackCandIds assigned to this segment
          //B2INFO("TrackIds from segment: "<<segmentArray[SLId].at(j).getTrackCandId().at(k));
          if (segmentArray[SLId - 2].at(j).getTrackCandId().at(k) == i) {

            for (int s = 0; s < TrackCandidates.at(i).getNSegments(); s++) { //check, if there were already another segment from the same superlayer assigned to this candidate
              if (segmentArray[SLId - 2].at(j).getSuperlayerId()
                  == TrackCandidates.at(i).getSegments().at(s).getSuperlayerId()) {

                already = true;
              }
            }


            if (already == false) {
              TrackCandidates.at(i).addSegment(segmentArray[SLId - 2].at(j));
              //B2INFO("Add to candidate "<<i<<" a segment with Id "<<cdcAxialSegments[j].getId()<<"  from SL "<<cdcAxialSegments[j].getSuperlayerId());

            }

            else { // Split the track candidate

              //create a new candidate by copying the old one (new Id) and adding to it the new segment
              trackcounter++;

              TrackCandidates.push_back(CDCTrackCandidate(candidate, trackcounter));

              TrackCandidates.at(trackcounter).addSegment(segmentArray[SLId - 2].at(j));
              cdcAxialSegments[j].setTrackCandId(trackcounter);
              //B2INFO("Split: Add to candidate "<<trackcounter<<" a segment with Id "<<segmentArray[SLId].at(j).getId()<<"  from SL "<<SLId);

            }

          } //endif

        }  //end for loop over all TrackCandIds
      }

    } //end for loop over all track candidates


    //}
    SLId = SLId - 2; //go one axial supelayer lower

  } //end while loop over all axial superlayers


  return TrackCandidates;
}


void AxialTrackFinder::FitTrackCandidate(CDCTrackCandidate& candidate)
{

  double x[100];
  double y[100];
  int nHits = candidate.getNHits(); //Nr of hits in a track
  int temp_nHits = 0; //Nr of hit in a track *excluding* the hits in the innermost superlayer, because they are too 'scattered' in the conformal plane

  //loop over all hits and get their conformal coordinates (exclude first superlayer!)
  for (int j = 0; j < nHits; j++) {

    if (candidate.getTrackHits().at(j).getSuperlayerId() != 0) {
      temp_nHits++;
      x[j] = candidate.getTrackHits().at(j).getConformalX();
      y[j] = candidate.getTrackHits().at(j).getConformalY();
    }
  }

  //mean values to calculate the linear regression
  double xmean = 0;
  double ymean = 0;

  for (int i = 0; i < temp_nHits; i++) {
    xmean = xmean + x[i]; //at first calculate the sum
    ymean = ymean + y[i];
  }

  xmean = xmean / temp_nHits; //obtain mean values
  ymean = ymean / temp_nHits;

  //parameters for the linear regression
  double x0 = 0; //x axis section
  double y0 = 0; //y axis section
  double gradient = 0; //gradient of the line

  double xdiff2 = 0; //(x-xmean)^2
  double totaldiff = 0; //(x-xmean)*(y-ymean)

  for (int i = 0; i < temp_nHits; i++) {
    xdiff2 = xdiff2 + (x[i] - xmean) * (x[i] - xmean);
    totaldiff = totaldiff + (x[i] - xmean) * (y[i] - ymean);
  }

  gradient = totaldiff / xdiff2;
  y0 = ymean - gradient * xmean;
  x0 = -y0 / gradient;

  double y_exp[100]; //expected y values according to the linear fit, necessary to calculate the chi2
  double Chi = 0; //chi2

  for (int i = 0; i < temp_nHits; i++) {
    y_exp[i] = gradient * x[i] + y0;
    Chi = Chi + fabs(((y[i] - y_exp[i]) * (y[i] - y_exp[i])) / y_exp[i]);
  }
  Chi = Chi / (3 * temp_nHits);

  candidate.setChiSquare(Chi);

  double R = sqrt(1 / x0 * 1 / x0 + 1 / y0 * 1 / y0); //Radius of the track circle in the normal plane

  double p = R * 1.5 * 0.00299792458;  //preliminary calculation of pt in GeV, magnetic field and c hardcoded for now...

  candidate.setPt(p);

  //Preliminary estimation of the charge of the track
  //the difference between the phi of the innermost and the outermost hit is considered
  //one has to take care of the 'special case' when the track is 'crossing 0'
  double phi1 = candidate.getInnerMostSegment().getInnerMostHit().getPhi();
  double phi2 = candidate.getOuterMostSegment().getOuterMostHit().getPhi();
  //B2INFO("Calculate Charge:  phi1 "<<phi1<<"   phi2 "<<phi2);
  double phiDiff = phi1 - phi2;
  if (abs(phi1 - phi2) > TMath::Pi()) {
    phiDiff = -phiDiff;
    //B2INFO("Switch sign");
  }

  if (phiDiff > 0) {
    candidate.setChargeSign(1);
  } else {
    candidate.setChargeSign(-1);
  }

}



void AxialTrackFinder::FitTrackCandidates(vector<CDCTrackCandidate> & candidates)
{

  for (unsigned int i = 0; i < candidates.size(); i++) {
    FitTrackCandidate(candidates.at(i));

  }
}

void AxialTrackFinder::CollectTrackCandidates(vector<CDCSegment> & cdcAxialSegments, vector<CDCTrackCandidate>& cdcTrackCandidates)
{

  if (cdcAxialSegments.size() == 0) B2WARNING("AxialTrackFinder: cdcAxialSegments collection is empty!");

  vector<CDCTrackCandidate> FinalTrackCandidates;  //vector to gather Track Candidates (who passed the Chi^2 test) from all start superlayers
  FinalTrackCandidates.reserve(32);

  int nSegments = cdcAxialSegments.size();
  int NTracks = 0;

  vector<int> UsedSegmentId;   //vector to store the Ids of segments, which were already used to reconstruct a track candidate
  vector<int> UsedTrackHitId;  //vector to store the Ids of hits, which were already used to reconstruct a track candidate

  int startSLId = 8;

  int counter = -1;  //count the number of track candidates


  B2DEBUG(100, "Find possible track candidates...");

  //loop over all possible starting superlayers, starting with the outermost superlayer
  //first all track candidates are found, after a simple fit those with too large Chi2 are discarded, all the other candidates are collected in FinalTrackCandidates
  while (startSLId > 0) {

    vector<CDCTrackCandidate> TrackCandidates(FindTrackCandidates(cdcAxialSegments, 15, 0.36, 0.005, startSLId));

    NTracks = TrackCandidates.size();
    B2DEBUG(100, "Number of TrackCandidates starting in SL " << startSLId << " : " << NTracks);

    FitTrackCandidates(TrackCandidates);

    //Discard bad chi2
    for (int i = 0; i < NTracks; i++) {

      if (float(TrackCandidates.at(i).getChiSquare() / TrackCandidates.at(i).getNHits()) < 0.00005) { // more or less aribtrary value used here..
        FinalTrackCandidates.push_back(TrackCandidates.at(i)); // TODO: this would really profit from std::move !!!

      } else B2DEBUG(100, "Throw away the track " << i);

    }
    B2DEBUG(100, "Number of TrackCandidates after Chi2 test: " << FinalTrackCandidates.size());

    startSLId = startSLId - 2;  //go one axial supelayer lower

    //cleanup
    for (int i = 0; i < nSegments; i++) {
      cdcAxialSegments[i].clearTrackCandId();
    }

  }//end while loop over superlayers


  B2DEBUG(100, "Select the best track candidates...");
  //Now the best ( = longest) TrackCandidates are chosen from the FinalTrackCandidates

  int NFinalTracks = FinalTrackCandidates.size();

  int NumberOfSegments = 5; //maximum possible track candidate length

  while (NumberOfSegments > 1) {  //loop over possible track lengths

    for (int i = 0; i < NFinalTracks; i++) {  //loop over all track candidates
      //B2INFO("--------------- Check Track nr "<<i<<" with "<<FinalTrackCandidates.at(i).getNSegments()<<" and used segment fraction: "<<UsedSegmentsFraction( FinalTrackCandidates.at(i),UsedSegmentId)<<" and used hit fraction: "<<UsedHitsFraction( FinalTrackCandidates.at(i),UsedTrackHitId));
      //check for correct number of segments and the fraction of already used segments and hits
      if (FinalTrackCandidates.at(i).getNSegments() == NumberOfSegments && UsedSegmentsFraction(FinalTrackCandidates.at(i), UsedSegmentId) < 0.61 && UsedHitsFraction(FinalTrackCandidates.at(i), UsedTrackHitId) < 0.61) {
        vector <int> Competitors;  //there may be several candidates sharing more or less the same hits
        Competitors.reserve(5);
        Competitors.push_back(i);
        int startWireId = FinalTrackCandidates.at(i).getOuterMostHit().getWireId();
        //check for 'similar' candidates
        for (int check = 0; check < NFinalTracks; check++) {
          if (FinalTrackCandidates.at(check).getNSegments() == NumberOfSegments && FinalTrackCandidates.at(check).getOuterMostHit().getWireId() <= startWireId + 1 && FinalTrackCandidates.at(check).getOuterMostHit().getWireId() >= startWireId - 1 && check != i && UsedSegmentsFraction(FinalTrackCandidates.at(check), UsedSegmentId) < 0.61 && UsedHitsFraction(FinalTrackCandidates.at(check), UsedTrackHitId) < 0.8) {
            //B2INFO("Competitor found!");
            Competitors.push_back(check);
          }
        }

        //Select the candidate with the lowest chi2, return its index in the FinalTrackCandidates vector
        int bestCandidateIndex = EvaluateBestCandidate(FinalTrackCandidates, Competitors);

        counter++;
        cdcTrackCandidates.push_back(FinalTrackCandidates.at(bestCandidateIndex));
        cdcTrackCandidates.back().setId(counter);
        B2DEBUG(100, "Nr " << counter << "   Final track with " << NumberOfSegments << " found");


        for (int j = 0; j < FinalTrackCandidates.at(bestCandidateIndex).getNSegments(); j++) {
          UsedSegmentId.push_back(FinalTrackCandidates.at(bestCandidateIndex).getSegments().at(j).getId());
          for (unsigned int hit = 0; hit < FinalTrackCandidates.at(bestCandidateIndex).getTrackHits().size(); hit ++) {
            UsedTrackHitId.push_back(FinalTrackCandidates.at(bestCandidateIndex).getTrackHits().at(hit).getStoreIndex());
          }
        }


        //extension for special cases, where short tracks found separately can be combined to one track

        if (NumberOfSegments == 3 || NumberOfSegments == 2) {
          int additionalSegments = 2;
          B2DEBUG(100, "------------ Extension for short tracks ");
          while (additionalSegments > 0) { //while loop, to be performed two times, first searching for 2 segment tracks, then 1 segment tracks
            bool sameSegment = false;
            for (int i_add = 0; i_add < NFinalTracks; i_add++) {  //loop over all candidates
              for (int id = 0; id < FinalTrackCandidates.at(i).getNSegments(); id++) { //check, if there were already the same segment assigned to this candidate
                for (int id2 = 0; id2 < FinalTrackCandidates.at(i_add).getNSegments(); id2++)
                  if (FinalTrackCandidates.at(i).getSegments().at(id).getId() == FinalTrackCandidates.at(i_add).getSegments().at(id2).getId()) {
                    sameSegment = true;
                  }
              }

              //check for right segments number and used fraction
              if (i_add != i && sameSegment == false  && FinalTrackCandidates.at(i_add).getNSegments() == additionalSegments && UsedSegmentsFraction(FinalTrackCandidates.at(i_add), UsedSegmentId) < 0.3) {

                //additional variable to check if short tracks can be combined to one
                double angle = FinalTrackCandidates.at(i_add).getSegments().at(0).getDirection().Angle(FinalTrackCandidates.at(i).getSegments().at(0).getDirection());
                if (angle > TMath::Pi() / 2) angle = angle - TMath::Pi() ; // -90 < angle < 90

                //check for shortest distance and angle
                if (ShortestDistance(FinalTrackCandidates.at(i_add).getSegments().at(0), FinalTrackCandidates.at(i).getSegments().at(0)) < 0.005
                    && abs(angle) < 0.3 && SimpleDistance(FinalTrackCandidates.at(i_add).getSegments().at(0), FinalTrackCandidates.at(i).getSegments().at(0)) < 70) {
                  B2DEBUG(100, "Two short tracks combined to one");

                  //Add all segment from one short track to the other and mark segment Ids as used
                  for (int s = 0; s < FinalTrackCandidates.at(i_add).getNSegments(); s++) {

                    FinalTrackCandidates.at(i).addSegment(FinalTrackCandidates.at(i_add).getSegments().at(s));
                    UsedSegmentId.push_back(FinalTrackCandidates.at(i_add).getSegments().at(s).getId());
                    for (unsigned int hit = 0; hit < FinalTrackCandidates.at(i_add).getTrackHits().size(); hit ++) {
                      UsedTrackHitId.push_back(FinalTrackCandidates.at(i_add).getTrackHits().at(hit).getStoreIndex());
                    }

                  }
                  //Refit the candidate
                  FitTrackCandidate(FinalTrackCandidates.at(i));


                }
              }
            }  //end loop over all candidates
            additionalSegments--;
          }
        } //end extension for short tracks
      }
    } //end for loop over all candidates

    NumberOfSegments--; //decrease required track length

  } //end while


}





