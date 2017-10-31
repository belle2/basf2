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
#include <analysis/modules/TreeFitter/InteractionPoint.h>//FIXME particle base is maybe better?
#include <analysis/modules/TreeFitter/DecayChain.h>


namespace TreeFitter {
  extern int vtxverbose;

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

    m_cand   = locate(particle);
    initConstraintList();
  }

  DecayChain::~DecayChain()
  {
    if (m_headOfChain && m_isOwner) delete m_headOfChain ;
  }

  void DecayChain::initConstraintList()
  {
    m_constraintlist.clear();

    m_headOfChain->addToConstraintList(m_constraintlist, 0);
    std::sort(m_constraintlist.begin(), m_constraintlist.end());
    mergedconstraint = MergedConstraint();
    for (ParticleBase::constraintlist::iterator it = m_constraintlist.begin(); it != m_constraintlist.end(); ++it) {

      B2DEBUG(30, "initCostraint:" << (*it).name());
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

  ErrCode DecayChain::init(FitParams* par)
  {
    ErrCode status; //seems like I should initialise it now if I want to use bitwise OR (|=) later

    // set everything to 0
    par->resetPar();
    status |= m_headOfChain->initPar1(par);

    // let the mother do it
    par->resetCov();
    status |= m_headOfChain->initCov(par);

    return status;
  }

  ErrCode DecayChain::filter(FitParams& par, bool firstpass)
  {
    ErrCode status;
    par.resetCov(1000);

    if (firstpass || !par.testCov()) {
      status |= m_headOfChain->initCov(&par);
    }

    //FT:
    //Make a reference to the smoothed parameters from the previous iteration
    //By using those (fixed) parameters when projecting to the next step instead of the
    //running parameters (which update after each constraint is applied) the fit
    //is more stable and less reliant on constraint ordering

    FitParams reference = par;

    if (m_mergedconstraintlist.empty()) {//FT:Merged constraints actually crash this so the old method is used; go back and find out why

      //FT: as far as I can tell, this is never empty?
      // JFK: CHANGEED TO NON CONST Mon 04 Sep 2017 04:58:05 AM CEST
      for (ParticleBase::constraintlist::iterator it = m_constraintlist.begin();
           it != m_constraintlist.end(); ++it) {

        //FIXME can we delete this?
        status |= it->filter(&par) ;
        std::cout << "filtering with reference to previous contrs :"   << std::endl;

        //status |= it->filter(&par, &reference);

        if (vtxverbose >= 2 && status.failure()) {
          std::cout << "status is failure after parsing constraint: ";

          it->print();
        }
      }
    } else {
      // JFK: CHANGED THIS TO NON CONST Mon 04 Sep 2017 04:55:24 AM CEST
      m_chi2SumConstraints = 0;
      // JFK: FIXME Mon 04 Sep 2017 06:54:12 AM CEST
      par.resetChiSquare();

      for (std::vector<Constraint*>::iterator it = m_mergedconstraintlist.begin();
           it != m_mergedconstraintlist.end(); ++it) {
        // JFK: changed this FIXME Mon 04 Sep 2017 02:54:50 AM CEST

        status |= (*it)->filter(&par);

        m_chi2SumConstraints += (*it)->getChi2();
      }
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
    B2DEBUG(80, "DecayChain::locate: Trying to locate " << particle->getName() << " in a " << m_particleMap.size() << " sized map.");
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
