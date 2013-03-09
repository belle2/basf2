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

double CellularSegmentFinder::SimpleDistance(CDCTrackHit hit1,
                                             CDCTrackHit hit2)
{

  double simpleDistance = sqrt(pow((hit1.getWirePosition().x() - hit2.getWirePosition().x()), 2) + pow(
                                 (hit1.getWirePosition().y() - hit2.getWirePosition().y()), 2));

  return simpleDistance;
}

int CellularSegmentFinder::WireIdDifference(CDCTrackHit hit1, CDCTrackHit hit2)
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

void CellularSegmentFinder::EvaluateStates(vector<CDCTrackHit> & cdcHits, int startLayer, int nLayers)
{

  bool stop = false; //boolean to mark when the states have reached their final values
  int nHits = cdcHits.size();

  while (stop == false) {
    stop = true;
    //start with the outermost layer and loop over all layers (except the innermost one, because this layer has no further neighbours and its state won't change anyway)
    for (int layerId = startLayer; layerId > (startLayer + 1 - nLayers); layerId = layerId - 1) {
      for (int i = 0; i < nHits; i++) {  //loop over all hits
        if (cdcHits.at(i).getLayerId() == layerId) { //hits from the current layer
          CDCTrackHit* hitCandidate = &cdcHits[i];

          for (int j = 0; j < nHits; j++) { //now loop again over all hits to find neighbours in the following layer

            if (cdcHits[j].getLayerId() == layerId - 1 && WireIdDifference(*hitCandidate, cdcHits[j]) < 2) { //only hits passing this cut are considered as neighbours
              //B2INFO ("Neighbour found: "<<cdcHits[j].getLayerId()<<"  "<<cdcHits[j].getWireId()<<" wireDiff: "<<WireIdDifference(*hitCandidate, cdcHits[j])<<" simple distance: "<<SimpleDistance(*hitCandidate,cdcHits[j]));

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
  B2DEBUG(149, "LayerId    WireId     State");
  for (int i = 0; i < nHits; i++) {
    //print the states of hits which were currently evaluated
    if (cdcHits[i].getLayerId() > (startLayer + 1 - nLayers) && cdcHits[i].getLayerId() <= startLayer) {
      B2DEBUG(149, "   " << cdcHits[i].getLayerId() << "      " << cdcHits[i].getWireId() << "      " << cdcHits[i].getCellState());
    }
  }

}

void CellularSegmentFinder::CheckLayerNeighbours(CDCTrackHit aHit, vector<CDCTrackHit> & cdcHits)
{
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  int nHits = cdcHits.size();
  for (int j = 0; j < nHits; j++) {
    //check for hits from the same layer and a neighbouring wireId
    if (cdcHits[j].getLayerId() == aHit.getLayerId() && WireIdDifference(aHit, cdcHits[j]) == 1) {
      //B2INFO("A possible neighbour in the same layer was found in "<<cdcHits[j].getLayerId()<<"  "<<cdcHits[j].getWireId()<<" (for "<<aHit.getLayerId()<<"  "<<aHit.getWireId()<<")" );
      //check the distance between the wires and compare it to the sum of the drifttimes (drift length actually...)
      //for hits which really are coming from the same tracks these two number should be similar (tracks passed between two wires)
      double wireInterval = 2 * TMath::Pi() * cdcg.innerRadiusWireLayer()[aHit.getLayerId()] / cdcg.nWiresInLayer(aHit.getLayerId());
      //B2INFO("Drifttime of the first  "<<aHit.getDriftTime()<<"   and of the second "<<cdcHits[j].getDriftTime()<<"  distance between wires "<<wireInterval);
      if (abs(aHit.getDriftTime() + cdcHits[j].getDriftTime() - wireInterval) < 0.05 * wireInterval) {
        //B2INFO("-----> Real neighbour found!");
        cdcHits[j].setSegmentIds(aHit.getSegmentIds());  //copy the segment Ids from the original hit to the "new same layer neighour"
      }
    }
  }

}

void CellularSegmentFinder::FindSegments(string CDCTrackHits, string CDCSegments)
{

  StoreArray<CDCTrackHit> cdcTrackHits(CDCTrackHits.c_str());
  StoreArray<CDCSegment> cdcSegments(CDCSegments.c_str());

  int nHits = cdcTrackHits.getEntries();
  B2DEBUG(100, "CellularSegmentFinder: Nr of TrackHits: " << nHits);

  //create vectors for easier handling
  vector<CDCTrackHit> cdcHits;
  vector<CDCSegment> Segments;

  //'fill' the vector with all TrackHits
  for (int i = 0; i < nHits; i++) {
    cdcHits.push_back(*cdcTrackHits[i]);
  }


  int segmentcounter = -1; //counts the number of segments
  int oldcounter = 0;      //count the number of segments already created in the previous superlayers

  for (int sl = 8 ; sl >= 0; sl--) { //loop over all superlayers, starting in the outermost one
    B2DEBUG(100, "-------------------------------------------------------");
    B2DEBUG(100, "                SUPERLAYER: " << sl)
    B2DEBUG(100, "********** EVALUATE STATES ****************")

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
    EvaluateStates(cdcHits, startLayerId, nLayers);

    //collect the hits based on their cellState
    B2DEBUG(100, "********** COLLECT HITS ****************")
    while ((basicStartLayerId - startLayerId) < 4) { //loop over different starting layers, start in the outermost one
      maximumCellState = basicMaximumCellState;

      while (maximumCellState > 2) {  //loop over different maximum cellStates, start with the highest one

        for (int i = 0; i < nHits; i++) {   //loop over all hits
          CDCTrackHit* hitCandidate = &cdcHits[i];

          //select hits with correct start parameters and no other segment id assigned (! to avoid duplication of segments)
          if (hitCandidate->getLayerId() == startLayerId && hitCandidate->getCellState() == maximumCellState && hitCandidate->getSegmentIds().size() == 0) {
            segmentcounter++;
            //B2INFO("Start new segment with "<<hitCandidate->getLayerId()<<"  "<<hitCandidate->getWireId());
            Segments.push_back(CDCSegment(sl, segmentcounter)); //create a new segment and add it to the collection
            hitCandidate->setSegmentId(segmentcounter);  //set the correct segment Id

            //check if there is a close hit in the same layer
            //if such a hit is found, assign the same segment Id to it
            CheckLayerNeighbours(*hitCandidate, cdcHits);

            //now start searching for hits in other layers
            int layerId = startLayerId - 1;  //decrease layerId and cellState for the search
            int cellState = maximumCellState - 1;

            int stopLayerId = startLayerId - nLayers;  //last layer


            while (layerId > stopLayerId && cellState >= 0) {

              int j = 0;
              while (j < nHits) {
                if (cdcHits[j].getLayerId() == layerId && cdcHits[j].getCellState() == cellState) { //first check for acceptable hit candidate
                  //B2INFO("         possible neighbour with WireId "<<cdcHits[j].getWireId()<<" and distance "<<SimpleDistance(*hitCandidate, cdcHits[j]) <<" and "<<WireIdDifference(*hitCandidate, cdcHits[j]));
                  if (WireIdDifference(*hitCandidate, cdcHits[j]) < 2) { //decisive cut

                    //B2INFO("Add "<<cdcHits[j].getLayerId()<<"  "<<cdcHits[j].getWireId());

                    //assign the correct segment Id
                    cdcHits[j].setSegmentIds(hitCandidate->getSegmentIds());

                    //reassign(!) the hitCandidate pointer to the last found hit
                    hitCandidate = &cdcHits[j];

                    //check if there is a close hit in the same layer
                    CheckLayerNeighbours(cdcHits[j], cdcHits);
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
    B2DEBUG(100, "********** CREATE SEGMENTS ****************")

    //use oldcounter to not overwrite already created segments
    for (int i = oldcounter; i <= segmentcounter; i++) {
      B2DEBUG(149, "Create Segment Nr " << i << "  in Superlayer " << sl);
      new(cdcSegments.nextFreeAddress()) CDCSegment(sl, i);    //create Segment and put it in the StoreArray

      //assign hits to this segment
      for (int j = 0; j < nHits; j++) {
        for (unsigned int k = 0; k < cdcHits[j].getSegmentIds().size(); k++) {
          if (cdcHits[j].getSegmentIds().at(k) == i) {
            cdcSegments[i]->addTrackHit(cdcHits[j]);
            B2DEBUG(149, "Add Hit " << cdcHits[j].getLayerId() << "  " << cdcHits[j].getWireId());
          }
        }

      }
      B2DEBUG(100, "---> Total Nr of Hits: " << cdcSegments[i]->getNHits());

    } //end loop over new segments

    oldcounter = segmentcounter + 1;

  } //end loop over all superlayers


}
