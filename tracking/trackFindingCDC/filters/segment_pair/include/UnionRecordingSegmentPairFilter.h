/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segment_pair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/UnionRecordingFilter.h>
#include <tracking/trackFindingCDC/filters/segment_pair/CDCSegmentPairVarSets.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCSegmentPairs.
    class UnionRecordingSegmentPairFilter: public UnionRecordingFilter<CDCSegmentPair> {

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      UnionRecordingSegmentPairFilter() :
        UnionRecordingFilter<CDCSegmentPair>("CDCSegmentPairTruthRecords.root")
      {;}

      /// Valid names of variable sets for segment pairs.
      virtual std::vector<std::string> getValidVarSetNames() const override
      { return {"fitless", "fit", "truth"}; }

      /// Create a concrete variables set for segment pairs from a name.
      virtual
      std::unique_ptr<BaseVarSet<CDCSegmentPair>> createVarSet(const std::string& name) const override
      {
        if (name == "fitless") {
          return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new CDCSegmentPairFitlessVarSet());
        } else if (name == "fit") {
          return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new CDCSegmentPairFitVarSet());
        } else if (name == "truth") {
          return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(new CDCSegmentPairTruthVarSet());
        } else {
          return std::unique_ptr<BaseVarSet<CDCSegmentPair> >(nullptr);
        }
      }
    };
  }
}
