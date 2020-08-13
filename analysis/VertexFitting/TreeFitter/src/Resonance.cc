/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Wouter Hulsbergen, Francesco Tenchini                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/Resonance.h>

namespace TreeFitter {

  Resonance::Resonance(Belle2::Particle* particle,
                       const ParticleBase* mother,
                       const ConstraintConfiguration& config,
                       bool forceFitAll) :
    InternalParticle(particle, mother, config, forceFitAll) {}

  ErrCode Resonance::initMotherlessParticle(FitParams& fitparams)
  {
    ErrCode status;
    for (auto daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
    }
    return status;
  }

  ErrCode Resonance::initParticleWithMother(FitParams& fitparams)
  {
    ErrCode status;
    for (auto daughter : m_daughters) {
      status |= daughter->initParticleWithMother(fitparams);
    }
    initMomentum(fitparams);
    return status;
  }

  Resonance::~Resonance() = default;

  std::string Resonance::parname(int index) const
  {
    return ParticleBase::parname(index + 4);
  }

}
