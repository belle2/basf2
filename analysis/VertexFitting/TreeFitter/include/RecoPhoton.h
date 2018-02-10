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

#include <analysis/VertexFitting/TreeFitter/RecoParticle.h>

namespace TreeFitter {

  /** representation of the photon constraint */
  class RecoPhoton : public RecoParticle {

  public:
    /** constructor */
    RecoPhoton(Belle2::Particle* bc, const ParticleBase* mother) ;

    /** destructor */
    virtual ~RecoPhoton() {};

    /** init particle with mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);

    /** init particle without mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** init covariance */
    ErrCode initCovariance(FitParams* fitparams) const;

    /** update or init params */
    ErrCode initParams();

    /** project photon consztraint */
    ErrCode projectRecoConstraint(const FitParams& fitparams, Projection& p) const;

    /** sets the size of the corresponding residual projection */
    virtual int dimM() const { return dim(); }

    /** how should the energy be calculated ? from momentum or from E ?  */
    virtual bool hasEnergy() const { return false; }

    /**set the size of the particle in the statevector */
    virtual int dim() const { return m_dim; }

    /** type */
    virtual int type()     const { return kRecoPhoton ; }

    /** add to list */
    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::photon, depth, dimM())) ;
    }

    /**  has energy in fit params? */
    static bool useEnergy(Belle2::Particle& cand) ;

  private:

    /** dimension of residuals and 'width' of H */
    const int m_dim;

    /** was initialized* */
    bool m_init ;

    /** has energy ins statevector */
    bool m_useEnergy ;

    /** constains measured params (x_c, y_c, z_c, E_c) */
    Eigen::Matrix<double, 1, 4> m_clusterPars;

    /** covariance (x_c,y_c,z_c,E_c) of measured pars */
    Eigen::Matrix<double, 4, 4> m_covariance;
  };

}
