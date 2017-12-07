/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates reconstructed resonances. This is not actually used or tested yet in basf2, but is kept for future use. Be careful! It likely won't work out of the box as of release-00-08-00

//#include <analysis/dataobjects/Particle.h>
#include <analysis/VertexFitting/TreeFitter/RecoResonance.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>

namespace TreeFitter {
  extern int vtxverbose ;

  RecoResonance::RecoResonance(Belle2::Particle* particle, const ParticleBase* mother)
    : RecoComposite(particle, mother) {}

  ErrCode RecoResonance::initParticleWithMother([[gnu::unused]] FitParams* fitparams)
  {
    return ErrCode::success;
  }

  ErrCode RecoResonance::initMotherlessParticle(FitParams* fitparams)
  {
    int posindex = posIndex();
    int momindex = momIndex();

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
    return ErrCode::success;
  }

  RecoResonance::~RecoResonance() {};

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
