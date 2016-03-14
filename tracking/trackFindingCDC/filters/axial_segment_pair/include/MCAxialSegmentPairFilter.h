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

#include <tracking/trackFindingCDC/filters/axial_segment_pair/BaseAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class MCAxialSegmentPairFilter:
      public MCSymmetricFilterMixin<Filter<CDCAxialSegmentPair> > {

    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<CDCAxialSegmentPair> > Super;

    public:
      /// Constructor
      explicit MCAxialSegmentPairFilter(bool allowReverse = true);

      /// Checks if a pair of axial segments is a good combination
      virtual CellWeight operator()(const Belle2::TrackFindingCDC::CDCAxialSegmentPair& axialSegmentPair)
      override final;

    }; // end class MCAxialSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
