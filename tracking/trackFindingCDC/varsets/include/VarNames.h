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

#include <cstddef>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class that specifies the names of the variables.
    template<class AObject>
    class VarNames {

    public:
      /// Basic type from which the variables are generated.
      using Object = AObject;

      /// Number of variables to be generated.
      static const std::size_t nVars = 0;

      /// Function looking up the variable name at the given index as compile time
      constexpr static char const* getName(int iName __attribute__((unused)))
      {
        return "";
      }
    };
  }
}
