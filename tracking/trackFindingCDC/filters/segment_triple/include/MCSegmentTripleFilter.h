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

#include <tracking/trackFindingCDC/filters/axial_axial/MCAxialAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include "BaseSegmentTripleFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of segment triples based on monte carlo information
    class MCSegmentTripleFilter  : public BaseSegmentTripleFilter {

    public:
      /// Empty constructor
      MCSegmentTripleFilter(bool allowReverse = true);

      /// Empty destructor
      virtual ~MCSegmentTripleFilter();

    public:
      /// May be used to clear information from former events. Currently unused.
      virtual void clear() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void initialize() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void terminate() IF_NOT_CINT(override final);

      /// Check if the segment triple is aligned in the Monte Carlo track. Signals NOT_A_CELL if not.
      virtual CellWeight isGoodSegmentTriple(const CDCSegmentTriple& triple) IF_NOT_CINT(override final);

    private:
      /// Sets the trajectories of the segment triple from Monte Carlo information. IS executed for good segment triples.
      void setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const;

    private:
      /// Switch to indicate if the reversed version of the segment triple shall also be accepted (default is true).
      bool m_allowReverse;

      /// Instance of the cell filter to reject neighborhoods of false cells.
      MCAxialAxialSegmentPairFilter m_mcAxialAxialSegmentPairFilter;
    }; // end class MCSegmentTripleFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //MCSEGMENTTRIPLEFILTER_H_
