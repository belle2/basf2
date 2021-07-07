/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/ckf/vtx/entities/CKFToVTXResult.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const relationVTXResultVarNames[] = {
    "vtx_highest_layer",
    "number_of_hits_related_vtx_track",
  };

  /// Vehicle class to transport the variable names
  class RelationVTXResultVarNames : public TrackFindingCDC::VarNames<CKFToVTXResult> {

  public:
    /// Number of variables to be generated.
    // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
    // at least tell cppcheck that everything is fine
    // cppcheck-suppress duplInheritedMember
    static const size_t nVars = TrackFindingCDC::size(relationVTXResultVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return relationVTXResultVarNames[iName];
    }
  };

  /**
   * Var set used in the VTX-CDC-Merger for calculating the probability of a VTX-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class RelationVTXResultVarSet : public TrackFindingCDC::VarSet<RelationVTXResultVarNames> {
  public:
    /// Generate and assign the variables from the object.
    bool extract(const CKFToVTXResult* object) final;
  };
}
