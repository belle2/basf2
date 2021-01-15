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
#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>

namespace TreeFitter {

  /** another unneccessary layer of abstraction */
  class InternalTrack : public ParticleBase {

  public:

    /** constructor */
    InternalTrack(Belle2::Particle* particle,
                  const ParticleBase* mother,
                  const ConstraintConfiguration& config,
                  bool forceFitAll,
                  bool noEnergySum,
                  bool forceMassConstraint
                 );

    /** destructor */
    virtual ~InternalTrack() {};

    /** init covariance */
    virtual ErrCode initCovariance(FitParams&) const override;

    /** project helix constraint */
    ErrCode projectHelixConstraint(const FitParams&, Projection&) const;

    /** init particle in case it has a mother */
    virtual ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init particle in case it has no mother */
    virtual ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** find out which constraint it is and project */
    ErrCode projectConstraint(const Constraint::Type type, const FitParams& fitparams, Projection& p) const override;

    /** space reserved in fit params */
    virtual int dim() const override;

    /**  type */
    virtual int type() const override { return kInternalTrack ; }

    /**   position index in fit params*/
    virtual int posIndex() const override;

    /** tau index in fit params only if it has a mother */
    virtual int tauIndex() const override;

    /** momentum index in fit params depending on whether it has a mother  */
    virtual int momIndex() const override;

    /** has energy in fitparams  */
    virtual bool hasEnergy() const override;

    /** has position index  */
    virtual bool hasPosition() const override;

    /** add to constraint list  */
    virtual void addToConstraintList(constraintlist& list, int depth) const override ;

    /** set mass constraint flag */
    void setMassConstraint(bool b) { m_massconstraint = b ; }

    /** Forces the four-momentum sum */
    void forceP4Sum(FitParams& fitparams) const override;

  protected:

    /** init momentum of *this and daughters */
    ErrCode initMomentum(FitParams& fitparams) const ;

  private:

    /** compare transverse track momentum*/
    bool static compTrkTransverseMomentum(const RecoTrack* lhs, const RecoTrack* rhs);

    /** has mass constraint */
    bool m_massconstraint ;

    /** Energy is not conserved at the decay (happens in Bremsstrahlung) */
    bool m_noEnergySum;

    /** is conversion */
    bool m_isconversion;

    /** B field value */
    double m_bfield;
  } ;

}
