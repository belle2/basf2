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
#include <analysis/VertexFitting/TreeFitter/FitParameterDimensionException.h>
#include <string>
namespace TreeFitter {


  FitParams::FitParams(const int dim)
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
    bool ok = true;
    for (int row = 0; row < m_dim; ++row) {
      ok = (m_globalCovariance(row, row) > 0);
      if (!ok) break;
    }
    return ok;
  }

  double FitParams::chiSquare() const
  {
    return m_chiSquare;
  }

  int FitParams::nDof() const
  {
    const int nConstr = nConstraints();
    const int nPars = dim();
    const int ndf = nConstr - nPars;
    if (ndf < 1) {
      const std::string error_string =
        "Not enough constraints for this fit. Try adding a mass or beam cosntraint. constraints: " + std::to_string(
          nConstr) + " parameters to extract: " + std::to_string(nPars) + " ndf: " + std::to_string(ndf);
      throw FitParameterDimensionException(error_string);
    }
    return ndf;
  }

} //TreeFitter namespace
