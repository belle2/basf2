/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jo-Frederik Krohn                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/Origin.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

namespace TreeFitter {

  Origin::Origin(Belle2::Particle* daughter,
                 bool forceFitAll,
                 std::vector<double> costumOriginVertex,
                 std::vector<double> costumOriginCovariance
                ) :
    ParticleBase("Origin"),
    m_constraintDimension(3),
    m_costumOriginVertex(costumOriginVertex),
    m_costumOriginCovariance(costumOriginCovariance),
    m_posVec(3),
    m_covariance(3, 3)
  {
    addDaughter(daughter, forceFitAll);
    initOrigin();
  }

  ErrCode Origin::initParticleWithMother([[gnu::unused]] FitParams* fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode Origin::initMotherlessParticle(FitParams* fitparams)
  {
    ErrCode status;
    const int posindex = posIndex();
    fitparams->getStateVector().segment(posindex, m_constraintDimension) = m_posVec.segment(0, 3);

    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
      status |= daughter->initParticleWithMother(fitparams);
    }
    return status;
  }

  ErrCode Origin::initOrigin()
  {
    ErrCode status;
    m_covariance = Eigen::Matrix<double, 3, 3>::Zero(3, 3);
    m_posVec(0) = m_costumOriginVertex[0];
    m_posVec(1) = m_costumOriginVertex[1];
    m_posVec(2) = m_costumOriginVertex[2];
    m_covariance(0, 0) = m_costumOriginCovariance[0];
    m_covariance(1, 1) = m_costumOriginCovariance[1];
    m_covariance(2, 2) = m_costumOriginCovariance[2];

    /* FIXME REMOVE:  <12-04-18, jkrohn> */
    std::cout << "Custom origin vertex\n" << m_posVec  << std::endl;
    std::cout << "Custom origin Covariance  \n" << m_covariance  << std::endl;

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode Origin::initCovariance(FitParams* fitpar) const
  {
    ErrCode status;
    const int posindex = posIndex();
    for (int row = 0; row < m_constraintDimension; ++row) {
      fitpar->getCovariance()(posindex + row, posindex + row) = 1000 * m_covariance(row, row);
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
      case Constraint::beamspot:
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
