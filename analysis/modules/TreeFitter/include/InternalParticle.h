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
    ErrCode projectKineConstraintCopy(const FitParams&, Projection&) const;
    /** project lifetime constraint */
    ErrCode projectLifeTimeConstraintCopy(const FitParams&, Projection&) const;
    /** enforece conservation of momentum sum*/
    virtual void forceP4SumCopy(FitParams&) const;
    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);
    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** find out which constraint it is and project */
    ErrCode projectConstraintCopy(const Constraint::Type type, const FitParams& fitparams, Projection& p) const;

    virtual int dim() const { return mother() ? 8 : 7 ; }
    //    virtual void updateIndex(int& offset) ;

    virtual ErrCode initPar1(FitParams*) ;
    virtual ErrCode initPar2(FitParams*) ;
    virtual ErrCode initCov(FitParams*) const ;
    virtual int type() const { return kInternalParticle ; }
    //    virtual void print(const FitParams*) const ;
    //    virtual const ParticleBase* locate(Particle* particle) const ;

    // parameter definition
    virtual int posIndex() const { return index()   ; }
    virtual int tauIndex() const { return mother() ? index() + 3 : -1 ; }
    virtual int momIndex() const { return mother() ? index() + 4 : index() + 3 ; }
    virtual bool hasEnergy() const { return true ; }
    virtual bool hasPosition() const { return true ; }
    virtual std::string parname(int index) const ;

    // constraints
    ErrCode projectKineConstraint(const FitParams&, Projection&) const ;
    ErrCode projectLifeTimeConstraint(const FitParams&, Projection&) const ;
    ErrCode projectConversionConstraint(const FitParams&, Projection& p) const ;
    virtual ErrCode projectConstraint(const Constraint::Type type,
                                      const FitParams& fitparams, Projection& p) const ;
    virtual void forceP4Sum(FitParams&) const ;

    virtual void addToConstraintList(constraintlist& list, int depth) const ;

    //    bool swapMotherDaughter(FitParams* fitparams, const ParticleBase* newmother) ;
    void setMassConstraint(bool b) { m_massconstraint = b ; }

    double phidomain(const double);

    //should be moved back to helixutils
    double helixPoca(const CLHEP::HepVector&, const CLHEP::HepVector&,
                     double&, double&, TVector3&, bool);
  protected:
    ErrCode initMom(FitParams* fitparams) const ;
    /** init momentum of *this and daughters */
    ErrCode initMomentum(FitParams* fitparams) const ;
    //    virtual void addToDaughterList(daucontainer& list) ; //moved to ParticleBase
  private:
    //    daucontainer m_daughters ;
    bool m_massconstraint ;
    bool m_lifetimeconstraint ;
    bool m_isconversion ;
  } ;

}


#endif //INTERNALPARTICLE_H
