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

#include <tracking/trackFindingCDC/filters/segment_pair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/MCSymmetricFilterMixin.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to stereo segment pairs based on MC information.
    class MCSegmentPairFilter : public MCSymmetricFilterMixin<Filter<CDCSegmentPair> > {

    private:
      /// Type of the super class
      typedef MCSymmetricFilterMixin<Filter<CDCSegmentPair> > Super;

    public:
      /// Constructor
      explicit MCSegmentPairFilter(bool allowReverse = true) :
        Super(allowReverse) {}

      /// Checks if a axial stereo segment pair is a good combination.
      virtual CellWeight operator()(const Belle2::TrackFindingCDC::CDCSegmentPair& axialSegmentPair)
      override final;

    }; // end class MCSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
