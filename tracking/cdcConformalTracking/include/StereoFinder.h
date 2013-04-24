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


#include <tracking/cdcConformalTracking/CDCSegment.h>
#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>

namespace Belle2 {

  /** Class to assign segments from stereo superlayers to already existing track candidates.*/
  class StereoFinder {
  public:

    /** Constructor. */
    StereoFinder();

    /** Destructor. */
    ~StereoFinder();

    /** Returns the distance between the centers of the outermost track segment and the other segment in the normal plane. */
    static double SimpleDistance(CDCTrackCandidate& track, CDCSegment& segment);

    /** Returns the shortest distance between the track and the segment in the conformal plane.
     * ShortestDistance - method from AxialTrackFinder ist called.
     * The returned distance is very small if the segment belong to the track.
     */
    static double ShortestDistance(CDCTrackCandidate& track, CDCSegment& segment);

    /** This method calculates the 'wire distance' between a track and a segment.
     *  The difference between the WireIds of the outermost hit in the track and the outermost hit in the segment is calculated.
     */
    static int WireIdDifference(CDCTrackCandidate& track, CDCSegment& segment);


    /**Function to check if there are more than one segment from one superlayer in this track candidate.
     * With the given track candidate and the index of a segment, it is checked if there are other segments with the same superlayer in this track candidate.
     * Returns false is the given segment is the only one. Returns true if there is more than one.
     */
    static bool OvercrowdedSuperlayer(CDCTrackCandidate& track, int SLId);

    /** Method to check if there are more than one segment per superlayer and remove the wrong ones.
     *  Superlayer with more the one segment are considered and the competing segments are compared to each other.
     *  The comparison in performed in the conformal plane (2D).
     *  The 'best matching' segment is chosen and the other segments are removed (!) from candidate.
     */

    static void CheckOvercrowdedSL(CDCTrackCandidate& axialCandidate, CDCTrackCandidate& stereoCandidate);

    /** Method to check if there are more than one segment per superlayer and remove the wrong ones.
     *  Superlayer with more the one segment are considered and the competing segments are compared to each other.
     *  The comparison in performed in the rz plane, where the hits are supposed to form a straight line.
     *  A linear fit in performed and the segment with a worst deviation is removed.
     *  The track candidate is refitted, as long as there are still overcrowded superlayers.
     *  At the ende the track candidate should have one best matching segment left in each stereo superlayer.
     */
    static void StereoFitCandidates(CDCTrackCandidate& candidate);

    /**Searches for matching stereo segments for the given track candidate.
     * First parameter: track candidate
     * Second parameter: name of the CDCSegments array.
     * Third parameter: cut on the distance between the track and the segment in the normal plane.
     * Fourth parameter: cut an the 'shortest' distance between the track and the segment in the conformal plane.
     * Fifth parameter: superlayer to be searched.
     */
    static void FindStereoSegments(CDCTrackCandidate& startTrack, std::vector<CDCSegment> & cdcStereoSegments, double SimpleDistanceCut, double ShortDistanceCut, int SLId);


    /**Main method to append the stereo segments to the track candidates.
     * First parameter is the CDCSegment vector ('input'), second the name of the CDCTrackCandidates array ('output').
     * For each superlayer the possible track candidates are found (FindStereoSegments).
     * In the next step the segment coordinates are moved (shiftAlongZ) according to the direction information of the track candidate they may belong to.
     * After the 'best matching' coordinates are found, the segments have to pass another more strict cut to be assigned to the track candidate.
     * At the end the superlayers are checked to have only one segment, the best one is selected and the others are removed.
     */
    static void AppendStereoSegments(std::vector<CDCSegment> & cdcStereoSegments, std::vector<CDCTrackCandidate> & CDCTrackCandidates);

  private:


  }; //end class StereoFinder
} //end namespace Belle2

#endif //STEREOFINDER
