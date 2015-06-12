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
#include <tracking/trackFindingCDC/filters/segment_track_chooser/SegmentTrackTruthVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment - track pairs
    class MCSegmentTrackChooser : public FilterOnVarSet<SegmentTrackTruthVarSet> {

    private:
      /// Type of the super class
      typedef FilterOnVarSet<SegmentTrackTruthVarSet> Super;

    public:
      /// Constructor
      MCSegmentTrackChooser() : Super() { }

      virtual CellWeight operator()(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>& testPair) IF_NOT_CINT(override final)
      {
        Super::operator()(testPair);
        const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValuesWithPrefix();

        if (varSet.at("truth") == 0.0)
          return NOT_A_CELL;
        else
          return 1.0;
      }
    };
  }
}
