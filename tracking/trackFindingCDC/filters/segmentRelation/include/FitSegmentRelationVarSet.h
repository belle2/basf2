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

#include <vector>
#include <string>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCRecoSegment2D.
    class CDCRecoSegment2D;

    /// Names of the variables to be generated.
    constexpr
    static char const* const segmentRelationFitNames[] = {
      "is_fitted",
      "curv",
      "curv_var",
      "chi2",
      "chi2_per_ndf",
      "ndf",
      "p_value",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment relation
     */
    class FitSegmentRelationVarNames : public VarNames<Relation<const CDCRecoSegment2D>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentRelationFitNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return segmentRelationFitNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitSegmentRelationVarSet : public VarSet<FitSegmentRelationVarNames> {

    private:
      /// Type of the super class
      typedef VarSet<FitSegmentRelationVarNames> Super;

    public:
      /// Construct the varset to be prepended to all variable names.
      explicit FitSegmentRelationVarSet();

      /// Generate and assign the variables from the segment relation
      virtual bool extract(const Relation<const CDCRecoSegment2D>* ptrSegmentRelation) override final;
    };
  }
}
