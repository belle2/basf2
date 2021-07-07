/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterTools/DecorrelationMatrixHelper.h>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <limits> // needed for getting full precision of doubles for printing
#include <iomanip>

namespace Belle2 {

  /**
   * Class holding a Matrix that can be used to decorrelate input data to Machine Learning classifiers.
   *
   * At the moment only the dimension is templated and defaults to Ndims = 9, as it will be mainly used for
   * ThreeHitFilters which have at the moment only 9 inputs.
   */
  template<size_t Ndims>
  class DecorrelationMatrix {
  public:
    /** typedef for consistent type across class */
    using MatrixT = Eigen::Matrix<double, Ndims, Ndims, Eigen::RowMajor>;

    /** default constructor: initializes to identity matrix or to passed matrix */
    explicit DecorrelationMatrix(const MatrixT& matrix = MatrixT::Identity()) : m_matrix(matrix) { }

    /** copy constructor */
    DecorrelationMatrix(const DecorrelationMatrix& matrix) = default;

    /** assignment operator */
    DecorrelationMatrix& operator=(const DecorrelationMatrix& rhs) = default;

    const MatrixT& getMatrix() const { return m_matrix; } /**< get the currently stored matrix */

    /**
     * calculate the transformation matrix that when applied to the input data yields linearly uncorrelated data.
     * @param inputData input data, where each vector represents a column in the MxNdims matrix, where M is the number of samples in
     * in the input data
     * @param normalise if set to true the covariance matrix of the transformed data will be identity, else it will only be diagonal
     */
    void calculateDecorrMatrix(std::array<std::vector<double>, Ndims> inputData, bool normalise = true);


    /** calculate the matrix that can be used to decorrelate the data that yield the passed covariance matrix. */
    void calculateDecorrMatrix(const MatrixT& covMatrix, bool normalise = true);

    /** "decorrelate" one measurement (i.e. apply the transformation that has previously been determined). */
    std::vector<double> decorrelate(const std::vector<double>& inputVec) const;

    /** "decorrelate" one measurement (i.e. apply the transformation that has previously been determined). */
    std::vector<double> decorrelate(const std::array<double, Ndims>& inputs) const;

    /** decorrelate M measurements (i.e. apply the transformation that has previously been determined).
     * NOTE: the transformation has to be calculated prior to the call of this function!
     */
    std::array<std::vector<double>, Ndims> decorrelate(const std::array<std::vector<double>, Ndims>& inputMat) const;

    /** print the matrix to a string.
     * Format: a row, a line, elements of rows delimited by a space. No trailing newline after last row!
     */
    std::string print() const;

    /** read from stream.
     * Expected format: a row per line, elements of rows delimited by a space.
     * returns true if operation was succesful, false elsewise
     *
     * NOTE: it is only checked if enough elements are present in each row and column. However, any superfluous values are ignored.
     */
    bool readFromStream(std::istream& is);

  private:
    MatrixT m_matrix; /**< internal matrix holding the data */


  };


  // ======================================== IMPLEMENTATION ========================================

  template<size_t Ndims>
  void DecorrelationMatrix<Ndims>::calculateDecorrMatrix(std::array<std::vector<double>, Ndims> inputData, bool normalise)
  {
    calculateDecorrMatrix(calculateCovMatrix(inputData), normalise);
  }

  template<size_t Ndims>
  void DecorrelationMatrix<Ndims>::calculateDecorrMatrix(const DecorrelationMatrix<Ndims>::MatrixT& covMatrix, bool normalise)
  {
    // make an eigen decomposition of the covariance matrix to define the transformation matrix
    Eigen::SelfAdjointEigenSolver<MatrixT> eigenSolver(covMatrix);
    const MatrixT U = eigenSolver.eigenvectors();

    if (normalise) {
      // numerically better (faster hopefully) to not invert a full matrix but to calculate the diagonal elements of the
      // (diagonal) matrix containing the eigenvalues used for normalization
      const MatrixT D = eigenSolver.eigenvalues().cwiseSqrt().cwiseInverse().asDiagonal();
      m_matrix = U * D;
    } else {
      m_matrix = U;
    }
  }

  template<size_t Ndims>
  std::vector<double> DecorrelationMatrix<Ndims>::decorrelate(const std::vector<double>& inputVec) const
  {
    using RVector = Eigen::Matrix<double, 1, Ndims, Eigen::RowMajor>;
    Eigen::Map<const RVector> input(inputVec.data(), Ndims);

    const RVector transform = input * m_matrix;

    return std::vector<double>(transform.data(), transform.data() + transform.size());
  }

  template<size_t Ndims>
  std::vector<double> DecorrelationMatrix<Ndims>::decorrelate(const std::array<double, Ndims>& input) const
  {
    return decorrelate(std::vector<double>(input.begin(), input.end()));
  }

  template<size_t Ndims>
  std::array<std::vector<double>, Ndims>
  DecorrelationMatrix<Ndims>::decorrelate(const std::array<std::vector<double>, Ndims>& inputMat) const
  {
    using DVector = Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor>;
    using DMatrix = Eigen::Matrix<double, Eigen::Dynamic, Ndims, Eigen::ColMajor>;

    size_t nSamples = inputMat[0].size();
    DMatrix dataMatrix(nSamples, Ndims);
    for (size_t i = 0; i < Ndims; ++i) {
      dataMatrix.col(i) = Eigen::Map<const DVector>(inputMat[i].data(), inputMat[i].size());
    }

    DMatrix transform = dataMatrix * m_matrix;

    std::array<std::vector<double>, Ndims> output;
    for (size_t i = 0; i < Ndims; ++i) {
      output[i] = std::vector<double>(transform.col(i).data(), transform.col(i).data() + transform.col(i).size());
    }

    return output;
  }

  template<size_t Ndims>
  std::string DecorrelationMatrix<Ndims>::print() const
  {
    std::stringstream sstr{};
    sstr << std::setprecision(std::numeric_limits<double>::digits10 + 1) <<  m_matrix; // ensure precision at write-out
    return sstr.str();
  }

  template<size_t Ndims>
  bool DecorrelationMatrix<Ndims>::readFromStream(std::istream& is)
  {
    MatrixT inMat{};
    using RVector = Eigen::Matrix<double, 1, Ndims, Eigen::RowMajor>; // typedef for row vector

    size_t iRow = 0;
    while (iRow < Ndims) { // read only as many lines as needed
      std::string line{};
      if (is.eof()) return false;
      std::getline(is, line);
      if (line.empty()) continue; // skip empty lines
      std::stringstream lstr(line);
      std::array<double, Ndims> linevalues;
      for (double& value : linevalues) { // read only as many values in the line as needed
        if (lstr.eof()) return false;
        lstr >> value;
      }
      inMat.row(iRow) = Eigen::Map<const RVector>(linevalues.data(), linevalues.size());
      ++iRow;
    }

    m_matrix = inMat;
    return true;
  }

} // end namespace Belle 2
