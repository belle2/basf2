#ifndef INTERNALRESONANCE_H
#define INTERNALRESONANCE_H

#include <analysis/modules/TreeFitter/InternalParticle.h>

namespace TreeFitter {
  class FitParams ;

  class Resonance : public InternalParticle {
  public:
    Resonance(Particle* particle, const ParticleBase* mother,
              bool forceFitAll) ;
    virtual ~Resonance() ;

    virtual int dim() const { return 4 ; }
    virtual int type() const { return kResonance ; }
    virtual std::string parname(int index) const ;

    virtual ErrCode initPar1(FitParams*) ;
    virtual ErrCode initPar2(FitParams*) ;

    virtual int posIndex() const { return mother()->posIndex()   ; }
    virtual int momIndex() const { return index() ; }
    virtual int tauIndex() const { return -1 ; }
    virtual bool hasPosition() const { return false ; }

  private:
  } ;

}


#endif //INTERNALRESONANCE_H
