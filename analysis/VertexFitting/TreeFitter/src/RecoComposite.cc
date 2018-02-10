/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/RecoComposite.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>

#include <TMatrixFSym.h>

namespace TreeFitter {

  RecoComposite::RecoComposite(Belle2::Particle* particle, const ParticleBase* mother)
    : ParticleBase(particle, mother), m_params(), m_hasEnergy(true)
  {
    updateParams();
  }

  ErrCode RecoComposite::initParticleWithMother(FitParams* fitparams)
  {
    return initTau(fitparams);
  }

  ErrCode RecoComposite::initMotherlessParticle(FitParams* fitparams)
  {
    const int posindex = posIndex();
    const int momindex = momIndex();

    fitparams->getStateVector().segment(posindex, 3) = m_params.segment(0, 3);

    if (7 == dimMeas()) {
      fitparams->getStateVector().segment(momindex, 4) = m_params.segment(3, 4);
    } else {
      fitparams->getStateVector().segment(momindex, 3) = m_params.segment(3, 3);
    }

    return ErrCode::success ;
  }

  void RecoComposite::updateParams()
  {
    const TVector3 pos = particle()->getVertex();
    const TVector3 mom = particle()->getMomentum();
    const double energy = particle()->getEnergy();
    const int size = dimMeas();
    m_params = EigenTypes::ColVector::Zero(size, 1);
    m_params(0) = pos.X();
    m_params(1) = pos.Y();
    m_params(2) = pos.Z();
    m_params(3) = mom.X();
    m_params(4) = mom.Y();
    m_params(5) = mom.Z();

    if (7 == size) {
      m_params(6) = energy;
    }

    m_covariance = EigenTypes::MatrixXd::Zero(size, size);
    const TMatrixFSym cov7in = getBasf2Particle()->getMomentumVertexErrorMatrix(); //this is (p,E,x)

    for (int row = 0; row < size - 3; ++row) { //first the p,E block
      for (int col = 0; col <= row; ++col) {
        m_covariance(3 + row, 3 + col) = cov7in[row][col];
      }
    }

    for (int row = 0; row < 3; ++row) { //then the x block
      for (int col = 0; col <= row; ++col) {
        m_covariance(row, col) = cov7in[3 + row][3 + col];
      }
    }
  }

  ErrCode RecoComposite::projectRecoComposite(const FitParams& fitparams, Projection& p) const
  {
    const int posindex = posIndex() ;
    const int momindex = momIndex() ;
    const int size = dimMeas();

    p.getResiduals().segment(0, 3) = fitparams.getStateVector().segment(posindex, 3) - m_params.segment(0, 3);

    if (size == 7) {
      p.getResiduals().segment(3, 4) = fitparams.getStateVector().segment(momindex, 4) - m_params.segment(3, 4);
    } else if (size == 6) {
      p.getResiduals().segment(3, 3) = fitparams.getStateVector().segment(momindex, 3) - m_params.segment(3, 3);
    }

    for (int row = 0; row < 3; ++row) {
      p.getH()(row,  posindex + row) = 1;
    }

    for (int row = 0; row < size - 3; ++row) {
      p.getH()(3 + row,  momindex + row) = 1;
    }
    p.getV().triangularView<Eigen::Lower>() = m_covariance.triangularView<Eigen::Lower>();

    return ErrCode::success ;
  }

  ErrCode RecoComposite::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::composite:
        status |= projectRecoComposite(fitparams, p);
        break ;
      case Constraint::geometric:
        status |= projectGeoConstraint(fitparams, p);
        break ;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status ;
  }

  double RecoComposite::chiSquare(const FitParams* fitparams) const
  {
    Projection p(fitparams->dim(), dimM());
    projectRecoComposite(*fitparams, p);
    return p.getChiSquare();
  }
}
