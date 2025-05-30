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

namespace TreeFitter {

  /** A class for composite particles, where the daughters must be ignored by the fitter */
  /** Currently used for bremsstrahlung correction and dummy invisibles */
  class Composite : public ParticleBase {

  public:
    /** constructor */
    Composite(Belle2::Particle* bc, const ParticleBase* mother, const ConstraintConfiguration& config,
              bool massconstraint = false);

    /** destructor */
    virtual ~Composite() {};

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** update changed params */
    void updateParams();

    /** project this particle constraint  */
    ErrCode projectComposite(const FitParams& fitparams, Projection& p) const;

    /** get dimension of constraint */
    virtual int dim() const override { return m_hasEnergy ? 8 : 7 ; }// (x,y,z,t,px,py,pz,(E))

    /** get dimension of measurement*/
    int dimM() const        { return m_hasEnergy ? 7 : 6 ; }

    /** project this constraint */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const override ;

    /** get type */
    virtual int type() const override { return kComposite ; }

    /** get position index in statevectof x,y,z,tau,px,py,pz */
    virtual int posIndex() const override { return index()   ; }

    /** get tau (lifetime) index in statevector */
    virtual int tauIndex() const override { return index() + 3 ; }

    /** get momentum index in statevector */
    virtual int momIndex() const override { return index() + 4 ; }

    /** return of this constraint/particle has an energy component */
    virtual bool hasEnergy() const override { return m_hasEnergy ; }

    /** return true FIXME */
    virtual bool hasPosition() const override { return true ; }

    /** add this to list */
    virtual void addToConstraintList(constraintlist& alist, int depth) const override
    {
      alist.push_back(Constraint(this, Constraint::composite, depth, dimM())) ;
      alist.push_back(Constraint(this, Constraint::geometric, depth, 3)) ;
      if (m_massconstraint) {
        alist.push_back(Constraint(this, Constraint::mass, depth, 1, 3));
      }
    }

  protected:

    /** column vector to store the measurement */
    Eigen::Matrix<double, 7, 1> m_params;

    /** only lower triangle filled! */
    Eigen::Matrix < double, -1, -1, 0, 7, 7 >  m_covariance;

    /** flag */
    bool m_hasEnergy;

    /** flag */
    bool m_massconstraint;
  };
}
