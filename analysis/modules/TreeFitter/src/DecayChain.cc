/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Build decaytree structure allowing to index particles and handle the filtering of constraints across the tree

#include <algorithm>
#include <framework/logging/Logger.h>

#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/DecayChain.h>

//#define THEOLDWAY 1

namespace TreeFitter {
  extern int vtxverbose ;

  void DecayChain::printConstraints(std::ostream& os) const
  {
    os << "Constraints in decay tree: " << std::endl ;
    for (ParticleBase::constraintlist::const_iterator
         it = m_constraintlist.begin() ;
         it != m_constraintlist.end(); ++it)
      it->print(os) ;
  }

  DecayChain::DecayChain(Particle* particle, bool forceFitAll)
    : m_dim(0), m_mother(0), m_isOwner(true)
  {
    m_mother = ParticleBase::createParticle(particle, 0, forceFitAll);
    m_mother->updateIndex(m_dim);
    m_cand   = locate(particle);
    initConstraintList() ;

    if (vtxverbose >= 2) {
      std::cout << "In DecayChain constructor: m_dim = " << m_dim << std::endl ;
      printConstraints() ;
    }
  }

  DecayChain::~DecayChain()
  {
    if (m_mother && m_isOwner) delete m_mother ;
  }

  void DecayChain::initConstraintList()
  {
    m_constraintlist.clear() ;
    m_mother->addToConstraintList(m_constraintlist, 0) ;
    // the order of the constraints is a rather delicate thing
    std::sort(m_constraintlist.begin(), m_constraintlist.end()) ;

    // merge all non-linear constraints
    m_mergedconstraintlist.clear() ;
    mergedconstraint = MergedConstraint() ;
    for (ParticleBase::constraintlist::iterator it =  m_constraintlist.begin() ;
         it != m_constraintlist.end(); ++it) {
      if (it->isLinear()) m_mergedconstraintlist.push_back(&(*it)) ;
      else  mergedconstraint.push_back(&(*it)) ;
    }

    if (mergedconstraint.dim() > 0)
      m_mergedconstraintlist.push_back(&mergedconstraint) ;
  }

  ErrCode DecayChain::init(FitParams* par)
  {
    ErrCode status ; //seems like I should initialise it now if I want to use bitwise OR (|=) later

    // set everything to 0
    par->resetPar() ;
    status |= m_mother->initPar1(par) ;

    // let the mother do it
    par->resetCov() ;
    status |= m_mother->initCov(par) ;

    if (vtxverbose >= 2) std::cout << "status after init: " << status << std::endl ;

    return status ;
  }

  ErrCode DecayChain::filter(FitParams& par, bool firstpass)
  {
    ErrCode status ;
    par.resetCov(1000) ;
    if (firstpass || !par.testCov()) status |= m_mother->initCov(&par);
    //
    if (vtxverbose >= 3 || (vtxverbose >= 2 && firstpass)) {
      std::cout << "DecayChain::filter, after initialization: " << std::endl;
      m_mother->print(&par);
    }

    //FT:
    //Make a reference to the smoothed parameters from the previous iteration
    //By using those (fixed) parameters when projecting to the next step instead of the
    //running parameters (which update after each constraint is applied) the fit
    //is more stable and less reliant on constraint ordering
    //
    //This change was first introduced when porting the TreeFitter from BaBar to LHC

    FitParams reference = par;

    //#ifdef THEOLDWAY
    if (m_mergedconstraintlist.empty()) {//FT:Merged constraints actually crash this so the old method is used; go back and find out why
      for (ParticleBase::constraintlist::const_iterator it = m_constraintlist.begin() ;
           it != m_constraintlist.end(); ++it) {
        //  status |= it->filter(&par) ;
        status |= it->filter(&par, &reference) ;
        if (vtxverbose >= 2 && status.failure()) {
          std::cout << "status is failure after parsing constraint: " ;
          it->print() ;
        }
      }
      //#else
    } else {
      for (std::vector<Constraint*>::const_iterator it = m_mergedconstraintlist.begin() ;
           it != m_mergedconstraintlist.end(); ++it) {
        status |= (*it)->filter(&par) ;
        //  status |= (*it)->filter(&par,&reference) ; //FT: temporarily disabled because of crashing
        if (vtxverbose >= 2 && status.failure()) {
          std::cout << "status is failure after parsing constraint: " ;
          (*it)->print() ;
        }
      }
    }
    //#endif


    if (vtxverbose >= 3) std::cout << "DecayChain::filter: status = " << status << std::endl ;
    return status ;
  }

  double DecayChain::chiSquare(const FitParams* par) const
  {
    return m_mother->chiSquare(par) ;
  }

  const ParticleBase* DecayChain::locate(Particle* particle) const   //FT: This needs investigation
  {
    const ParticleBase* rc(0);
    // return _mother->locate(bc) ;
    B2DEBUG(80, "DecayChain::locate: Trying to locate " << particle->getName() << " in a " << m_particleMap.size() << " sized map.");
    ParticleMap::const_iterator it = m_particleMap.find(particle) ;
    if (it == m_particleMap.end()) {
      rc = m_mother->locate(particle) ;
      // used to add every candidate here, but something goes wrong
      // somewhere. now only cache pointers that we internally reference.
      if (rc && rc->particle())
        const_cast<DecayChain*>(this)->m_particleMap[rc->particle()] = rc ;
    } else {
      rc = it->second ;
    }
    return rc ;
  }

  int DecayChain::index(Particle* particle) const
  {
    int rc = -1 ;
    const ParticleBase* base = locate(particle);
    if (base) rc = base->index();
    return rc;
  }

  int DecayChain::posIndex(Particle* particle) const
  {
    int rc = -1 ;
    const ParticleBase* base = locate(particle);
    if (base) rc = base->posIndex();
    return rc;
  }

  int DecayChain::momIndex(Particle* particle) const
  {
    int rc = -1 ;
    const ParticleBase* base = locate(particle);
    if (base) rc = base->momIndex() ;
    return rc ;
  }

  int DecayChain::tauIndex(Particle* particle) const
  {
    int rc = -1 ;
    const ParticleBase* base = locate(particle) ;
    if (base) rc = base->tauIndex() ;
    return rc ;
  }
}
