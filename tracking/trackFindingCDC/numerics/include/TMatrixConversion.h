/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/CovarianceMatrix.h>

#include <TMatrixDSymfwd.h>

/** Declarations (pure .h) **/

namespace Belle2 {
  namespace TrackFindingCDC {

    struct TMatrixConversion {

      /// Create a covariance matrix from a TMatrix representation
      template <int N>
      static CovarianceMatrix<N> fromTMatrix(const TMatrixDSym& tCov);

      /// Translate the covariance matrix to the TMatrix representation
      template <int N>
      static TMatrixDSym toTMatrix(const CovarianceMatrix<N>& cov);
    };
  }
}

/** Definitiions (.icc.h) **/

#include <TMatrixDSym.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <int N>
    CovarianceMatrix<N> TMatrixConversion::fromTMatrix(const TMatrixDSym& tCov)
    {
      assert(tCov.GetNrows() == N);
      assert(tCov.GetNcols() == N);
      assert(tCov.GetRowLwb() == 0);
      assert(tCov.GetColLwb() == 0);

      CovarianceMatrix<N> result;
      for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
          result(i, j) = tCov(i, j);
        }
      }
      return result;
    }

    template <int N>
    TMatrixDSym TMatrixConversion::toTMatrix(const CovarianceMatrix<N>& cov)
    {
      TMatrixDSym result(N);
      for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
          result(i, j) = cov(i, j);
        }
      }
      return result;
    }
  }
}
