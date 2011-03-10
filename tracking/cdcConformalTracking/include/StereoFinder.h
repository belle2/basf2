/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STEREOFINDER_H
#define STEREOFINDER_H


#include <tracking/karlsruhe/CDCSegment.h>
#include <tracking/karlsruhe/CDCTrackCandidate.h>

namespace Belle2 {

  /** Class to assign segments from stereo superlayers to already existing track candidates.*/
  class StereoFinder {
  public:

    /** Constructor. */
    StereoFinder();

    /** Destructor. */
    ~StereoFinder();

    /** Returns the distance between the centers of the outermost track segment and the other segment in the normal plane. */
    static double SimpleDistance(CDCTrackCandidate track, CDCSegment segment);

    /** Returns the shortest distance between the track and the segment in the conformal plane.
     * ShortestDistance - method from AxialTrackFinder ist called.
     * The returned distance is very small if the segment belong to the track.
     */
    static double ShortestDistance(CDCTrackCandidate track, CDCSegment segment);

    /**Function to check if there are more than one segment from one superlayer in this track candidate.
     * With the given track candidate and the index of a segment, it is checked if there are other segments with the same superlayer in this track candidate.
     * Returns false is the given segment is the only one. Returns true if there is more than one.
     */
    static bool SegmentFromOvercrowdedSL(CDCTrackCandidate track, int SegmentIndex) ;

    /**Fits a track candidate and removes bad segments (this method is still under developement...).
     * Performs a simple straight line fit in the conformal plane and a straight line fit in the r z plane.
     * In the case of bad Chi2, the distance between the segments and the fit line is checked.
     * If this distance is above a given cut, the segment is removed and the candidate is refitted.
     */
    static void StereoFitCandidates(CDCTrackCandidate & candidate);

    /**Searches for matching stereo segments for the given track candidate.
     * First parameter: track candidate
     * Second parameter: name of the CDCSegments array.
     * Third parameter: cut on the distance between the track and the segment in the normal plane.
     * Fourth parameter: cut an the 'shortest' distance between the track and the segment in the conformal plane.
     * Fifth parameter: superlayer to be searched.
     */
    static void FindStereoSegments(CDCTrackCandidate startTrack, std::vector<CDCSegment> & cdcStereoSegments, double SimpleDistanceCut, double ShortDistanceCut, int SLId);


    /**Main method to append the stereo segments to the track candidates.
     * First parameter is the CDCSegment vector ('input'), second the name of the CDCTrackCandidates array ('output').
     * For each superlayer the possible track candidates are found (FindStereoSegments).
     * In the next step the segment coordinates are moved (shiftAlongZ) according to the direction information of the track candidate they may belong to.
     * After the 'best matching' coordinates are found, the segments have to pass another more strict cut to be assigned to the track candidate.
     * At the end the (stereo) candidates are fittet and segments too far away from the fit line are removed.
     */
    static void AppendStereoSegments(std::vector<CDCSegment> & cdcStereoSegments, std::string CDCTrackCandidates);


  private:


  }; //end class StereoFinder
} //end namespace Belle2

#endif //STEREOFINDER
