/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <algorithm>
#include <framework/logging/Logger.h>

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/InteractionPoint.h>
#include <analysis/VertexFitting/TreeFitter/DecayChain.h>


namespace TreeFitter {

  DecayChain::DecayChain(Belle2::Particle* particle, bool forceFitAll, const int ipDimension)
    : m_dim(0), m_headOfChain(0), m_isOwner(true)
  {
    if (ipDimension > 1) {
      m_headOfChain = ParticleBase::createInteractionPoint(particle, forceFitAll, ipDimension);
    } else {
      //use the B,D or whatever as head
      m_headOfChain = ParticleBase::createParticle(particle, 0, forceFitAll);
    }
    m_headOfChain->updateIndex(m_dim);

    m_cand = locate(particle);
    initConstraintList();
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
    std::sort(m_constraintlist.begin(), m_constraintlist.end());
  }

  ErrCode DecayChain::initialize(FitParams* par)
  {
    ErrCode status;
    par->resetStateVector();
    status |= m_headOfChain->initMotherlessParticle(par);
    par->resetCovariance();
    status |= m_headOfChain->initCovariance(par);
    return status;
  }

  ErrCode DecayChain::filter(FitParams& par, bool firstpass)
  {
    ErrCode status;
    par.resetCovariance();
    if (firstpass || !par.testCovariance()) {
      status |= m_headOfChain->initCovariance(&par);
    }

    m_chi2SumConstraints = 0;
    par.resetChiSquare();
    for (auto constraint : m_constraintlist) {

      status |= constraint.filter(&par);

      m_chi2SumConstraints += constraint.getChi2();
    }
    return status;
  }

  double DecayChain::chiSquare(const FitParams* par) const
  {
    return m_headOfChain->chiSquare(par);
  }

  const ParticleBase* DecayChain::locate(Belle2::Particle* particle) const
  {
    //FIXME this can be done easier
    const ParticleBase* rc(0);
    const auto mapRow = m_particleMap.find(particle) ;

    if (mapRow == m_particleMap.end()) {
      //JFK: take head of chain and recursively find particle in it 2017-11-15
      rc = m_headOfChain->locate(particle);

      if (rc && rc->particle()) {
        const_cast<DecayChain*>(this)->m_particleMap[rc->particle()] = rc;
      }
    } else {
      //JFK: only used for "head of tree" 2017-11-15
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
