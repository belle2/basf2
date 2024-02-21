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

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Names of the variables to be generated.
    constexpr
    static char const* const cdcWireHitVarNames[] = {
      "adc",
      "tot",
      "tdc",
      "slayer",
    };

    /// Vehicle class to transport the variable names
    class CDCWireHitVarNames : public VarNames<CDCWireHit> {

    public:
      /// Number of variables to be generated.
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(cdcWireHitVarNames);

      /// Get the name of the column.
      constexpr
      static char const* getName(int iName)
      {
        return cdcWireHitVarNames[iName];
      }
    };

    /**
    * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
    * which knows the truth information if two tracks belong together or not.
    */
    class CDCWireHitVarSet : public VarSet<CDCWireHitVarNames> {
      /// Type of the base class
      using Super = VarSet<CDCWireHitVarNames>;

    public:
      CDCWireHitVarSet();

      void initialize() override;

      /// Generate and assign the variables from the object.
      bool extract(const CDCWireHit* object) final;

    private:
    };
  }
}
