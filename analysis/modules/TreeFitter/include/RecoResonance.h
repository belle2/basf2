/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECORESONANCE_H
#define RECORESONANCE_H

#include <analysis/modules/TreeFitter/RecoComposite.h>
//#include <CLHEP/Matrix/Vector.h>
//#include <CLHEP/Matrix/SymMatrix.h>

namespace TreeFitter {

  class RecoResonance : public RecoComposite {
  public:
    RecoResonance(Particle* bc, const ParticleBase* mother) ;
    virtual ~RecoResonance() ;

    virtual int dim() const { return hasEnergy() ? 4 : 3 ; } // (px,py,pz,(E))

    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const ;
    virtual ErrCode initPar1(FitParams*) ;
    virtual ErrCode initPar2(/* FitParams* */) ; //FT: argument temporarily removed to avoid compiler warnings
    virtual int type() const { return kRecoResonance ; }

    virtual int posIndex() const { return mother()->posIndex()  ; }
    virtual int momIndex() const { return index() ; }
    virtual int tauIndex() const { return -1 ; }

    virtual std::string parname(int index) const ;

    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::resonance, depth, dimM())) ;
    }

  private:
  } ;

}

#endif //RECORESONANCE_H
