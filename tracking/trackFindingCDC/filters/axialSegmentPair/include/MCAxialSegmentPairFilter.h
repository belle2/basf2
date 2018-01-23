/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/axialSegmentPair/BaseAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class MCAxialSegmentPairFilter : public MCSymmetric<BaseAxialSegmentPairFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseAxialSegmentPairFilter>;

    public:
      /// Constructor
      explicit MCAxialSegmentPairFilter(bool allowReverse = true);

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Checks if a pair of axial segments is a good combination
      Weight operator()(const CDCAxialSegmentPair& axialSegmentPair) final;

    private:
      /// Parameter : Switch to require the segment combination contain mostly correct rl information
      bool m_param_requireRLPure = true;
    };
  }
}
