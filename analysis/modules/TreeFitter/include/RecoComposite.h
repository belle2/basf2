#ifndef RECOCOMPOSITE_H
#define RECOCOMPOSITE_H

#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>

namespace TreeFitter {

  class RecoComposite : public ParticleBase {
  public:
    RecoComposite(Particle* bc, const ParticleBase* mother) ;
    virtual ~RecoComposite() ;

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
    HepVector m_m ;    // 'measurement' (x,y,z,px,py,pz,E)
    HepSymMatrix m_matrixV ; // covariance in measurement
    bool m_hasEnergy ;
  } ;

}

#endif //RECOCOMPOSITE_H
