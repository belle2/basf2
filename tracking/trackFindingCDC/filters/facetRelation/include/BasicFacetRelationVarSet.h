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

#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCFacet;

    /// Names of the variables to be generated
    constexpr
    static char const* const basicFacetRelationVarNames[] = {
      "superlayer_id",
      "from_middle_phi",
      "to_middle_phi",
    };

    /// Vehicle class to transport the variable names
    struct BasicFacetRelationVarNames : public VarNames<Relation<const CDCFacet>> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(basicFacetRelationVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicFacetRelationVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a facet relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicFacetRelationVarSet : public VarSet<BasicFacetRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const Relation<const CDCFacet>* ptrFacetRelation) final;
    };
  }
}
