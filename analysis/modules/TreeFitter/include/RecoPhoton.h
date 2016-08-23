#ifndef RECOPHOTON_H
#define RECOPHOTON_H

#include <analysis/modules/TreeFitter/RecoParticle.h>

using namespace CLHEP;

namespace TreeFitter {
  class RecoPhoton : public RecoParticle {
  public:
    RecoPhoton(Particle* bc, const ParticleBase* mother) ;
    virtual ~RecoPhoton() ;

    virtual int dimM() const { return m_useEnergy ? 3 : 2 ; }
    virtual ErrCode initPar2(FitParams*) ;
    virtual ErrCode initCov(FitParams*) const ;
    virtual int type()     const { return kRecoPhoton ; }
    virtual ErrCode projectRecoConstraint(const FitParams&, Projection&) const ;
    ErrCode updCache() ;

    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::photon, depth, dimM())) ;
    }

    static bool useEnergy(Particle& cand) ;

  private:
    bool m_init ;
    bool m_useEnergy ;
    HepVector m_m ;
    HepSymMatrix m_matrixV ;
  } ;

}
#endif //RECOPHOTON_H
