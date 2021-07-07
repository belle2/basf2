/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      explicit MCSegmentRelationFilter(bool allowReverse = true);

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
