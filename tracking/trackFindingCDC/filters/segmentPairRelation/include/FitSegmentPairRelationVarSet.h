/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
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
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCSegmentPair.
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    constexpr
    static char const* const fitSegmentRelationNames[] = {
      "is_fitted",
      "curv",
      "curv_var",
      "z0",
      "z0_var",
      "tanl",
      "tanl_var",
      "chi2",
      "chi2_per_ndf",
      "ndf",
      "p_value",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment relation
     */
    class FitSegmentPairRelationVarNames : public VarNames<Relation<const CDCSegmentPair>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(fitSegmentRelationNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return fitSegmentRelationNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment pair relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitSegmentPairRelationVarSet : public VarSet<FitSegmentPairRelationVarNames> {

    private:
      /// Type of the super class
      typedef VarSet<FitSegmentPairRelationVarNames> Super;

    public:
      /// Construct the varset to be prepended to all variable names.
      explicit FitSegmentPairRelationVarSet();

      /// Generate and assign the variables from the segment relation
      virtual bool extract(const Relation<const CDCSegmentPair>* ptrSegmentPairRelation) override final;
    };
  }
}
