/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/RecoParticle.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>

namespace TreeFitter {

  RecoParticle::RecoParticle(Belle2::Particle* particle, const ParticleBase* mother)
    : ParticleBase(particle, mother) {}

  ErrCode RecoParticle::initMotherlessParticle([[gnu::unused]] FitParams& fitparams)
  {
    return ErrCode(ErrCode::Status::success);
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
