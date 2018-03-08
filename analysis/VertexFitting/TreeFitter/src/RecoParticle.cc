/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/RecoParticle.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>

namespace TreeFitter {

  RecoParticle::RecoParticle(Belle2::Particle* particle, const ParticleBase* mother)
    : ParticleBase(particle, mother) {}

  ErrCode RecoParticle::initMotherlessParticle([[gnu::unused]] FitParams* fitparams)
  {
    return ErrCode::success;
  }

  std::string RecoParticle::parname(int index) const
  {
    return ParticleBase::parname(index + 4);
  }

  ErrCode RecoParticle::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status ;
    switch (type) {
      case Constraint::track :
      case Constraint::photon :
      case Constraint::klong :
        status |= projectRecoConstraint(fitparams, p);
        break ;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status;
  }

}
