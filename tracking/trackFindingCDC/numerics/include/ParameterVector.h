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

#include <Eigen/Dense>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Vector type for n related parameters
    template <int N>
    using ParameterVector = Eigen::Matrix<double, N, 1>;

    struct ParameterVectorUtil {

      /// Gets a sub vector from a parameter vector
      template <class AParameterVector, int I = 0, int N = 0>
      static AParameterVector getSub(const ParameterVector<N>& par)
      {
        constexpr const int M = AParameterVector::RowsAtCompileTime;
        return par.template segment<M>(I);
      }

      /// Combines two parameter vectors by stacking them over each other.
      template <int N1, int N2>
      static ParameterVector < N1 + N2 > stack(const ParameterVector<N1>& upperPar,
                                               const ParameterVector<N2>& lowerPar)
      {
        ParameterVector < N1 + N2 > result;
        result << upperPar, lowerPar;
        return result;
      }
    };

  }
}
