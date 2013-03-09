/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CELLULARSEGMENTFINDER_H_
#define CELLULARSEGMENTFINDER_H_


#include <tracking/cdcConformalTracking/CDCTrackHit.h>
#include <tracking/cdcConformalTracking/CDCSegment.h>


namespace Belle2 {

  /** Class to create segments ( = tracklets within one superlayer) using cellular automaton (CA).*/
  class CellularSegmentFinder {
  public:

    /** Constructor. */
    CellularSegmentFinder();

    /** Destructor.*/
    ~CellularSegmentFinder();

    /** This method calculates the distance (in 2D) between two hits.
     *  It was originally used as criteria for the hits to be neighbours, but the method was replaced by a more fast and simple WireIdDifference method.
     *  It may however be still useful for some checks.
     */
    static double SimpleDistance(CDCTrackHit hit1, CDCTrackHit hit2);

    /** This method calculates the 'wire distance' between two hits.
     *  The difference between the WireIds of both hits is calculated.
     *  To get the right number even if the difference is 'crossing' the 0 WireId, CDCGeometryParameters are called to get the total wire number per layer and to correct the difference.
     */
    static int WireIdDifference(CDCTrackHit hit1, CDCTrackHit hit2);

    /** This method evaluates CA-states of the hits.
     *  First parameter: reference (!) to the hit vector.
     *  Second parameter: starting outermost layer.
     *  Third parameter: number of layer int the given superlayer (usually 6, in the innermost 8)
     *  This method is executed for each superlayer. The states of all hits in this superlayer are evaluated.
     */
    static void EvaluateStates(std::vector<CDCTrackHit> & cdcHits, int startLayer, int nLayers);

    /** This method checks for hit neighbours in the same layer.
     *  Per default only 'single hit chain' is constructed. There are however often two hits in one layer.
     *  So for a given hit (first parameter) it is checked, if there is a close neighbour in the same layer.
     *  If such a neighbour hit is found, it is checked if the drift times of the two hits are 'compatible' (their sum should be approximately the distance between the wires).
     *  If such a neighbour hit is found, this hit get the same segment ids assigned as the given hit.
     */
    static void  CheckLayerNeighbours(CDCTrackHit aHit, std::vector<CDCTrackHit> & cdcHits);

    /** This is the main method of the CellularSegmentFinder.
      * First parameter: name of the StoreArray with CDCTrackHits (input)
      * Second parameter: name of the StoreArray with CDCSegments (output)
      * This method uses cellular automaton to find segments.
      * In the first step the states of all hits are evaluated. The criteria for the hits to be 'neighbours' is their WireId. WireIdDifference of two hits in the consecutive layers should be < 2.
      * In the next step the hits are collected to segments: the segments start in the outermost layer and the segment ids are assigned to the hits.
      * An "average" track produces one or two hits in one CDC layer. The SegmentFinder searches only for a 'single hit chain' at first.
      * An additional method is executed each time a neighbouring hit is found: it is checked if there is a close neighbour in the same layer (WireIdDifference == 1) and this hit get also the same segment Id.
      * In the last step all the segments are finally created and added to the StoreArray (and all hits get assigned accordingly to their segment Ids, the same hit can be used by different segments)
      */
    static void FindSegments(std::string CDCTrackHits, std::string CDCSegments);

  private:


  }; //end class CellularSegmentFinder
} //end namespace Belle2

#endif //CellularSEGMENTFINDER
