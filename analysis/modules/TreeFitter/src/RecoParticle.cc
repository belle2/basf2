//#include <stdio.h>
//#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

#include <analysis/modules/TreeFitter/RecoParticle.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/HelixUtils.h>

namespace TreeFitter {
  extern int vtxverbose ;

  RecoParticle::RecoParticle(Particle* particle, const ParticleBase* mother)
    : ParticleBase(particle, mother)
  {
  }

  RecoParticle::~RecoParticle()
  {
  }

  std::string RecoParticle::parname(int index) const
  {
    return ParticleBase::parname(index + 4) ;
  }

  ErrCode RecoParticle::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status ;
    switch (type) {
      case Constraint::track:
      case Constraint::photon:
        status |= projectRecoConstraint(fitparams, p) ;
        break ;
      default:
        //      status |= ParticleBase::projectConstraint(type,fitparams,p) ;
        std::cout << ParticleBase::projectConstraint(type, fitparams, p) << std::endl << std::flush;
    }
    return status ;
  }

  double RecoParticle::chiSquare(const FitParams* fitparams) const
  {
    // project
    Projection p(fitparams->dim(), dimM()) ;
    projectRecoConstraint(*fitparams, p) ;
    return p.chiSquare() ;
  }
}
