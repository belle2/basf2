/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DECAYCHAIN_H
#define DECAYCHAIN_H

#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/MergedConstraint.h>

namespace TreeFitter {

  class FitParams ;
  class ParticleBase ;

  /** this class does a lot of stuff:
  Build decaytree structure allowing to index particles and handle the filtering of constraints across the tree
  */
  class DecayChain {

  public:

    /**  empty constructor  */
    DecayChain() : m_dim(0), m_headOfChain(0), m_cand(0), m_isOwner(true) {} //Default constructor (actually never used)
    /**  constructor   */
    DecayChain(Belle2::Particle* bc, bool forceFitAll = false, const int ipDimension = 0);

    /**  destructor   */
    ~DecayChain();

    /** initalize the chain */
    ErrCode initialize(FitParams* par);

    /** filter down the chain */
    ErrCode filterCopy(FitParams& par, bool firstpass);

    /** get dimension   */
    int dim() const { return m_dim;}

    /** init contraintlist   */
    void initConstraintList();

    /**    */
    double chiSquare(const FitParams* par) const;

    /** get mother  */
    ParticleBase* mother() { return m_headOfChain ; }

    /** get candidate    */
    const ParticleBase* cand() { return m_cand ; }

    /** get mother    */
    const ParticleBase* mother() const { return m_headOfChain ; }

    /** convert Belle2::particle into particle base(fitter base particle)    */
    const ParticleBase* locate(Belle2::Particle* bc) const ;

    /**    */
    int index(Belle2::Particle* bc) const ;

    /**    */
    int posIndex(Belle2::Particle* bc) const ;

    /**    */
    int momIndex(Belle2::Particle* bc) const ;

    /**    */
    int tauIndex(Belle2::Particle* bc) const ;

    /**    */
    void setOwner(bool b) { m_isOwner = b ;}

    /**    */
    int momIndex() const ;

    double getChi2Sum() const {return m_chi2SumConstraints; }

    double getChainsChi2(const FitParams* par)const {return m_headOfChain->chiSquare(par);}

  private:

    /** chi2 sum for the constraints has to be devided by the number of constraints in the getter */
    double m_chi2SumConstraints;

    /**    */
    int m_dim ;

    /**    */
    ParticleBase* m_headOfChain ;     // head of decay tree

    /**    */
    const ParticleBase* m_cand ; // fit candidate (not same to mother in case of bs/be constraint)

    /**    */
    ParticleBase::constraintlist m_constraintlist ;

    /**    */
    std::vector<Constraint*> m_mergedconstraintlist ;

    /**    */
    MergedConstraint mergedconstraint ;

    /**    */
    typedef std::map<Belle2::Particle*, const ParticleBase*> ParticleMap ;

    /**    */
    ParticleMap m_particleMap ;

    /**    */
    bool m_isOwner ;

  };

}



#endif //DECAYCHAIN_H
