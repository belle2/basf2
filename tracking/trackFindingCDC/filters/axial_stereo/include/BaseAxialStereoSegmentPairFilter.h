/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BASEAXIALSTEREOSEGMENTPAIRFILTER_H
#define BASEAXIALSTEREOSEGMENTPAIRFILTER_H

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class BaseAxialStereoSegmentPairFilter {

    public:

      /// Constructor
      BaseAxialStereoSegmentPairFilter() {;}

      /// Empty destructor
      virtual ~BaseAxialStereoSegmentPairFilter() {;}

    public:

      /// Clears all remember information from the last event
      virtual void clear() {;}

      /// Forwards the modules initialize to the filter
      virtual void initialize() {;}

      /// Forwards the modules initialize to the filter
      virtual void terminate() {;}

      /// Checks if a pair of axial segments is a good combination. Base implementation always rejects the cell by returning NOT_A_CELL.
      virtual CellWeight isGoodAxialStereoSegmentPair(const CDCAxialStereoSegmentPair&) { return NOT_A_CELL; }

    }; // end class BaseAxialStereoSegmentPairFilter


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //BASEAXIALSTEREOSEGMENTPAIRFILTER_H
