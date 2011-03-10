/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/StereoFinder.h"

#include <cmath>

#include "TMath.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"
#include "TCanvas.h"

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <tracking/cdcConformalTracking/AxialTrackFinder.h>

using namespace std;
using namespace Belle2;


StereoFinder::StereoFinder()
{
}

StereoFinder::~StereoFinder()
{
}

double StereoFinder::ShortestDistance(CDCTrackCandidate track, CDCSegment segment)
{
  return AxialTrackFinder::ShortestDistance(track.getOuterMostSegment(),
                                            segment);
}

double StereoFinder::SimpleDistance(CDCTrackCandidate track, CDCSegment segment)
{
  return AxialTrackFinder::SimpleDistance(track.getOuterMostSegment(),
                                          segment);
}

void StereoFinder::FindStereoSegments(CDCTrackCandidate startTrack,
                                      vector<CDCSegment> & cdcStereoSegments, double SimpleDistanceCut,
                                      double ShortDistanceCut, int SLId)
{

  int nSegments = cdcStereoSegments.size();

  for (int i = 0; i < nSegments; i++) { //loop over all segments
    //B2INFO("SL: "<<cdcStereoSegments[i].getSuperlayerId()<<"  Simple Distance: "<<SimpleDistance(startTrack, cdcStereoSegments[i])<<" Shortest Distance: "<<ShortestDistance(startTrack, cdcStereoSegments[i]));
    //first necessary neighbouring condition and check the correct superlayer
    if (cdcStereoSegments[i].getSuperlayerId() == SLId && SimpleDistance(
          startTrack, cdcStereoSegments[i]) < SimpleDistanceCut
        && cdcStereoSegments[i].getIsGood() == true) {
      //the decisive neighbouring condition
      if (ShortestDistance(startTrack, cdcStereoSegments[i])
          < ShortDistanceCut) {
        //add the Id of the track candidate to the segment
        cdcStereoSegments[i].setTrackCandId(startTrack.getId());
        // B2INFO("Possible stereo Candidate for track "<<startTrack.getId()<<"  in SL "<<SLId);
        //  for (int test = 0; test< cdcStereoSegments[i].getTrackCandId().size(); test++){
        //  B2INFO(" Segment with ID  "<<cdcStereoSegments[i].getId()<<"  has TrackCandId "<<cdcStereoSegments[i].getTrackCandId()[test]);
        //  }
      }
    }

  }//end loop over all segments
}

bool StereoFinder::SegmentFromOvercrowdedSL(CDCTrackCandidate track, int SegmentIndex)
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

//This method still needs much improvement and refactoring...
void StereoFinder::StereoFitCandidates(CDCTrackCandidate & candidate)
{
  if (candidate.getNSegments() > 0) {
    TGraph * zGraph;
    TGraph * confGraph;
    double x[100];
    double y[100];
    double r[100];
    double z[100];
    int nHits = candidate.getNHits(); //Nr of hits in a track
    bool refit = true;

    while (refit == true) { //while loop to refit the candidate if one segment is removed
      refit = false;
      //loop over all hits and get their conformal coordinates and also theor r and z coordinates
      for (int j = 0; j < nHits; j++) {
        x[j] = candidate.getTrackHits().at(j).getConformalX();
        y[j] = candidate.getTrackHits().at(j).getConformalY();
        r[j] = sqrt(candidate.getTrackHits().at(j).getWirePosition().X() * candidate.getTrackHits().at(j).getWirePosition().X() + candidate.getTrackHits().at(j).getWirePosition().Y() * candidate.getTrackHits().at(j).getWirePosition().Y());
        z[j] = candidate.getTrackHits().at(j).getWirePosition().z();
      }

      //Create two different graphs
      confGraph = new TGraph(nHits, x, y);
      zGraph = new TGraph(nHits, r, z);

      double cMin = confGraph->GetXaxis()->GetXmin();
      double cMax = confGraph->GetXaxis()->GetXmax();

      double zMin = zGraph->GetXaxis()->GetXmin();
      double zMax = zGraph->GetXaxis()->GetXmax();

      //Fit the both graphs with simple linear fit
      confGraph->Fit("pol1", "Q", "", cMin, cMax);
      TF1 *confFit = confGraph->GetFunction("pol1");

      zGraph->Fit("pol1", "Q", "", zMin, zMax);
      TF1 *zFit = zGraph->GetFunction("pol1");

      double confChi = confFit->GetChisquare();
      double zChi = zFit->GetChisquare();

      //Check the Chi2 of the fit, if it is too bad search for segment to remove
      if (zChi > 0.0000001 || confChi > 0.0000001) {
        //B2INFO("    Chi2 too bad, search for bad stereo segments to remove");
        TVector3 cfitPoint;   //some point on the fit line (conformal plane)
        cfitPoint.SetX(x[0]);
        cfitPoint.SetY(confFit->GetParameter(1) * x[0]);
        cfitPoint.SetZ(0);

        TVector3 cfitDirection;
        cfitDirection.SetX(x[10] - x[0]);
        cfitDirection.SetY(confFit->GetParameter(1) * x[10] - confFit->GetParameter(1) * x[0]);
        cfitDirection.SetZ(0);

        TVector3 zfitPoint;   //some point on the fit line (rz plane)
        zfitPoint.SetX(r[0]);
        zfitPoint.SetY(zFit->GetParameter(1) * r[0]);
        zfitPoint.SetZ(0);

        TVector3 zfitDirection;
        zfitDirection.SetX(r[10] - r[0]);
        zfitDirection.SetY(zFit->GetParameter(1) * r[10] - zFit->GetParameter(1) * r[0]);
        zfitDirection.SetZ(0);

        for (int sl = 8; sl > 0; sl --) { //loop over all superlayers

          int counter = 0;
          double minDistance = 100;
          int index = 0;

          for (int i = 0; i < candidate.getNSegments(); i++) { //count if there is more the one segment with the same SLId in this candidate
            if (candidate.getSegments().at(i).getSuperlayerId() == sl) {
              counter++;
            }
          }
          //if there is more then one segment from the same superlayer
          if (counter > 1) {
            //B2INFO("SL "<<sl<<"  overcrowded! "<<counter);
            double zDeviation;
            double cDeviation;
            //look on the segments from the overcrowded superlayer and check which one lies best on the fit lines
            for (int i = 0; i < candidate.getNSegments(); i++) {
              if (candidate.getSegments().at(i).getSuperlayerId() == sl) {
                double zSegX = sqrt(candidate.getSegments().at(i).getOuterMostHit().getWirePosition().X() * candidate.getSegments().at(i).getOuterMostHit().getWirePosition().X() + candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Y() * candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Y());
                double zSegY = candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Z();
                TVector3 zSegmentPoint;
                zSegmentPoint.SetX(zSegX);
                zSegmentPoint.SetY(zSegY);
                zSegmentPoint.SetZ(0);


                TVector3 cSegmentPoint;
                cSegmentPoint.SetX((candidate.getSegments().at(i).getOuterMostHit().getConformalX() + candidate.getSegments().at(i).getInnerMostHit().getConformalX()) / 2);
                cSegmentPoint.SetY((candidate.getSegments().at(i).getOuterMostHit().getConformalY() + candidate.getSegments().at(i).getInnerMostHit().getConformalY()) / 2);
                cSegmentPoint.SetZ(0);


                cDeviation = fabs((cSegmentPoint.y() - (confFit->GetParameter(1) * cSegmentPoint.x() + confFit->GetParameter(0))) / (confFit->GetParameter(1) * cSegmentPoint.x() + confFit->GetParameter(0)));

                zDeviation = fabs((zSegmentPoint.y() - (zFit->GetParameter(1) * zSegmentPoint.x() + zFit->GetParameter(0))) / (zFit->GetParameter(1) * zSegmentPoint.x() + zFit->GetParameter(0)));
                //B2INFO("ZDeviation: "<<zDeviation<<"  CDeviation: "<<cDeviation);

                if (zDeviation < minDistance) {
                  minDistance = zDeviation;
                  index = i;
                  //B2INFO("New minDistance "<<minDistance<< " from Segment "<<candidate.getSegments().at(index).getId());
                }

              }
            } //end loop over all segments
            // B2INFO("Best Segment in SL "<<sl<<"  was Segment "<<candidate.getSegments().at(index).getId());

            //Now loop again over all Segments, remove bad segments and refit the track candidate
            for (int i = 0; i < candidate.getNSegments(); i++) { //loop over all segments
              if (candidate.getSegments().at(i).getSuperlayerId() == sl && i != index) {
                int badId = candidate.getSegments().at(i).getId();
                candidate.removeSegment(badId);
                //B2INFO("Remove bad segment with Id "<<badId);
                refit = true;
                //------------------------------- refit
                //loop over all hits and get their conformal coordinates
                for (int j = 0; j < nHits; j++) {
                  x[j] = candidate.getTrackHits().at(j).getConformalX();
                  y[j] = candidate.getTrackHits().at(j).getConformalY();
                  r[j] = sqrt(candidate.getTrackHits().at(j).getWirePosition().X() * candidate.getTrackHits().at(j).getWirePosition().X() + candidate.getTrackHits().at(j).getWirePosition().Y() * candidate.getTrackHits().at(j).getWirePosition().Y());
                  z[j] = candidate.getTrackHits().at(j).getWirePosition().z();
                }

                confGraph = new TGraph(nHits, x, y);
                zGraph = new TGraph(nHits, r, z);

                double cMin = confGraph->GetXaxis()->GetXmin();
                double cMax = confGraph->GetXaxis()->GetXmax();

                double zMin = zGraph->GetXaxis()->GetXmin();
                double zMax = zGraph->GetXaxis()->GetXmax();

                confGraph->Fit("pol1", "Q", "", cMin, cMax);
                //  TF1 *confFit = confGraph->GetFunction("pol1");

                zGraph->Fit("pol1", "Q", "", zMin, zMax);
                //  TF1 *zFit = zGraph->GetFunction("pol1");

                //double confChi = confFit->GetChisquare();
                //double zChi = zFit->GetChisquare();
                //--------------------------refit end
              }
            }//end loop over all segments

          }//endif overcrowded superlayer

          //Now check the "not overcrowded" superlayers
          //Basically the same procedure but with looser cut
          else {
            //B2INFO("Check normal superlayers");
            double zDeviation;
            double cDeviation;
            //These values are still under test.... Try different cuts for different superlayers
            //double zCut = 2.;
            //double cCut = 1. ;

            double zCut1 = 3.;
            double cCut1 = 2;

            double zCut2 = 2.;
            double cCut2 = 1;

            double zCut3 = 1.5;
            double cCut3 = 1.0;



            for (int i = 0; i < candidate.getNSegments(); i++) {
              bool doit = false;
              if (candidate.getSegments().at(i).getSuperlayerId() == sl) {
                //B2INFO("SL: "<<sl);
                double zSegX = sqrt(candidate.getSegments().at(i).getOuterMostHit().getWirePosition().X() * candidate.getSegments().at(i).getOuterMostHit().getWirePosition().X() + candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Y() * candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Y());
                double zSegY = candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Z();
                TVector3 zSegmentPoint;
                zSegmentPoint.SetX(zSegX);
                zSegmentPoint.SetY(zSegY);
                zSegmentPoint.SetZ(0);

                TVector3 cSegmentPoint;
                cSegmentPoint.SetX((candidate.getSegments().at(i).getOuterMostHit().getConformalX() + candidate.getSegments().at(i).getInnerMostHit().getConformalX()) / 2);
                cSegmentPoint.SetY((candidate.getSegments().at(i).getOuterMostHit().getConformalY() + candidate.getSegments().at(i).getInnerMostHit().getConformalY()) / 2);
                cSegmentPoint.SetZ(0);


                cDeviation = fabs((cSegmentPoint.y() - (confFit->GetParameter(1) * cSegmentPoint.x() + confFit->GetParameter(0))) / (confFit->GetParameter(1) * cSegmentPoint.x() + confFit->GetParameter(0)));
                zDeviation = fabs((zSegmentPoint.y() - (zFit->GetParameter(1) * zSegmentPoint.x() + zFit->GetParameter(0))) / (zFit->GetParameter(1) * zSegmentPoint.x() + zFit->GetParameter(0)));
                //B2INFO("zDeviation: "<<zDeviation<<"   cDeviation: "<<cDeviation );

                if (sl<4 && zDeviation > zCut1 && cDeviation > cCut1) doit = true;
                if (sl > 3 && sl < 7 && zDeviation > zCut2 && cDeviation > cCut2) doit = true;
                if (sl > 6 && zDeviation > zCut3 && cDeviation > cCut3) doit = true;

                if (doit == true && candidate.getNSegments() > 1) {
                  int badId = candidate.getSegments().at(i).getId();
                  candidate.removeSegment(badId);
                  //B2INFO("Remove bad segment with Id "<<badId);
                  refit = true;
                  //------------------------------- refit
                  //loop over all hits and get their conformal coordinates
                  for (int j = 0; j < nHits; j++) {
                    x[j] = candidate.getTrackHits().at(j).getConformalX();
                    y[j] = candidate.getTrackHits().at(j).getConformalY();
                    r[j] = sqrt(candidate.getTrackHits().at(j).getWirePosition().X() * candidate.getTrackHits().at(j).getWirePosition().X() + candidate.getTrackHits().at(j).getWirePosition().Y() * candidate.getTrackHits().at(j).getWirePosition().Y());
                    z[j] = candidate.getTrackHits().at(j).getWirePosition().z();

                  }

                  confGraph = new TGraph(nHits, x, y);
                  zGraph = new TGraph(nHits, r, z);


                  double cMin = confGraph->GetXaxis()->GetXmin();
                  double cMax = confGraph->GetXaxis()->GetXmax();

                  double zMin = zGraph->GetXaxis()->GetXmin();
                  double zMax = zGraph->GetXaxis()->GetXmax();

                  confGraph->Fit("pol1", "Q", "", cMin, cMax);
                  //  TF1 *confFit = confGraph->GetFunction("pol1");

                  zGraph->Fit("pol1", "Q", "", zMin, zMax);
                  //   TF1 *zFit = zGraph->GetFunction("pol1");

                  // double confChi = confFit->GetChisquare();
                  // double zChi = zFit->GetChisquare();
                  //--------------------------refit end

                }
              }


            }


          }

        }//end loop over all superlayers

      }//endif bad chi2
    }  //end while

  }
}




void StereoFinder::AppendStereoSegments(vector<CDCSegment> & cdcStereoSegments,
                                        string CDCTrackCandidates)
{

  StoreArray<CDCTrackCandidate> cdcTrackCandidates(CDCTrackCandidates.c_str());  //output track candidates

  int nSegments = cdcStereoSegments.size();
  int nTracks = cdcTrackCandidates.GetEntries();

  vector<CDCTrackCandidate> StereoCandidates; //vector to hold track candidates with only the stereo segments

  //create a TrackCandidate with the same Id as in the given StoreArray and place it in vector
  for (int i = 0; i < nTracks; i++) {
    CDCTrackCandidate track(i);
    StereoCandidates.push_back(track);
  }

  int SL = 8; //start superlayer
  double simpleCut = 30; //cut on the simple distance
  double shortDistanceCut = 0.05 ; //cut on the 'short' distance for the first search loop
  double strictCut = 0.012 ; //cut on the 'short' distance for the final decision (after shifting the stereo segments)
  double angleCut = 0.3 ; //cut on the angle between the track candidate and the segment

  while (SL > 1) { //loop over all stereo superlayers

    for (int i = 0; i < nTracks; i++) { //loop over all Tracks

      //recalculate the cut depending on the superlayer difference
      int SLDiff = abs(cdcTrackCandidates[i]->getOuterMostSegment().getSuperlayerId() - SL);
      simpleCut = 30 + (SLDiff * 10 - 10);

      //search for matching stereo segments in the given superlayer
      //B2INFO("Find Stereo Segments for "<<i);
      FindStereoSegments(*cdcTrackCandidates[i],
                         cdcStereoSegments, simpleCut, shortDistanceCut, SL);

    } //end loop over all tracks

    for (int j = 0; j < nSegments; j++) { //loop over all segments

      CDCSegment segment = cdcStereoSegments[j];

      // B2INFO("Segment "<<j<<" from SL "<<segment.getSuperlayerId())
      for (unsigned int k = 0; k < segment.getTrackCandId().size(); k++) { //loop over all track candidate to which this segment may belong
        int trackId = segment.getTrackCandId().at(k);
        // B2INFO("-----possible ID "<<trackId)
        //shift all hits in this segment according to the given track candidate
        for (int i = 0; i < segment.getNHits(); i++) {

          segment.getTrackHits().at(i).shiftAlongZ(
            cdcTrackCandidates[segment.getTrackCandId().at(
                                 k)]->getDirection(),
            cdcTrackCandidates[segment.getTrackCandId().at(
                                 k)]->getOuterMostHit());
          segment.update();

        }
        //check if now the segment can pass more strict cuts

        double angle = cdcTrackCandidates[trackId]->getOuterMostSegment().getDirection().Angle(segment.getDirection());
        //if (angle > TMath::Pi() / 2)  angle = angle - TMath::Pi(); // -90 < angle < 90
        // B2INFO("shortest Distance: "<<ShortestDistance(*cdcTrackCandidates[trackId], segment)<< "  angle: " <<abs(angle));
        if (ShortestDistance(*cdcTrackCandidates[trackId], segment) < strictCut
            && abs(angle) < angleCut) {
          //B2INFO("           stereo segment added")
          StereoCandidates.at(trackId).addSegment(segment);
        }

      } //end loop over track candidates
      cdcStereoSegments[j].clearTrackCandId();

    } //end loop over all segments

    SL = SL - 2;

  } //end while loop


  //additional quality check: fit the temporarily created candidates consisting only from stereo segments and remove bad segments
  for (int i = 0; i < nTracks; i++) {
    //B2INFO("---------------------------- Fit candidate "<<i);
    StereoFitCandidates(StereoCandidates.at(i));
    //B2INFO("Chi2: "<<StereoCandidates.at(i).getChiSquare());
  }

  //Add the final stereo segments to the corresponding track candidates
  B2INFO("Add stereo segments to the track candidates");
  for (int i = 0; i < nTracks; i++) {
    //  B2INFO("******* Finalize track "<<i)
    for (int j = 0; j < StereoCandidates.at(i).getNSegments(); j++) {
      //  B2INFO("    Superlayer "<<StereoCandidates.at(i).getSegments().at(j).getSuperlayerId());
      cdcTrackCandidates[i]->addSegment(StereoCandidates.at(i).getSegments().at(j));
      //B2INFO("Add stereo segment to track "<<i);
    }
  }

  //Estimate momentum
  for (int i = 0; i < nTracks; i++) {
    cdcTrackCandidates[i]->estimateMomentum();
  }



}


