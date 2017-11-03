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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCAxialSegmentPair;

    /// Names of the variables to be generated
    constexpr
    static char const* const basicAxialSegmentPairVarNames[] = {
      "stereo_kind",
      "from_size",
      "to_size",
      "from_n_layers",
      "to_n_layers",
      "sl_id",
    };

    /// Vehicle class to transport the variable names
    struct BasicAxialSegmentPairVarNames : public VarNames<CDCAxialSegmentPair > {

      /// Number of variables to be generated
      static const size_t nVars = size(basicAxialSegmentPairVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicAxialSegmentPairVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicAxialSegmentPairVarSet : public VarSet<BasicAxialSegmentPairVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const CDCAxialSegmentPair* ptrAxialSegmentPair) override;
    };
  }
}
