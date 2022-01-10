/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/FitParameterDimensionException.h>
#include <string>

namespace TreeFitter {


  FitParams::FitParams(const int dim)
    :
    m_dim(dim),
    m_chiSquare(1e10),
    m_nConstraints(0),
    m_dimensionReduction(0),
    m_nConstraintsVec(dim, 0),
    m_globalState(dim),
    m_globalCovariance(dim, dim)
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
        "Not enough constraints for this fit. Try adding a mass or beam constraint. constraints: " + std::to_string(
          nConstr) + " parameters to extract: " + std::to_string(nPars) + " ndf: " + std::to_string(ndf);
      throw FitParameterDimensionException(error_string);
    }
    return ndf;
  }

} //TreeFitter namespace
