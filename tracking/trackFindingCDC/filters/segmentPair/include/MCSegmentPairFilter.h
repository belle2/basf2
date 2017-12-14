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

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>

#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to stereo segment pairs based on MC information.
    class MCSegmentPairFilter : public MCSymmetric<BaseSegmentPairFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseSegmentPairFilter>;

    public:
      /// Constructor
      explicit MCSegmentPairFilter(bool allowReverse = true);

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Checks if a axial stereo segment pair is a good combination.
      Weight operator()(const CDCSegmentPair& segmentPair) final;

    private:
      /// Parameter : Switch to require the segment combination contain mostly correct rl information
      bool m_param_requireRLPure = true;

      /// Parameter : Minimum segment size to pass as monte carlo truth
      int m_param_minSegmentSize = 4;
    };
  }
}
