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
#include <tracking/trackFindingCDC/filters/new_segment/NewSegmentTruthVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment - track pairs
    class MCNewSegmentsFilter : public FilterOnVarSet<NewSegmentTruthVarSet> {

    private:
      /// Type of the super class
      typedef FilterOnVarSet<NewSegmentTruthVarSet> Super;

    public:
      /// Constructor
      MCNewSegmentsFilter() : Super() { }

      virtual CellWeight operator()(const CDCRecoSegment2D& segment) IF_NOT_CINT(override final)
      {
        Super::operator()(segment);
        const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValuesWithPrefix();

        if (varSet.at("truth") == 0.0)
          return NOT_A_CELL;
        else
          return 1.0;
      }
    };
  }
}
