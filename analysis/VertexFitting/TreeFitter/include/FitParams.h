/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    explicit FitParams(const int dim);

    /** Destructor */
    ~FitParams() {};

    /** copy constructor */
    FitParams(const FitParams& toCopy)
      : m_dim(toCopy.m_dim),
        m_chiSquare(toCopy.m_chiSquare),
        m_nConstraints(toCopy.m_nConstraints),
        m_dimensionReduction(toCopy.m_dimensionReduction),
        m_globalState(Eigen::Matrix < double, -1, 1, 0, MAX_MATRIX_SIZE, 1 > (toCopy.m_globalState)),
        m_globalCovariance(Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > (toCopy.m_globalCovariance))
    { }

    /** Assignment operator. */
    FitParams& operator =(const FitParams& other)
    {
      m_dim = other.m_dim;
      m_chiSquare = other.m_chiSquare;
      m_nConstraints = other.m_nConstraints;
      m_dimensionReduction = other.m_dimensionReduction;
      m_globalState = other.m_globalState;
      m_globalCovariance = other.m_globalCovariance;
      return *this;
    }

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

    /** reset the statevector */
    void resetStateVector();

    /** reset the statevector */
    void resetCovariance();

    /** get the states dimension */
    int getDimensionOfState() const {return m_dim;};

    /** test if the covariance makes sense */
    bool testCovariance() const;

    /** get chi2 of statevector*/
    double chiSquare() const {return m_chiSquare;};

    /** get number of degrees of freedom */
    int nDof() const;

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

    /** dimension of statevector */
    int m_dim;

    /** chi2 */
    double m_chiSquare;

    /** number of constraints */
    int m_nConstraints;

    /** reduce the ndf used in the chi2 by this count */
    int m_dimensionReduction;

    /** vector with the number of constraints per parameter */
    std::vector<int> m_nConstraintsVec;

    /** vector holding all parameters of this fit */
    Eigen::Matrix < double, -1, 1, 0, MAX_MATRIX_SIZE, 1 > m_globalState;

    /** covariance of the global state */
    Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE > m_globalCovariance;

  };
}
