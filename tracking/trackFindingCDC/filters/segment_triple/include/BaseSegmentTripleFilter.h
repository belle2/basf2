/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BASESEGMENTTRIPLEFILTER_H_
#define BASESEGMENTTRIPLEFILTER_H_

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/algorithms/NeighborWeight.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment triples based on simple criterions
    class BaseSegmentTripleFilter {

    public:
      /** Constructor. */
      BaseSegmentTripleFilter() {;}

      /** Destructor.*/
      virtual ~BaseSegmentTripleFilter() {;}

    public:
      /// Clears all remember information from the last event
      virtual void clear() {;}

      /// Forwards the modules initialize to the filter
      virtual void initialize() {;}

      /// Forwards the modules initialize to the filter
      virtual void terminate() {;}

      /// Base implementation return NOT_A_NEIGHBOT for all segment triples.
      virtual CellWeight isGoodSegmentTriple(const CDCSegmentTriple&) { return NOT_A_NEIGHBOR; }

    }; // end class BaseSegmentTripleFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //BASESEGMENTTRIPLEFILTER_H_
