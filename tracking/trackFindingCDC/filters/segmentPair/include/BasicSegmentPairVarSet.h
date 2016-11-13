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

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    constexpr
    static char const* const basicSegmentPairVarNames[] = {
      "axial_first",
      "axial_size",
      "stereo_size",
      "sl_id_pair",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a segment pair
     */
    class BasicSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(basicSegmentPairVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return basicSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a segment relation.
     */
    class BasicSegmentPairVarSet : public VarSet<BasicSegmentPairVarNames> {

    public:
      /// Generate and assign the variables from the segment pair
      bool extract(const CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
