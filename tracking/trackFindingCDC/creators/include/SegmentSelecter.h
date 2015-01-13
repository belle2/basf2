/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SEGMENTSELECTER_H_
#define SEGMENTSELECTER_H_

#include <tracking/cdcLocalTracking/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>

//#include <set>
//#include <vector>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Implementation of the unique assignement analysis of hits to segments. To be removed soonish
    class SegmentSelecter {

    public:

      /// Empty constructor.
      SegmentSelecter();

      /// Empty destructor.
      virtual ~SegmentSelecter();

      /// Copies segments from the in vector to the out vector, based on selection criteria.
      void selectSegments(
        std::vector<CDCRecoSegment2D>& in,
        std::vector<CDCRecoSegment2D>& out
      ) const;

    private:

    }; // end class SegmentSelecter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SEGMENTSELECTER_H_
