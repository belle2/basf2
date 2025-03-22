/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/PlainMatrix.h>

#include <Eigen/Core>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Obtain a view of the values in the given plain matrix as an Eigen::Matrix
    template <class T, int M, int N>
    auto mapToEigen(PlainMatrix<T, M, N>& plainMatrix)
    {
      return Eigen::Map<Eigen::Matrix<T, M, N, Eigen::ColMajor> >(plainMatrix.data());
    }

    /// Obtain a view of the values in the given plain matrix as an Eigen::Matrix - const version
    template <class T, int M, int N>
    auto mapToEigen(const PlainMatrix<T, M, N>& plainMatrix)
    {
      return Eigen::Map<const Eigen::Matrix<T, M, N, Eigen::ColMajor>>(plainMatrix.data());
    }

    /// Trivial reflection for the Eigen::Matrix to be viewed as an Eigen::Matrix
    template <class T, int M, int N>
    auto& mapToEigen(Eigen::Matrix<T, M, N>& matrix)
    {
      return matrix;
    }

    /// Trivial reflection for the Eigen::Matrix to be viewed as an Eigen::Matrix - const version
    template <class T, int M, int N>
    const auto& mapToEigen(const Eigen::Matrix<T, M, N>& matrix)
    {
      return matrix;
    }

  }
}
