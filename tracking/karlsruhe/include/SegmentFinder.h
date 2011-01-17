/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/


#ifndef SEGMENTFINDER_H
#define SEGMENTFINDER_H

#include "TH1F.h"

#include <tracking/karlsruhe/CDCTrackHit.h>
#include <tracking/karlsruhe/CDCSegment.h>



namespace Belle2 {

  /** Class to create segments ( = tracklets within one superlayer).*/
  class SegmentFinder {
  public:

    /** Constructor. */
    SegmentFinder();

    /** Destructor.*/
    ~SegmentFinder();

    /** Utility function: checks if the given value is between min and max. */
    static bool IsValueWithinRange(double value, double min, double max);


    /**Calculates the conformal coordinates of a CDCTrackHitArray and adds the results to the member variables of the TrackHits. */
    static void ConformalTransformation(std::string CDCTrackHitArray);


    /** Classifies the Segments as good or bad.
     * Segments with number of hits between minHits and maxHits and WireId difference below nWires are classified as good.
     * */
    static void ClassifySegments(std::string SegmentsCDCArray, int minHits, int maxHits, int nWires);

    /** Collects the TrackHits to Segments for each axial superlayer.
     * Uses as Input an CDCTrackHitArray and as output an SegmentsCDCArray.
     * For each superlayer a root-histogram with the phi-values of the TrackHits is created.
     * Before the filling of the histogram the phi values are shifted in a way, that no cluster is separated by the filling process (no simultaneous entries in the first and last bins).
     * Then clusters in this histograms are found and the corresponding TrackHits are grouped in a Segment object.
     * root_output: boolean to define if some output files with the histograms should be created (one .root and one .pdf file)
     */
    static void FindSegments(std::string CDCTrackHitArray, std::string SegmentsCDCArray, bool root_output = true);


  private:


  }; //end class SegmentFinder
} //end namespace Belle2

#endif //SEGMENTFINDER


