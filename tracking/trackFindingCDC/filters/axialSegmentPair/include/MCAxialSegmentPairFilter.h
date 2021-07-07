/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
