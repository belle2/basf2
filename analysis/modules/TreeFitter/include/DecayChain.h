/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DECAYCHAIN_H
#define DECAYCHAIN_H

//#include <map>

#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/MergedConstraint.h>

//#include <analysis/dataobjects/Particle.h>

namespace TreeFitter {

  class FitParams ;
  class ParticleBase ;

  class DecayChain {
  public:
    DecayChain() : m_mother(0) {}

    DecayChain(Particle* bc, bool forceFitAll = false)  ;
    ~DecayChain() ;

    int dim() const { return m_dim ; }

    void initConstraintList() ;
    ErrCode init(FitParams* par) ;
    //    ErrCode filter(FitParams* par, bool firstpass = true) ;
    ErrCode filter(FitParams& par, bool firstpass = true) ;//FT: passing as reference in order to save it
    double chiSquare(const FitParams* par) const ;

    ParticleBase* mother() { return m_mother ; }
    const ParticleBase* cand() { return m_cand ; }
    const ParticleBase* mother() const { return m_mother ; }
    const ParticleBase* locate(Particle* bc) const ;

    int index(Particle* bc) const ;
    int posIndex(Particle* bc) const ;
    int momIndex(Particle* bc) const ;
    int tauIndex(Particle* bc) const ;
    void setOwner(bool b) { m_isOwner = b ;}
    int momIndex() const ;

    void printConstraints(std::ostream& os = std::cout) const ;

  private:
    int m_dim ;
    ParticleBase* m_mother ;     // head of decay tree
    const ParticleBase* m_cand ; // fit candidate (not same to mother in case of bs/be constraint)
    ParticleBase::constraintlist m_constraintlist ;
    std::vector<Constraint*> m_mergedconstraintlist ;
    MergedConstraint mergedconstraint ;
    typedef std::map<Particle*, const ParticleBase*> ParticleMap ;
    ParticleMap m_particleMap ;
    bool m_isOwner ;
  } ;

}



#endif //DECAYCHAIN_H
