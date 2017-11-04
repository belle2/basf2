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

#include <Eigen/Dense>
#include <TVectorD.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  /// Convert a ROOT matrix to Eigen
  template <unsigned int NRows, unsigned int NCols, class AMatrix>
  Eigen::Matrix<double, NRows, NCols, Eigen::RowMajor> convertToEigen(const AMatrix& matrix)
  {
    B2ASSERT("Matrix should be in the form " << NRows << "x" << NCols << ", not " << matrix.GetNrows() << "x" << matrix.GetNcols(),
             matrix.GetNcols() == NCols and matrix.GetNrows() == NRows);
    return Eigen::Matrix<double, NRows, NCols, Eigen::RowMajor>(matrix.GetMatrixArray());
  };

  /// Convert a ROOT matrix to Eigen - TVector specialisation
  template <unsigned int NRows>
  Eigen::Matrix<double, NRows, 1> convertToEigen(const TVectorD& matrix)
  {
    B2ASSERT("Matrix should be in the form " << NRows << "x" << 1 << ", not " << matrix.GetNrows() << "x" << 1,
             matrix.GetNrows() == NRows);
    return Eigen::Matrix<double, NRows, 1>(matrix.GetMatrixArray());
  };
}
