/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECOTRACK_H
#define RECOTRACK_H

#include <analysis/VertexFitting/TreeFitter/RecoParticle.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>

#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Matrix/Matrix.h>

//using namespace CLHEP;

namespace TreeFitter {

  /** reprasentation of all charged final states FIXME rename since this name is taken in tracking    */
  class RecoTrack : public RecoParticle {

  public:
    /**    */
    RecoTrack(Belle2::Particle* bc, const ParticleBase* mother) ;

    /**    */
    virtual ~RecoTrack() ;

    /**   init with mother particle (replacing initPar2)  */
    virtual ErrCode initParticleWithMother(FitParams* fitparams);

    /** init without mother particle   */
    virtual ErrCode initMotherlessParticle(FitParams* fitparams);

    /** init covariance matrix of this particle constraint */
    ErrCode initCovariance(FitParams* fitparams) const;

    /** update m_flt */
    ErrCode updFltToMotherCopy(const FitParams& fitparams);

    /**   project the constraint (calculate residuals)  */
    virtual ErrCode projectRecoConstraint(const FitParams&, Projection&) const;

    /** updated the cahed parameters */
    ErrCode updateParams(double flt);

    /**    */
    virtual int dimM() const { return 5 ; }

    /**  type of the constraint   */
    virtual int type() const { return kRecoTrack ; }

    /**    */
    virtual int nFinalChargedCandidates() const { return 1 ; }

    /**    */
    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::track, depth, dimM())) ;
    }

    /**    */
    ErrCode updFltToMother(const FitParams& fitparams) ;

    /**    */
    void setFlightLength(double flt) { m_flt = flt ; }

  private:

    /**  b filed along z   */
    double m_bfield; //Bfield along Z

    /**  trackfit result from reconstruction   */
    const Belle2::TrackFitResult* m_trackfit;

    /** flag to mark the particle as initialised   */
    bool m_cached ;

    /**    */
    double m_flt ;

    /** column vector to store the measurement */
    Eigen::Matrix<double, 1, 5> m_params;

    /** only lower triangle filled! */
    Eigen::Matrix<double, 5, 5>m_covariance;
  };
}
#endif //RECOTRACK_H
