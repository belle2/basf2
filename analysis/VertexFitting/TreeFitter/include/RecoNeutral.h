/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/RecoParticle.h>

#include <Eigen/Core>

namespace TreeFitter {

  /** representation of the neutral particle constraint */
  class RecoNeutral : public RecoParticle {

  public:
    /** constructor */
    RecoNeutral(Belle2::Particle* bc, const ParticleBase* mother) ;

    /** destructor */
    virtual ~RecoNeutral() {};

    /** init particle with mother */
    virtual ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init particle without mother */
    virtual ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** init covariance */
    ErrCode initCovariance(FitParams& fitparams) const override;

    /** update or init params */
    ErrCode initParams();

    /** project neutral particle constraint */
    ErrCode projectRecoConstraint(const FitParams& fitparams, Projection& p) const override;

    /** sets the size of the corresponding residual projection */
    virtual int dimM() const override { return dim(); }

    /** how should the energy be calculated ? from momentum or from E ?  */
    virtual bool hasEnergy() const override { return false; }

    /**set the size of the particle in the statevector */
    virtual int dim() const override { return m_dim; }

    /** type */
    virtual int type() const override { return kRecoNeutral ; }

    /** add to list */
    virtual void addToConstraintList(constraintlist& alist, int depth) const override
    {
      alist.push_back(Constraint(this, Constraint::neutralHadron, depth, dimM())) ;
    }

  private:

    /** dimension of residuals and 'width' of H */
    const int m_dim;

    /** was initialized */
    bool m_init ;

    /** constrains measured params (x_c, y_c, z_c, E_c) */
    Eigen::Matrix<double, 1, 4> m_clusterPars;

    /** covariance (x_c,y_c,z_c,E_c) of measured pars */
    Eigen::Matrix<double, 4, 4> m_covariance;

    /** index with the highest momentum. We have to make sure this does not change during the fit.  */
    int m_i1;
    /** random other index */
    int m_i2;
    /** another random index */
    int m_i3;

    /** scale the momentum / energy by this correction factor */
    const float m_momentumScalingFactor;

    /** invariant mass */
    const double m_mass;

    /** (mdst) source of particle */
    const Belle2::Particle::EParticleSourceObject m_particleSource;
  };

}
