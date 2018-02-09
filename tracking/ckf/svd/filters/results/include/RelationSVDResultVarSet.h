/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/ckf/svd/entities/CKFToSVDResult.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const relationSVDResultVarNames[] = {
    "svd_highest_layer",
    "number_of_hits_related_svd_track",
  };

  /// Vehicle class to transport the variable names
  class RelationSVDResultVarNames : public TrackFindingCDC::VarNames<CKFToSVDResult> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(relationSVDResultVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return relationSVDResultVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class RelationSVDResultVarSet : public TrackFindingCDC::VarSet<RelationSVDResultVarNames> {
  public:
    /// Generate and assign the variables from the object.
    bool extract(const CKFToSVDResult* object) final;
  };
}
