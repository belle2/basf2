/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FITPARAMS_H
#define FITPARAMS_H

#include <vector>
//#include <TMath.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/DiagMatrix.h>

using namespace CLHEP;

namespace TreeFitter {
  class ParticleBase ;

  class FitParams {
  public:
    // Class that contains the parameters and covariance for the
    // vertex fit.
    FitParams(int dim) ;
    ~FitParams() ;

    HepSymMatrix& cov() { return m_cov ; }
    HepVector& par() { return m_par ; }
    double& par(int row) { return m_par(row) ; }
    double cov(int row) const { return m_cov.fast(row, row) ; }

    HepSymMatrix cov(const std::vector<int>& indexVec) const ;
    HepVector par(const std::vector<int>& indexVec) const ;

    const HepSymMatrix& cov() const { return m_cov ; }
    const HepVector& par() const { return m_par ; }
    const double& par(int row) const { return m_par(row) ; }

    HepDiagMatrix& scale() { return m_scale ; }

    int& nConstraintsVec(int row) { return m_nConstraintsVec[row - 1] ; }

    //int dim() const { return _par.num_row() ; }
    int dim() const { return m_dim ; }
    double chiSquare() const { return m_chiSquare ; }

    int nConstraints() const { return m_nConstraints ; }
    int nDof() const { return nConstraints() - dim() ; }
    double err(int row) const { return sqrt(m_cov(row, row)) ; }

    void resize(int newdim) ;
    void resetPar() ;
    void resetCov(double scale = 100) ;
    void print() const ;
    bool testCov() const ;
    void addChiSquare(double chisq, int nconstraints)
    {
      m_chiSquare += chisq ;
      m_nConstraints += nconstraints ;
    }

    typedef std::vector< std::pair<const ParticleBase*, int> > indexmap ;
    void copy(const FitParams& rhs, const indexmap& anindexmap) ;
  protected:
    FitParams() {}
  private:
    int m_dim;
    HepVector m_par;
    HepSymMatrix m_cov;
    HepDiagMatrix m_scale;
    double m_chiSquare;
    int m_nConstraints;
    std::vector<int> m_nConstraintsVec ; // vector with number of constraints per parameter
  } ;
}

#endif //FITPARAMS_H
