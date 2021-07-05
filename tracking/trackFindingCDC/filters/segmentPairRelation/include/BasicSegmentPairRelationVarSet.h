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

#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const basicSegmentRelationVarNames[] = {
      "middle_is_axial",
      "middle_sl_id",
      "middle_size",
      "from_size",
      "to_size",
    };

    /// Vehicle class to transport the variable names
    struct BasicSegmentPairRelationVarNames : public VarNames<Relation<const CDCSegmentPair>> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(basicSegmentRelationVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicSegmentRelationVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from an axial stereo segment pair relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicSegmentPairRelationVarSet : public VarSet<BasicSegmentPairRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const Relation<const CDCSegmentPair>* ptrSegmentPairRelation) final;
    };
  }
}
