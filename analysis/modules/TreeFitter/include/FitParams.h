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

#include <analysis/modules/TreeFitter/EigenTypes.h>

namespace TreeFitter {
  class ParticleBase ;
  /** Class to store and manage fitparams (statevector) */
  class FitParams {

  public:

    /** Constructor */
    FitParams(int dim);

    /** Destructor */
    ~FitParams();

    /** getter for the states covariance */
    EigenTypes::MatrixXd& getCovariance() { return m_globalCovariance; }
    /** const getter for the states covariance */
    const EigenTypes::MatrixXd& getCovariance() const { return m_globalCovariance; }

    /** getter for the fit parameters/statevector */
    EigenTypes::ColVector& getStateVector() {return m_globalState;}

    /** const getter for the fit parameters/statevector */
    const EigenTypes::ColVector& getStateVector() const { return m_globalState; }

    void setElmInStateVec(int elm, double val)
    {
      m_globalState(elm) = val;
    }
    double getElmInStateVec(int elm) {return m_globalState(elm); }


    /** get a reference to an element of the state vector todo replace by setter? */
    double& getRefToElementOfStateVec(int row) { return m_globalState(row, 0); } //vector defined as column vector
    /** get a const reference to an element of the state vector todo replace by setter? */
    const double& getRefToElementOfStateVec(int row) const { return m_globalState(row, 0); } //vector defined as column vector this is even more weird

    /** get an covaraince diagonal element   */
    double getCovDiaElement(int counter) { return m_globalCovariance(counter, counter); }

    /** get a sub block from the covaraince matrix */
    EigenTypes::MatrixXd getMaskInCovariance(const std::vector<int>& indexVec) const;
    /** get a sub block from the state vector */
    EigenTypes::ColVector getMaskInStateVec(const std::vector<int>& indexVec) const;

    /** reset the staevector */
    void resetStateVector();
    /** reset the staevector */
    void resetCovariance();
    /** resize and reset statevec and its covariance */
    void resizeAndResetStateAndCov(int newdim);

    /** get the states dimension */
    int getDimensionOfState() const {return m_dim;};
    /** get dimension sum of constraints */
    int getNConstraints() {return m_nConstraints;}
    /** get degress of freedom */
    int getNDOF() {return m_nConstraints - m_dim; }
    /** this si weird */
    //int getElmInConstraintVec(int row) { return m_globalState(row, 1);}
    /** this si weird */
    //void setElmInConstraintVec(int row, double value) {m_globalState(row, 1) = value;}
    /** test if the covariance makes sense */
    bool testCovariance() const;
    /** increment nconstraints vec */
    int& incrementNConstraintsVec(int row) { return m_nConstraintsVec[row];}

    /** BELOW COMES DEPRECATED STUFF */

    /** get entire covariance of statevector */
    CLHEP::HepSymMatrix& cov() { return m_cov; } //deprecated

    /** get non const statevector */
    CLHEP::HepVector& par() { return m_par; } //deprecated

    /**  get reference(!) to element in statevector
     * FIXME maybe a setter is better?
     * */
    double& par(int row) { return m_par(row); } //deprecated

    /** get sigma2 (diag element) for row */
    double cov(int row) const { return m_cov.fast(row, row); } //deprecated

    /** get non const sub covariance matrix of statevector */
    CLHEP::HepSymMatrix cov(const std::vector<int>& indexVec) const; //deprecated

    /** get parameters of statevector with index */
    CLHEP::HepVector par(const std::vector<int>& indexVec) const; //deprecated


    /** get statevectors covariance */
    const CLHEP::HepSymMatrix& cov() const { return m_cov; } //deprecated

    /** get const const statevector */
    const CLHEP::HepVector& par() const { return m_par; }//deprecated

    /**get const parameter in row (element of statevector) */
    const double& par(int row) const { return m_par(row); } //JFK: this is deprecated 2017-09-21

    /** getter for the scale matrix (matrix with large value on diag, 0 on offdiag)
     *  FIXME unused
     * */
    CLHEP::HepDiagMatrix& scale() { return m_scale; } //JFK: remove 2017-09-21

    /** returns a reference(!) to the number of constraints for rows parameter. Used to reset that value.
     *  FIXME this is stupid.
     *    replace with with setter.
     *    only used in resetCov and KalmanCalculator.cc
     * */
    int& nConstraintsVec(int row) { return m_nConstraintsVec[row - 1]; } //JFK: this is deprecated 2017-09-21

    /** get dimension od statevector */
    int dim() const { return m_dim; } //JFK: this is deprecated 2017-09-21

    /** get chi2 of statevector*/
    double chiSquare() const { return m_chiSquare; }

    /** get number of constraints */
    int nConstraints() const { return m_nConstraints; }

    /** get numer of degrees of freedom */
    int nDof() const { return nConstraints() - dim(); }// 6

    /** return sigma (sqrt(diagonal element)) of row */
    double err(int row) const { return sqrt(m_cov(row, row)); }

    /** resize (enlarge!) the statevector */
    void resize(int newdim);

    /** set statevector elements to 0*/
    void resetPar();

    /** resets the lower (to 0) triangle (excluded diag) of the cov and multiplies the diag with scale */
    void resetCov(double scale = 100); //JFK: this is deprecated 2017-09-21

    /** check if global cov makes sense*/
    bool testCov() const;

    /** increment global chi2 */
    void addChiSquare(double chisq, int nconstraints)
    {
      m_chiSquare += chisq;
      m_nConstraints += nconstraints;
    }
    /** reset chi2 */
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

    /** vector holding all parameters of this fit
     * */
    EigenTypes::ColVector m_globalState;

    /** covariance of the global state */
    EigenTypes::MatrixXd m_globalCovariance;

    /** dimension of statevector */
    int m_dim;

    /** statevector */
    CLHEP::HepVector m_par; //JFK: this is deprecated 2017-09-21

    /** covariance of the state vector*/
    CLHEP::HepSymMatrix m_cov; //JFK: this is deprecated 2017-09-21

    /** this is used in Fitter.cc to sclae down the values if niter > 10. Unused in our version FIXME delete?  */
    CLHEP::HepDiagMatrix m_scale; //JFK: can this be removed? 2017-09-21

    /** chi2 */
    double m_chiSquare;

    /** number of conatraints */
    int m_nConstraints;

    /** vector with the number of constraints per parameter */
    std::vector<int> m_nConstraintsVec;
  } ;
}

#endif //FITPARAMS_H
