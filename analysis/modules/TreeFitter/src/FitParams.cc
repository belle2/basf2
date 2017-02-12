/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Class to store the fit parameters.

#include <iostream>
#include <iomanip>

#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>

namespace TreeFitter {

  FitParams::FitParams(int dim)
    : m_dim(dim), m_par(dim, 0), m_cov(dim, 0), m_scale(dim, 1),
      m_chiSquare(0), m_nConstraints(0), m_nConstraintsVec(dim, 0) {}

  FitParams::~FitParams() {}

  void FitParams::resetPar()
  {
    for (int row = 1; row <= m_dim; ++row)
      m_par(row) = 0 ;
  }

  void FitParams::resetCov(double scale)
  {
    for (int row = 1; row <= m_dim; ++row) {
      for (int col = 1; col < row; ++col)
        m_cov.fast(row, col) = 0 ;
      m_cov.fast(row, row) *= scale ;
      if (m_cov.fast(row, row) < 0) m_cov.fast(row, row) *= -1 ;
    }
    m_chiSquare = 0 ;
    m_nConstraints = 0 ;
    for (int row = 1; row <= m_dim; ++row)
      nConstraintsVec(row) = 0 ;
  }

  bool FitParams::testCov() const
  {
    bool okay = true ;
    for (int row = 1; row <= m_dim && okay; ++row)
      okay = m_cov.fast(row, row) > 0 ;
    return okay ;
  }

  void FitParams::print() const
  {
    std::cout << std::setw(3) << "index" << std::setw(15) << "val" << std::setw(15) << "err" << std::endl ;
    std::cout << std::setprecision(5) ;
    for (int row = 1; row <= m_dim; ++row)
      std::cout << std::setw(3) << row - 1
                << std::setw(15) << m_par(row)
                << std::setw(15) << sqrt(m_cov(row, row)) << std::endl ;
  } ;

  HepSymMatrix FitParams::cov(const std::vector<int>& indexVec) const
  {
    int nrow = indexVec.size() ;
    HepSymMatrix thecov(nrow, 0) ;
    for (int row = 1; row <= nrow; ++row)
      for (int col = 1; col <= row ; ++col)
        thecov(row, col) = m_cov(indexVec[row - 1] + 1, indexVec[col - 1] + 1);
    return thecov;
  }

  HepVector FitParams::par(const std::vector<int>& indexVec) const
  {
    int nrow = indexVec.size() ;
    HepVector thepar(nrow, 0) ;
    for (int row = 1; row <= nrow; ++row)
      thepar(row) = m_par(indexVec[row - 1] + 1) ;
    return thepar ;
  }

  void FitParams::resize(int newdim)
  {
    if (newdim > m_dim) {
      m_dim = newdim ;
      // very expensive, but okay ...
      HepVector newpar(newdim, 0) ;
      newpar.sub(1, m_par);

      HepSymMatrix newcov(newdim, 0) ;
      newcov.sub(1, m_cov) ;

      m_par = newpar ;
      m_cov = newcov ;
      m_dim = newdim ;
      m_nConstraintsVec.resize(newdim, 0) ;
    }
  }
}
