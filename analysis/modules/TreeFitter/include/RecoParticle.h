/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECOPARTICLE_H
#define RECOPARTICLE_H

#include <analysis/modules/TreeFitter/ParticleBase.h>

#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/Matrix.h>

//class HepVector ;
//class HepSymMatrix ;
//class HepMatrix ;

namespace TreeFitter {

  class RecoParticle : public ParticleBase {
  public:
    RecoParticle(Particle* bc, const ParticleBase* mother) ;
    virtual ~RecoParticle() ;

    virtual int dimM() const = 0; // dimension of the measurement
    virtual ErrCode initPar1(FitParams*) { return ErrCode::success ; }
    //virtual ErrCode initCov(FitParams*) const ;
    virtual std::string parname(int index) const ;
    virtual int dim() const { return 3; }   //(px,py,pz)

    virtual int momIndex() const { return index() ; }
    virtual bool hasEnergy() const { return false ; }

    virtual ErrCode projectRecoConstraint(const FitParams& fitparams, Projection& p) const = 0 ;
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const ;
    virtual double chiSquare(const FitParams* fitparams) const ;

  protected:

  private:
  } ;

}
#endif //RECOPARTICLE_H
