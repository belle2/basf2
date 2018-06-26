/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VertexFitting/TreeFitter/RecoComposite.h>

namespace TreeFitter {

  /** A class for resonances */
  class RecoResonance : public RecoComposite {

  public:

    /** constructor */
    RecoResonance(Belle2::Particle* bc, const ParticleBase* mother);

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);

    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** destructor */
    virtual ~RecoResonance() {};

    /** dimenstion of the vector */
    virtual int dim() const { return hasEnergy() ? 4 : 3;} // (px,py,pz,(E))

    /** project the constraint */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;

    /** get type */
    virtual int type() const { return kRecoResonance; }

    /** get position index in statevector x,y,z,tau,px,py,pz */
    virtual int posIndex() const { return mother()->posIndex(); }

    /** get momentum index in statevector */
    virtual int momIndex() const { return index(); }

    /** get tau (decay time) index in statevector */
    virtual int tauIndex() const { return -1; }

    /** parameter name */
    virtual std::string parname(int index) const;

    /** add to the list of constraints */
    virtual void addToConstraintList(constraintlist& list, int depth) const
    {
      list.push_back(Constraint(this, Constraint::resonance, depth, dimM()));
    }

  };

}
