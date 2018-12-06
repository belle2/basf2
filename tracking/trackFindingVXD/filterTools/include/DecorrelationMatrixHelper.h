/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <Eigen/Core>
#include <framework/logging/Logger.h>

#include <array>
#include <vector>
#include <numeric>
#include <iostream>

namespace Belle2 {

  /**
   * calculates the empirical covariance matrix from the inputData.
   * @param inputData data "matrix", where each vector represents a column in the n x Ndims matrix, where n is the number of samples
   * in the input data.
   *
   * NOTE: if the input does not consist of an array of same-length vectors an error message is issued and the Ndims x Ndims
   * identity matrix is returned.
   *
   * Implements the calculation of the empirical covariance matrix: \f$\frac{1}{n} \sum_{i=1}^n (y_i - \bar{y})^T(y_i - \bar{y})\f$,
   * where \f$ \bar{y}_i = \sum_{j=1}^n x_{ij}\f$ is the mean (row) vector containing the mean
   * value of every "feature" in the data. Every (random) vector \f$y_i\f$ is a row vector containing one measurement.
   */
  template<size_t Ndims>
  const Eigen::Matrix<double, Ndims, Ndims, Eigen::RowMajor> calculateCovMatrix(std::array<std::vector<double>, Ndims> inputData)
  {
    // typedefs used in function
    using RVector = Eigen::Matrix<double, 1, Ndims, Eigen::RowMajor>; // typedef for row vector type of one measurement
    using DMatrix = Eigen::Matrix<double, Eigen::Dynamic, Ndims, Eigen::ColMajor>; // typedef for dataMatrix type
    using DVector = Eigen::Matrix<double, Eigen::Dynamic, 1>; // typedef for column vector in dataMatrix

    // check if the inputData represent a "full" data matrix
    size_t nSamples = inputData[0].size();
    for (size_t i = 1; i < Ndims; ++i) {
      if (inputData[i].size() != nSamples) {
        B2ERROR("The input data is no MxN matrix, cannot calculate covariance matrix! Returning identity");
        return Eigen::Matrix<double, Ndims, Ndims, Eigen::RowMajor>::Identity();
      }
    }

    // calculate the mean of every column and store them in a row vector
    RVector meanVector{};
    for (size_t i = 0; i < Ndims; ++i) {
      meanVector(i) = std::accumulate(inputData[i].begin(), inputData[i].end(), 0.0) / inputData[i].size();
    }

    // map the data to a more easily accessible Eigen::Matrix where each row is one measurement (consisting of Ndims values)
    DMatrix dataMatrix(nSamples, Ndims);
    for (size_t i = 0; i < Ndims; ++i) {
      dataMatrix.col(i) = Eigen::Map<const DVector>(inputData[i].data(), inputData[i].size());
    }

    // define a matrix where each row holds the mean vector to be able to substract the mean value from each column directly
    DMatrix meanMatrix(nSamples, Ndims);
    for (size_t i = 0; i < Ndims; ++i) {
      meanMatrix.col(i) = DVector::Ones(nSamples) * meanVector(i);
    }

    // calculate the cov matrix as product of dataMatrix reduced by meanValues in order to (hopefully) utilize Eigens optimizations
    // COULDDO: test if this can be accelerated by splitting the process in smaller matrices
    return (dataMatrix - meanMatrix).transpose() * (dataMatrix - meanMatrix) / (nSamples);
  }

} // end namespace
