/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini,Jo-Frederik Krohn                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

namespace TreeFitter {

  /**  */
  class RecoComposite : public ParticleBase {

  public:
    /**  */
    RecoComposite(Belle2::Particle* bc, const ParticleBase* mother) ;

    /**  */
    virtual ~RecoComposite() {};

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);

    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** update chaed params */
    void updateParams();

    /** project this particle constraint  */
    ErrCode projectRecoComposite(const FitParams& fitparams, Projection& p) const;

    /** dimension of the measurement vector */
    int dimMeas() const        { return m_hasEnergy ? 7 : 6 ; }

    /** get dimension of cosntraint */
    virtual int dim() const { return m_hasEnergy ? 8 : 7 ; }// (x,y,z,t,px,py,pz,(E))

    /** get dimension  of measurement*/
    int dimM() const        { return m_hasEnergy ? 7 : 6 ; }

    /** project this constraint */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const ;

    /** get ype */
    virtual int type() const { return kRecoComposite ; }

    /** get position index in statevectof x,y,z,tau,px,py,pz */
    virtual int posIndex() const { return index()   ; }

    /** get tau (lifetime) index in statevector */
    virtual int tauIndex() const { return index() + 3 ; }

    /** get momentum index in statevector */
    virtual int momIndex() const { return index() + 4 ; }

    /** return of this constraint/particle has an energy component */
    virtual bool hasEnergy() const { return m_hasEnergy ; }

    /** return true FIXME */
    virtual bool hasPosition() const { return true ; }

    /** get chi2 */
    virtual double chiSquare(const FitParams* fitparams) const ;

    /** add this to list */
    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::composite, depth, dimM())) ;
      alist.push_back(Constraint(this, Constraint::geometric, depth, 3)) ;
    }

  protected:

    /** column vector to store the measurement */
    EigenTypes::ColVector m_params;

    /** only lower triangle filled! */
    EigenTypes::MatrixXd  m_covariance;

    /** flag  */
    bool m_hasEnergy;
  };
}
