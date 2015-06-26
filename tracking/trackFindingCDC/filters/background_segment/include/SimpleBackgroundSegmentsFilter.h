/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>
#include <tracking/trackFindingCDC/filters/background_segment/BackgroundSegmentVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment - track pairs
    class SimpleBackgroundSegmentsFilter : public FilterOnVarSet<BackgroundSegmentVarSet> {

    private:
      /// Type of the super class
      typedef FilterOnVarSet<BackgroundSegmentVarSet> Super;

    public:
      /// Constructor
      SimpleBackgroundSegmentsFilter() : Super() { }

    public:
      virtual CellWeight operator()(const CDCRecoSegment2D& segment) IF_NOT_CINT(override final);

    private:

    };
  }
}
