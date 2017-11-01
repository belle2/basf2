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

#include <analysis/modules/TreeFitter/RecoParticle.h>


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
    ErrCode updateParams();
    /** project photon consztraint */
    ErrCode projectRecoConstraintCopy(const FitParams& fitparams, Projection& p) const;

    /** */
    virtual int dimM() const { return m_useEnergy ? 3 : 2 ; }
    /** */
    virtual ErrCode initPar2(FitParams*) ;
    /** */
    virtual ErrCode initCov(FitParams*) const ;
    /** */
    virtual int type()     const { return kRecoPhoton ; }
    /** */
    virtual ErrCode projectRecoConstraint(const FitParams&, Projection&) const ;
    /** */
    ErrCode updCache() ;

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
    /** */
    CLHEP::HepVector m_m ;
    /** */
    CLHEP::HepSymMatrix m_matrixV ;

    /** constains params of this constraint */
    Eigen::Matrix<double, 1, 4> m_params;

    /** constains covaraince of this constraint */
    Eigen::Matrix<double, 4, 4> m_covariance;
  };

}
#endif //RECOPHOTON_H
