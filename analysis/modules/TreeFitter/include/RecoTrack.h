/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
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

  class RecoTrack : public RecoParticle {
  public:
    RecoTrack(Belle2::Particle* bc, const ParticleBase* mother) ;
    virtual ~RecoTrack() ;

    virtual ErrCode initPar2(FitParams*) ;
    virtual ErrCode initCov(FitParams*) const ;
    virtual int dimM() const { return 5 ; }
    virtual int type() const { return kRecoTrack ; }

    virtual ErrCode projectRecoConstraint(const FitParams&, Projection&) const ;
    ErrCode updCache(double flt) ;

    virtual int nFinalChargedCandidates() const { return 1 ; }

    virtual void addToConstraintList(constraintlist& alist, int depth) const
    {
      alist.push_back(Constraint(this, Constraint::track, depth, dimM())) ;
    }
    ErrCode updFltToMother(const FitParams& fitparams) ;
    void setFlightLength(double flt) { m_flt = flt ; }
    //    const TrkFit* trkFit() const { return m_trkFit ; }
  private:
    //    const BField* m_bfield ;
    double m_bfield; //Bfield along Z
    //    const TrkFit* m_trkFit ;
    const Belle2::TrackFitResult* m_trackfit;
    bool m_cached ;
    double m_flt ;
    CLHEP::HepVector    m_m ;
    CLHEP::HepSymMatrix m_matrixV ;
  } ;

}
#endif //RECOTRACK_H
