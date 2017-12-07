/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECOPHOTON_H
#define RECOPHOTON_H

#include <analysis/VertexFitting/TreeFitter/RecoParticle.h>


namespace TreeFitter {
  /** */
  class RecoPhoton : public RecoParticle {
  public:
    /** constructor */
    RecoPhoton(Belle2::Particle* bc, const ParticleBase* mother) ;
    /** destructor */
    virtual ~RecoPhoton();

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
    /** project photon consztraint */
    ErrCode projectRecoConstraintOld(const FitParams& fitparams, Projection& p) const;



    /** */
    virtual int dimM() const { return m_useEnergy ? 3 : 2 ; }
    /** */
    virtual int type()     const { return kRecoPhoton ; }

    /** */
    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::photon, depth, dimM())) ;
    }

    /** */
    static bool useEnergy(Belle2::Particle& cand) ;

  private:
    /** */
    bool m_init ;
    /** */
    bool m_useEnergy ;

    /** constains params of this constraint */
    Eigen::Matrix<double, 1, 4> m_params;

    /** constains covaraince of this constraint */
    Eigen::Matrix<double, 4, 4> m_covariance;
  };

}
#endif //RECOPHOTON_H
