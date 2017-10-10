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

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Utility functions for the PlainMatrix
    struct PlainMatrixUtil {

      /// Construct a diagonal matrix - currently private as it is unused
      template <class T, int M>
      static PlainMatrix<T, M, M> Diag(const PlainMatrix<T, M, 1>& diagEntries)
      {
        PlainMatrix<T, M, M> result{{}}; // Value initialize to zero
        for (size_t s = 0, i = 0; s < result.size(); s += M + 1, ++i)
          result[s] = diagEntries(i, 0);
        return result;
      }

      /// Construct a matrix from two independent blocks stacked along the diagonal
      template <class T, int K, int L, int M, int N>
      static PlainMatrix < T, K + M, L + N >
      BlockStack(const PlainMatrix<T, K, L>& a, const PlainMatrix<T, M, N>& b)
      {
        const int nRows = K + M;
        // const int nCols = L + N;
        PlainMatrix < T, K + M, L + N > result{{}}; // Value initialize to zero

        const int nARows = K;
        const int nACols = L;
        for (int iACol = 0; iACol < nACols; ++iACol) {
          std::copy(a.data() + iACol * nARows,
                    a.data() + (iACol + 1) * nARows,
                    result.data() + iACol * nRows);
        }

        const int nBRows = M;
        const int nBCols = N;
        const int skipA = nACols * nRows;
        const int skipFront = nARows;
        for (int iBCol = 0; iBCol < nBCols; ++iBCol) {
          std::copy(b.data() + iBCol * nBRows,
                    b.data() + (iBCol + 1) * nBRows,
                    result.data() + skipA + skipFront + iBCol * nRows);
        }
        return result;
      }

      /// Construct a matrix from two independent blocks stacked vertically
      template <class T, int K, int M, int N>
      static PlainMatrix < T, K + M, N >
      VStack(const PlainMatrix<T, K, N>& a, const PlainMatrix<T, M, N>& b)
      {
        const int nRows = K + M;
        const int nCols = N;
        PlainMatrix < T, K + M, N > result;

        const int nARows = K;
        const int nBRows = M;
        const int skipFront = nARows;
        for (int iCol = 0; iCol < nCols; ++iCol) {
          std::copy(a.data() + iCol * nARows,
                    a.data() + (iCol + 1) * nARows,
                    result.data() + iCol * nRows);

          std::copy(b.data() + iCol * nBRows,
                    b.data() + (iCol + 1) * nBRows,
                    result.data() + skipFront + iCol * nRows);
        }
        return result;
      }

      /// Construct a matrix from two independent blocks stacked horizontally
      template <class T, int M, int L, int N>
      static PlainMatrix < T, M, L + N >
      HStack(const PlainMatrix<T, M, L>& a, const PlainMatrix<T, M, N>& b)
      {
        PlainMatrix < T, M, L + N > result;
        std::copy(a.data(), a.data() + a.size(), result.data());
        std::copy(b.data(), b.data() + b.size(), result.data() + a.size());
        return result;
      }
    };
  }
}
