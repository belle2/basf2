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

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

#include <tracking/trackFindingCDC/filters/segmentTriple/BaseSegmentTripleFilter.h>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCSegmentTriple;

    /// Filter for the constuction of segment triples based on monte carlo information
    class MCSegmentTripleFilter  : public MCSymmetric<BaseSegmentTripleFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseSegmentTripleFilter>;

    public:
      /// Constructor initializing the symmetry flag.
      explicit MCSegmentTripleFilter(bool allowReverse = true);

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize the before event processing.
      void initialize() final;

    public:
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
