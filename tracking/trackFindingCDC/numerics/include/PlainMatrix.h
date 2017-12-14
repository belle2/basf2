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

#include <ostream>
#include <array>
#include <algorithm>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  A matrix implementation to be used as an interface typ through out the track finder.
     *
     *  This represents a fixed sized matrix with dimensions known at compile time to
     *  replace the Eigen::Matrix as vocabulary type at object interfaces.
     *  Because of its limited feature set it significantly reduces the compilation time
     *  of the whole cdc track finding as the Eigen headers do not have to be included
     *  in each translation unit.
     *
     *  It exposes only a limited number of methods for basic interactions.
     *  For all the linear algebra algorithms use the mapToEigen function in EigenView.h
     *  But it is advised to use the linear algebra only in .cc such that transitive includes
     *  do not suffer from Eigens heavily templated implementation.
     *
     *  The arrangement of element in this matrix is in column-major order
     *  (same as the default for Eigen).
     */
    template <class T, int M, int N>
    class PlainMatrix {

      static_assert(M > 0, "Dimensions must be greater zero");
      static_assert(N > 0, "Dimensions must be greater zero");

      /// Total number of elements
      static const int S = M * N;

    public:
      /// Default initializing of the matrix
      PlainMatrix() = default;

      /// Construct from initialiser list - also for value initialisation
      explicit PlainMatrix(std::initializer_list<T> values)
        : m_values{}
      {
        assert(m_values.size() >= values.size());
        std::copy(values.begin(), values.end(), m_values.begin());
      }

      /// Construct from the given values.
      explicit PlainMatrix(std::array<T, S> values)
        : m_values(values)
      {
      }

      /// Construct a matrix initialized with zeros
      static PlainMatrix<T, M, N> Zero()
      {
        PlainMatrix<T, M, N> result{{}}; // Value initialize to zero
        return result;
      }

      /// Construct an identity matrix
      static PlainMatrix<T, M, N> Identity()
      {
        PlainMatrix<T, M, N> result{{}}; // Value initialize to zero
        for (int s = 0; s < M * N; s += M + 1)
          result.m_values[s] = 1;
        return result;
      }

      /// Construct a matrix with all elements set to a constant
      static PlainMatrix<T, M, N> Constant(T t)
      {
        PlainMatrix<T, M, N> result;
        result.m_values.fill(t);
        return result;
      }

    public:
      /// Access to the flat value array
      T* data()
      {
        return m_values.data();
      }

      /// Constant access to the flat value array
      const T* data() const
      {
        return m_values.data();
      }

      /// Flat element access at the given row i and column j
      T& operator[](int s)
      {
        return m_values[s];
      }

      /// Constant flat element access at the given row i and column j
      T operator[](int s) const
      {
        return m_values[s];
      }

      /// Element access at the given row i and column j
      T& operator()(int i, int j = 0)
      {
        return m_values.operator[](j * M + i);
      }

      /// Constant element access at the given row i and column j
      T operator()(int i, int j = 0) const
      {
        return m_values.operator[](j * M + i);
      }

      /// Output operator for debugging purposes
      friend std::ostream& operator<<(std::ostream& out, const PlainMatrix<T, M, N>& rhs)
      {
        for (int i = 0; i < rhs.rows(); ++i) {
          out << rhs(i, 0) << ", ";
          for (int j = 1; j < rhs.cols(); ++j) {
            out << rhs(i, j);
          }
          out << "\n";
        }
        return out;
      }

      /// Elementwise addition of two matrices
      PlainMatrix<T, M, N> operator+(const PlainMatrix<T, M, N>& rhs) const
      {
        PlainMatrix<T, M, N> result;
        std::transform(begin(), end(), rhs.begin(), result.begin(), std::plus<T>());
        return result;
      }

      /// Elementwise subtraction of two matrices
      PlainMatrix<T, M, N> operator-(const PlainMatrix<T, M, N>& rhs) const
      {
        PlainMatrix<T, M, N> result;
        std::transform(begin(), end(), rhs.begin(), result.begin(), std::minus<T>());
        return result;
      }

      /// Naive matrix multiplication
      template <int O>
      PlainMatrix<T, M, O> operator*(const PlainMatrix<T, N, O>& rhs) const
      {
        PlainMatrix<T, M, O> result{{}}; // Value initialize to zero
        for (int m = 0; m < M; ++m) {
          for (int o = 0; o < O; ++o) {
            for (int n = 0; n < N; ++n) {
              result(m, o) += operator()(m, n) * rhs(n, o);
            }
          }
        }
        return result;
      }

      /// Elementwise division of the elements of the matrix by a number
      PlainMatrix<T, M, N> operator/(T rhs) const
      {
        PlainMatrix<T, M, N> result;
        std::transform(begin(), end(), result.begin(), [&rhs](const T & t) { return t / rhs; });
        return result;
      }

      /// Elementwise division of a number by the elements of the matrix
      friend PlainMatrix<T, M, N> operator/(T lhs, PlainMatrix<T, M, N>& rhs)
      {
        PlainMatrix<T, M, N> result;
        std::transform(rhs.begin(), rhs.end(), result.begin(), [&lhs](const T & t) {
          return lhs / t;
        });
        return result;
      }

      /// Get the K x L block from the matrix starting at the element at position (i, j)
      template <int K, int L>
      PlainMatrix<T, K, L> block(int i = 0, int j = 0) const
      {
        assert(K + i <= M && "Selected block reaches outside of the matrix");
        assert(L + j <= N && "Selected block reaches outside of the matrix");

        PlainMatrix<T, K, L> result;
        const int skipFront = j * M + i;
        for (int iCol = 0; iCol < L; ++iCol) {
          std::copy(begin() + skipFront + iCol * M,
                    begin() + skipFront + iCol * M + K,
                    result.data() + iCol * L);
        }
        return result;
      }

      /// Get the K top rows of the matrix
      template <int K>
      PlainMatrix<T, K, N> head() const
      {
        return block<K, N>(0, 0);
      }

      /// Get the K bottom rows of the matrix
      template <int K>
      PlainMatrix<T, K, N> tail() const
      {
        return block<K, N>(M - K, 0);
      }

      /// Total number of values in the matrix
      static constexpr size_t size()
      {
        return M * N;
      }

      /// Total number of rows in the matrix
      static constexpr int rows()
      {
        return M;
      }

      /// Total number of columns in the matrix
      static constexpr int cols()
      {
        return N;
      }

    private:
      /// The begin iterator of the flattened values
      T* begin()
      {
        return data();
      }

      /// The end iterator of the flattened values
      T* end()
      {
        return data() + size();
      }

      /// The begin const_iterator of the flattened values
      const T* begin() const
      {
        return data();
      }

      /// The end const_iterator of the flattened values
      const T* end() const
      {
        return data() + size();
      }

    private:
      /// Memory of the flat value content.
      std::array<T, M* N> m_values;
    };
  }
}
