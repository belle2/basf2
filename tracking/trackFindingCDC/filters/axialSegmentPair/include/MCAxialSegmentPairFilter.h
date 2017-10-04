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
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class MCAxialSegmentPairFilter : public MCSymmetric<BaseAxialSegmentPairFilter> {

    private:
      /// Type of the super class
      using Super = MCSymmetric<BaseAxialSegmentPairFilter>;

    public:
      /// Constructor
      explicit MCAxialSegmentPairFilter(bool allowReverse = true);

      /// Default destructor
      ~MCAxialSegmentPairFilter();

      /// Checks if a pair of axial segments is a good combination
      Weight operator()(const CDCAxialSegmentPair& axialSegmentPair) final;
    };

  }
}
