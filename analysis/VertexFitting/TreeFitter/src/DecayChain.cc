/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <algorithm>
#include <framework/logging/Logger.h>

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/DecayChain.h>


namespace TreeFitter {

  DecayChain::DecayChain(Belle2::Particle* particle,
                         const ConstraintConfiguration& config,
                         bool forceFitAll
                        ) :
    m_dim(0),
    m_headOfChain(nullptr),
    m_isOwner(true),
    m_config(config)
  {

    if (config.m_ipConstraint && config.m_customOrigin) {
      B2FATAL("Setup error. Can't have both custom origin and ip constraint.");
    }
    config.m_headOfTreePDG = std::abs(particle->getPDGCode());
    if (config.m_ipConstraint || config.m_customOrigin) {
      m_headOfChain = ParticleBase::createOrigin(particle,
                                                 config,
                                                 forceFitAll
                                                );
    } else if ((!config.m_customOrigin) && (!config.m_ipConstraint)) {

      m_headOfChain = ParticleBase::createParticle(particle, nullptr, config, forceFitAll);
    }

    m_headOfChain->updateIndex(m_dim);

    m_cand = locate(particle);
  }

  DecayChain::~DecayChain()
  {
    if (m_headOfChain && m_isOwner) {
      delete m_headOfChain;
    }
  }

  void DecayChain::initConstraintList()
  {
    m_constraintlist.clear();
    m_headOfChain->addToConstraintList(m_constraintlist, 0);
    removeConstraintFromList();
    std::sort(m_constraintlist.begin(), m_constraintlist.end());
  }

  void DecayChain::removeConstraintFromList()
  {
    for (auto removeConstraint : m_config.m_removeConstraintList) {
      m_constraintlist.erase(std::remove_if(m_constraintlist.begin(), m_constraintlist.end(),
      [&](const Constraint & constraint) { return constraint.name() == removeConstraint ;}),
      m_constraintlist.end());
    }
  }

  ErrCode DecayChain::initialize(FitParams& par)
  {
    ErrCode status;
    par.resetStateVector();
    status |= m_headOfChain->initMotherlessParticle(par);
    par.resetCovariance();
    status |= m_headOfChain->initCovariance(par);
    initConstraintList();
    return status;
  }

  ErrCode DecayChain::filter(FitParams& par)
  {
    ErrCode status;
    par.resetCovariance();
    status |= m_headOfChain->initCovariance(par);
    par.resetChiSquare();
    for (auto constraint : m_constraintlist) {
      status |= constraint.filter(par);
    }
    return status;
  }

  ErrCode DecayChain::filterWithReference(FitParams& par, const FitParams& ref)
  {
    ErrCode status;
    par.resetCovariance();
    status |= m_headOfChain->initCovariance(par);
    par.resetChiSquare();
    for (auto constraint : m_constraintlist) {
      status |= constraint.filterWithReference(par, ref);
    }
    return status;
  }


  double DecayChain::chiSquare(const FitParams& par) const
  {
    return m_headOfChain->chiSquare(par);
  }

  const ParticleBase* DecayChain::locate(Belle2::Particle* particle) const
  {
    const ParticleBase* rc(nullptr);
    const auto mapRow = m_particleMap.find(particle) ;

    if (mapRow == m_particleMap.end()) {
      //take head of chain and recursively find particle in it
      rc = m_headOfChain->locate(particle);

      if (rc && rc->particle()) {
        const_cast<DecayChain*>(this)->m_particleMap[rc->particle()] = rc;
      }
    } else {
      //only used for "head of tree"
      rc = mapRow->second;// (B2::Particle, Particlebase)
    }
    return rc;
  }

  int DecayChain::index(Belle2::Particle* particle) const
  {
    const ParticleBase* base = locate(particle);
    if (base) {
      return base->index();
    }
    return -1;
  }

  int DecayChain::posIndex(Belle2::Particle* particle) const
  {
    const ParticleBase* base = locate(particle);
    if (base) {
      return base->posIndex();
    }
    return -1;
  }

  int DecayChain::momIndex(Belle2::Particle* particle) const
  {
    const ParticleBase* base = locate(particle);
    if (base) {
      return base->momIndex();
    }
    return -1;
  }

  int DecayChain::tauIndex(Belle2::Particle* particle) const
  {
    const ParticleBase* base = locate(particle);
    if (base) {
      return base->tauIndex();
    }
    return -1;
  }
}
