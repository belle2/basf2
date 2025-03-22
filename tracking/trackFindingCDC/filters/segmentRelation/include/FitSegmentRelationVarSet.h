/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/varsets/VarSet.h>
#include <tracking/trackingUtilities/varsets/VarNames.h>

#include <tracking/trackingUtilities/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitSegmentRelationVarNames[] = {
      "is_fitted",
      "curv",
      "curv_var",
      "chi2",
      "chi2_no_rl",
      "chi2_per_ndf",
      "chi2_no_rl_per_ndf",
      "ndf",
      "p_value",
    };

    /// Vehicle class to transport the variable names
    struct FitSegmentRelationVarNames : public TrackingUtilities::VarNames<TrackingUtilities::Relation<const CDCSegment2D>> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = TrackingUtilities::size(fitSegmentRelationVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitSegmentRelationVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitSegmentRelationVarSet : public TrackingUtilities::VarSet<FitSegmentRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const TrackingUtilities::Relation<const CDCSegment2D>* ptrSegmentRelation) final;
    };
  }
}
