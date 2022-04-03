/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/Origin.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>
#include <framework/geometry/B2Vector3.h>

namespace TreeFitter {

  Origin::Origin(Belle2::Particle* daughter,
                 const ConstraintConfiguration& config,
                 bool forceFitAll
                ) :
    ParticleBase("Origin"),
    m_constraintDimension(config.m_originDimension), //this also affects the geometric constraint
    m_customOriginVertex(config.m_customOriginVertex),
    m_customOriginCovariance(config.m_customOriginCovariance),
    m_posVec(config.m_originDimension),
    m_covariance(config.m_originDimension, config.m_originDimension),
    m_isBeamSpot(config.m_ipConstraint),
    m_inflationFactorCovZ(config.m_inflationFactorCovZ)
  {
    addDaughter(daughter, config, forceFitAll);
    initOrigin();
  }

  ErrCode Origin::initParticleWithMother([[gnu::unused]] FitParams& fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode Origin::initMotherlessParticle(FitParams& fitparams)
  {
    ErrCode status;
    const int posindex = posIndex();
    fitparams.getStateVector().segment(posindex, m_constraintDimension) = m_posVec.segment(0, m_constraintDimension);

    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
      status |= daughter->initParticleWithMother(fitparams);
    }

    return status;
  }

  ErrCode Origin::initOrigin()
  {
    if (m_beamSpot && m_isBeamSpot && m_constraintDimension == 3) {
      m_covariance = Eigen::Matrix<double, 3, 3>::Zero(3, 3);
      const Belle2::B2Vector3D& vertexVector = m_beamSpot->getIPPosition();
      const TMatrixDSym& covVertex = m_beamSpot->getCovVertex();
      m_posVec(0) = vertexVector.x();
      m_posVec(1) = vertexVector.y();
      m_posVec(2) = vertexVector.z();
      m_covariance(0, 0) = covVertex(0 , 0);
      m_covariance(1, 1) = covVertex(1 , 1);
      m_covariance(2, 2) = covVertex(2 , 2);
      m_covariance(1, 0) = covVertex(1 , 0);
      m_covariance(2, 0) = covVertex(2 , 0);
      m_covariance(2, 1) = covVertex(2 , 1);

    } else if (m_beamSpot && m_isBeamSpot && m_constraintDimension == 2) {
      m_covariance = Eigen::Matrix<double, 2, 2>::Zero(2, 2);
      const Belle2::B2Vector3D& vertexVector = m_beamSpot->getIPPosition();
      const TMatrixDSym& covVertex = m_beamSpot->getCovVertex();
      m_posVec(0) = vertexVector.x();
      m_posVec(1) = vertexVector.y();
      m_covariance(0, 0) = covVertex(0 , 0);
      m_covariance(1, 1) = covVertex(1 , 1);
      m_covariance(1, 0) = covVertex(1 , 0);

    } else if (!m_isBeamSpot && m_constraintDimension == 3) {

      if (!(m_customOriginVertex.size() == 3) || !(m_customOriginCovariance.size() == 9)) {
        B2FATAL("Incorrect dimension of customOriginVertex or customOriginCovariance. customOriginVertex dim = "
                << m_customOriginVertex.size() << " customOriginCovariance dim = " << m_customOriginCovariance.size());
      } else if ((m_customOriginCovariance.at(0) < 0) || (m_customOriginCovariance.at(4) < 0) || (m_customOriginCovariance.at(8) < 0)) {
        B2WARNING("An element of customOriginCovariance diagonal is smaller than 0.");
        return ErrCode(ErrCode::Status::badsetup);
      }
      m_posVec(0) = m_customOriginVertex[0];
      m_posVec(1) = m_customOriginVertex[1];
      m_posVec(2) = m_customOriginVertex[2];
      m_covariance(0, 0) = m_customOriginCovariance[0];
      m_covariance(0, 1) = m_customOriginCovariance[1];
      m_covariance(1, 0) = m_customOriginCovariance[3];
      m_covariance(1, 1) = m_customOriginCovariance[4];

      // all with z
      m_covariance(1, 2) = m_inflationFactorCovZ * m_customOriginCovariance[5];
      m_covariance(2, 0) = m_inflationFactorCovZ * m_customOriginCovariance[6];
      m_covariance(2, 1) = m_inflationFactorCovZ * m_customOriginCovariance[7];
      m_covariance(2, 2) = m_inflationFactorCovZ * m_customOriginCovariance[8];
      m_covariance(0, 2) = m_inflationFactorCovZ * m_customOriginCovariance[2];
    } else {
      B2FATAL("The Origin is nether beamspot nor custom. This is ether a configuration error or no beam parameters were found to build the beam spot.");
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode Origin::initCovariance(FitParams& fitpar) const
  {
    ErrCode status;
    const int posindex = posIndex();
    for (int row = 0; row < m_constraintDimension; ++row) {
      fitpar.getCovariance()(posindex + row, posindex + row) = 1000 * m_covariance(row, row);
    }
    for (auto daughter : m_daughters) {
      status |= daughter->initCovariance(fitpar);
    }
    return status;
  }

  ErrCode Origin::projectOriginConstraint(const FitParams& fitparams, Projection& p) const
  {
    const int posindex = posIndex();

    p.getResiduals().segment(0, m_constraintDimension) =
      m_posVec.segment(0, m_constraintDimension) -
      fitparams.getStateVector().segment(posindex, m_constraintDimension);

    for (int row = 0; row < m_constraintDimension; ++row) {
      p.getH()(row, posindex + row) = -1;

      for (int col = 0; col <= row; ++col) {
        p.getV()(row, col) = m_covariance(row, col);
      }
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode Origin::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::origin:
        status |= projectOriginConstraint(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status;
  }

  void Origin::addToConstraintList(constraintlist& list, int depth) const
  {
    for (auto daughter : m_daughters) {
      daughter->addToConstraintList(list, depth - 1);
    }
    list.push_back(Constraint(this, Constraint::origin, depth, m_constraintDimension)) ;
  }
}
