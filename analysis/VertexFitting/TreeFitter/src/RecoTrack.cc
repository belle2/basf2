/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>

#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/Track.h>

#include <TMath.h>

namespace TreeFitter {
  constexpr double pi = TMath::Pi();
  constexpr double twoPi = TMath::TwoPi();

  RecoTrack::RecoTrack(Belle2::Particle* particle, const ParticleBase* mother) :
    RecoParticle(particle, mother),
    m_bfield(0),
    m_trackfit(particle->getTrackFitResult()),
    m_cached(false),
    m_flt(0),
    m_params(5),
    m_covariance(5, 5),
    m_momentumScalingFactor(particle->getMomentumScalingFactor())
  {
    m_bfield = Belle2::BFieldManager::getFieldInTesla(Belle2::B2Vector3D(0, 0, 0)).Z(); //Bz in Tesla
    m_covariance = Eigen::Matrix<double, 5, 5>::Zero(5, 5);
  }

  ErrCode RecoTrack::initParticleWithMother(FitParams& fitparams)
  {
    //initPar2
    if (m_flt == 0) {
      const_cast<RecoTrack*>(this)->updFltToMother(fitparams);
    }
    Belle2::B2Vector3D recoP = m_trackfit->getHelix(m_momentumScalingFactor).getMomentumAtArcLength2D(m_flt, m_bfield);
    const int momindex = momIndex();
    fitparams.getStateVector()(momindex) = recoP.X();
    fitparams.getStateVector()(momindex + 1) = recoP.Y();
    fitparams.getStateVector()(momindex + 2) = recoP.Z();
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoTrack::initMotherlessParticle([[gnu::unused]] FitParams& fitparams)
  {
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode RecoTrack::initCovariance(FitParams& fitparams) const
  {
    // we only need a rough estimate of the covariance
    TMatrixFSym p4Err = particle()->getMomentumErrorMatrix();
    const int momindex = momIndex();

    for (int row = 0; row < 3; ++row) {
      fitparams.getCovariance()(momindex + row, momindex + row) = 1000 * p4Err[row][row];
    }

    return ErrCode(ErrCode::Status::success);
  }


  ErrCode RecoTrack::updFltToMother(const FitParams& fitparams)
  {
    const int posindexmother = mother()->posIndex();
    m_flt = m_trackfit->getHelix(m_momentumScalingFactor).getArcLength2DAtXY(
              fitparams.getStateVector()(posindexmother),
              fitparams.getStateVector()(posindexmother + 1));
    return ErrCode(ErrCode::Status::success);
  } ;

  ErrCode RecoTrack::updateParams(double flt)
  {
    m_flt = flt;
    std::vector<float> trackParameter = m_trackfit->getTau();
    for (unsigned int i = 0; i < trackParameter.size(); ++i) {
      m_params(i) = trackParameter[i];
      if (i == 2) m_params(2) /= m_momentumScalingFactor; // index 2 is the curvature of the track
    }
    TMatrixDSym cov = m_trackfit->getCovariance5();
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
                            Belle2::B2Vector3D(
                              positionAndMom(0),
                              positionAndMom(1),
                              positionAndMom(2)),
                            Belle2::B2Vector3D(
                              positionAndMom(3),
                              positionAndMom(4),
                              positionAndMom(5)),
                            charge(),
                            m_bfield
                          );

    HelixUtils::getJacobianToCartesianFrameworkHelix(jacobian,
                                                     positionAndMom(0),
                                                     positionAndMom(1),
                                                     positionAndMom(2),
                                                     positionAndMom(3),
                                                     positionAndMom(4),
                                                     positionAndMom(5),
                                                     m_bfield,
                                                     charge()
                                                    );
    if (!m_cached) {
      auto* nonconst =  const_cast<RecoTrack*>(this);
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

    //account for periodic boundary in phi residual
    double phiResidual = p.getResiduals().segment(0, 5)(1);
    phiResidual = std::fmod(phiResidual + pi, twoPi);
    if (phiResidual < 0) phiResidual += twoPi;
    phiResidual -= pi;
    p.getResiduals().segment(0, 5)(1) = phiResidual;

    p.getV().triangularView<Eigen::Lower>() =  m_covariance.triangularView<Eigen::Lower>();

    p.getH().block<5, 3>(0, posindexmother) = -1.0 * jacobian.block<5, 3>(0, 0);
    p.getH().block<5, 3>(0, momindex) = -1.0 * jacobian.block<5, 3>(0, 3);

    return status;
  }

}//end recotrack
