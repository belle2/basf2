/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FITPARAMS_H
#define FITPARAMS_H

#include <vector>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/DiagMatrix.h>


namespace TreeFitter {
  class ParticleBase ;

  /** Class to store and manage fitparams (statevector) */
  class FitParams {

  public:

    /** Constructor */
    FitParams(int dim);

    /** Destructor */
    ~FitParams();

    /** get entire covariance of statevector */
    CLHEP::HepSymMatrix& cov() { return m_cov; }

    /** get non const statevector */
    CLHEP::HepVector& par() { return m_par; }

    /**  get reference(!) to element in statevector
     * FIXME maybe a setter is better?
     * */
    double& par(int row) { return m_par(row); }

    /** get sigma2 (diag element) for row */
    double cov(int row) const { return m_cov.fast(row, row); }

    /** get non const sub covariance matrix of statevector */
    CLHEP::HepSymMatrix cov(const std::vector<int>& indexVec) const;

    /** get parameters of statevector with index */
    CLHEP::HepVector par(const std::vector<int>& indexVec) const;

    /** get statevectors covariance */
    const CLHEP::HepSymMatrix& cov() const { return m_cov; }

    /** get const const statevector */
    const CLHEP::HepVector& par() const { return m_par; }

    /**get const parameter in row (element of statevector) */
    const double& par(int row) const { return m_par(row); }

    /** getter for the scale matrix (matrix with large value on diag, 0 on offdiag)
     *  FIXME unused
     * */
    CLHEP::HepDiagMatrix& scale() { return m_scale; }

    /** returns a reference(!) to the number of constraints for rows parameter. Used to reset that value.
     *  FIXME this is stupid.
     *    replace with with setter.
     *    only used in resetCov and KalmanCalculator.cc
     * */
    int& nConstraintsVec(int row) { return m_nConstraintsVec[row - 1]; }

    /** get dimension od statevector */
    int dim() const { return m_dim; }

    /** get chi2 of statevector*/
    double chiSquare() const { return m_chiSquare; }

    /** get number of constraints */
    int nConstraints() const { return m_nConstraints; }

    /** get numer of degrees of freedom */
    int nDof() const { return nConstraints() - dim(); }// 6
    //int nDof() const { return dim(); }
    //int nDof() const { return dim()- nConstraints(); }

    /** return sigma (sqrt(diagonal element)) of row */
    double err(int row) const { return sqrt(m_cov(row, row)); }

    /** resize (enlarge!) the statevector */
    void resize(int newdim);

    /** set statevector elements to 0*/
    void resetPar();

    /** resets the lower (to 0) triangle (excluded diag) of the cov and multiplies the diag with scale */
    void resetCov(double scale = 100);

    /** unused FIXME delete? */
    void print() const;

    /** check if global cov makes sense*/
    bool testCov() const;

    /** increment global chi2 */
    void addChiSquare(double chisq, int nconstraints)
    {
      m_chiSquare += chisq;
      m_nConstraints += nconstraints;
    }

    void resetChiSquare()
    {
      m_chiSquare = 0;
    }

    /** index map */
    typedef std::vector< std::pair<const ParticleBase*, int> > indexmap;


  protected:

    /** constructor for ROOT?? FIXME do we need this? */
    FitParams() {}

  private:

    /** dimension of statevector */
    int m_dim;

    /** statevector */
    CLHEP::HepVector m_par;

    /** covariance of the state vector*/
    CLHEP::HepSymMatrix m_cov;

    /** this is used in Fitter.cc to sclae down the values if niter > 10. Unused in our version FIXME delete?  */
    CLHEP::HepDiagMatrix m_scale;

    /** chi2 */
    double m_chiSquare;

    /** number of conatraints */
    int m_nConstraints;

    /** vector with the number of constraints per parameter */
    std::vector<int> m_nConstraintsVec;
  } ;
}

#endif //FITPARAMS_H
