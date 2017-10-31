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

#include <analysis/modules/TreeFitter/RecoParticle.h>

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

    /**    */
    virtual ErrCode initPar2(FitParams*) ;

    /**    */
    virtual ErrCode initCov(FitParams*) const ;

    /**    */
    virtual int dimM() const { return 5 ; }

    /**  type of the constraint   */
    virtual int type() const { return kRecoTrack ; }

    /**   project the constraint (calculate residuals)  */
    virtual ErrCode projectRecoConstraint(const FitParams&, Projection&) const ;

    /**    */
    ErrCode updCache(double flt) ;

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

    /**    */
    CLHEP::HepVector    m_m ;

    /**    */
    CLHEP::HepSymMatrix m_matrixV ;
  };
}
#endif //RECOTRACK_H
