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
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    constexpr
    static char const* const truthSegmentPairRelationVarNames[] = {
      "truth_tanl",
      "truth_z",
      "truth_curv",
      "truth_track_fraction",
      "__weight__",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment pair relation
     */
    class TruthSegmentPairRelationVarNames : public VarNames<Relation<const CDCSegmentPair>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(truthSegmentPairRelationVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return truthSegmentPairRelationVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment pair relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class TruthSegmentPairRelationVarSet : public VarSet<TruthSegmentPairRelationVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<TruthSegmentPairRelationVarNames>;

    public:
      /// Construct the varset.
      explicit TruthSegmentPairRelationVarSet();

      /// Generate and assign the variables from the segment pair relation
      bool extract(const Relation<const CDCSegmentPair>* ptrSegmentPairRelation) override;
    };
  }
}
