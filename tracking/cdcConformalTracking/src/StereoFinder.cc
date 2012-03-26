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
#include <cstdlib>

#include "TGraph.h"
#include "TAxis.h"
#include "TF1.h"


#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <tracking/cdcConformalTracking/CellularSegmentFinder.h>
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

int StereoFinder::WireIdDifference(CDCTrackCandidate track, CDCSegment segment)
{
  return CellularSegmentFinder::WireIdDifference(track.getOuterMostHit(), segment.getOuterMostHit());
}

void StereoFinder::FindStereoSegments(CDCTrackCandidate startTrack, vector<CDCSegment> & cdcStereoSegments, double SimpleDistanceCut,
                                      double ShortDistanceCut, int SLId)
{
  int nSegments = cdcStereoSegments.size();

  bool skip = false;    //we want the outermost segments of the track to be a axial segment, so all stereo segments 'higher' than the outermost axial segment will be skipped
  if (startTrack.getOuterMostSegment().getSuperlayerId() < SLId) skip = true;

  if (skip == false) {
    for (int i = 0; i < nSegments; i++) { //loop over all segments
      //first necessary neighbouring condition and check the correct superlayer
      if (cdcStereoSegments[i].getSuperlayerId() == SLId) {
        //the decisive neighbouring condition
        if (SimpleDistance(startTrack, cdcStereoSegments[i]) < SimpleDistanceCut && ShortestDistance(startTrack, cdcStereoSegments[i]) < ShortDistanceCut) {
          //add the Id of the track candidate to the segment
          cdcStereoSegments[i].setTrackCandId(startTrack.getId());
          // B2INFO("Possible stereo Candidate for track "<<startTrack.getId()<<"  in SL "<<SLId);
        }
      }

    }//end loop over all segments
  } //end if skip false
}

void StereoFinder::StereoFitCandidates(CDCTrackCandidate& candidate)
{
  B2DEBUG(100, "Perform a stereo fit: candidate with " << candidate.getNSegments() << " segments");
  if (candidate.getNSegments() > 0 && candidate.getNHits() > 10 && candidate.getNHits() < 200) {
    //create a graph to fit r vs z
    TGraph* zGraph;
    double r[200];
    double z[200];
    int nHits = candidate.getNHits(); //Nr of hits in a track
    bool refit = true;

    while (refit == true) { //while loop to refit the candidate if one segment is removed, should be performed as long as there are 'overcrowded' superlayers
      refit = false;
      //loop over all hits and get their r and z coordinates
      for (int j = 0; j < nHits; j++) {
        r[j] = sqrt(candidate.getTrackHits().at(j).getWirePosition().X() * candidate.getTrackHits().at(j).getWirePosition().X() + candidate.getTrackHits().at(j).getWirePosition().Y() * candidate.getTrackHits().at(j).getWirePosition().Y());
        z[j] = candidate.getTrackHits().at(j).getWirePosition().z();
      }
      //create graph and do a linear fit
      zGraph = new TGraph(nHits, r, z);
      double zMin = zGraph->GetXaxis()->GetXmin();
      double zMax = zGraph->GetXaxis()->GetXmax();
      zGraph->Fit("pol1", "Q", "", zMin, zMax);
      TF1* zFit = zGraph->GetFunction("pol1");

      double zChi2 = zFit->GetChisquare();

      //in case of bad chi2 some segments should be removed
      if (zChi2 > 0.001) {
        TVector3 zfitPoint;   //some point on the fit line (rz plane)
        zfitPoint.SetX(r[0]);
        zfitPoint.SetY(zFit->GetParameter(1) * r[0]);
        zfitPoint.SetZ(0);

        TVector3 zfitDirection;
        zfitDirection.SetX(r[10] - r[0]);
        zfitDirection.SetY(zFit->GetParameter(1) * r[10] - zFit->GetParameter(1) * r[0]);
        zfitDirection.SetZ(0);

        for (int sl = 8; sl > 0; sl --) { //loop over all superlayers

          double maxDistance = 0;
          int index = 0;
          if (OvercrowdedSuperlayer(candidate, sl) == true) { //compare the segments from overcrowded superlayers
            double zDeviation = 100;
            for (int i = 0; i < candidate.getNSegments(); i++) {
              if (candidate.getSegments().at(i).getSuperlayerId() == sl) {
                double zSegX = sqrt(candidate.getSegments().at(i).getOuterMostHit().getWirePosition().X() * candidate.getSegments().at(i).getOuterMostHit().getWirePosition().X() + candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Y() * candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Y());
                double zSegY = candidate.getSegments().at(i).getOuterMostHit().getWirePosition().Z();
                TVector3 zSegmentPoint;
                zSegmentPoint.SetX(zSegX);
                zSegmentPoint.SetY(zSegY);
                zSegmentPoint.SetZ(0);
                zDeviation = fabs((zSegmentPoint.y() - (zFit->GetParameter(1) * zSegmentPoint.x() + zFit->GetParameter(0))) / (zFit->GetParameter(1) * zSegmentPoint.x() + zFit->GetParameter(0)));

                //B2INFO("ZDeviation: "<<zDeviation);
                //select the segment with the largest residual/deviation from the fit line
                if (zDeviation > maxDistance) {
                  maxDistance = zDeviation;
                  index = i;
                }

              }
            }
            //remove the worst segment and refit
            candidate.removeSegment(candidate.getSegments().at(index).getId());
            //B2INFO("Remove bad segment with Id "<<index);
            refit = true;
          }
        }//end loop over superlayers

      }

    } //end while loop
  } //end if

  else B2WARNING("This stereo candidate has too few or too many hits and wont be fitted!");
  B2DEBUG(100, "End stereo fit, number of segments reduced to " << candidate.getNSegments());
}


bool StereoFinder::OvercrowdedSuperlayer(CDCTrackCandidate track, int SLId)
{

  bool overcrowded = false;
  int segmentCounter = 0;

  for (int i = 0; i < track.getNSegments(); i++) {
    if (track.getSegments().at(i).getSuperlayerId() == SLId) {
      segmentCounter++;
    }
  }

  if (segmentCounter > 1) {
    overcrowded = true;
  }

  return overcrowded;
}

void StereoFinder::CheckOvercrowdedSL(CDCTrackCandidate& axialCandidate, CDCTrackCandidate& stereoCandidate)
{
  //B2INFO("...... start check overcrowded SLs for ");
  double minDistance;
  int bestId;         //id of the best segments
  for (int sl = 7; sl > 0; sl = sl - 2) { //loop over all superlayers
    bestId = -999;
    minDistance = 10;

    if (OvercrowdedSuperlayer(stereoCandidate, sl) == true) { //check if the superlayer is 'overcrowded' = has more than one segment
      B2INFO("Overcrowded SL! This Candidate has several segments in Superlayer " << sl);

      for (int i = 0; i < stereoCandidate.getNSegments(); i++) {
        if (stereoCandidate.getSegments().at(i).getSuperlayerId() == sl) {  //loop over all segments from the overcrowded superlayer
          //B2INFO("Shift....");
          //shift all stereo hits according to the given track
          for (int j = 0; j < stereoCandidate.getSegments().at(i).getNHits(); j++) {
            stereoCandidate.getSegments().at(i).getTrackHits().at(j).shiftAlongZ(axialCandidate.getDirection(), axialCandidate.getOuterMostHit());
            stereoCandidate.getSegments().at(i).update();
          }
          //check if now the segment can pass more strict cuts
          double angle = axialCandidate.getOuterMostSegment().getDirection().Angle(stereoCandidate.getSegments().at(i).getDirection());
          double shortestDistance = ShortestDistance(axialCandidate, stereoCandidate.getSegments().at(i));
          B2INFO("id: " << stereoCandidate.getSegments().at(i).getId() << "  shortest Distance: " << shortestDistance << "  angle: " << abs(angle));
          B2INFO("z coordinate: " << stereoCandidate.getOuterMostHit().getWirePosition().z());
          if (shortestDistance < minDistance) { //select the best segment
            minDistance = shortestDistance;
            bestId = stereoCandidate.getSegments().at(i).getId();
          }
        }

      }
      B2INFO("Best segment " << bestId << " with " << minDistance);
      while (OvercrowdedSuperlayer(stereoCandidate, sl) ==  true) {
        //remove all segments which 'lost' against the best segment in the overcrowded superlayers
        for (int i = 0; i < stereoCandidate.getNSegments(); i++) {
          if (stereoCandidate.getSegments().at(i).getSuperlayerId() == sl && stereoCandidate.getSegments().at(i).getId() != bestId) {
            B2INFO("!!! Remove Segment with ID " << stereoCandidate.getSegments().at(i).getId());
            stereoCandidate.removeSegment(stereoCandidate.getSegments().at(i).getId());

          }
        }
      }//end while
    }
  }//end loop over all superlayers

}


void StereoFinder::AppendStereoSegments(vector<CDCSegment> & cdcStereoSegments, string CDCTrackCandidates)
{
  StoreArray<CDCTrackCandidate> cdcTrackCandidates(CDCTrackCandidates.c_str());  //output track candidates

  int nSegments = cdcStereoSegments.size();
  int nTracks = cdcTrackCandidates.getEntries();

  vector <CDCTrackCandidate> StereoCandidates; //vector to hold track candidates with only the stereo segments

  //create an 'empty' TrackCandidate with the same Id as in the StoreArray with axial TrackCandidates and place it in vector
  for (int i = 0; i < nTracks; i++) {
    CDCTrackCandidate track(i);
    StereoCandidates.push_back(track);
  }

  int SL = 7; //start superlayer
  double simpleCut = 20; //cut on the simple distance
  double shortDistanceCut = 0.05 ; //cut on the 'short' distance for the first search loop
  double strictCut = 0.007 ; //cut on the 'short' distance for the final decision (after shifting the stereo segments)
  double angleCut = 0.4 ; //cut on the angle between the track candidate and the segment

  while (SL > 0) { //loop over all stereo superlayers

    for (int i = 0; i < nTracks; i++) { //loop over all Tracks

      //recalculate the cut depending on the superlayer difference (e.g. the cut should be different for consecutive superlayers as for superlayers with a gap in between)
      int SLDiff = abs(cdcTrackCandidates[i]->getOuterMostSegment().getSuperlayerId() - SL);
      simpleCut = 15 + (SLDiff * 10 - 10);

      //search for matching stereo segments in the given superlayer
      //the matching stereo segments get the trackId assigned to them
      //B2INFO("Find Stereo Segments for "<<i<<" (outermost wireId: "<<cdcTrackCandidates[i]->getOuterMostHit().getWireId()<<")");
      FindStereoSegments(*cdcTrackCandidates[i], cdcStereoSegments, simpleCut, shortDistanceCut, SL);

    } //end loop over all tracks

    SL = SL - 2;

  } //end while loop

  //Now check for all segments their possible tracks
  for (int j = 0; j < nSegments; j++) { //loop over all segments

    CDCSegment segment = cdcStereoSegments[j];
    //B2INFO("Segment "<<segment.getId()<<" from SL "<<segment.getSuperlayerId())
    for (unsigned int k = 0; k < segment.getTrackCandId().size(); k++) { //loop over all track candidate to which this segment may belong
      int trackId = segment.getTrackCandId().at(k);
      //B2INFO("-----possible ID "<<trackId)
      //shift all hits in this segment according to the given track candidate
      //B2INFO("Shift....");
      for (int i = 0; i < segment.getNHits(); i++) {
        segment.getTrackHits().at(i).shiftAlongZ(cdcTrackCandidates[segment.getTrackCandId().at(k)]->getDirection(),
                                                 cdcTrackCandidates[segment.getTrackCandId().at(k)]->getOuterMostHit());
        segment.update();
      }
      //check if now the segment can pass more strict cuts
      //only segments which can pass the cut after the shift are added to the track candidates
      double angle = cdcTrackCandidates[trackId]->getOuterMostSegment().getDirection().Angle(segment.getDirection());
      //B2INFO("shortest Distance: "<<ShortestDistance(*cdcTrackCandidates[trackId], segment)<< "  angle: " <<abs(angle));
      if (ShortestDistance(*cdcTrackCandidates[trackId], segment) < strictCut && abs(angle) < angleCut) {
        //B2INFO("           stereo segment added to track "<<trackId)
        StereoCandidates.at(trackId).addSegment(segment);
      }

    } //end loop over track candidates
    cdcStereoSegments[j].clearTrackCandId();

  } //end loop over all segments

  for (int i = 0; i < nTracks; i++) {

  }

  //it may happen, that several segments from the same superlayer are assigned to the same track (because of the shift and the uknown z-coordinate)
  //so, for each track it is checked if there is only one stereo segment in each superlayer, if not, the best segment is chosen and the others are removed
  for (int i = 0; i < nTracks; i++) {
    //CheckOvercrowdedSL(*cdcTrackCandidates[i],StereoCandidates[i]);
    StereoFitCandidates(StereoCandidates[i]);
  }

  //Add the final stereo segments to the corresponding track candidates
  B2INFO("Add stereo segments to the track candidates...");
  for (int i = 0; i < nTracks; i++) {
    for (int j = 0; j < StereoCandidates.at(i).getNSegments(); j++) {
      //shift all stereo hits according to the final track
      for (int k = 0; k < StereoCandidates.at(i).getSegments().at(j).getNHits(); k++) {
        StereoCandidates.at(i).getSegments().at(j).getTrackHits().at(k).shiftAlongZ(cdcTrackCandidates[i]->getDirection(), cdcTrackCandidates[i]->getOuterMostHit());
        StereoCandidates.at(i).getSegments().at(j).update();
      }
      cdcTrackCandidates[i]->addSegment(StereoCandidates.at(i).getSegments().at(j));
    }
  }

  //Estimate momentum the finished tracks
  for (int i = 0; i < nTracks; i++) {
    cdcTrackCandidates[i]->estimateMomentum();
  }

}



