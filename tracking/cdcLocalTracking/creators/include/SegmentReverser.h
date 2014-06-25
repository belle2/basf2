/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SEGMENTREVERSER_H_
#define SEGMENTREVERSER_H_

#include <tracking/cdcLocalTracking/eventdata/segments/CDCRecoSegment2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the appending of reversed segments
    class SegmentReverser {

    public:
      /// Constructor.
      SegmentReverser();

      /// Destructor.
      virtual ~SegmentReverser();

      /// Appends a copy of each segment in the vector to the end of the vector.
      void appendReversed(std::vector<CDCRecoSegment2D>& segments) const;

    private:
      /// Copies the hit content of the given segment to the other given segment but in reverse order.
      inline void reverse(const CDCRecoSegment2D& segment, CDCRecoSegment2D& reverseSegment) const;

    }; // end class SegmentReverser

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SEGMENTREVERSER_H_
