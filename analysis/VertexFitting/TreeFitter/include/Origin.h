/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jo-Frederik Krohn                                         *
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
  class Origin : public ParticleBase {

  public:

    /** Constructor */
    Origin(Belle2::Particle* particle,
           const bool forceFitAll,
           const std::vector<double> customOriginVertex,
           const std::vector<double> customOriginCovariance,
           const bool isBeamSpot
          );

    /** Constructor */
    Origin(Belle2::Particle* daughter);

    /** destructor */
    virtual ~Origin() {};

    /** init particle, used if it has a mother */
    virtual  ErrCode initParticleWithMother(FitParams& fitparams);

    /** init particle, used if it has no mother */
    virtual  ErrCode initMotherlessParticle(FitParams& fitparams);

    /** init the origin "particle"  */
    ErrCode initOrigin();

    /** space reserved in fit pars*/
    virtual int dim() const { return m_constraintDimension; }

    /** init covariance matrix of the constraint  */
    virtual ErrCode initCovariance(FitParams& fitpar) const;

    /* particle type */
    virtual int type() const { return kOrigin; }

    /** the actuall constraint projection  */
    ErrCode projectOriginConstraint(const FitParams& fitpar, Projection&) const;

    /** the abstract projection  */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const;

    /** adds the origin as a particle to the constraint list  */
    virtual void addToConstraintList(constraintlist& list, int depth) const;

    /** vertex position index in the statevector */
    virtual int posIndex() const { return index(); }

    /**  momentum index in the statevector. no value for beamspot as a particle */
    virtual int momIndex() const { return -1; }

    /**  the lifetime index. the origin does not have a lifetime */
    virtual int tauIndex() const { return -1; }

    /** has energy  */
    virtual bool hasEnergy() const { return false; }

    /** get name  */
    virtual std::string name() const { return "Origin"; }

  private:

    /** dimension of the constraint   */
    const int m_constraintDimension;

    /** vertex coordinates */
    const std::vector<double> m_customOriginVertex;

    /** vertex covariance */
    const std::vector<double> m_customOriginCovariance;

    /** vertex position of the origin */
    Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor, 3, 1> m_posVec;

    /** covariance of the origin
     * dont know size but I know the max size
     * */
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor, 3, 3> m_covariance;

    /** is this the beam constraint? */
    const bool m_isBeamSpot;

    /** the parameters are initialize elsewhere this is just a pointer to that */
    Belle2::DBObjPtr<Belle2::BeamParameters> m_beamParams;

  };
}
