/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    virtual ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** destructor */
    virtual ~RecoResonance() {};

    /** dimension of the vector */
    virtual int dim() const override { return hasEnergy() ? 4 : 3;} // (px,py,pz,(E))

    /** project the constraint */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const override;

    /** get type */
    virtual int type() const override { return kRecoResonance; }

    /** get position index in statevector x,y,z,tau,px,py,pz */
    virtual int posIndex() const override { return mother()->posIndex(); }

    /** get momentum index in statevector */
    virtual int momIndex() const override { return index(); }

    /** get tau (decay time) index in statevector */
    virtual int tauIndex() const override { return -1; }

    /** parameter name */
    virtual std::string parname(int index) const override;

    /** add to the list of constraints */
    virtual void addToConstraintList(constraintlist& list, int depth) const override
    {
      list.push_back(Constraint(this, Constraint::resonance, depth, dimM()));
    }

  };

}
