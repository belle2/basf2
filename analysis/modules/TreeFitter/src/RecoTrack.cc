/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates and initialises charged particles using helix parameters. Defines the projection of reconstruction (helix) constraints to the fit.

//#include <stdio.h>
//#include <Beta/BtaCandidate.hh>

//#include <analysis/dataobjects/Particle.h>

//#include <BetaRecoAdapter/BtaAbsRecoObject.hh>
//#include <TrkBase/TrkFit.hh>
//#include <TrkBase/TrkRecoTrk.hh>
//#include <TrkBase/TrkPoca.hh>
//#include <TrkBase/TrkDifTraj.hh>
#include <analysis/modules/TreeFitter/RecoTrack.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/HelixUtils.h>

#include <framework/gearbox/Const.h>

#include <TMath.h>
//using std::cout;
//using std::endl;

namespace TreeFitter {

  extern int vtxverbose ;

  RecoTrack::RecoTrack(Particle* particle, const ParticleBase* mother)
    : RecoParticle(particle, mother), m_bfield(0), m_trackfit(0), m_cached(false), m_flt(0), m_m(7), m_matrixV(7)
  {
    //FT: FIX ME: This initialises the BField at the IP. This might not be a correct assumption, but is the easiest and fastest for now. Check the impact of using the field at the perigee, or perhaps at the decay vertex as appropriate, especially for significantly displaced vertices.
    m_bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T; //Bz in Tesla
    B2DEBUG(80, "RecoTrack - Bz from BFieldManager: " << m_bfield);
    //    PdtPid::PidType pidType = PdtPid::pion ;
    //    if( bc()->pdtEntry() ) pidType = bc()->pdtEntry()->pidId() ;
    //    _trkFit = bc()->recoTrk()->fitResult(pidType) ;
    if (m_trackfit == 0) {
      //FT: this is superflous as m_trackfit has just been initialised, but we'll need the statement in future developments.

      //FT: For now we still use the pion track hypothesis. Later: add multiple hypotheses, add a flag to allow users to choose whether they want the "true" hypothesis or just the pion (for cases where the pion works better, for whatever reason)
      m_trackfit = particle->getTrack()->getTrackFitResult(Const::pion);
    }
  }

  RecoTrack::~RecoTrack() {}

  ErrCode RecoTrack::initPar2(FitParams* fitparams)
  {
    if (m_flt == 0) const_cast<RecoTrack*>(this)->updFltToMother(*fitparams) ;
    TVector3 recoP = m_trackfit->getHelix().getMomentumAtArcLength2D(m_flt, m_bfield) ;
    int momindex = momIndex();
    fitparams->par(momindex + 1) = recoP.X() ;
    fitparams->par(momindex + 2) = recoP.Y() ;
    fitparams->par(momindex + 3) = recoP.Z() ;
    return ErrCode();
  }

  ErrCode RecoTrack::initCov(FitParams* fitparams) const
  {
    // we only need a rough estimate of the covariance
    TMatrixFSym p4Err = particle()->getMomentumErrorMatrix();
    int momindex = momIndex();
    for (int row = 1; row <= 3; ++row)
      fitparams->cov()(momindex + row, momindex + row) = 1000 * p4Err[row - 1][row - 1];
    return ErrCode();
  }

  ErrCode RecoTrack::updFltToMother(const FitParams& fitparams)
  {
    int posindexmother = mother()->posIndex() ;
    TVector3 pt(fitparams.par()(posindexmother + 1),
                fitparams.par()(posindexmother + 2),
                fitparams.par()(posindexmother + 3)) ;
    //FT: is this correct? Might need to make it 3D
    m_flt = m_trackfit->getHelix().getArcLength2DAtXY(pt.X(), pt.Y());
    // FIX ME: use helix poca to get estimate of flightlength first

    // FT: This needs a control statement...
    /*
    double lowrange  = _trkFit->traj().lowRange() ;
    double highrange = _trkFit->traj().hiRange() ;
    if(     _flt < lowrange)  _flt = lowrange ;
    else if(_flt > highrange) _flt = highrange ;
    */
    return ErrCode() ;
  } ;

  ErrCode RecoTrack::updCache(double flt)
  {
    if (vtxverbose >= 2)
      std::cout << "RecoTrack::updCache: " << name().c_str()
                << " from " << m_flt << " to " << flt << std::endl ;
    m_flt = flt ;
    //FT: make this a straightforward cast
    std::vector<float> tau = m_trackfit->getTau();
    for (int i = 1; i <= 5; ++i)
      m_m(i) = tau[i - 1];
    // FIX ME: bring z0 in the correct domain ...
    TMatrixDSym cov = m_trackfit->getCovariance5();
    for (int row = 1; row <= 5; ++row)
      for (int col = 1; col <= row; ++col)
        m_matrixV(row, col) = cov[row - 1][col - 1];
    m_cached = true ;
    //
    return ErrCode() ;
  }

  HepVector symdiag(const HepSymMatrix& m)   //FT: I don't think this is actually ever used outside of printouts
  {
    HepVector rc(m.num_row()) ;
    for (int i = 1; i <= m.num_row(); ++i)
      rc(i) = sqrt(m.fast(i, i)) ;
    return rc ;
  }

  ErrCode  RecoTrack::projectRecoConstraint(const FitParams& fitparams, Projection& p) const
  {
    ErrCode status ;
    // create HepVector with parameters
    HepVector vertexpars(6) ;
    int posindexmother = mother()->posIndex() ;
    for (int row = 1; row <= 3; ++row)
      vertexpars(row) = fitparams.par()(posindexmother + row) ;
    int momindex = momIndex() ;
    for (int row = 1; row <= 3; ++row)
      vertexpars(3 + row) = fitparams.par()(momindex + row) ;

    // translate into trackparameters
    HepVector helixpars(6) ;
    HepMatrix jacobian(6, 6) ;
    HelixUtils::helixFromVertex(vertexpars, charge(), m_bfield, helixpars, jacobian) ;

    // get the measured track parameters at the poca to the mother
    if (!m_cached) {
      RecoTrack* nonconst =  const_cast<RecoTrack*>(this) ;
      if (m_flt == 0) nonconst->updFltToMother(fitparams) ;
      nonconst->updCache(m_flt) ;
    }

    if (vtxverbose >= 5) {
      std::cout << "vertexpars = " << vertexpars.T() << std::endl ;
      std::cout << "pred = " << helixpars.T() << std::endl ;
      std::cout << "m   = " << m_m.T() << std::endl ;
      std::cout << "sig = " << symdiag(m_matrixV).T() << std::endl ;
    }

    // get the measured track parameters at the flightlength of the vertex
    // double flt = helixpars(6) ;
    //const double fltupdcut = 1000 ; //cm
    //if( fabs(flt - _flt) > fltupdcut )
    //  status |= const_cast<RecoTrack*>(this)->updCache(flt) ;

    // fill the residual and cov matrix
    for (int row = 1; row <= 5; ++row) {
      p.r(row) = helixpars(row) - m_m(row) ;
      for (int col = 1; col <= row; ++col)
        p.Vfast(row, col) = m_matrixV.fast(row, col) ;
    }

    // bring phi-residual in the correct domain ([-pi,pi])
    p.r(2) = HelixUtils::phidomain(p.r(2)) ;
    // FIX ME: bring z0 residual in the correct domain --> this needs some thinking

    // calculate the full projection matrix from the jacobian
    // assumes that H is reset !
    for (int row = 1; row <= 5; ++row) {
      // the position
      for (int col = 1; col <= 3; ++col)
        p.H(row, posindexmother + col) = jacobian(row, col) ;
      // the momentum
      for (int col = 1; col <= 3; ++col)
        p.H(row, momindex + col) = jacobian(row, col + 3) ;
    }
    return status ;
  }

}
