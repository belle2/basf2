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

namespace TreeFitter {

  /** representation of the Klong constraint */
  class RecoKlong : public RecoParticle {

  public:
    /** constructor */
    RecoKlong(Belle2::Particle* bc, const ParticleBase* mother) ;

    /** destructor */
    virtual ~RecoKlong() {};

    /** init particle with mother */
    virtual ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init particle without mother */
    virtual ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** init covariance */
    ErrCode initCovariance(FitParams& fitparams) const override;

    /** update or init params */
    ErrCode initParams();

    /** project klong constraint */
    ErrCode projectRecoConstraint(const FitParams& fitparams, Projection& p) const override;

    /** sets the size of the corresponding residual projection */
    virtual int dimM() const override { return 3; }

    /** how should the energy be calculated ? from momentum or from E ?  */
    virtual bool hasEnergy() const override { return true; }

    /**set the size of the particle in the statevector */
    virtual int dim() const override { return 4; }

    /** type */
    virtual int type()     const override { return kRecoKlong ; }

    /** add to list */
    virtual void addToConstraintList(constraintlist& alist, int depth) const override
    {
      alist.push_back(Constraint(this, Constraint::klong, depth, dimM())) ;
    }

    /** has energy in fit params? */
    static bool useEnergy(Belle2::Particle& cand) ;

  private:

    /** dimension of residuals and 'width' of H */
    const int m_dim;

    /** was initialized* */
    bool m_init ;

    /** has energy ins statevector */
    bool m_useEnergy ;

    /** constrains measured params (x_c, y_c, z_c, E_c) */
    Eigen::Matrix<double, 1, 4> m_clusterPars;

    /** covariance (x_c,y_c,z_c,E_c) of measured pars */
    Eigen::Matrix<double, 4, 4> m_covariance;

    /** index with the highest momentum. We have to make sure this does not change during the fit.  */
    int m_i1;
    /** random index */
    int m_i2;
    /**  another random index */
    int m_i3;
  };

}
