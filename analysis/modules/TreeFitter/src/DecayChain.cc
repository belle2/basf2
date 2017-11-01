/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Build decaytree structure allowing to index particles and handle the filtering of constraints across the tree

#include <algorithm>
#include <framework/logging/Logger.h>

#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/InteractionPoint.h>
#include <analysis/modules/TreeFitter/DecayChain.h>


namespace TreeFitter {
  extern int vtxverbose;

  DecayChain::DecayChain(Belle2::Particle* particle, bool forceFitAll, const int ipDimension)
    : m_dim(0), m_headOfChain(0), m_isOwner(true)
  {
    if (ipDimension > 1) {
      B2DEBUG(30, "--DecayChain::constructor with beam contr");
      m_headOfChain = ParticleBase::createInteractionPoint(particle, forceFitAll, ipDimension);
    } else {
      //use the B,D or whatever as head
      B2DEBUG(30, "--DecayChain::constructor without beam contr");
      m_headOfChain = ParticleBase::createParticle(particle, 0, forceFitAll);
    }
    m_headOfChain->updateIndex(m_dim);

    m_cand   = locate(particle);
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
    mergedconstraint = MergedConstraint();
    for (ParticleBase::constraintlist::iterator it = m_constraintlist.begin(); it != m_constraintlist.end(); ++it) {

      B2DEBUG(30, "--DecayChain::initConstraintList name:" << (*it).name());
      if (true) {
        m_mergedconstraintlist.push_back(&(*it)); //FT: never filter constraints together
      } else {
        mergedconstraint.push_back(&(*it));
      }
    }

    if (mergedconstraint.dim() > 0) {
      m_mergedconstraintlist.push_back(&mergedconstraint);
    }
  }

  ErrCode DecayChain::initialize(FitParams* par)
  {
    B2DEBUG(81, "--DecayChain::initialize: head:" << m_headOfChain->name());
    ErrCode status; //seems like I should initialise it now if I want to use bitwise OR (|=) later
    par->resetStateVector();
    status |= m_headOfChain->initMotherlessParticle(par);
    par->resetCovariance();
    status |= m_headOfChain->initCovariance(par);
    return status;
  }

  ErrCode DecayChain::filterCopy(FitParams& par, bool firstpass)
  {
    B2DEBUG(81, "--Filtering DecayChain ");
    ErrCode status;
    par.resetCovariance();
    if (firstpass || !par.testCovariance()) {
      B2DEBUG(81, "--Filtering DecayChain: Init Covariance");
      status |= m_headOfChain->initCovariance(&par);
    }

    //JFK: I removed the other thing since we didnt use it 2017-09-27
    m_chi2SumConstraints = 0;
    par.resetChiSquare();
    for (std::vector<Constraint*>::iterator it = m_mergedconstraintlist.begin();
         it != m_mergedconstraintlist.end(); ++it) {

      B2DEBUG(81, "--Filtering DecayChain: Current Constraint:" << (*it)->name());

      status |= (*it)->filterCopy(&par);

      m_chi2SumConstraints += (*it)->getChi2();
    }
    return status;
  }

  double DecayChain::chiSquare(const FitParams* par) const
  {
    return m_headOfChain->chiSquare(par);
  }

  const ParticleBase* DecayChain::locate(Belle2::Particle* particle) const   //FT: This needs investigation
  {
    const ParticleBase* rc(0);
    B2DEBUG(81, "--DecayChain::locate: Trying to locate " << particle->getName() << " in a " << m_particleMap.size() << " sized map.");
    ParticleMap::const_iterator it = m_particleMap.find(particle) ;

    if (it == m_particleMap.end()) {
      rc = m_headOfChain->locate(particle);

      if (rc && rc->particle()) {
        const_cast<DecayChain*>(this)->m_particleMap[rc->particle()] = rc;
      }
    } else {
      rc = it->second;
    }
    return rc;
  }

  int DecayChain::index(Belle2::Particle* particle) const
  {
    int rc = -1 ;
    const ParticleBase* base = locate(particle);
    if (base) {
      rc = base->index();
    }
    return rc;
  }

  int DecayChain::posIndex(Belle2::Particle* particle) const
  {
    int rc = -1;
    const ParticleBase* base = locate(particle);
    if (base) {
      rc = base->posIndex();
    }
    return rc;
  }

  int DecayChain::momIndex(Belle2::Particle* particle) const
  {
    int rc = -1;
    const ParticleBase* base = locate(particle);
    if (base) {
      rc = base->momIndex();
    }
    return rc;
  }

  int DecayChain::tauIndex(Belle2::Particle* particle) const
  {
    int rc = -1;
    const ParticleBase* base = locate(particle);
    if (base) {
      rc = base->tauIndex();
    }
    return rc;
  }
}
