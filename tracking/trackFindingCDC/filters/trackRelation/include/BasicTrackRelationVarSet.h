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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Names of the variables to be generated.
    constexpr
    static char const* const basicTrackRelationVarNames[] = {
      "sl_id_pair",
      "delta_sl_id",
      "from_size",
      "to_size",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a track pair
     */
    class BasicTrackRelationVarNames : public VarNames<Relation<const CDCTrack> > {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(basicTrackRelationVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return basicTrackRelationVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a track relation.
     */
    class BasicTrackRelationVarSet : public VarSet<BasicTrackRelationVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<BasicTrackRelationVarNames>;

    public:
      /// Construct the varset.
      explicit BasicTrackRelationVarSet();

      /// Generate and assign the variables from the track relation
      bool extract(const Relation<const CDCTrack>* ptrTrackRelation) override;
    };
  }
}
