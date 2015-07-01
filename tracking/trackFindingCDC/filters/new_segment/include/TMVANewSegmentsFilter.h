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

#include <tracking/trackFindingCDC/filters/base/TMVAFilter.h>
#include <tracking/trackFindingCDC/filters/new_segment/NewSegmentVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Background cluster detection based on TMVA.
    class TMVANewSegmentsFilter: public TMVAFilter<NewSegmentVarSet> {

    public:
      /// Constructor initialising the TMVAFilter with standard training name for this filter.
      TMVANewSegmentsFilter(const std::string& filename  = "NewSegmentsFilter") :
        TMVAFilter<NewSegmentVarSet>(filename)
      {;}

    };
  }
}
