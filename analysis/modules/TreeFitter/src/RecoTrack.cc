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
#include <analysis/modules/TreeFitter/RecoTrack.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/HelixUtils.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <TMath.h>

#define NUMERICAL_JACOBIAN
//#define HELIX_TEST

namespace TreeFitter {

  extern int vtxverbose;

  RecoTrack::RecoTrack(Belle2::Particle* particle, const ParticleBase* mother)
    : RecoParticle(particle, mother), m_bfield(0), m_trackfit(0), m_cached(false),
      m_flt(0), m_m(7), m_matrixV(7), m_params(5), m_covariance(5, 5)
  {
    //FT: FIX ME: This initialises the BField at the IP. This might not be a correct assumption, but is the easiest and fastest for now. Check the impact of using the field at the perigee, or perhaps at the decay vertex as appropriate, especially for significantly displaced vertices.
    m_bfield = Belle2::BFieldManager::getField(TVector3(0, 0, 0)).Z() / Belle2::Unit::T; //Bz in Tesla
    B2DEBUG(80, "RecoTrack - Bz from BFieldManager: " << m_bfield);
    m_covariance = Eigen::Matrix<double, 5, 5>::Zero(5, 5);
    if (m_trackfit == 0) {
      //FT: this is superflous as m_trackfit has just been initialised, but we'll need the statement in future developments.
      //FT: For now we still use the pion track hypothesis. Later: add multiple hypotheses, add a flag to allow users to choose whether they want the "true" hypothesis or just the pion (for cases where the pion works better, for whatever reason)
      m_trackfit = particle->getTrack()->getTrackFitResultWithClosestMass(Belle2::Const::pion);
    }
  }

  RecoTrack::~RecoTrack() {}

  ErrCode RecoTrack::initParticleWithMother(FitParams* fitparams)
  {

    B2DEBUG(85, "---------- RecoTrack::initParticleWithMother                                   ");
    //initPar2
    if (m_flt == 0) {
      const_cast<RecoTrack*>(this)->updFltToMother(*fitparams);
    }
    TVector3 recoP = m_trackfit->getHelix().getMomentumAtArcLength2D(m_flt, m_bfield);
    int momindex = momIndex();
    fitparams->getStateVector()(momindex) = recoP.X();
    fitparams->getStateVector()(momindex + 1) = recoP.Y();
    fitparams->getStateVector()(momindex + 2) = recoP.Z();
    B2DEBUG(85, "---------- RecoTrack::initParticleWithMother momentum set to:\n" << fitparams->getStateVector().segment(momindex, 3));
    return ErrCode::success;
  }
  ErrCode RecoTrack::initMotherlessParticle([[gnu::unused]] FitParams* fitparams) //__attribute__((unused)))
  {
    return ErrCode::success;
  }




  ErrCode RecoTrack::initPar2(FitParams* fitparams)
  {
    if (m_flt == 0) {
      const_cast<RecoTrack*>(this)->updFltToMother(*fitparams);
    }
    TVector3 recoP = m_trackfit->getHelix().getMomentumAtArcLength2D(m_flt, m_bfield);
    int momindex = momIndex();
    fitparams->par(momindex + 1) = recoP.X();
    fitparams->par(momindex + 2) = recoP.Y();
    fitparams->par(momindex + 3) = recoP.Z();
    return ErrCode();
  }

  ErrCode RecoTrack::initCov(FitParams* fitparams) const
  {
    // we only need a rough estimate of the covariance
    TMatrixFSym p4Err = particle()->getMomentumErrorMatrix();
    int momindex = momIndex();
    for (int row = 1; row <= 3; ++row) {
      fitparams->cov()(momindex + row, momindex + row) = 1000 * p4Err[row - 1][row - 1];
    }
    return ErrCode();
  }
  ErrCode RecoTrack::initCovariance(FitParams* fitparams) const
  {
    // we only need a rough estimate of the covariance
    TMatrixFSym p4Err = particle()->getMomentumErrorMatrix();
    int momindex = momIndex();

    for (int row = 0; row < 3; ++row) {
      //std::cout << "RecoTrack::initCovariance writing :" << 1000 * p4Err[row][row]  << std::endl;
      fitparams->getCovariance()(momindex + row, momindex + row) = 1000 * p4Err[row][row];
    }
    B2DEBUG(85, "----------     RecoTrack::initCovariance to:\n" << fitparams->getCovariance());

    return ErrCode();
  }




  ErrCode RecoTrack::updFltToMother(const FitParams& fitparams)
  {
    int posindexmother = mother()->posIndex();
    TVector3 pt(fitparams.par()(posindexmother + 1),
                fitparams.par()(posindexmother + 2),
                fitparams.par()(posindexmother + 3));
    //FT: is this correct? Might need to make it 3D
    m_flt = m_trackfit->getHelix().getArcLength2DAtXY(pt.X(), pt.Y());
    // FIX ME: use helix poca to get estimate of flightlength first
    return ErrCode();
  } ;
  ErrCode RecoTrack::updFltToMotherCopy(const FitParams& fitparams)
  {
    int posindexmother = mother()->posIndex();
    //FT: is this correct? Might need to make it 3D
    m_flt = m_trackfit->getHelix().getArcLength2DAtXY(
              fitparams.getStateVector()(posindexmother),
              fitparams.getStateVector()(posindexmother + 1));
    // FIX ME: use helix poca to get estimate of flightlength first
    return ErrCode();
  } ;

  ErrCode RecoTrack::updCache(double flt)
  {
    if (vtxverbose >= 2)
      std::cout << "RecoTrack::updCache: " << name().c_str()
                << " from " << m_flt << " to " << flt << std::endl ;
    m_flt = flt;
    //FT: make this a straightforward cast
    std::vector<float> tau = m_trackfit->getTau();
    for (int i = 1; i <= 5; ++i) {
      m_m(i) = tau[i - 1];
    }
    // FIX ME: bring z0 in the correct domain ...
    TMatrixDSym cov = m_trackfit->getCovariance5();
    //could also be TMatrixDSym cov = (m_trackfit->getUncertainHelix()).getCovariance();
    for (int row = 1; row <= 5; ++row) {
      for (int col = 1; col <= row; ++col) {
        m_matrixV(row, col) = cov[row - 1][col - 1];
      }
    }
    m_cached = true ;
    return ErrCode() ;
  }
  ErrCode RecoTrack::updateParams(double flt)
  {
    m_flt = flt;
    std::vector<float> tau = m_trackfit->getTau();
    //JFK: FIXME this is castable but I dont get how Eigen::VectorXf::map works 2017-09-26
    for (unsigned int i = 0; i < tau.size(); ++i) {
      m_params(i) = tau[i];
    }
    B2DEBUG(85, "----------       RecoTrack::updateParams this is the measurement:\n" << m_params);
    // FIX ME: bring z0 in the correct domain ...
    TMatrixDSym cov = m_trackfit->getCovariance5();
    //could also be TMatrixDSym cov = (m_trackfit->getUncertainHelix()).getCovariance();
    for (int row = 0; row < 5; ++row) {
      for (int col = 0; col <= row; ++col) {
        m_covariance(row, col) = cov[row][col];
      }
    }

    B2DEBUG(85, "----------       RecoTrack::updateParams this is the measurement COVARIANCE:\n" << m_covariance);
    m_cached = true;
    return ErrCode::success;
  }





  CLHEP::HepVector symdiag(const CLHEP::HepSymMatrix& m)   //FT: I don't think this is actually ever used outside of printouts
  {
    CLHEP::HepVector rc(m.num_row());
    for (int i = 1; i <= m.num_row(); ++i) {
      rc(i) = sqrt(m.fast(i, i));
    }
    return rc;
  }

  ErrCode  RecoTrack::projectRecoConstraint(const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    // create HepVector with parameters
    int posindexmother = mother()->posIndex();
    TVector3 position(fitparams.par()(posindexmother + 1),
                      fitparams.par()(posindexmother + 2),
                      fitparams.par()(posindexmother + 3));
    int momindex = momIndex();
    TVector3 momentum(fitparams.par()(momindex + 1),
                      fitparams.par()(momindex + 2),
                      fitparams.par()(momindex + 3));

#ifdef HELIX_TEST
    //Test the Helix functions
    HelixUtils::helixTest();
#endif
    // translate into trackparameters
    Belle2::Helix helix;
    double flt;
    CLHEP::HepMatrix jacobian(5, 6);

#ifndef NUMERICAL_JACOBIAN
    HelixUtils::helixFromVertex(position, momentum, charge(), m_bfield, helix, flt, jacobian);
#else
    HelixUtils::helixFromVertexNumerical(position, momentum, charge(), m_bfield, helix, flt, jacobian);
#endif
    // get the measured track parameters at the poca to the mother
    if (!m_cached) {
      RecoTrack* nonconst =  const_cast<RecoTrack*>(this);
      if (m_flt == 0) {nonconst->updFltToMother(fitparams);}
      nonconst->updCache(m_flt);
    }


    // get the measured track parameters at the flightlength of the vertex
    // double flt = helixpars(6) ;
    //const double fltupdcut = 1000 ; //cm
    //if( fabs(flt - _flt) > fltupdcut )
    //  status |= const_cast<RecoTrack*>(this)->updCache(flt) ;


    CLHEP::HepVector helixpars(5);
    helixpars[HelixUtils::iD0]        = helix.getD0();
    helixpars[HelixUtils::iPhi0]      = helix.getPhi0();
    helixpars[HelixUtils::iOmega]     = helix.getOmega();
    helixpars[HelixUtils::iZ0]        = helix.getZ0();
    helixpars[HelixUtils::iTanLambda] = helix.getTanLambda();

    if (vtxverbose >= 5) {
      std::cout << "vertex position = ";
      position.Print(); std::cout << std::endl ;
      std::cout << "vertex momentum = ";
      momentum.Print(); std::cout << std::endl ;
      std::cout << "helix = " << helixpars.T() << std::endl ;
      std::cout << "flt   = " << flt << std::endl ;
      std::cout << "m   = " << m_m.T() << std::endl ;
      std::cout << "sig = " << symdiag(m_matrixV).T() << std::endl ;
      std::cout << "V matrix = " << m_matrixV << std::endl ;
    }

    // fill the residual and cov matrix
    for (int row = 1; row <= 5; ++row) {
      p.r(row) = helixpars(row) - m_m(row);
      for (int col = 1; col <= row; ++col) {
        p.Vfast(row, col) = m_matrixV.fast(row, col);
      }
    }

    // bring phi-residual in the correct domain ([-pi,pi])
    if (vtxverbose >= 8)     std::cout << "bring phi-residual in the correct domain ([-pi,pi]): " << p.r(2);
    p.r(2) = HelixUtils::phidomain(p.r(2));
    if (vtxverbose >= 8) {
      std::cout << " -> " << p.r(2) << std::endl;
      std::cout << "And now the Jacobian:" << std::endl;
      std::cout << jacobian << std::endl;
    }
    // FIX ME: bring z0 residual in the correct domain --> this needs some thinking

    // calculate the full projection matrix from the jacobian
    // assumes that H is reset !
    for (int row = 1; row <= 5; ++row) {
      // the position
      for (int col = 1; col <= 3; ++col) {
        p.H(row, posindexmother + col) = jacobian(row, col);
      }

      // the momentum
      for (int col = 1; col <= 3; ++col) {
        p.H(row, momindex + col) = jacobian(row, col + 3);
      }
    }

    if (vtxverbose >= 8) {
      std::cout << "And now the H matrix:" << std::endl;
      std::cout << p.H() << std::endl;
    }
    return status;
  }
  ErrCode  RecoTrack::projectRecoConstraintCopyCopy(const FitParams& fitparams, Projection& p) const
  {
    B2DEBUG(85, "----------           RecoTrack::projectRecoConstraintCopy\n");
    ErrCode status;
    int posindexmother = mother()->posIndex();
    int momindex = momIndex();
//    EigenTypes::ColVector position = fitparams.getStateVector().segment(posindexmother, 3);
//    EigenTypes::ColVector momentum = fitparams.getStateVector().segment(momindex, 3);
    //EigenTypes::MatrixXd jacobian = EigenTypes::MatrixXd::Zero(5, 6);

    TVector3 position(fitparams.getStateVector()(posindexmother + 0),
                      fitparams.getStateVector()(posindexmother + 1),
                      fitparams.getStateVector()(posindexmother + 2));
    TVector3 momentum(fitparams.getStateVector()(momindex + 0),
                      fitparams.getStateVector()(momindex + 1),
                      fitparams.getStateVector()(momindex + 2));
    CLHEP::HepMatrix jacobian(5, 6);


#ifdef HELIX_TEST
    //Test the Helix functions
    HelixUtils::helixTest();
#endif
    // translate into trackparameters
    Belle2::Helix helix;
    double flt;
#ifndef NUMERICAL_JACOBIAN
    HelixUtils::helixFromVertex(position, momentum, charge(), m_bfield, helix, flt, jacobian);
#else
    HelixUtils::helixFromVertexNumerical(position, momentum, charge(), m_bfield, helix, flt, jacobian);
#endif
    // get the measured track parameters at the poca to the mother
    if (!m_cached) {
      RecoTrack* nonconst =  const_cast<RecoTrack*>(this);
      if (m_flt == 0) {nonconst->updFltToMother(fitparams);}
      nonconst->updateParams(m_flt);
    }
    Eigen::Matrix<double, 1, 5> helixpars(5);
    helixpars(0) = helix.getD0();
    helixpars(1) = helix.getPhi0();
    helixpars(2) = helix.getOmega();
    helixpars(3) = helix.getZ0();
    helixpars(4) = helix.getTanLambda();
    // fill the residual and cov matrix
    //
    p.getResiduals().segment(0, 5) = helixpars - m_params;//.segment(0, 5);
    p.getResiduals()(1) = HelixUtils::phidomain(p.getResiduals()(1));
    EigenTypes::MatrixXd writtenCov = m_covariance.block<5, 5>(0, 0);

    p.getV().triangularView<Eigen::Lower>() =  writtenCov.triangularView<Eigen::Lower>();
//  k p.getV().triangularView<Eigen::Lower>() =  m_covariance.triangularView<Eigen::Lower>();
    // FIX ME: bring z0 residual in the correct domain --> this needs some thinking
    // calculate the full projection matrix from the jacobian
    // assumes that H is reset !
    //p.getH().block<5, 3>(0, posindexmother) = jacobian.block<5, 3>(0, 0);
    //p.getH().block<5, 3>(0, momindex)       = jacobian.block<5, 3>(0, 3);
    for (int row = 0; row < 5; ++row) {
      // the position
      for (int col = 0; col < 3; ++col) {
//        std::cout << "WRITING INTO H pos" << jacobian(row, col)  << std::endl;
        p.getH()(row, posindexmother + col) = jacobian(row + 1, col + 1);
      }
      // the momentum
      for (int col = 0; col < 3; ++col) {
//        std::cout << "WRITING INTO H mom" << jacobian(row, col + 3)  << std::endl;
        p.getH()(row, momindex + col) = jacobian(row + 1, col + 4);
      }
    }
    //std::cout << "projected track residuals \n" << p.getResiduals().transpose() << std::endl;
    //std::cout << "projected track p.getH()\n" << p.getH() << std::endl;
    return status;
  }
  ErrCode  RecoTrack::projectRecoConstraintCopy(const FitParams& fitparams, Projection& p) const
  {
    B2DEBUG(85, "----------           RecoTrack::projectRecoConstraintCopy\n");
    ErrCode status;
    int posindexmother = mother()->posIndex();
    int momindex = momIndex();

    Eigen::Matrix<double, 1, 6> positionAndMom = Eigen::Matrix<double, 1, 6>::Zero(1, 6);
    positionAndMom.segment(0, 3) =  fitparams.getStateVector().segment(posindexmother, 3);
    positionAndMom.segment(3, 3) =  fitparams.getStateVector().segment(momindex, 3);
    Eigen::Matrix<double, 5, 6> jacobian = Eigen::Matrix<double, 5, 6>::Zero(5, 6);
#ifdef HELIX_TEST
    //Test the Helix functions
    HelixUtils::helixTest();
#endif
    // translate into trackparameters
    Belle2::Helix helix;
#ifndef NUMERICAL_JACOBIAN
    double flt;
    HelixUtils::helixFromVertex(position, momentum, charge(), m_bfield, helix, flt, jacobian);

#else
    HelixUtils::getHelixAndJacobianFromVertexNumerical(positionAndMom, charge(), m_bfield, helix, jacobian);
#endif
    // get the measured track parameters at the poca to the mother
    if (!m_cached) {
      RecoTrack* nonconst =  const_cast<RecoTrack*>(this);
      if (m_flt == 0) {nonconst->updFltToMother(fitparams);}
      nonconst->updateParams(m_flt);
    }
    Eigen::Matrix<double, 1, 5> helixpars(5);
    helixpars(0) = helix.getD0();
    helixpars(1) = helix.getPhi0();
    helixpars(2) = helix.getOmega();
    helixpars(3) = helix.getZ0();
    helixpars(4) = helix.getTanLambda();

    // fill the residual and cov matrix
    p.getResiduals().segment(0, 5) = helixpars - m_params;//.segment(0, 5);
    p.getResiduals()(1) = HelixUtils::phidomain(p.getResiduals()(1));
    EigenTypes::MatrixXd writtenCov = m_covariance.block<5, 5>(0, 0);

    p.getV().triangularView<Eigen::Lower>() =  writtenCov.triangularView<Eigen::Lower>();
    for (int row = 0; row < 5; ++row) {
      // the position
      for (int col = 0; col < 3; ++col) {
        p.getH()(row, posindexmother + col) = jacobian(row, col);
      }
      // the momentum
      for (int col = 0; col < 3; ++col) {
        p.getH()(row, momindex + col) = jacobian(row , col + 3);
      }
    }
    return status;
  }



}//end recotrack
