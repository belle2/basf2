/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>

namespace TreeFitter {
  class FitParams;

  /** class for resonances as internal particles */
  class Resonance : public InternalParticle {
  public:
    /** constructor */
    Resonance(Belle2::Particle* particle,
              const ParticleBase* mother,
              bool forceFitAll);

    /** destructor  */
    virtual ~Resonance() ;

    /** initialise a motherless particle */
    ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** initialise a particle with a mother */
    ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** dimension (4) */
    virtual int dim() const override { return 4; }
    /** particle type */
    virtual int type() const override { return kResonance; }
    /** parameter name */
    virtual std::string parname(int index) const override;

    /** get position index in statevector x,y,z,tau,px,py,pz */
    virtual int posIndex() const override { return mother()->posIndex(); }
    /** get momentum index in statevector */
    virtual int momIndex() const override { return index(); }
    /** get tau (lifetime) index in statevector */
    virtual int tauIndex() const override { return -1; }
    /** does this class have position? NO */
    virtual bool hasPosition() const override { return false; }

  private:
  };

}
