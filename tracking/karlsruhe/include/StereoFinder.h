/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#ifndef STEREOFINDER_H
#define STEREOFINDER_H


#include <tracking/karlsruhe/CDCSegment.h>
#include <tracking/karlsruhe/CDCTrack.h>

namespace Belle2 {

  class StereoFinder {
  public:

    /*! Constructor. */
    StereoFinder();

    /*! Destructor. */
    ~StereoFinder();


    /*!Find for a given Track a matching stereo Segment in the given superlayer. If a matching segment is found: returns his index in the StoreArray. If no mathing Segment is found: returns 9999. */
    /*!Seaches for a matching Segment for a given Track in a given superlayer. Two cuts are used: maxSimpleDistance: distance in the normal planebetween the innermost track point and the outermost segment point, angle between track and segment direction in the conformal plane. The algorithm cuts on angle and searches for segments with minimal distance (it loops also over all segments in the track). The index of the best segment found in the StoreArray is returned. If no segment within the given cuts is found, 9999 is returned.
    */
    static int FindNextStereoSegment(CDCTrack startTrack, std::string SegmentsCDCArray, int superlayerId, double maxSimpleDistance, double maxAngle);

    /*!Appends stereo Segments to a Track according to the direction information of the Segments.*/
    /*!Uses as input a StereoSegmentsCDCArray and a TracksCDCArray, output is a changed TracksCDCArray with new stereo segments appended to existing tracks. Search starts in the outermost stereo superlayer and the FindNextStereoSegment method is applied to find mathing segments in the following superlayers. As soon as a segment is found, his coordinates are shifted along the wire (shiftAlongZ method of the CDCSegment class) to find best matching to the track. The index gained by this shift is used to shift all other segment in a way to simplify the search for further segments for this track. This is performed for each Track. Each Segment can be used only once. */
    static void AppendStereoSegments(std::string StereoSegmentsCDCArray, std::string TracksCDCArray);

  private:


  }; //end class StereoFinder
} //end namespace Belle2

#endif //STEREOFINDER
