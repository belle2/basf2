/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef AXIALTRACKFINDER_H
#define AXIALTRACKFINDER_H

#include <tracking/cdcConformalTracking/CDCSegment.h>
#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>


namespace Belle2 {
  /** Class to create track candidates in the CDC by connecting segments from axial superlayers. */
  class AxialTrackFinder {
  public:

    /** Constructor. */
    AxialTrackFinder();

    /** Destructor.*/
    ~AxialTrackFinder();

    /** Returns the shortest distance between the first point and the straight line in the given direction through this point and the other point.
     * Performs some simple geometric calculations.
     * A perpendicular (to the given direction) line through the second point is formed, then the intersection point between this line and a line build through the first point and his direction is calculated.
     * The distance between the second point and this intersection point is returned.
     */
    static double ShortestDistance(TVector3& firstPoint, TVector3& firstDirection, TVector3& secondPoint);

    /** Returns the shortest distance between one point of one segment and the straight line build through the outer most point of the other segment and his direction in the conformal plane.
     * Performs some simple geometrical calculations (see ShortestDistance(TVector3 firstPoint, TVector3 firstDirection, TVector3 secondPoint))in the conformal plane (where segments and tracks are assumed to form straight lines).
     * This returned distance is very small for segments belonging to the same track.
     */
    static double ShortestDistance(CDCSegment& segment1, CDCSegment& segment2);

    /** Returns the distance between the centers of both segments in the normal plane.  */
    static double SimpleDistance(CDCSegment& segment1, CDCSegment& segment2);

    /** Returns a fraction of segments already used to reconstruct another track.
     * First parameter is a track which segments will be checked.
     * Second parameter is a vector with Id's of already used segments.
     */
    static float UsedSegmentsFraction(CDCTrackCandidate& track, std::vector<int>& UsedSegmentId);

    /** Returns a fraction of hits already used to reconstruct another track.
     * First parameter is a track which hits will be checked.
     * Second parameter is a vector with Id's of already used hits.
     */
    static float UsedHitsFraction(CDCTrackCandidate& track, std::vector<int>& UsedTrackHitId);

    /** Returns the track candidate with the smallest chi2 from the given track candidate vector.
     *  One 'real' track may have multiple track candidates which differ by some wrongly assigned hits/segments.
     *  If the tracks are 'too similar', the one with the lowest chi2 (from the linear fit in the conformal plane) is selected.
     */
    static int EvaluateBestCandidate(const std::vector <CDCTrackCandidate>& candidates, const std::vector <int>& indices);


    /** Searchs for track candidates from given segments and within the given cuts.
     * First parameter: vector with axial Segments.
     * Second parameter: cut on the distance between two segments in the normal plane (required condition for segments to be 'neighbours').
     * Third parameter: cut on the angle between the two segments in the conformal plane.
     * Fourth parameter: cut an the 'shortest' distance between two segments in the conformal plane (essential condition for segments to be 'neighbours').
     * Fifth parameter: superlayer in which the track candidates should start.
     * First the method searches for segments in the given starting superlayer, creates an equal number of track candidates and adds these segments to these tracks.
     * In the next step the method searches for further segments for already existing track candidates in other superlayers.
     * If more than one segment from the same superlayer is 'neighbour' to another, the candidate is split and the two candidates are propagated independently.
     * A vector filled with found track candidates is returned.
     */
    static std::vector<CDCTrackCandidate> FindTrackCandidates(std::vector<CDCSegment>& cdcAxialSegments, double SimpleDistanceCut, double AngleCut, double ShortDistanceCut, int StartSLId);

    /** Performs a simple linear fit of the track in the conformal plane, calculates and assigns chi2, transverse momentum and charge estimation to the track candidate.
     *  Conformal coordinates of all hits (excluding the first layer) of the given track candidate are fitted with a straight line.
     *  Resulting chi2 value as well as an estimation of the transverse momentum value and charge are assigned to the track candidate.
     */
    static void FitTrackCandidate(CDCTrackCandidate& candidate);

    /** Performs a simple linear fit of all the tracks in the conformal plane, calculates and assigns chi2, transverse momentum and charge estimation to the track candidates.
     *  Conformal coordinates of all hits (excluding the first layer) of the given track candidate are fitted with a straight line.
     */
    static void FitTrackCandidates(std::vector<CDCTrackCandidate>& candidates);

    /** Main method of to create track candidates from segments.
     * First parameter is a vector with axial Segments ('input'), second the name of the CDCTrackCandidates array ('output').
     * For each start superlayer the possible track candidates are found (FindTrackCandidates) and fitted (FitTrackCandidates). Those with bad chi2 are rejected.
     * In the next step the best tracks are collected. When a track is added to the final array, 'his' segments/hits are marked as used.
     * Only candidates with enough unused segments can be added to the final array. (The search starts with the longest candidates).
     * An additional step is performed for 'short' (2 or 3 segments) candidates to find out if they can be combined to one 'long' (4 or 5 segments) candidate.
     */
    static void CollectTrackCandidates(std::vector<CDCSegment>& cdcAxialSegments, std::vector<CDCTrackCandidate>& CDCTrackCandidates);


  private:


  }; //end class AxialTrackFinder
} //end namespace Belle2

#endif //AXIALTRACKFINDER

