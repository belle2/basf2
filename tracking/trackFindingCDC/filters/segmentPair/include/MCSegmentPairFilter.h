/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
