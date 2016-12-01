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

#include <tracking/trackFindingCDC/ca/Relation.h>

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
      "chi2_per_ndf",
      "ndf",
      "p_value",
    };

    /// Vehicle class to transport the variable names
    struct FitSegmentRelationVarNames : public VarNames<Relation<const CDCSegment2D>> {

      /// Number of variables to be generated
      static const size_t nVars = size(fitSegmentRelationVarNames);

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
    class FitSegmentRelationVarSet : public VarSet<FitSegmentRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const Relation<const CDCSegment2D>* ptrSegmentRelation) final;
    };
  }
}
