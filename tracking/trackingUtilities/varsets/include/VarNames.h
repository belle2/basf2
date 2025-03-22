/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
