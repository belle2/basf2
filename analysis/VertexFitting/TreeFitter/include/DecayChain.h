/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/MergedConstraint.h>

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
    DecayChain(Belle2::Particle* bc,
               bool forceFitAll = false,
               const bool ipConstraint = false,
               const bool customOrigin = false,
               const std::vector<double> customOriginVertex = {0, 0, 0},
               const std::vector<double> customOriginCovariance = {0, 0, 0, 0, 0, 0, 0, 0, 0,}
              );

    /**  destructor   */
    ~DecayChain();

    /** initalize the chain */
    ErrCode initialize(FitParams* par);

    /** filter down the chain */
    ErrCode filter(FitParams& par, bool firstpass);

    /** get dimension   */
    int dim() const { return m_dim;}

    /** init contraintlist   */
    void initConstraintList();

    /** get the chi2 for the head of the chain */
    double chiSquare(const FitParams* par) const;

    /** get mother */
    ParticleBase* mother() { return m_headOfChain ; }

    /** get candidate */
    const ParticleBase* cand() { return m_cand ; }

    /** get mother */
    const ParticleBase* mother() const { return m_headOfChain ; }

    /** convert Belle2::particle into particle base(fitter base particle)    */
    const ParticleBase* locate(Belle2::Particle* bc) const ;

    /** get the particle index */
    int index(Belle2::Particle* bc) const ;

    /**  get the vertex index of the particle in state vector */
    int posIndex(Belle2::Particle* bc) const ;

    /** get momentum index of the particle in the state vector*/
    int momIndex(Belle2::Particle* bc) const ;

    /** get tau (i.e. decay length) index of the particle in the state vector */
    int tauIndex(Belle2::Particle* bc) const ;

    /** !NOT IMPLEMENTED   */
    int momIndex() const ;

    /** get chi2 sum for the constraints */
    double getChi2Sum() const {return m_chi2SumConstraints; }

    /** !DUPLICATED FUNCTION get the chi2 for the head of the chain */
    double getChainsChi2(const FitParams* par)const {return m_headOfChain->chiSquare(par);}

  private:

    /** chi2 sum for the constraints has to be devided by the number of constraints in the getter */
    mutable double m_chi2SumConstraints;

    mutable int m_dim ; /**< the dimension of constraint */

    mutable ParticleBase* m_headOfChain ; /**< head of decay tree*/

    const ParticleBase* m_cand ; /**< fit candidate (not same to mother in case of bs/be constraint) */

    /** list of constraints */
    ParticleBase::constraintlist m_constraintlist ;

    ///** merged constraints */
    //std::vector<Constraint*> m_mergedconstraintlist ;

    ///**    */
    //MergedConstraint mergedconstraint ;

    /** typedef for a map of a particle to a TreeFitter::ParticleBase */
    typedef std::map<Belle2::Particle*, const ParticleBase*> ParticleMap ;

    /** the map from Belle2::Particles to TreeFitter::ParticleBase */
    ParticleMap m_particleMap ;

    /** internal class member to check if we own the chain */
    const bool m_isOwner ;

  };

}
