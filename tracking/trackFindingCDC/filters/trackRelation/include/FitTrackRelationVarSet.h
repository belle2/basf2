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
    /// Forward declaration of the CDCTrack.
    class CDCTrack;

    /// Names of the variables to be generated.
    constexpr
    static char const* const fitTrackRelationNames[] = {
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
      "p_value_cut",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a track relation
     */
    class FitTrackRelationVarNames : public VarNames<Relation<const CDCTrack>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(fitTrackRelationNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return fitTrackRelationNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a track relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitTrackRelationVarSet : public VarSet<FitTrackRelationVarNames> {

    public:
      /// Generate and assign the variables from the track relation
      bool extract(const Relation<const CDCTrack>* ptrTrackRelation) final;
    };
  }
}
