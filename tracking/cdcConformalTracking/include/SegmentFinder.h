/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    /** Classifies the Segments as good or bad.
     * Segments with number of hits between minHits and maxHits and WireId difference below nWires are classified as good.
     * */
    static void ClassifySegments(std::string CDCSegments, int minHits, int maxHits, int nWires);

    /** Collects the TrackHits to Segments for each axial superlayer.
     * Uses as Input an CDCTrackHitArray and as output an SegmentsCDCArray.
     * For each superlayer a root-histogram with the phi-values of the TrackHits is created.
     * Before the filling of the histogram the phi values are shifted in a way, that no cluster is separated by the filling process (no simultaneous entries in the first and last bins).
     * Then clusters in this histograms are found and the corresponding TrackHits are grouped in a Segment object.
     * root_output: boolean to define if some output files with the histograms should be created (one .root and one .pdf file)
     */
    static void FindSegments(std::string CDCTrackHits, std::string CDCSegments, bool root_output = false);

    // Methods for Segment Fitting
    //--------------------------------------------------------------------------------------
    /** Searches for layers within the Segment which contain to many hits.
     *  All layers from this segment are retrieved and the number of hits in each layer is counted.
     *  If there are more hits in this layer than given number (nWires), the layer is classified as overcrowded.
     *  A vector with Id's of overcrowded layers is returned.
     */
    //static std::vector<int> FindOvercrowdedLayers(CDCSegment segment, int nWires) ;
    /** Fits a the segments in the conformal plane and removes bad Hits.
     * A simple linear fit in the conformal plane is performed.
     * Distance from each hit point to the fit line is calculated.
     * Those hits for which this distance is above given cuts (still under developement) are removed from segment, afterwards the segment is refitted.
     * When a layer is overcrowded with hits (too many hits in same layer), the cuts are more strict.
     */
    //static void FitSegments(std::string CDCSegments);
    //--------------------------------------------------------------------------------------

  private:


  }; //end class SegmentFinder
} //end namespace Belle2

#endif //SEGMENTFINDER


