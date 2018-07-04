/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

namespace TreeFitter {

  FitParams::FitParams(int dim)
    : m_globalState(dim),
      m_globalCovariance(dim, dim),
      m_dim(dim),
      m_chiSquare(1e10),
      m_nConstraints(0),
      m_dimensionReduction(0),
      m_nConstraintsVec(dim, 0)
  {
    resetStateVector();
    resetCovariance();
  }

  void FitParams::resetStateVector()
  {
    m_globalState = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(m_dim);
  }

  void FitParams::resetCovariance()
  {
    m_globalCovariance =
      Eigen::Matrix < double, -1, -1, 0, MAX_MATRIX_SIZE, MAX_MATRIX_SIZE >::Zero(m_dim, m_dim);

    std::fill(m_nConstraintsVec.begin(), m_nConstraintsVec.end(), 0);
    m_chiSquare = 0;
    m_nConstraints = 0;
  }

  bool FitParams::testCovariance() const
  {
    bool okay = true;
    for (int row = 0; row < m_dim && okay; ++row) {
      okay = (m_globalCovariance(row, row) > 0);
    }
    return okay;
  }

  double FitParams::chiSquare() const
  {
    return m_chiSquare;
  }

  int FitParams::nDof() const
  {
    const double ndf = nConstraints() - dim();
    const double ndf_reduced = ndf - m_dimensionReduction;
    if (ndf < 1) { B2FATAL("Not enough constraints for this fit. Add a mass or a beamcosntraint. n constraints (equations) " << nConstraints() << " free parameters " << dim()); }
    if (ndf_reduced < 1) { B2WARNING("Potentially underconstraint topology. Will try to fit this anyway. Degrees of freedom (in theory) " << ndf_reduced);}
    return ndf;
  }


} //TreeFitter namespace
