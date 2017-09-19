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

#include <tracking/trackFindingCDC/numerics/PlainMatrix.h>

#include <Eigen/Core>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class T, int M, int N>
    auto mapToEigen(PlainMatrix<T, M, N>& plainMatrix)
    {
      return Eigen::Map<Eigen::Matrix<T, M, N, Eigen::ColMajor> >(plainMatrix.data());
    }

    template <class T, int M, int N>
    auto mapToEigen(const PlainMatrix<T, M, N>& plainMatrix)
    {
      return Eigen::Map<const Eigen::Matrix<T, M, N, Eigen::ColMajor>>(plainMatrix.data());
    }

    template <class T, int M, int N>
    auto& mapToEigen(Eigen::Matrix<T, M, N>& matrix)
    {
      return matrix;
    }

    template <class T, int M, int N>
    const auto& mapToEigen(const Eigen::Matrix<T, M, N>& matrix)
    {
      return matrix;
    }

  }
}
