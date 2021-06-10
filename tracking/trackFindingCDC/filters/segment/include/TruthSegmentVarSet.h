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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Names of the variables to be generated
    constexpr
    static char const* const truthSegmentVarNames[] = {
      "segment_is_fake_truth",
      "track_is_already_found_truth",
      "segment_is_new_track_truth",
      "truth",
    };

    /// Vehicle class to transport the variable names
    struct TruthSegmentVarNames : public VarNames<CDCSegment2D> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(truthSegmentVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return truthSegmentVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class TruthSegmentVarSet : public VarSet<TruthSegmentVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<TruthSegmentVarNames>;

    public:
      /// Require the Monte Carlo truth information at initialisation
      void initialize() final;

      /// Prepare the Monte Carlo truth information at start of the event
      void beginEvent() final;

      /// Generate and assign the contained variables
      bool extract(const CDCSegment2D* segment) override;
    };
  }
}
