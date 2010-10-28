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

  class AxialTrackFinder {
  public:

    /*! Constructor. */
    AxialTrackFinder();

    /*! Destructor.*/
    ~AxialTrackFinder();

    /*!Calculates the shortest distance between one point of the segment and the straight line build through the outer most point of the Track and his direction in the conformal plane. */
    /*!Performs some simple geometrical calculations in the conformal plane (where segments and tracks are assumed to form straight lines). A perpendicular (to the track direction) line through one segment point (inner most) is formed, then the intersection point between this line and a line build through the outer most point of the Track and his direction is calculated. The distance between the segment point and this intersection point is returned.
     */
    static double ShortestDistance(CDCTrack track, CDCSegment segment);

    /*!Finds for a given track a matching Segment in the given superlayer. If a matching segment is found: returns his index in the StoreArray. If no mathing Segment is found: returns 9999.*/
    /*!Seaches for a matching Segment for a given Track in a given superlayer. Three cuts are used: maxSimpleDistance: distance in the normal plane between the innermost track point and the outermost segment point, maxDistance: shortest distance between the Segment and the Track in the conformal plane, maxAngle: angle between track and segment direction in the conformal plane. The algorithm cuts on simple distance and snd angle and earches for segments with minimal distance and. The index of the best segment found in the StoreArray is returned. If no segment within the given cuts is found, 9999 is returned.    */
    static int FindNextSegment(CDCTrack track, std::string SegmentsCDCArray, int superlayerId, double maxSimpleDistance, double maxDistance, double maxAngle);

    /*!Connects the Segments to a Track according to the direction information of the Segments.*/
    /*!Uses as Input an SegmentsCDCArray and as output an TracksCDCArray. Search starts in the outermost superlayer and the FindNextSegment method is applied to find mathing segments in the following superlayers. Each Segment can be used only once. If the search for tracks starting in the outermost superlayer is complete, new tracks are started in the next to outermost superlayer.
     */
    static void ConnectSegments(std::string SegmentsCDCArray, std::string TracksCDCArray);


  private:


  }; //end class AxialTrackFinder
} //end namespace Belle2

#endif //AXIALTRACKFINDER

