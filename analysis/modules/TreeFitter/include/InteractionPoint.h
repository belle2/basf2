/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMSPOT_H
#define BEAMSPOT_H

#include <analysis/modules/TreeFitter/InternalParticle.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>

namespace TreeFitter {

  class InteractionPoint : public InternalParticle {
  public:
    InteractionPoint(Particle* particle, bool forceFitAll) ;
    //    InteractionPoint(Particle* particle, bool forceFitAll,
    //                     bool addupsilon) ; //FT: we don't actually have addupsilon in Belle2, this can be trimmed

    virtual ~InteractionPoint() ;

    ErrCode initBeamSpot(Particle* particle) ;

    virtual int dim() const { return 3 ; } // (x,y,z)

    virtual ErrCode initPar1(FitParams*) ;
    virtual ErrCode initPar2(FitParams*) ;
    virtual ErrCode initCov(FitParams*) const ;

    virtual int type() const { return kInteractionPoint ; }

    virtual double chiSquare(const FitParams* par) const ;

    ErrCode projectIPConstraint(const FitParams& fitpar, Projection&) const ;
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const ;

    virtual void addToConstraintList(constraintlist& alist, int depth) const ;

    virtual int posIndex() const { return index() ; }
    virtual int momIndex() const { return -1; }//daughters().front()->momIndex() ; } // terrible hack
    virtual int tauIndex() const { return -1; }
    virtual bool hasEnergy() const { return false ; }
    virtual std::string name() const { return "InteractionPoint" ; }

  private:
    bool m_constrainXY ;
    bool m_constrainXYZ ;
    CLHEP::HepVector m_ipPos ;       // interaction point position
    CLHEP::HepSymMatrix m_ipCov ;    // cov matrix
    CLHEP::HepSymMatrix m_ipCovInv ; // inverse of cov matrix
  } ;

}


#endif //BEAMSPOT_H
