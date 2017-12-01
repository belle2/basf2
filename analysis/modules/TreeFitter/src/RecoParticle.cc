/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates a particle object from a charged or neutral final state. Base class for RecoTrack and RecoPhoton. This doesn't do much by itself; most of the functionality is in those individual classes.

//#include <stdio.h>
//#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

#include <analysis/modules/TreeFitter/RecoParticle.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/HelixUtils.h>

namespace TreeFitter {
  extern int vtxverbose ;

  RecoParticle::RecoParticle(Belle2::Particle* particle, const ParticleBase* mother)
    : ParticleBase(particle, mother)
  {
  }

  RecoParticle::~RecoParticle()
  {
  }

  ErrCode RecoParticle::initMotherlessParticle([[gnu::unused]]  FitParams* fitparams)
  {
    return ErrCode::success;
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
        status |= ParticleBase::projectConstraint(type, fitparams, p);
        //      status |= ParticleBase::projectConstraint(type,fitparams,p) ;
        //FT: This printout is annoying, make it B2INFO or B2DEBUG
    }
    return status ;
  }
  ErrCode RecoParticle::projectConstraintCopy(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status ;
    switch (type) {
      case Constraint::track:
//        status |= projectRecoConstraintCopyCopy(fitparams, p) ;
      case Constraint::photon:
        status |= projectRecoConstraintCopy(fitparams, p) ;
        break ;
      default:
        //      status |= ParticleBase::projectConstraint(type,fitparams,p) ;
        //FT: This printout is annoying, make it B2INFO or B2DEBUG
        status |= ParticleBase::projectConstraintCopy(type, fitparams, p);
    }
    return status ;
  }


  double RecoParticle::chiSquare(const FitParams* fitparams) const
  {
    Projection p(fitparams->dim(), dimM());
    projectRecoConstraint(*fitparams, p);
    return p.chiSquare();
  }
}
