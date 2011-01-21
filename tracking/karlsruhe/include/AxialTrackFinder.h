/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#ifndef AXIALTRACKFINDER_H
#define AXIALTRACKFINDER_H

#include <tracking/karlsruhe/CDCSegment.h>
#include <tracking/karlsruhe/CDCTrack.h>


namespace Belle2 {
  /** Class to create track candidates in the CDC by connecting segments from axial superlayers. */
  class AxialTrackFinder {
  public:

    /** Constructor. */
    AxialTrackFinder();

    /** Destructor.*/
    ~AxialTrackFinder();

    /** Returns the shortest distance between the first point and the straight line in the given direction through this point and the other point.
     * Performs some simple geometrical calculations.
     * A perpendicular (to the given direction) line through the second point is formed, then the intersection point between this line and a line build through the first point and his direction is calculated.
     * The distance between the second point and this intersection point is returned.
     */
    static double ShortestDistance(TVector3 firstPoint, TVector3 firstDirection, TVector3 secondPoint);

    /** Returns the shortest distance between one point of one segment and the straight line build through the outer most point of the other segment and his direction in the conformal plane.
     * Performs some simple geometrical calculations (see ShortestDistance(TVector3 firstPoint, TVector3 firstDirection, TVector3 secondPoint))in the conformal plane (where segments and tracks are assumed to form straight lines).
     * This returned distance is very small for segments belonging to the same track.
     */
    static double ShortestDistance(CDCSegment segment1, CDCSegment segment2);

    /** Returns the distance between the centers of both segments in the normal plane.  */
    static double SimpleDistance(CDCSegment segment1, CDCSegment segment2);

    /** Returns a fraction of segment alredy used to reconstruct another track.
     * First parameter ist a track which segments will be checked.
     * Second parameter is a vector with Ids of already used segments.
     */
    static float UsedSegmentsFraction(CDCTrack track, std::vector<int> & UsedSegmentId);

    /** Searchs for track candidates from given segments and within the given cuts.
     * First parameter: name of the CDCSegments array.
     * Second parameter: cut on the distance between two segments in the normal plane (required condition for segments to be 'neighbours').
     * Third parameter: cut an the 'shortest' distance between two segments in the conformal plane (essential condition for segments to be 'neighbours').
     * Fourth parameter: superlayer in which the track candidates should start.
     * First the method searchs for good segments in the given starting superlayer, creates an equal number of track candidates and adds these segments to these tracks.
     * In the next step the method searchs for further segments for already existing track candidates in other superlayers.
     * If more then one segment from the same superlayer is 'neighbour' to another, the candidate is split and the two candidates are propagated independently.
     * A vector filled with found track candidates is returned.
     */
    static std::vector<CDCTrack> FindTrackCandidates(std::string SegmentsCDCArray, double SimpleDistanceCut, double ShortDistanceCut, int StartSLId);

    /** Performs a simple linear fit of the track in the conformal plane, assigns Chi2 and p to the track.
     *  Conformal coordinates of all hits (excluding the first layer) of the given track candidate are fitted with a straight line.
     *  Resulting chi2 value as well as a (very preliminary!) estimation of the absolute momentum value are assigned to the track candidate.
     */
    static void FitTrackCandidate(CDCTrack & candidate);

    /** Performs a simple linear fit of all the tracks in the conformal plane, assigns Chi2 and p to the tracks.*/
    static void FitTrackCandidates(std::vector<CDCTrack> & candidates);

    /** Main method of to create track candidates from segments.
     * First parameter is the name of the CDCSegments array ('input'), second the name of the CDCTracks array ('output').
     * For each start superlayer the possible track candidates are found (FindTrackCandidates) and fitted (FitTrackCandidates). Those with bad Chi2 are rejected.
     * In the next step the best tracks are collected. When a track is added to the final array, 'his' segments are marked as used.
     * Only candidates with enough unused segments can be added to the final array. (The search starts with the longest candidates).
     * An additional step ist performed for 'short' (2 or 3 segments) candidates to find out if they can be combined to one 'long' (4 or 5 segments) candidate.
     */
    static void CollectTrackCandidates(std::string SegmentsCDCArray, std::string TracksCDCArray);


  private:


  }; //end class AxialTrackFinder
} //end namespace Belle2

#endif //AXIALTRACKFINDER

