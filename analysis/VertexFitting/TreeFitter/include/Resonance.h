/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>
namespace TreeFitter {
  class FitParams;
  class ConstraintConfiguration;

  /** class for resonances as internal particles */
  class Resonance : public InternalParticle {
  public:
    /** constructor */
    Resonance(Belle2::Particle* particle,
              const ParticleBase* mother,
              const ConstraintConfiguration& config,
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
