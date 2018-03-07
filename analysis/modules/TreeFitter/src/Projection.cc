/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <iostream>
#include <iomanip>

#include <framework/logging/Logger.h>

#include <analysis/modules/TreeFitter/Projection.h>

namespace TreeFitter {


  Projection::Projection(int dimP, int dimC) :
    m_matrixH(dimC, dimP), //JFK: this is deprecated 2017-09-21
    m_r(dimC), //JFK: this is deprecated 2017-09-21
    m_matrixV(dimC), //JFK: this is deprecated 2017-09-21j
    m_H(dimC, dimP),
    m_V(dimC, dimC),
    m_residual(dimC),
    m_dimCov(dimC),
    m_dimProj(dimP),
    m_offset(0) {}

  Projection::~Projection() {};

  void Projection::reset()  //JFK: this is deprecated 2017-09-21
  {
    // fill everything with '0'.  this implementation is kind of
    // tricky, but it is fast.
    int dimC = m_matrixH.num_row() ;
    int dimP = m_matrixH.num_col() ;
    //      std::cout << "Projection::reset(): resetting H(" << dimC << "," << dimP << ")" << std::endl << std::flush;
    memset(&(m_matrixH(1, 1)), 0, dimP * dimC * sizeof(double));
    memset(&(m_r(1))  , 0, dimC * sizeof(double));
    memset(&(m_matrixV(1, 1)), 0, dimC * (dimC + 1) / 2 * sizeof(double));
    m_offset = 0 ;
  }

  void Projection::resetProjection()
  {

    m_residual = EigenTypes::ColVector::Zero(m_dimCov);
    //m_residual = Eigen::Matrix<double, 5, 1>::Zero(5, 1);
    //EigenTypes::MatrixXd invCov = m_V.inverse();
    m_H = EigenTypes::MatrixXd::Zero(m_dimCov, m_dimProj);
    //m_V.triangularView<Eigen::Lower>() = EigenTypes::MatrixXd::Zero(m_dimCov, m_dimCov).triangularView<Eigen::Lower>();
    //JFK: save mode to find my bug ... 2017-09-30
    m_V = EigenTypes::MatrixXd::Zero(m_dimCov, m_dimCov);
    //m_V = Eigen::Matrix<double, 5, 5>::Zero(5, 5);
    m_offset = 0;
  }

  double Projection::chiSquare() const //JFK: this is deprecated 2017-09-21
  {
    CLHEP::HepSymMatrix W = m_matrixV ;
    int ierr;  W.inverse(ierr) ;
    double result = W.similarity(m_r);
    return result;
  }

  double Projection::getChiSquare()
  {
    EigenTypes::MatrixXd VSym = m_V.selfadjointView<Eigen::Lower>();
    //Eigen::Matrix<double, 5, 5> VSym = m_V.selfadjointView<Eigen::Lower>();
    return m_residual.transpose() * VSym.inverse() * m_residual;
  }

} //END namespace treefitter
