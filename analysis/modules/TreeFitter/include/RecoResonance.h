/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECORESONANCE_H
#define RECORESONANCE_H

#include <analysis/modules/TreeFitter/RecoComposite.h>

namespace TreeFitter {

  /** */
  class RecoResonance : public RecoComposite {
  public:
    /** constructor */
    RecoResonance(Belle2::Particle* bc, const ParticleBase* mother);

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);
    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);
    /**  */
    virtual ~RecoResonance();

    /** */
    virtual int dim() const { return hasEnergy() ? 4 : 3; } // (px,py,pz,(E))

    /** */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;
    /** */
    virtual int type() const { return kRecoResonance; }

    /** */
    virtual int posIndex() const { return mother()->posIndex(); }
    /** */
    virtual int momIndex() const { return index(); }
    /** */
    virtual int tauIndex() const { return -1; }

    /** */
    virtual std::string parname(int index) const;

    /** */
    virtual void addToConstraintList(constraintlist& list, int depth) const
    {
      list.push_back(Constraint(this, Constraint::resonance, depth, dimM()));
    }

  };

}

#endif //RECORESONANCE_H
