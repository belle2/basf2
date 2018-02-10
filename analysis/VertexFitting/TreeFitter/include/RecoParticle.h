/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo_frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

namespace TreeFitter {

  /** base for RecoPhoton RecoTrack */
  class RecoParticle : public ParticleBase {

  public:

    /** hier koennte ihre werbung stehen */
    RecoParticle(Belle2::Particle* bc, const ParticleBase* mother);

    /** destructor */
    virtual ~RecoParticle() {};

    /** init particle without mother */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** FIXME obsolete and potial hazard??? */
    virtual int dimM() const = 0;

    /** name */
    virtual std::string parname(int index) const;

    /** this here sets the size in the state vector */
    virtual int dim() const { return 3; }

    /** get momentum index */
    virtual int momIndex() const { return index(); }

    /** has an energy in the statevector? */
    virtual bool hasEnergy() const { return false; }

    /** abstract projection */
    virtual ErrCode projectRecoConstraint(const FitParams& fitparams, Projection& p) const = 0;

    /** abstract abstract projection */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;

    /** get chi2 */
    virtual double chiSquare(const FitParams* fitparams) const;

  };

}
