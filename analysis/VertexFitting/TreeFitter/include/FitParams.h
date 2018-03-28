/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/EigenStackConfig.h>
#include <Eigen/Core>

#include <vector>

namespace TreeFitter {

  class ParticleBase;

  /** Class to store and manage fitparams (statevector) */
  class FitParams {

  public:

    /** Constructor */
    FitParams(int dim);

    /** Destructor */
    ~FitParams() {};

    /** getter for the states covariance */
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > & getCovariance()
    {
      return m_globalCovariance;
    }

    /** const getter for the states covariance */
    const Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > & getCovariance() const
    {
      return m_globalCovariance;
    }

    /** getter for the fit parameters/statevector */
    Eigen::Matrix < double, -1, 1, 0, MAX_MATRIX_SIZE, 1 > & getStateVector() {return m_globalState;}

    /** const getter for the fit parameters/statevector */
    const Eigen::Matrix < double, -1, 1, 0, MAX_MATRIX_SIZE, 1 > & getStateVector() const
    {
      return m_globalState;
    }

    /** get a reference to an element of the state vector todo replace by setter? */
    double& getRefToElementOfStateVec(int row) { return m_globalState(row, 0); }

    /** get a const reference to an element of the state vector todo replace by setter? */
    const double& getRefToElementOfStateVec(int row) const { return m_globalState(row, 0); }

    /** get an covaraince diagonal element   */
    double getCovDiaElement(int counter) { return m_globalCovariance(counter, counter); }

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

    /** test if the covariance makes sense */
    bool testCovariance() const;

    /** increment nconstraints vec */
    int& incrementNConstraintsVec(int row) { return m_nConstraintsVec[row];}

    /** returns a reference(!) to the number of constraints for rows parameter. Used to reset that value.
     *  FIXME this is stupid.
     *    replace with with setter.
     *    only used in resetCov and KalmanCalculator.cc
     * */
    int& nConstraintsVec(int row) { return m_nConstraintsVec[row - 1]; }

    /** get dimension od statevector */
    int dim() const { return m_dim; }

    /** get chi2 of statevector*/
    double chiSquare() const;

    /** get number of constraints */
    int nConstraints() const { return m_nConstraints; }

    /** get numer of degrees of freedom */
    int nDof() const { return std::abs(nConstraints() - dim()); }// FIXME why can this be negative?

    /** resize (enlarge!) the statevector */
    void resize(int newdim);

    /** set statevector elements to 0*/
    void resetPar();

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
      m_nConstraints = 0;
    }

    /** index map */
    typedef std::vector< std::pair<const ParticleBase*, int> > indexmap;

  protected:

    /** constructor  */
    FitParams() {}

  private:

    /** vector holding all parameters of this fit */
    Eigen::Matrix < double, -1, 1, 0, MAX_MATRIX_SIZE, 1 > m_globalState;

    /** covariance of the global state */
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > m_globalCovariance;

    /** dimension of statevector */
    int m_dim;

    /** chi2 */
    double m_chiSquare;

    /** number of conatraints */
    int m_nConstraints;

    /** vector with the number of constraints per parameter */
    std::vector<int> m_nConstraintsVec;
  };
}
