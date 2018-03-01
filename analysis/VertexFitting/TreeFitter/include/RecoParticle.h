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

    /** dimension of the constraint */
    virtual int dimM() const = 0;

    /** name */
    virtual std::string parname(int index) const;

    /** this here sets the size in the state vector
     * we are only interested in the momenta of photons and tracks
     * as the postion were the track ends or the cluster is, is
     * not relevant for physics  */
    virtual int dim() const { return 3; }

    /** get momentum index */
    virtual int momIndex() const { return index(); }

    /** has an energy in the statevector? */
    virtual bool hasEnergy() const { return false; }

    /** abstract projection */
    virtual ErrCode projectRecoConstraint(const FitParams& fitparams, Projection& p) const = 0;

    /** abstract abstract projection */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;

  };

}
