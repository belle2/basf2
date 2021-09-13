/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>

namespace TreeFitter {

  /** another unnecessary layer of abstraction */
  class InternalParticle : public ParticleBase {

  public:

    /** constructor */
    InternalParticle(Belle2::Particle* particle,
                     const ParticleBase* mother,
                     const ConstraintConfiguration& config,
                     bool forceFitAll
                    ) ;

    /** destructor */
    virtual ~InternalParticle() {};

    /** init covariance */
    virtual ErrCode initCovariance(FitParams&) const override;

    /** project kinematical constraint */
    ErrCode projectKineConstraint(const FitParams&, Projection&) const;

    /** enforce conservation of momentum sum*/
    virtual void forceP4Sum(FitParams&) const override;

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** find out which constraint it is and project */
    ErrCode projectConstraint(const Constraint::Type type, const FitParams& fitparams, Projection& p) const override;

    /** space reserved in fit params, if has mother then it has tau */
    virtual int dim() const override;

    /**  type */
    virtual int type() const override { return kInternalParticle ; }

    /**   position index in fit params*/
    virtual int posIndex() const override;

    /** tau index in fit params only if it has a mother */
    virtual int tauIndex() const override;

    /** momentum index in fit params depending on whether it has a mother  */
    virtual int momIndex() const override;

    /** has energy in fitparams  */
    virtual bool hasEnergy() const override { return true ; }

    /** has position index  */
    virtual bool hasPosition() const override;

    /** name  */
    virtual std::string parname(int index) const override ;

    /** add to constraint list  */
    virtual void addToConstraintList(constraintlist& list, int depth) const override ;

    /** set mass constraint flag */
    void setMassConstraint(bool b) { m_massconstraint = b ; }

    /** rotate in positive phi domain  */
    double phidomain(const double);

  protected:

    /** init momentum of *this and daughters */
    ErrCode initMomentum(FitParams& fitparams) const ;

  private:

    /** compare transverse track momentum*/
    bool static compTrkTransverseMomentum(const RecoTrack* lhs, const RecoTrack* rhs);

    /** has mass constraint */
    bool m_massconstraint ;

    /** shares vertex with mother, that means decay vertex = productionvertex */
    bool m_shares_vertex_with_mother;

    /** use a geo metric constraint */
    bool m_geo_constraint;

    /** has lifetime constraint  */
    bool m_lifetimeconstraint ;

    /** is conversion  */
    bool m_isconversion ;

    /** automatically figure out if mother and particle vertex should be the same
     * and also add geometric constraints */
    bool m_automatic_vertex_constraining;
  } ;

}
