/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef INTERNALPARTICLE_H
#define INTERNALPARTICLE_H

#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <vector>

//using namespace CLHEP;

namespace TreeFitter {

  class InternalParticle : public ParticleBase {
  public:
    InternalParticle(Belle2::Particle* particle, const ParticleBase* mother,
                     bool forceFitAll) ;
    virtual ~InternalParticle() ;

    /** init covariance */
    virtual ErrCode initCovariance(FitParams*) const;
    /** project kinematical constraint */
    ErrCode projectKineConstraint(const FitParams&, Projection&) const;
    /** project lifetime constraint */
    ErrCode projectLifeTimeConstraint(const FitParams&, Projection&) const;
    /** enforece conservation of momentum sum*/
    virtual void forceP4Sum(FitParams&) const;
    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);
    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** find out which constraint it is and project */
    ErrCode projectConstraint(const Constraint::Type type, const FitParams& fitparams, Projection& p) const;

    /**  */
    virtual int dim() const { return mother() ? 8 : 7 ; }

    /**  */
    virtual int type() const { return kInternalParticle ; }

    /**  */
    virtual int posIndex() const { return index()   ; }
    /**  */
    virtual int tauIndex() const { return mother() ? index() + 3 : -1 ; }
    /**  */
    virtual int momIndex() const { return mother() ? index() + 4 : index() + 3 ; }
    /**  */
    virtual bool hasEnergy() const { return true ; }
    /**  */
    virtual bool hasPosition() const { return true ; }
    /**  */
    virtual std::string parname(int index) const ;

    /**  */
    virtual void addToConstraintList(constraintlist& list, int depth) const ;

    /**  */
    void setMassConstraint(bool b) { m_massconstraint = b ; }

    /**  */
    double phidomain(const double);

    //should be moved back to helixutils
    /**  */
    double helixPoca(const CLHEP::HepVector&, const CLHEP::HepVector&,
                     double&, double&, TVector3&, bool);
  protected:
    /** init momentum of *this and daughters */
    ErrCode initMomentum(FitParams* fitparams) const ;
    //    virtual void addToDaughterList(daucontainer& list) ; //moved to ParticleBase
  private:
    /**  */
    bool m_massconstraint ;
    /**  */
    bool m_lifetimeconstraint ;
    /**  */
    bool m_isconversion ;
  } ;

}


#endif //INTERNALPARTICLE_H
