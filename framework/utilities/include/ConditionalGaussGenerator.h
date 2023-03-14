/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>
#include <cmath>
#include <Eigen/Dense>

namespace Belle2 {

  /** Class implementing n-dimensional random number generator from Gaussian distribution
   *  where the first component of the vector is generated first (using some external source)
   *  and the remaining components are generated based on the value of the first component.
   *  The obtained numbers have identical statistical behaviour like when all components
   *  are generated simultaneously */
  class ConditionalGaussGenerator {

  public:

    /** dummy constructor without arguments  */
    ConditionalGaussGenerator() {}

    /** constructor which takes vector of central values and covariance matrix as input */
    ConditionalGaussGenerator(const Eigen::VectorXd& mu, const Eigen::MatrixXd& covMat);

    /** generate random vector based on the provided first component x0
     *
     * @param x0 the first component of the random vector
     * @return the vector of the random numbers, it starts with x0
     */
    Eigen::VectorXd generate(double x0) const;

    /** get the spread of first component which can be used by external generator */
    double getX0spread()         const { return std::sqrt(m_covMat(0, 0)); }

    /** get the vector including central values of the distribution */
    Eigen::VectorXd getMu()      const { return m_mu; }

    /** get the covariance matrix describing n-dimensional Gaussian distribution */
    Eigen::MatrixXd getCovMat()  const { return m_covMat; }
  private:
    Eigen::VectorXd m_mu;          ///< central values of the distribution
    Eigen::MatrixXd m_covMat;      ///< covariance matrix of the distribution

    Eigen::MatrixXd m_vBaseMat;          ///< transformation matrix between eigen-system of m_covMat and nominal system
    std::vector<Eigen::VectorXd> m_cOrt; ///< array of vectors describing free degrees of freedom of random generator
    Eigen::VectorXd m_v0norm;            ///< vector which scales with dx0
  };
}
