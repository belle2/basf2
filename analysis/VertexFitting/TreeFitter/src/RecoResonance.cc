/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/RecoResonance.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>

namespace TreeFitter {

  RecoResonance::RecoResonance(Belle2::Particle* particle, const ParticleBase* mother)
    : RecoComposite(particle, mother) {}

  ErrCode RecoResonance::initParticleWithMother([[gnu::unused]] FitParams* fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoResonance::initMotherlessParticle(FitParams* fitparams)
  {
    const int posindex = posIndex();
    const int momindex = momIndex();

    //quick map for parameters
    int indexmap[7];
    for (int i = 0; i < 3; ++i) {
      indexmap[i]   = posindex + i;
    }
    for (int i = 0; i < 4; ++i) {
      indexmap[i + 3] = momindex + i;
    }

    // copy the 'measurement' -> this overwrites mother position !
    for (int row = 0; row < dimM(); ++row) {
      fitparams->getStateVector()(indexmap[row]) = m_params[row];
    }
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoResonance::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::resonance:
        status |= projectRecoComposite(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status;
  }

  std::string RecoResonance::parname(int index) const
  {
    return ParticleBase::parname(index + 4);
  }
}
