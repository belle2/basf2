/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCSEGMENTTRIPLEFILTER_H_
#define MCSEGMENTTRIPLEFILTER_H_

#include <tracking/cdcLocalTracking/filters/axial_axial/MCAxialAxialSegmentPairFilter.h>
#include <tracking/cdcLocalTracking/eventdata/tracks/CDCSegmentTriple.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Filter for the constuction of segment triples based on monte carlo information
    class MCSegmentTripleFilter {

    public:
      /// Empty constructor
      MCSegmentTripleFilter();

      /// Empty destructor
      ~MCSegmentTripleFilter();

    public:
      /// May be used to clear information from former events. Currently unused.
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

      /// Check if the segment triple is aligned in the Monte Carlo track. Signals NOT_A_CELL if not.
      CellWeight isGoodSegmentTriple(const CDCSegmentTriple& triple, bool allowBackward = false) const;

    private:
      /// Sets the trajectories of the segment triple from Monte Carlo information. IS executed for good segment triples.
      void setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const;

    private:
      MCAxialAxialSegmentPairFilter m_mcAxialAxialSegmentPairFilter;
    }; // end class MCSegmentTripleFilter
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //MCSEGMENTTRIPLEFILTER_H_
