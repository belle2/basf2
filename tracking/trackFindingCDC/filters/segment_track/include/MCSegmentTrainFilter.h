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
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrainTruthVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment - track pairs
    class MCSegmentTrainFilter : public FilterOnVarSet<SegmentTrainTruthVarSet> {

    private:
      /// Type of the super class
      typedef FilterOnVarSet<SegmentTrainTruthVarSet> Super;

    public:
      /// Constructor
      MCSegmentTrainFilter() : Super() { }

      virtual CellWeight operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>& testPair) IF_NOT_CINT(
        override final)
      {
        Super::operator()(testPair);
        const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValuesWithPrefix();

        if (varSet.at("truth") == 0.0)
          return 0.0;
        else
          return 1.0;
      }
    };
  }
}
