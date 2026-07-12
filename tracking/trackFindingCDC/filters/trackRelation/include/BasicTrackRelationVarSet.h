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

#include <tracking/trackingUtilities/utilities/Relation.h>

namespace Belle2 {
  namespace TrackingUtilities {
    class CDCTrack;
  }
  namespace TrackFindingCDC {

    /// Names of the variables to be generated
    constexpr
    static char const* const basicTrackRelationVarNames[] = {
      "sl_id_pair",
      "delta_sl_id",
      "from_size",
      "to_size",
    };

    /// Vehicle class to transport the variable names
    struct BasicTrackRelationVarNames : public
      TrackingUtilities::VarNames<TrackingUtilities::Relation<const TrackingUtilities::CDCTrack> > {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = TrackingUtilities::size(basicTrackRelationVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return basicTrackRelationVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class BasicTrackRelationVarSet : public TrackingUtilities::VarSet<BasicTrackRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const TrackingUtilities::Relation<const TrackingUtilities::CDCTrack>* ptrTrackRelation) final;
    };
  }
}
