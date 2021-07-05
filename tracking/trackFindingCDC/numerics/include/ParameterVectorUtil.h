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

#include <tracking/trackFindingCDC/numerics/ParameterVector.h>

#include <tracking/trackFindingCDC/numerics/EigenView.h>

#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Structure to gather some utility functions for the ParameterVector.
    struct ParameterVectorUtil {

      /// Gets a sub vector from a parameter vector
      template <class AParameterVector, int I = 0, int N = 0>
      static AParameterVector getSub(const ParameterVector<N>& par)
      {
        constexpr const int M =
          std::remove_reference_t<decltype(mapToEigen(AParameterVector()))>::RowsAtCompileTime;
        return par.template block<M, 1>(I, 0);
      }

      /// Combines two parameter vectors by stacking them over each other.
      template <int N1, int N2>
      static ParameterVector < N1 + N2 > stack(const ParameterVector<N1>& upperPar,
                                               const ParameterVector<N2>& lowerPar)
      {
        ParameterVector < N1 + N2 > result;
        mapToEigen(result) << mapToEigen(upperPar), mapToEigen(lowerPar);
        return result;
      }
    };
  }
}
