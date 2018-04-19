/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <TMath.h>

#define NUMERICAL_JACOBIAN

namespace TreeFitter {

  RecoTrack::RecoTrack(Belle2::Particle* particle, const ParticleBase* mother) :
    RecoParticle(particle, mother),
    m_bfield(0),
    m_trackfit(0),
    m_cached(false),
    m_flt(0),
    m_params(5),
    m_covariance(5, 5)
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

  ErrCode RecoTrack::initParticleWithMother(FitParams* fitparams)
  {

    //initPar2
    if (m_flt == 0) {
      const_cast<RecoTrack*>(this)->updFltToMother(*fitparams);
    }
    TVector3 recoP = m_trackfit->getHelix().getMomentumAtArcLength2D(m_flt, m_bfield);
    const int momindex = momIndex();
    fitparams->getStateVector()(momindex) = recoP.X();
    fitparams->getStateVector()(momindex + 1) = recoP.Y();
    fitparams->getStateVector()(momindex + 2) = recoP.Z();
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoTrack::initMotherlessParticle([[gnu::unused]] FitParams* fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoTrack::initCovariance(FitParams* fitparams) const
  {
    // we only need a rough estimate of the covariance
    TMatrixFSym p4Err = particle()->getMomentumErrorMatrix();
    const int momindex = momIndex();

    for (int row = 0; row < 3; ++row) {
      //B2DEBUG(19, "RecoTrack::initCovariance writing :" << 1000 * p4Err[row][row]);
      fitparams->getCovariance()(momindex + row, momindex + row) = 1000 * p4Err[row][row];
    }

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoTrack::updFltToMother(const FitParams& fitparams)
  {
    const int posindexmother = mother()->posIndex();
    //FT: is this correct? Might need to make it 3D
    m_flt = m_trackfit->getHelix().getArcLength2DAtXY(
              fitparams.getStateVector()(posindexmother),
              fitparams.getStateVector()(posindexmother + 1));
    // FIX ME: use helix poca to get estimate of flightlength first
    return ErrCode(ErrCode::Status::success);
  } ;

  ErrCode RecoTrack::updateParams(double flt)
  {
    m_flt = flt;
    std::vector<float> trackParameter = m_trackfit->getTau();
    //JFK: FIXME this is castable but I dont get how Eigen::VectorXf::map works 2017-09-26
    for (unsigned int i = 0; i < trackParameter.size(); ++i) {
      m_params(i) = trackParameter[i];
    }
    // FIX ME: bring z0 in the correct domain ...
    TMatrixDSym cov = m_trackfit->getCovariance5();
    //FIXME is this symmetric?
    for (int row = 0; row < 5; ++row) {
      for (int col = 0; col <= row; ++col) {
        m_covariance(row, col) = cov[row][col];
      }
    }

    m_cached = true;
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoTrack::projectRecoConstraint(const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    const int posindexmother = mother()->posIndex();
    const int momindex = momIndex();

    Eigen::Matrix<double, 1, 6> positionAndMom = Eigen::Matrix<double, 1, 6>::Zero(1, 6);
    positionAndMom.segment(0, 3) = fitparams.getStateVector().segment(posindexmother, 3);
    positionAndMom.segment(3, 3) = fitparams.getStateVector().segment(momindex, 3);
    Eigen::Matrix<double, 5, 6> jacobian = Eigen::Matrix<double, 5, 6>::Zero(5, 6);

    Belle2::Helix helix = Belle2::Helix(
                            TVector3(
                              positionAndMom(0),
                              positionAndMom(1),
                              positionAndMom(2)),
                            TVector3(
                              positionAndMom(3),
                              positionAndMom(4),
                              positionAndMom(5)),
                            charge(),
                            m_bfield
                          );

#ifndef NUMERICAL_JACOBIAN
    double flt;
    HelixUtils::helixFromVertex(position, momentum, charge(), m_bfield, helix, flt, jacobian);
#else
    HelixUtils::getJacobianFromVertexNumerical(positionAndMom, charge(), m_bfield, helix, jacobian, 1e-5);
    //HelixUtils::getHelixAndJacobianFromVertexNumerical(positionAndMom, charge(), m_bfield, helix, jacobian);
#endif

    // get the measured track parameters at the poca to the mother
    if (!m_cached) {
      RecoTrack* nonconst =  const_cast<RecoTrack*>(this);
      if (m_flt == 0) { nonconst->updFltToMother(fitparams); }
      nonconst->updateParams(m_flt);
    }

    Eigen::Matrix<double, 1, 5> helixpars(5);
    helixpars(0) = helix.getD0();
    helixpars(1) = helix.getPhi0();
    helixpars(2) = helix.getOmega();
    helixpars(3) = helix.getZ0();
    helixpars(4) = helix.getTanLambda();

    p.getResiduals().segment(0, 5) = m_params - helixpars;

    p.getResiduals()(1) = HelixUtils::phidomain(p.getResiduals()(1));

    Eigen::Matrix<double, 5, 5> writtenCov = m_covariance.block<5, 5>(0, 0);

    p.getV().triangularView<Eigen::Lower>() =  writtenCov.triangularView<Eigen::Lower>();

    //dr/dx
    for (int row = 0; row < 5; ++row) {
      // the position
      for (int col = 0; col < 3; ++col) {
        p.getH()(row, posindexmother + col) = -1.0 * jacobian(row, col);
      }
      // the momentum
      for (int col = 0; col < 3; ++col) {
        p.getH()(row, momindex + col) = -1.0 * jacobian(row , col + 3);
      }
    }
    return status;
  }

}//end recotrack
