/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <Eigen/Dense>
#include <TVectorD.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  /// Convert a ROOT matrix to Eigen. Checks for the correct row and column number.
  template <unsigned int NRows, unsigned int NCols, class AMatrix>
  Eigen::Matrix<double, NRows, NCols, Eigen::RowMajor> convertToEigen(const AMatrix& matrix)
  {
    B2ASSERT("Matrix should be in the form " << NRows << "x" << NCols << ", not " << matrix.GetNrows() << "x" << matrix.GetNcols(),
             matrix.GetNcols() == NCols and matrix.GetNrows() == NRows);
    return Eigen::Matrix<double, NRows, NCols, Eigen::RowMajor>(matrix.GetMatrixArray());
  };

  /// Convert a ROOT matrix to Eigen - TVector specialisation.  Checks for the correct row number.
  template <unsigned int NRows>
  Eigen::Matrix<double, NRows, 1> convertToEigen(const TVectorD& matrix)
  {
    B2ASSERT("Matrix should be in the form " << NRows << "x" << 1 << ", not " << matrix.GetNrows() << "x" << 1,
             matrix.GetNrows() == NRows);
    return Eigen::Matrix<double, NRows, 1>(matrix.GetMatrixArray());
  };
}
