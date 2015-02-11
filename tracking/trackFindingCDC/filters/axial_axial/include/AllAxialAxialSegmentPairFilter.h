/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALLAXIALAXIALSEGMENTPAIRFILTER_H_
#define ALLAXIALAXIALSEGMENTPAIRFILTER_H_

#include "BaseAxialAxialSegmentPairFilter.h"
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialAxialSegmentPair.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter accepting all axial to axial segment pairs.
    class AllAxialAxialSegmentPairFilter  : public BaseAxialAxialSegmentPairFilter {

    public:

      /// Constructor
      AllAxialAxialSegmentPairFilter() {;}

      /// Empty destructor
      virtual ~AllAxialAxialSegmentPairFilter() {;}

    public:
      /// Clears all remember information from the last event
      virtual void clear() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void initialize() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void terminate() override final {;}

      /// Checks if a pair of axial segments is a good combination
      virtual CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair&) override final;

    }; // end class AllAxialAxialSegmentPairFilter


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //ALLAXIALAXIALSEGMENTPAIRFILTER_H_
