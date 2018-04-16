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

#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

#include <framework/dbobjects/BeamParameters.h>
#include <framework/database/DBObjPtr.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

namespace TreeFitter {

  /** representation of the beamspot as a particle */
  class InteractionPoint : public ParticleBase {

  public:

    /** Constructor */
    InteractionPoint(Belle2::Particle* particle, bool forceFitAll, int dimension);

    /** Constructor */
    InteractionPoint(Belle2::Particle* daughter);

    virtual ~InteractionPoint() {};

    /** init particle, used if it has a mother */
    virtual  ErrCode initParticleWithMother(FitParams* fitparams);

    /** init particle, used if it has no mother */
    virtual  ErrCode initMotherlessParticle(FitParams* fitparams);

    /** init the IP "particle"  */
    ErrCode initBeamSpot(Belle2::Particle* particle);

    /** init the IP "particle"  */
    ErrCode initBeamSpot();

    /** space reserved in fit pars*/
    virtual int dim() const { return m_constraintDimension; }

    /** init covariance matrix of the constraint  */
    virtual ErrCode initCovariance(FitParams* fitpar) const;

    /* particle type */
    virtual int type() const { return kInteractionPoint; }

    /** the actuall constraint projection  */
    ErrCode projectIPConstraint(const FitParams& fitpar, Projection&) const;

    /** the abstract projection  */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;

    /** adds the IP as a particle to the contraint list  */
    virtual void addToConstraintList(constraintlist& list, int depth) const;

    /** vertex position index in the statevector */
    virtual int posIndex() const { return index(); }

    /**  momentum index in the statevector. no value for beamspot as a particle */
    virtual int momIndex() const { return -1; }

    /**  the lifetime index. the IP does not have a lifetime */
    virtual int tauIndex() const { return -1; }

    /** hast energy  */
    virtual bool hasEnergy() const { return false; }

    /** get name  */
    virtual std::string name() const { return "InteractionPoint"; }

  private:

    /** dimension of the constraint dim=2::IPTube; dim=3::IPSpot  */
    const int m_constraintDimension;

    /** the parameters are initialze elsewhere this is just a pointer to that */
    Belle2::DBObjPtr<Belle2::BeamParameters> m_beamParams;

    /** vertex position of the IP */
    Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor, 3, 1> m_ipPosVec;

    /** covariance of the IP
     * dont know size but I know the max size
     * */
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor, 3, 3> m_ipCovariance;

  };
}
