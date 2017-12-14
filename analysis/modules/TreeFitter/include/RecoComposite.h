/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECOCOMPOSITE_H
#define RECOCOMPOSITE_H

#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>

namespace TreeFitter {

  class RecoComposite : public ParticleBase {
  public:
    RecoComposite(Belle2::Particle* bc, const ParticleBase* mother) ;
    virtual ~RecoComposite() ;

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);
    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);
    /** update chaed params */
    void updateParams();
    /** project this particle constraint  */
    ErrCode projectRecoCompositeCopy(const FitParams& fitparams, Projection& p) const;
    /** dimension of the measurement vector */
    int dimMeas() const        { return m_hasEnergy ? 7 : 6 ; }


    // the number of parameters
    virtual int dim() const { return m_hasEnergy ? 8 : 7 ; }// (x,y,z,t,px,py,pz,(E))

    // the number of 'measurements'
    int dimM() const        { return m_hasEnergy ? 7 : 6 ; }
    ErrCode projectRecoComposite(const FitParams&, Projection&) const ;
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const ;

    virtual ErrCode initPar1(FitParams*) ;
    virtual ErrCode initPar2(FitParams*) ;
    virtual int type() const { return kRecoComposite ; }

    virtual int posIndex() const { return index()   ; }
    virtual int tauIndex() const { return index() + 3 ; }
    virtual int momIndex() const { return index() + 4 ; }

    virtual bool hasEnergy() const { return m_hasEnergy ; }
    virtual bool hasPosition() const { return true ; }

    virtual void updCache() ;
    virtual double chiSquare(const FitParams* fitparams) const ;

    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::composite, depth, dimM())) ;
      alist.push_back(Constraint(this, Constraint::geometric, depth, 3)) ;
    }

  protected: // I hate this, so we need to change the design ...
    // cache
    CLHEP::HepVector m_m ;    // 'measurement' (x,y,z,px,py,pz,E)
    /** column vector to store the measurement */
    EigenTypes::ColVector m_params;
    /** only lower triangle filled! */
    EigenTypes::MatrixXd  m_covariance;
    CLHEP::HepSymMatrix m_matrixV ; // covariance in measurement
    bool m_hasEnergy ;
  } ;

}

#endif //RECOCOMPOSITE_H
