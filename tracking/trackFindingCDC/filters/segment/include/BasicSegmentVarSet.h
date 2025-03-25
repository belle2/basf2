/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/varsets/VarSet.h>
#include <tracking/trackingUtilities/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    /// Names of the variables to be generated
    constexpr
    static char const* const basicSegmentVarNames[] = {
      "size",
      "superlayer_id",
    };

    /// Vehicle class to transport the variable names
    struct BasicSegmentVarNames : public TrackingUtilities::VarNames<TrackingUtilities::CDCSegment2D> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = TrackingUtilities::size(basicSegmentVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicSegmentVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicSegmentVarSet : public TrackingUtilities::VarSet<BasicSegmentVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const TrackingUtilities::CDCSegment2D* ptrSegment2D) final;
    };
  }
}
