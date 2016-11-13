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

#include <tracking/trackFindingCDC/filters/axialSegmentPair/MCAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/BaseSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of segment triples based on monte carlo information
    class MCSegmentTripleFilter  : public MCSymmetricFilterMixin<Filter<CDCSegmentTriple> > {

    private:
      /// Type of the super class
      using Super = MCSymmetricFilterMixin<Filter<CDCSegmentTriple> >;

    public:
      /// Constructor initializing the symmetry flag.
      explicit MCSegmentTripleFilter(bool allowReverse = true);

    public:
      /// Used to prepare the Monte Carlo information for this event.
      void beginEvent() final;

      /// Forwards the modules initialize to the filter
      void initialize() final;

      /// Forwards the modules initialize to the filter
      void terminate() final;

      /// Check if the segment triple is aligned in the Monte Carlo track. Signals NAN if not.
      Weight operator()(const CDCSegmentTriple& segmentTriple) final;

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
    };
  }
}
