/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CellularSegmentFinder.h"

#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>


#include "TMath.h"
#include <cmath>
#include <cstdlib>


using namespace std;
using namespace Belle2;
using namespace CDC;


CellularSegmentFinder::CellularSegmentFinder()
{
}

CellularSegmentFinder::~CellularSegmentFinder()
{
}

double CellularSegmentFinder::SimpleDistance(const CDCTrackHit& hit1, const CDCTrackHit& hit2)
{

  double simpleDistance = sqrt(pow((hit1.getWirePosition().x() - hit2.getWirePosition().x()), 2) + pow(
                                 (hit1.getWirePosition().y() - hit2.getWirePosition().y()), 2));

  return simpleDistance;
}

int CellularSegmentFinder::WireIdDifference(const CDCTrackHit& hit1, const CDCTrackHit& hit2)
{

  //create instance of CDCGeometryParameters to get number of wires per layer
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  //calculate wireId difference
  unsigned int wireIdDifference = abs(hit1.getWireId() - hit2.getWireId());

  //for the case that the wireIdDifference is 'crossing 0' (e.g. first WireId 1 and second WireId 300, the difference is not 159), some modification is needed
  if (wireIdDifference > cdcg.nWiresInLayer(hit1.getLayerId()) / 2) {
    wireIdDifference = cdcg.nWiresInLayer(hit1.getLayerId()) - wireIdDifference;
  }

  return wireIdDifference;
}

void CellularSegmentFinder::EvaluateStates(vector<CDCTrackHit>& cdcHits, const int startLayer, const int nLayers)
{
  B2DEBUG(100, "Evaluate states ...");
  bool stop = false; //boolean to mark when the states have reached their final values
  int nHits = cdcHits.size();

  while (stop == false) {
    stop = true;
    //start with the outermost layer and loop over all layers (except the innermost one, because this layer has no further neighbours and its state won't change anyway)
    for (int layerId = startLayer; layerId > (startLayer + 1 - nLayers); --layerId) {
      for (int i = 0; i < nHits; ++i) {  //loop over all hits
        if (cdcHits.at(i).getLayerId() == layerId) { //hits from the current layer
          CDCTrackHit* hitCandidate = &cdcHits[i];

          for (int j = 0; j < nHits; j++) { //now loop again over all hits to find neighbours in the following layer
            if (cdcHits[j].getLayerId() == layerId - 1 && WireIdDifference(*hitCandidate, cdcHits[j]) < 2) { //only hits passing this cut are considered as neighbours

              if (hitCandidate->getCellState() == cdcHits[j].getCellState()) {

                //increment the state of the candidate
                hitCandidate->setTempCellState(hitCandidate->getCellState() + 1);
                stop = false;
              }
            }
          } //end second loop over all hits
        }
      } //end loop over all hits
    } //end loop over all layers


    //Simultaneous update of cellStates for all hits
    for (int i = 0; i < nHits; i++) {
      cdcHits[i].updateCellState();
    }


  } //end while

  //Print the final results
  /*  B2DEBUG(150,"LayerId    WireId     State");
    for (int i = 0; i < nHits; i++) {
      //print the states of hits which were currently evaluated
      if (cdcHits[i].getLayerId() > (startLayer + 1 - nLayers) && cdcHits[i].getLayerId() <= startLayer) {
        B2DEBUG(150,"   " << cdcHits[i].getLayerId() << "      " << cdcHits[i].getWireId() << "      " << cdcHits[i].getCellState());
      }
    }*/

}

void CellularSegmentFinder::CheckLayerNeighbours(const CDCTrackHit& aHit, vector<CDCTrackHit> & cdcHits)
{
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  int nHits = cdcHits.size();
  for (int j = 0; j < nHits; j++) {
    //check for hits from the same layer and a neighbouring wireId
    if (cdcHits[j].getLayerId() == aHit.getLayerId() && WireIdDifference(aHit, cdcHits[j]) == 1) {
      //check the distance between the wires and compare it to the sum of the drifttimes (drift length actually...)
      //for hits which really are coming from the same tracks these two number should be similar (tracks passed between two wires)
      double wireInterval = 2 * TMath::Pi() * cdcg.innerRadiusWireLayer()[aHit.getLayerId()] / cdcg.nWiresInLayer(aHit.getLayerId());
      if (abs(aHit.getDriftTime() + cdcHits[j].getDriftTime() - wireInterval) < 0.05 * wireInterval) {
        cdcHits[j].setSegmentIds(aHit.getSegmentIds());  //copy the segment Ids from the original hit to the "new same layer neighour"
      }
    }
  }

}

void CellularSegmentFinder::FindSegments(vector<CDCTrackHit> & cdcHits, vector <CDCSegment>& Segments)
{

  vector<CDCTrackHit> hitsArray [9]; //array to store TrackHit vectors, with track hits belonging to only one superlayer
  int nHits = cdcHits.size();
  B2DEBUG(100, "CellularSegmentFinder: Nr of TrackHits: " << nHits);
  for (int sl = 0; sl < 9 ; ++sl) {
    for (int i = 0; i < nHits; ++i) {
      if (cdcHits.at(i).getSuperlayerId() == sl) {
        hitsArray[sl].push_back(cdcHits.at(i));
      }
    }
  }

  Segments.reserve(nHits);


  int segmentcounter = -1; //counts the number of segments
  int oldcounter = 0;      //count the number of segments already created in the previous superlayers

  for (int sl = 8 ; sl >= 0; sl--) { //loop over all superlayers, starting in the outermost one

    //calculate the outermost layerId for this superlayer
    //'basic' means the ideal case where in each layer a hit will be found and so at the end the startLayerId should get the maximumCellState
    //however there are segment which are only a few hits long, or segments with missing hits
    //for that reason the startLayerId and maximumCellState will be changed in the further iterations

    int basicStartLayerId = sl * 6 + 7;
    int startLayerId = basicStartLayerId;

    //set number of layers for this superlayer as well as the maximum possible cellState
    int nLayers = 6;
    int basicMaximumCellState = 5;
    int maximumCellState = basicMaximumCellState;
    //one has to distinguish between the innermost superlayer (8 layers) and other superlayers (6 layers)
    if (sl == 0) {
      nLayers = 8 ;
      basicMaximumCellState = 7;
    } else {
      nLayers = 6;
      basicMaximumCellState = 5;
    }

    //now evaluate states of the hits

    //EvaluateStates(cdcHits, startLayerId, nLayers);
    EvaluateStates(hitsArray[sl], startLayerId, nLayers);


    //collect the hits based on their cellState
    while ((basicStartLayerId - startLayerId) < 4) { //loop over different starting layers, start in the outermost one
      maximumCellState = basicMaximumCellState;

      while (maximumCellState > 2) {  //loop over different maximum cellStates, start with the highest one

        for (unsigned int i = 0; i < hitsArray[sl].size(); i++) {   //loop over all hits

          CDCTrackHit* hitCandidate = & hitsArray[sl].at(i);

          //select hits with correct start parameters and no other segment id assigned (! to avoid duplication of segments)
          if (hitCandidate->getLayerId() == startLayerId && hitCandidate->getCellState() == maximumCellState && hitCandidate->getSegmentIds().size() == 0) {
            segmentcounter++;
            B2DEBUG(150, "Start new segment with " << hitCandidate->getLayerId() << "  " << hitCandidate->getWireId());
            Segments.push_back(CDCSegment(sl, segmentcounter)); //create a new segment and add it to the collection
            hitCandidate->setSegmentId(segmentcounter);  //set the correct segment Id

            //check if there is a close hit in the same layer
            //if such a hit is found, assign the same segment Id to it
            CheckLayerNeighbours(*hitCandidate, hitsArray[sl]);

            //now start searching for hits in other layers
            int layerId = startLayerId - 1;  //decrease layerId and cellState for the search
            int cellState = maximumCellState - 1;

            int stopLayerId = startLayerId - nLayers;  //last layer


            while (layerId > stopLayerId && cellState >= 0) {

              unsigned int j = 0;
              while (j < hitsArray[sl].size()) {
                if (hitsArray[sl].at(j).getLayerId() == layerId && hitsArray[sl].at(j).getCellState() == cellState) { //first check for acceptable hit candidate

                  if (WireIdDifference(*hitCandidate, hitsArray[sl].at(j)) < 2) { //decisive cut

                    B2DEBUG(150, "Add " << hitsArray[sl].at(j).getLayerId() << "  " << hitsArray[sl].at(j).getWireId());

                    //assign the correct segment Id
                    hitsArray[sl].at(j).setSegmentIds(hitCandidate->getSegmentIds());

                    //reassign(!) the hitCandidate pointer to the last found hit
                    hitCandidate = &hitsArray[sl].at(j);

                    //check if there is a close hit in the same layer
                    CheckLayerNeighbours(hitsArray[sl].at(j), hitsArray[sl]);
                    break;   //stop looping

                  }
                }
                j++;
              }  //end while hits

              layerId --;
              cellState --;


            } //end while layerId & cellState
          }
        } //end for loop over all hits

        maximumCellState--;

      } //end while maximumCellState

      startLayerId--;
    } //end while startLayerId


    //now that each hit has some segment Ids assigned, one can create the segments

    //use oldcounter to not overwrite already created segments
    for (int i = oldcounter; i <= segmentcounter; i++) {
      B2DEBUG(150, " Create Segment " << i);
      //assign hits to this segment
      for (unsigned int j = 0; j < hitsArray[sl].size(); j++) {
        for (unsigned int k = 0; k < hitsArray[sl].at(j).getSegmentIds().size(); k++) {
          if (hitsArray[sl].at(j).getSegmentIds().at(k) == i) {
            Segments.at(i).addTrackHit(hitsArray[sl].at(j));
            B2DEBUG(150, "Add Hit " << hitsArray[sl].at(j).getLayerId() << "  " << hitsArray[sl].at(j).getWireId());
          }
        }

      }
      B2DEBUG(150, "---> Total Nr of Hits: " << Segments.at(i).getNHits());

    } //end loop over new segments

    oldcounter = segmentcounter + 1;

  } //end loop over all superlayers


}
