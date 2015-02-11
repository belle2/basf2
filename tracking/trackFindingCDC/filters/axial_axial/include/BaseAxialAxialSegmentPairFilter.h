/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BASEAXIALAXIALSEGMENTPAIRFILTER_H_
#define BASEAXIALAXIALSEGMENTPAIRFILTER_H_


#include "BaseAxialAxialSegmentPairFilter.h"
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialAxialSegmentPair.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Base class for filter for the constuction of axial to axial segment pairs.
    class BaseAxialAxialSegmentPairFilter {

    public:

      /// Constructor
      BaseAxialAxialSegmentPairFilter() {;}

      /// Empty destructor
      virtual ~BaseAxialAxialSegmentPairFilter() {;}

    public:

      /// Clears all remember information from the last event
      virtual void clear() {;}

      /// Forwards the modules initialize to the filter
      virtual void initialize() {;}

      /// Forwards the modules initialize to the filter
      virtual void terminate() {;}

      /// Checks if a pair of axial segments is a good combination
      virtual CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair&) { return NOT_A_CELL;}

    }; // end class BaseAxialAxialSegmentPairFilter


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //BASEAXIALAXIALSEGMENTPAIRFILTER_H_
