/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentRelation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Filter for the constuction of segment relations based on MC information.
    class MCSegmentRelationFilter : public MCSymmetric<BaseSegmentRelationFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseSegmentRelationFilter >;

    public:
      /// Constructor
      MCSegmentRelationFilter(bool allowReverse = true);

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Tell Root to look at this operator
      using Super::operator();

    private:
      /// Parameter : Switch to require the segment combination contain mostly correct rl information
      bool m_param_requireRLPure = false;

      /// Checks if a segment relation is a good combination.
      Weight operator()(const CDCSegment2D& fromSegment, const CDCSegment2D& toSegment) final;
    };
  }
}
