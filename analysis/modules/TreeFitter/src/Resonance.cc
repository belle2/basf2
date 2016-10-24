/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates and initialises internal particles in the case where you want to treat them as resonances. Not to be confused with RecoResonance, which loads a pre-reconstructed resonance (i.e. from V0, etc)

//#include <iomanip>
//#include <algorithm>

//#include <analysis/dataobjects/Particle.h>
#include <analysis/modules/TreeFitter/Resonance.h>



namespace TreeFitter {

  extern int vtxverbose ;

  Resonance::Resonance(Particle* particle, const ParticleBase* mother,
                       bool forceFitAll)
    : InternalParticle(particle, mother, forceFitAll)
  {
  }

  Resonance::~Resonance() {}

  ErrCode Resonance::initPar1(FitParams* fitparams)
  {
    ErrCode status ;
    for (daucontainer::const_iterator it = begin() ; it != end() ; ++it)
      status |= (*it)->initPar1(fitparams) ;
    return status ;
  }

  ErrCode Resonance::initPar2(FitParams* fitparams)
  {
    ErrCode status ;
    for (daucontainer::const_iterator it = begin() ; it != end() ; ++it)
      status |= (*it)->initPar2(fitparams) ;
    initMom(fitparams) ;
    return status ;
  }

  std::string Resonance::parname(int index) const
  {
    return ParticleBase::parname(index + 4) ;
  }

}
