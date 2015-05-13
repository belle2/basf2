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

#include "EmptyVarSet.h"

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class that specifies the names of the variables.
    template<class T>
    class VarNames {

    public:
      /// Basic type from which the variables are generated.
      typedef T Object;

      /// Number of variables to be generated.
      static const size_t nNames = 0;

      /// Names of the variables to be generated.
      //IF_NOT_CINT(constexpr)
      //static char const* const names[nNames] = {};

      IF_NOT_CINT(constexpr)
      static char const* getName(int __attribute__((unused)) iName)
      {
        return "";
      }

      /// Marking that no further variables nested variables are to be extracted
      typedef EmptyVarSet NestedVarSet;

      /// Unpack the object for for the nested variable set
      static const Object* getNested(const Object* obj) { return obj; }
    };
  }
}
