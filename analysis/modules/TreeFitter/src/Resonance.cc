/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates and initialises internal particles in the case where you want to treat them as resonances. Not to be confused with RecoResonance, which loads a pre-reconstructed resonance (i.e. from V0, etc)

#include <analysis/modules/TreeFitter/Resonance.h>

namespace TreeFitter {

  extern int vtxverbose;

  Resonance::Resonance(Belle2::Particle* particle, const ParticleBase* mother,
                       bool forceFitAll)
    : InternalParticle(particle, mother, forceFitAll) {}  //ParticleBase("Resonance unused atm") {}


  ErrCode Resonance::initMotherlessParticle(FitParams* fitparams)
  {
    ErrCode status;
    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
    }
    return status;
  }

  ErrCode Resonance::initParticleWithMother(FitParams* fitparams)
  {
    ErrCode status;
    for (auto daughter : m_daughters) {
      status |= daughter->initParticleWithMother(fitparams);
    }
    initMomentum(fitparams);
    return status;
  }

  Resonance::~Resonance() {};

  std::string Resonance::parname(int index) const
  {
    return ParticleBase::parname(index + 4);
  }

}
