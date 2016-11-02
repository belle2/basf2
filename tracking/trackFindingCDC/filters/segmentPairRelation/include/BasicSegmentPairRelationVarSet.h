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
    static char const* const basicSegmentRelationNames[] = {
      "middle_is_axial",
      "middle_sl_id",
      "middle_size",
      "from_size",
      "to_size",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment relation
     */
    class BasicSegmentPairRelationVarNames : public VarNames<Relation<const CDCSegmentPair>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(basicSegmentRelationNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return basicSegmentRelationNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment pair relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class BasicSegmentPairRelationVarSet : public VarSet<BasicSegmentPairRelationVarNames> {

    private:
      /// Type of the super class
      using Super = VarSet<BasicSegmentPairRelationVarNames>;

    public:
      /// Construct the varset to be prepended to all variable names.
      explicit BasicSegmentPairRelationVarSet();

      /// Generate and assign the variables from the segment relation
      virtual bool extract(const Relation<const CDCSegmentPair>* ptrSegmentPairRelation) override final;
    };
  }
}
