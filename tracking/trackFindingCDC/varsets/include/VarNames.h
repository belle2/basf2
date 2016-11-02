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

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class that specifies the names of the variables.
    template<class AObject>
    class VarNames {

    public:
      /// Basic type from which the variables are generated.
      using Object = AObject;

      /// Number of variables to be generated.
      static const size_t nNames = 0;

      /// Names of the variables to be generated.
      //constexpr
      //static char const* const names[nNames] = {};

      constexpr
      static char const* getName(int /*iName*/)
      { return ""; }

      /// Marking that no further variables nested variables are to be extracted
      using NestedVarSet = EmptyVarSet<Object>;

      /// Unpack the object for for the nested variable set
      static const Object* getNested(const Object* obj)
      { return obj; }

    };
  }
}
