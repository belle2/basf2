/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn, Fabian Krinner     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

namespace TreeFitter {
  /** mediates 1->1 transition, that actually is the same particle */
  class FeedthroughParticle : public ParticleBase {

  public:

    /** Constructor */
    FeedthroughParticle(Belle2::Particle*   particle,
                        const ParticleBase* mother,
                        const ConstraintConfiguration& config,
                        bool  forceFitAll
                       );

    /** Destructor */
    ~FeedthroughParticle() {};

    /** init particle that does not need a mother vertex  */
    virtual ErrCode initMotherlessParticle(FitParams&) override;

    /** init particle that does need a mother vertex  */
    virtual ErrCode initParticleWithMother(FitParams&) override {return ErrCode(ErrCode::Status::success);}

    /** init covariance matrix, do nothing, since the particle is just a feedthrough */
    virtual ErrCode initCovariance(FitParams&) const override;

    /**  get dimension of constraint */
    virtual int dim() const override {return 0;} // This does not have any parameters

    /** add to constraint list  */
    virtual void addToConstraintList(constraintlist& alist, int depth) const override;

    /** get particle type */
    int type() const override {return kFeedthroughParticle;}

    /** take posIndex from daughter */
    int posIndex() const override;

    /** take momIndex from daughter */
    int momIndex() const override;

  };

}
