/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <vector>

namespace TreeFitter {

  /** another unneccessary layer of abstraction */
  class InternalParticle : public ParticleBase {

  public:

    /** constructor */
    InternalParticle(Belle2::Particle* particle,
                     const ParticleBase* mother,
                     bool forceFitAll) ;

    /** destructor */
    virtual ~InternalParticle() {};

    /** init covariance */
    virtual ErrCode initCovariance(FitParams*) const;

    /** project kinematical constraint */
    ErrCode projectKineConstraint(const FitParams&, Projection&) const;

    /** enforce conservation of momentum sum*/
    virtual void forceP4Sum(FitParams&) const;

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);

    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** find out which constraint it is and project */
    ErrCode projectConstraint(const Constraint::Type type, const FitParams& fitparams, Projection& p) const;

    /** space reserved in fit params, if has mother then it has tau */
    virtual int dim() const { return mother() ? 8 : 7 ;}

    /**  type */
    virtual int type() const { return kInternalParticle ; }

    /**   position index in fit params*/
    virtual int posIndex() const { return index()   ; }

    /** tau index in fit params only if it has a mother */
    virtual int tauIndex() const { return mother() ? index() + 3 : -1; }

    /** momentum index in fit params depending on whether it has a mother  */
    virtual int momIndex() const { return mother() ? index() + 4 : index() + 3 ; }

    /** has energy in fitparams  */
    virtual bool hasEnergy() const { return true ; }

    /** has position index  */
    virtual bool hasPosition() const { return true ; }

    /** name  */
    virtual std::string parname(int index) const ;

    /** add to constraint list  */
    virtual void addToConstraintList(constraintlist& list, int depth) const ;

    /** set mass constraint flag */
    void setMassConstraint(bool b) { m_massconstraint = b ; }

    /** rotate in positive phi domain  */
    double phidomain(const double);

  protected:

    /** init momentum of *this and daughters */
    ErrCode initMomentum(FitParams* fitparams) const ;

  private:

    /** has mass cosntraint */
    bool m_massconstraint ;

    /** has lifetime constraint  */
    bool m_lifetimeconstraint ;

    /** is conversion  */
    bool m_isconversion ;
  } ;

}
