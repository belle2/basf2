/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/VertexFitting/TreeFitter/RecoParticle.h>

#include <analysis/dataobjects/Particle.h>

namespace TreeFitter {

  /** representation of all charged final states FIXME rename since this name is taken in tracking    */
  class RecoTrack : public RecoParticle {

  public:
    /** constructor */
    RecoTrack(Belle2::Particle* bc, const ParticleBase* mother) ;

    /** destructor */
    virtual ~RecoTrack() {};

    /**   init with mother particle (replacing initPar2)  */
    virtual ErrCode initParticleWithMother(FitParams& fitparams) override;

    /** init without mother particle   */
    virtual ErrCode initMotherlessParticle(FitParams& fitparams) override;

    /** init covariance matrix of this particle constraint */
    ErrCode initCovariance(FitParams& fitparams) const override;

    /** update m_flt */
    ErrCode updFltToMotherCopy(const FitParams* fitparams);

    /**   project the constraint (calculate residuals)  */
    virtual ErrCode projectRecoConstraint(const FitParams&, Projection&) const override;

    /** updated the cached parameters */
    ErrCode updateParams(double flt);

    /** dimension (5) */
    virtual int dimM() const override { return 5 ; }

    /**  type of the constraint   */
    virtual int type() const override { return kRecoTrack ; }

    /** number of final charged candidates */
    virtual int nFinalChargedCandidates() const override { return 1 ; }

    /** add to the list of constraints */
    virtual void addToConstraintList(constraintlist& alist, int depth) const override
    {
      alist.push_back(Constraint(this, Constraint::track, depth, dimM(), 1)) ;
    }

    /** update flight length to mother */
    ErrCode updFltToMother(const FitParams& fitparams) ;

    /** setter for the flight length */
    void setFlightLength(double flt) { m_flt = flt ; }

  private:

    /**  B field along z   */
    double m_bfield; //Bfield along Z

    /**  trackfit result from reconstruction   */
    const Belle2::TrackFitResult* m_trackfit;

    /** flag to mark the particle as initialised   */
    bool m_cached ;

    /** helix arc length at vertex */
    double m_flt ;

    /** column vector to store the measurement */
    Eigen::Matrix<double, 1, 5> m_params;

    /** only lower triangle filled! */
    Eigen::Matrix<double, 5, 5>m_covariance;

    /** scale the momenta by this correction factor */
    const float m_momentumScalingFactor;
  };
}
