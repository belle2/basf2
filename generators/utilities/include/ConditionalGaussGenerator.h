/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>
#include <Eigen/Dense>

namespace Belle2 {

  class ConditionalGaussGenerator {

  public:

    ConditionalGaussGenerator() {}
    ConditionalGaussGenerator(Eigen::VectorXd mu, Eigen::MatrixXd covMat);

    Eigen::VectorXd generate(double x0) const;

    double getX0spread()         const { return sqrt(m_covMat(0, 0)); }
    Eigen::VectorXd getMu()      const { return m_mu; }
    Eigen::MatrixXd getCovMat()  const { return m_covMat; }
  private:
    Eigen::VectorXd m_mu;
    Eigen::MatrixXd m_covMat;

    Eigen::MatrixXd m_vBaseMat;
    std::vector<Eigen::VectorXd> m_cOrt;
    Eigen::VectorXd m_v0norm;
  };
}
