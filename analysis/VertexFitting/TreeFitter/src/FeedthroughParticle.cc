/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn, Fabian Krinner     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/Particle.h>
#include <analysis/VertexFitting/TreeFitter/FeedthroughParticle.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <framework/logging/Logger.h>

using std::vector;

namespace TreeFitter {

  FeedthroughParticle::FeedthroughParticle(Belle2::Particle*   particle,
                                           const ParticleBase* mother,
                                           const ConstraintConfiguration& config,
                                           bool  forceFitAll
                                          ) :
    ParticleBase(particle, mother, &config)
  {
    if (particle) {
      if (particle->getNDaughters() > 1) {
        B2ERROR("FeedthroughParticles can only be initialized with zero or one daughters.");
      }
      if (particle->getNDaughters() == 1) {
        addDaughter(particle->getDaughters()[0], config, forceFitAll);
      }
    } else {
      B2ERROR("Trying to create a FeedthroughParticle from NULL. This should never happen.");
    }
  }

  ErrCode FeedthroughParticle::initMotherlessParticle(FitParams& fitparams)
  {
    ErrCode status;

    for (ParticleBase* daughter : m_daughters) {
      status |= daughter->initMotherlessParticle(fitparams);
    }
    for (ParticleBase* daughter : m_daughters) {
      daughter->initParticleWithMother(fitparams);
    }
    return status;
  }

  ErrCode FeedthroughParticle::initCovariance(FitParams& fitparams) const
  {
    ErrCode status;
    for (ParticleBase* daughter : m_daughters) {
      status |= daughter->initCovariance(fitparams);
    }
    return status;
  }

  void FeedthroughParticle::addToConstraintList(constraintlist& list,
                                                int depth) const
  {
    for (ParticleBase* daughter : m_daughters) {
      daughter->addToConstraintList(list, depth - 1);
    }
  }

  int FeedthroughParticle::posIndex() const
  {
    int rc = -1;
    if (m_daughters.size()) {
      rc = m_daughters[0]->posIndex();
    }
    return rc;
  }

  int FeedthroughParticle::momIndex() const
  {
    int rc = -1;
    if (m_daughters.size()) {
      rc = m_daughters[0]->momIndex();
    }
    return rc;
  }



}

