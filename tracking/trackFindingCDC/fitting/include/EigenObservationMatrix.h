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

#include <Eigen/Core>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCObservations2D;
    class CDCSZObservations;

    /**
     *  Matrix type used to wrap the raw memory chunk of values
     *  generated from the various hit types for structured vectorized access.
     */
    using EigenObservationMatrix =
      Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;

    /**
     *  Returns the observations structured as an Eigen matrix
     *  This returns a reference to the stored observations.
     *  @note      Operations may alter the content of the underlying memory and
     *             render it useless for subceeding calculations.
     */
    EigenObservationMatrix getEigenObservationMatrix(CDCObservations2D* observations2D);

    /**
     *  Returns the observations structured as an Eigen matrix
     *  This returns a reference to the stored observations.
     *  @note      Operations may alter the content of the underlying memory and
     *             render it useless for subceeding calculations.
     */
    EigenObservationMatrix getEigenObservationMatrix(CDCSZObservations* szObservations);

    /**
     *  Constructs a symmetric matrix of weighted sums of x, y, r^2 and drift lengts as relevant for
     circle fits.
     *
     *  Cumulates weights, x positions, y positions, quadratic cylindrical radii and signed drift
     lengths and products thereof
     *  @returns symmetric matrix s with the following:
     *  * \f$ s_{00} = \sum w \f$
     *  * \f$ s_{01} = \sum x * w \f$
     *  * \f$ s_{02} = \sum y * w \f$
     *  * \f$ s_{03} = \sum (r^2 - l^2) * w \f$
     *  * \f$ s_{04} = \sum l * w \f$
     *
     *  * \f$ s_{11} = \sum x * x * w \f$
     *  * \f$ s_{12} = \sum x * y * w \f$
     *  * \f$ s_{13} = \sum x * (r^2 - l^2) * w \f$
     *  * \f$ s_{14} = \sum x * l * w \f$
     *
     *  * \f$ s_{22} = \sum y * y * w \f$
     *  * \f$ s_{23} = \sum y * (r^2 - l^2) * w \f$
     *  * \f$ s_{24} = \sum y * l * w \f$
     *
     *  * \f$ s_{33} = \sum (r^2 - l^2) * (r^2 - l^2) * w \f$
     *  * \f$ s_{34} = \sum (r^2 - l^2) * l * w \f$

     *  * \f$ s_{44} = \sum l * l * w \f$
     *  * + symmetric.
     */
    Eigen::Matrix<double, 5, 5> getWXYRLSumMatrix(CDCObservations2D& observations2D);

    /**
     *  Constructs a symmetric matrix of weighted sums of x, y and drift lengts as relevant for line
     * fits.
     *
     *  Cumulates weights, x positions, y positions and signed drift legnths and products thereof
     *  @returns symmetric matrix s with the following:
     *  * \f$ s_{00} = \sum w \f$
     *  * \f$ s_{01} = \sum x * w \f$
     *  * \f$ s_{02} = \sum y * w \f$
     *  * \f$ s_{03} = \sum l * w \f$
     *
     *  * \f$ s_{11} = \sum x * x * w \f$
     *  * \f$ s_{12} = \sum x * y * w \f$
     *  * \f$ s_{13} = \sum x * l * w \f$
     *
     *  * \f$ s_{22} = \sum y * y * w \f$
     *  * \f$ s_{23} = \sum y * l * w \f$
     *
     *  * \f$ s_{33} = \sum (r^2 - l^2) * l * w \f$
     *  * + symmetric
     */
    Eigen::Matrix<double, 4, 4> getWXYLSumMatrix(CDCObservations2D& observations2D);

    /**
     *  Constructs a symmetric matrix of weighted sums of x, y, r^2 as relevant for circle fits.
     *
     *  Cumulates weights, x positions, y positions, quadratic cylindrical radii and products
     * thereof
     *  @returns symmetric matrix s with the following:
     *  * \f$ s_{00} = \sum w \f$
     *  * \f$ s_{01} = \sum x * w \f$
     *  * \f$ s_{02} = \sum y * w \f$
     *  * \f$ s_{03} = \sum (r^2 - l^2) * w \f$
     *
     *  * \f$ s_{11} = \sum x * x * w \f$
     *  * \f$ s_{12} = \sum x * y * w \f$
     *  * \f$ s_{13} = \sum x * (r^2 - l^2) * w \f$
     *
     *  * \f$ s_{22} = \sum y * y * w \f$
     *  * \f$ s_{23} = \sum y * (r^2 - l^2) * w \f$
     *  * \f$ s_{24} = \sum y * l * w \f$
     *
     *  * \f$ s_{33} = \sum (r^2 - l^2) * (r^2 - l^2) * w \f$
     *  * + symmetric.
     */
    Eigen::Matrix<double, 4, 4> getWXYRSumMatrix(CDCObservations2D& observations2D);

    /**
     *  Constructs a symmetric matrix of weighted sums of x, y as relevant for line fits.
     *
     *  Cumulates weights, x positions, y positions and products thereof
     *  @returns symmetric matrix s with the following:
     *  * \f$ s_{00} = \sum w \f$
     *  * \f$ s_{01} = \sum x * w \f$
     *  * \f$ s_{02} = \sum y * w \f$
     *
     *  * \f$ s_{11} = \sum x * x * w \f$
     *  * \f$ s_{12} = \sum x * y * w \f$
     *
     *  * \f$ s_{22} = \sum y * y * w \f$
     *  * + symmetric.
     */
    Eigen::Matrix<double, 3, 3> getWXYSumMatrix(CDCObservations2D& observations2D);

    /**
     *  Constructs a symmetric matrix of weighted sums of s, z as relevant for line fits.
     *
     *  Cumulates weights, s positions, z positions and products thereof
     *  @returns symmetric matrix s with the following:
     *  * \f$ s_{00} = \sum w \f$
     *  * \f$ s_{01} = \sum s * w \f$
     *  * \f$ s_{02} = \sum z * w \f$
     *
     *  * \f$ s_{11} = \sum s * s * w \f$
     *  * \f$ s_{12} = \sum s * z * w \f$
     *
     *  * \f$ s_{22} = \sum z * z * w \f$
     *  * + symmetric.
     */
    Eigen::Matrix<double, 3, 3> getWSZSumMatrix(CDCSZObservations& szObservations);
  }
}
