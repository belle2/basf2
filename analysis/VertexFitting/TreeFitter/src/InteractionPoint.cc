/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/InteractionPoint.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

namespace TreeFitter {

  InteractionPoint::InteractionPoint(Belle2::Particle* daughter,
                                     bool forceFitAll,
                                     int dimension) :
    ParticleBase("InteractionPoint"),
    m_constraintDimension(dimension),
    m_ipPosVec(dimension),
    m_ipCovariance(dimension, dimension)
  {
    addDaughter(daughter, forceFitAll);
    initBeamSpot();
  }

  ErrCode InteractionPoint::initParticleWithMother([[gnu::unused]] FitParams* fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode InteractionPoint::initMotherlessParticle(FitParams* fitparams)
  {
    ErrCode status;
    const int posindex = posIndex();
    fitparams->getStateVector().segment(posindex, m_constraintDimension) = m_ipPosVec.segment(0, m_constraintDimension);

    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
      status |= daughter->initParticleWithMother(fitparams);
    }
    return status;
  }

  ErrCode InteractionPoint::initBeamSpot()
  {
    ErrCode status;

    if (m_beamParams) {
      const TVector3& vertexVector = m_beamParams->getVertex();
      const TMatrixDSym& covVertex = m_beamParams->getCovVertex();
      if (m_constraintDimension == 3) {
        m_ipPosVec(0) = vertexVector.x();
        m_ipPosVec(1) = vertexVector.y();
        m_ipPosVec(2) = vertexVector.z();
        m_ipCovariance(0, 0) = covVertex(0 , 0);
        m_ipCovariance(1, 1) = covVertex(1 , 1);
        m_ipCovariance(2, 2) = covVertex(2 , 2);
        m_ipCovariance(1, 0) = covVertex(1 , 0);
        m_ipCovariance(2, 0) = covVertex(2 , 0);
        m_ipCovariance(2, 1) = covVertex(2 , 1);
      } else if (m_constraintDimension == 2) {
        m_ipPosVec(1) = vertexVector.x();
        m_ipPosVec(2) = vertexVector.y();
        m_ipCovariance(0, 0) = covVertex(0 , 0);
        m_ipCovariance(1, 1) = covVertex(1 , 1);
        m_ipCovariance(1, 0) = covVertex(1 , 0);
      } else {
        B2FATAL("The IP constraint has to have a dimension of either 0, 2 or 3. You specified: " << m_constraintDimension);
      }
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode InteractionPoint::initCovariance(FitParams* fitpar) const
  {
    ErrCode status;
    const int posindex = posIndex();
    for (int row = 0; row < m_constraintDimension; ++row) {
      fitpar->getCovariance()(posindex + row, posindex + row) = 1000 * m_ipCovariance(row, row);
    }
    for (auto daughter : m_daughters) {
      status |= daughter->initCovariance(fitpar);
    }
    return status;
  }

  ErrCode InteractionPoint::projectIPConstraint(const FitParams& fitparams, Projection& p) const
  {
    const int posindex = posIndex();

    p.getResiduals().segment(0, m_constraintDimension) =
      m_ipPosVec.segment(0, m_constraintDimension) -
      fitparams.getStateVector().segment(posindex, m_constraintDimension);

    for (int row = 0; row < m_constraintDimension; ++row) {
      p.getH()(row, posindex + row) = -1;

      for (int col = 0; col <= row; ++col) {
        p.getV()(row, col) = m_ipCovariance(row, col);
      }
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode InteractionPoint::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::beamspot:
        status |= projectIPConstraint(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status;
  }

  void InteractionPoint::addToConstraintList(constraintlist& list, int depth) const
  {
    for (auto daughter : m_daughters) {
      daughter->addToConstraintList(list, depth - 1);
    }
    list.push_back(Constraint(this, Constraint::beamspot, depth, m_constraintDimension)) ;
  }
}
