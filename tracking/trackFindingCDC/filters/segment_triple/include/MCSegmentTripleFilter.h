/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/axial_segment_pair/MCAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/filters/segment_triple/BaseSegmentTripleFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of segment triples based on monte carlo information
    class MCSegmentTripleFilter  : public MCSymmetricFilterMixin<Filter<CDCSegmentTriple> > {

    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<CDCSegmentTriple> > Super;

    public:
      /// Constructor initializing the symmetry flag.
      explicit MCSegmentTripleFilter(bool allowReverse = true);

    public:
      /// May be used to clear information from former events. Currently unused.
      virtual void clear() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void initialize() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void terminate() IF_NOT_CINT(override final);

      /// Check if the segment triple is aligned in the Monte Carlo track. Signals NOT_A_CELL if not.
      virtual CellWeight operator()(const CDCSegmentTriple& triple) IF_NOT_CINT(override final);

    private:
      /// Sets the trajectories of the segment triple from Monte Carlo information. Is executed for good segment triples.
      void setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const;

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse) override
      {
        Super::setAllowReverse(allowReverse);
        m_mcAxialSegmentPairFilter.setAllowReverse(allowReverse);
      }

    private:
      /// Instance of the cell filter to reject neighborhoods of false cells.
      MCAxialSegmentPairFilter m_mcAxialSegmentPairFilter;

    }; // end class MCSegmentTripleFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
