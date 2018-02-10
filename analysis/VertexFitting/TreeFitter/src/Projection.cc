/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/Projection.h>

namespace TreeFitter {

  Projection::Projection(int dimP, int dimC) :
    m_H(dimC, dimP),
    m_V(dimC, dimC),
    m_residual(dimC),
    m_dimCov(dimC),
    m_dimProj(dimP),
    m_offset(0) {}


  void Projection::resetProjection()
  {
    m_residual = EigenTypes::ColVector::Zero(m_dimCov);
    m_H = EigenTypes::MatrixXd::Zero(m_dimCov, m_dimProj);
    m_V = EigenTypes::MatrixXd::Zero(m_dimCov, m_dimCov);
    m_offset = 0;
  }

  double Projection::getChiSquare()
  {
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> VSym = m_V.selfadjointView<Eigen::Lower>();
    return m_residual.transpose() * VSym.inverse() * m_residual;
  }

} //END namespace treefitter
