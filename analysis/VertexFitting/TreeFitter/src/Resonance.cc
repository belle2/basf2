/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
