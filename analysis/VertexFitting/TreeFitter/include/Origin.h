/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

#include <mdst/dbobjects/BeamSpot.h>
#include <framework/database/DBObjPtr.h>

namespace TreeFitter {

  /** representation of the beamspot as a particle */
  class Origin : public ParticleBase {

  public:

    /** Constructor */
    Origin(Belle2::Particle* particle,
           const ConstraintConfiguration& config,
           const bool forceFitAll
          );

    /** Constructor */
    explicit Origin(Belle2::Particle* daughter);

    /** destructor */
    virtual ~Origin() {};

    /** init particle, used if it has a mother */
    virtual  ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init particle, used if it has no mother */
    virtual  ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** init the origin "particle"  */
    ErrCode initOrigin();

    /** space reserved in fit pars*/
    virtual int dim() const override { return m_constraintDimension; }

    /** init covariance matrix of the constraint  */
    virtual ErrCode initCovariance(FitParams& fitpar) const override;

    /* particle type */
    virtual int type() const override { return kOrigin; }

    /** the actual constraint projection  */
    ErrCode projectOriginConstraint(const FitParams& fitpar, Projection&) const;

    /** the abstract projection  */
    virtual ErrCode projectConstraint(Constraint::Type, const FitParams&, Projection&) const override;

    /** adds the origin as a particle to the constraint list  */
    virtual void addToConstraintList(constraintlist& list, int depth) const override;

    /** vertex position index in the statevector */
    virtual int posIndex() const override { return index(); }

    /**  momentum index in the statevector. no value for beamspot as a particle */
    virtual int momIndex() const override { return -1; }

    /**  the lifetime index. the origin does not have a lifetime */
    virtual int tauIndex() const override { return -1; }

    /** has energy  */
    virtual bool hasEnergy() const override { return false; }

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
    Belle2::DBObjPtr<Belle2::BeamSpot> m_beamSpot;

    /** inflated the covariance matrix in z by this number */
    const int m_inflationFactorCovZ;
  };
}
