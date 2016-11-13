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

#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration.
    class CDCRecoSegment2D;

    /// Names of the variables to be generated.
    constexpr
    static char const* const basicSegmentVarNames[] = {
      "size",
      "superlayer_id",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class BasicSegmentVarNames : public VarNames<CDCRecoSegment2D> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(basicSegmentVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return basicSegmentVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class BasicSegmentVarSet : public VarSet<BasicSegmentVarNames> {

    public:
      /// Generate and assign the variables from the facet relation
      bool extract(const CDCRecoSegment2D* ptrSegment2D) final;
    };
  }
}
