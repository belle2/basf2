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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Names of the variables to be generated.
    constexpr
    static char const* const basicSegmentRelationVarNames[] = {
      "stereo_kind",
      "from_size",
      "to_size",
      "sl_id",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment pair
     */
    class BasicSegmentRelationVarNames : public VarNames<Relation<const CDCRecoSegment2D> > {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(basicSegmentRelationVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return basicSegmentRelationVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment relation.
     */
    class BasicSegmentRelationVarSet : public VarSet<BasicSegmentRelationVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<BasicSegmentRelationVarNames>;

    public:
      /// Construct the varset.
      explicit BasicSegmentRelationVarSet();

      /// Generate and assign the variables from the segment relation
      bool extract(const Relation<const CDCRecoSegment2D>* ptrSegmentRelation) override;
    };
  }
}
