/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn, Fabian Krinner     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/UncertainHelix.h>
#include <mdst/dataobjects/MCParticle.h>

#include <analysis/VertexFitting/TreeFitter/InternalTrack.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/HelixUtils.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>

using std::vector;

namespace TreeFitter {
  constexpr double pi    = TMath::Pi();
  constexpr double twoPi = TMath::TwoPi();

  InternalTrack::InternalTrack(Belle2::Particle*  particle,
                               const ParticleBase* mother,
                               const ConstraintConfiguration& config,
                               bool forceFitAll,
                               bool noEnergySum,
                               bool forceMassConstraint
                              ) :
    ParticleBase(particle, mother, &config), // config pointer here to allow final states not to have it
    m_massconstraint(false),
    m_noEnergySum(noEnergySum),
    m_isconversion(false),
    m_bfield(0.)
  {
    m_bfield = Belle2::BFieldManager::getField(TVector3(0, 0, 0)).Z() / Belle2::Unit::T; //Bz in Tesla

    if (particle) {
      for (Belle2::Particle* daughter : particle->getDaughters()) {
        addDaughter(daughter, config, forceFitAll);
      }
    } else {
      B2ERROR("Trying to create an InternalTrack from NULL. This should never happen.");
    }
    m_massconstraint = std::find(config.m_massConstraintListPDG.begin(), config.m_massConstraintListPDG.end(),
                                 std::abs(m_particle->getPDGCode())) != config.m_massConstraintListPDG.end();
    if (forceMassConstraint) {
      m_massconstraint = true;
    }
  }

  bool InternalTrack::compTrkTransverseMomentum(const RecoTrack* lhs, const RecoTrack* rhs)
  {
    return lhs->particle()->getMomentum().Perp() > rhs->particle()->getMomentum().Perp();
  }

  ErrCode InternalTrack::initMotherlessParticle(FitParams& fitparams)
  {
    ErrCode status;
    for (ParticleBase* daughter : m_daughters)

    {
      status |= daughter->initMotherlessParticle(fitparams);
    }
    int posindex = posIndex();
    // FIXME update this and check if this position is already initialised
    // lower stream vertices might be better

    if (hasPosition()) {
      fitparams.getStateVector().segment(posindex, 3) = Eigen::Matrix<double, 3, 1>::Zero(3);
      TVector3 vtx = particle()->getVertex();
      if (vtx.Mag2()) {
        // Checking Mag2() instead of Mag() avoids a Sqrt(...)
        fitparams.getStateVector()(posindex) = vtx.X();
        fitparams.getStateVector()(posindex + 1) = vtx.Y();
        fitparams.getStateVector()(posindex + 2) = vtx.Z();
      } else {
        // Init containers
        vector<ParticleBase*> alldaughters;
        vector<ParticleBase*> vtxdaughters;
        vector<RecoTrack*>    trkdaughters;

        ParticleBase::collectVertexDaughters(alldaughters, posindex);

        for (ParticleBase* daughter : alldaughters) {
          if (daughter->type() == ParticleBase::TFParticleType::kRecoTrack) {
            trkdaughters.push_back(static_cast<RecoTrack*>(daughter));
          } else if (daughter->hasPosition()  && fitparams.getStateVector()(daughter->posIndex()) != 0) {
            vtxdaughters.push_back(daughter);
          }
        }

        if (trkdaughters.size() >= 2) {
          std::sort(trkdaughters.begin(), trkdaughters.end(), compTrkTransverseMomentum);

          RecoTrack* dau1 = trkdaughters[0];
          RecoTrack* dau2 = trkdaughters[1];

          Belle2::Helix helix1 = dau1->particle()->getTrack()->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(std::abs(
                                   dau1->particle()->getPDGCode())))->getHelix();
          Belle2::Helix helix2 = dau2->particle()->getTrack()->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(std::abs(
                                   dau2->particle()->getPDGCode())))->getHelix();

          double flt1(0), flt2(0);
          TVector3 v;
          HelixUtils::helixPoca(helix1, helix2, flt1, flt2, v, m_isconversion);

          fitparams.getStateVector()(posindex)     = v.x();
          fitparams.getStateVector()(posindex + 1) = v.y();
          fitparams.getStateVector()(posindex + 2) = v.z();

          dau1->setFlightLength(flt1);
          dau2->setFlightLength(flt2);

        } else if (mother() && mother()->posIndex() >= 0) {
          const int posindexmother = mother()->posIndex();
          const int dim = m_config->m_originDimension; //TODO access mother
          fitparams.getStateVector().segment(posindex, dim) = fitparams.getStateVector().segment(posindexmother, dim);
        } else {
          /** (0,0,0) is the best guess in any other case */
          fitparams.getStateVector().segment(posindex, 3) = Eigen::Matrix<double, 1, 3>::Zero(3);
        }
      }
    }

    for (ParticleBase* daughter : m_daughters) {
      daughter->initParticleWithMother(fitparams);
    }

    initMomentum(fitparams);
    return status;
  }

  ErrCode InternalTrack::initParticleWithMother(FitParams& fitparams)
  {
    const int posindex = posIndex();
    if (mother() &&
        fitparams.getStateVector()(posindex) == 0 &&
        fitparams.getStateVector()(posindex + 1) == 0 && \
        fitparams.getStateVector()(posindex + 2) == 0) {
      const int posIndexMother = mother()->posIndex();
      const int dim = m_config->m_originDimension; // TODO access mother (copied from InternalParticle.cc)
      fitparams.getStateVector().segment(posindex, dim) = fitparams.getStateVector().segment(posIndexMother, dim);
    }
    return initTau(fitparams); // This will never have Tau (copy anyway from InternalParticle.cc)
  }

  ErrCode InternalTrack::initMomentum(FitParams& fitparams) const
  {
    int momindex = momIndex();
    fitparams.getStateVector().segment(momindex, 4) = Eigen::Matrix<double, 4, 1>::Zero(4);

    for (ParticleBase* daughter : m_daughters) {
      int daumomindex = daughter->momIndex();
      int maxrow = daughter->hasEnergy() ? 4 : 3;

      fitparams.getStateVector().segment(momindex, maxrow) += fitparams.getStateVector().segment(daumomindex, maxrow);

      if (maxrow == 3) {
        double mass     = daughter->pdgMass();
        double pSquared = fitparams.getStateVector().segment(daumomindex, maxrow).squaredNorm();
        fitparams.getStateVector()(momindex + 3) += std::sqrt(pSquared + mass * mass);
      }
    }
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode InternalTrack::initCovariance(FitParams& fitparams) const
  {
    ErrCode status;
    const int posindex = posIndex();
    if (posindex >= 0) {
      for (int i = 0; i < 3; ++i) {
        fitparams.getCovariance()(posindex + i, posindex + i) = 1;
      }
    }

    const int momindex = momIndex();
    if (momindex >= 0) {
      const int maxrow = hasEnergy() ? 4 : 3;
      for (int i = 0; i < maxrow; ++i) {
        fitparams.getCovariance()(momindex + i, momindex + i) = .1;
      }
    }
    for (ParticleBase* daughter : m_daughters) {
      status |= daughter->initCovariance(fitparams);
    }
    return status;
  }

  void InternalTrack::addToConstraintList(constraintlist& list,
                                          int depth) const
  {
    list.push_back(Constraint(this, Constraint::helix, depth, 5, 5));

    if (m_massconstraint) {
      list.push_back(Constraint(this, Constraint::mass, depth, 1, 3));
    }

    for (ParticleBase* daughter : m_daughters) {
      daughter->addToConstraintList(list, depth - 1);
    }
  }

  ErrCode InternalTrack::projectConstraint(const Constraint::Type type,
                                           const FitParams&       fitparams,
                                           Projection&            p) const
  {
    ErrCode status;
    switch (type) {
      case Constraint::helix:
        status |= projectHelixConstraint(fitparams, p);
        break;
      case Constraint::mass:
        status |= projectMassConstraint(fitparams, p);
        break;
      default:
        status |= ParticleBase::projectConstraint(type, fitparams, p);
    }
    return status;
  }

  ErrCode InternalTrack::projectHelixConstraint(const FitParams& fitparams,
                                                // cppcheck-suppress constParameter ; projection p is in fact set in this function
                                                Projection&       p) const
  {
    ErrCode status;
    const int posIndexMother = mother()->posIndex();
    const int momindex       = momIndex();
    const int posindex       = posIndex();
    Eigen::Matrix<double, 1, 6> positionAndMomentumIn  = Eigen::Matrix<double, 1, 6>::Zero(1, 6);
    Eigen::Matrix<double, 1, 6> positionAndMomentumOut = Eigen::Matrix<double, 1, 6>::Zero(1, 6);

    positionAndMomentumIn.segment(0, 3)  = fitparams.getStateVector().segment(posIndexMother, 3);
    positionAndMomentumIn.segment(3, 3)  = fitparams.getStateVector().segment(momindex, 3);
    positionAndMomentumOut.segment(0, 3) = fitparams.getStateVector().segment(posindex, 3);
    for (ParticleBase* daughter : m_daughters) {
      const int momIndexDaughter = daughter->momIndex();
      positionAndMomentumOut.segment(3, 3) += fitparams.getStateVector().segment(momIndexDaughter, 3);
    }

    TVector3 vertexPosition(positionAndMomentumIn(0), positionAndMomentumIn(1), positionAndMomentumIn(2));
    TVector3 momentum(positionAndMomentumIn(3), positionAndMomentumIn(4), positionAndMomentumIn(5));

    TMatrixDSym carthesianCovariance(6);
    for (size_t i = 0 ; i < 3; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        carthesianCovariance(i  , j) = fitparams.getCovariance()(posindex + i, posindex + j);
        carthesianCovariance(i  , j + 3) = fitparams.getCovariance()(posindex + i, momindex + j);
        carthesianCovariance(i + 3, j) = fitparams.getCovariance()(momindex + i, posindex + j);
        carthesianCovariance(i + 3, j + 3) = fitparams.getCovariance()(momindex + i, momindex + j);
      }
    }

    Belle2::UncertainHelix helixIn(
      vertexPosition,
      momentum,
      charge(),
      m_bfield,
      carthesianCovariance,
      1.
    );
    // Set up covariance matrix
    Eigen::Matrix<double, 5, 5> covarianceMatrix = Eigen::Matrix<double, 5, 5>::Zero(5, 5);
    TMatrixDSym covarianceMatrixROOT = helixIn.getCovariance();
    for (int i = 0; i < 5; ++i) {
      for (int j = 0; j < 5; ++j) {
        covarianceMatrix(i, j) = covarianceMatrixROOT(i, j);
      }
    }

    Belle2::Helix helixOut = Belle2::Helix(
                               TVector3(positionAndMomentumOut(0), positionAndMomentumOut(1), positionAndMomentumOut(2)),
                               TVector3(positionAndMomentumOut(3), positionAndMomentumOut(4), positionAndMomentumOut(5)),
                               charge(),
                               m_bfield
                             );

    Eigen::Matrix<double, 5, 6> jacobian = Eigen::Matrix<double, 5, 6>::Zero(5, 6);
    HelixUtils::getJacobianToCartesianFrameworkHelix(
      jacobian,
      positionAndMomentumIn(0),
      positionAndMomentumIn(1),
      positionAndMomentumIn(2),
      positionAndMomentumIn(3),
      positionAndMomentumIn(4),
      positionAndMomentumIn(5),
      m_bfield,
      charge()
    );

    p.getResiduals().segment(0, 5)(0) = helixIn.getD0()        - helixOut.getD0();
    p.getResiduals().segment(0, 5)(1) = helixIn.getPhi0()      - helixOut.getPhi0();
    p.getResiduals().segment(0, 5)(2) = helixIn.getOmega()     - helixOut.getOmega();
    p.getResiduals().segment(0, 5)(3) = helixIn.getZ0()        - helixOut.getZ0();
    p.getResiduals().segment(0, 5)(4) = helixIn.getTanLambda() - helixOut.getTanLambda();

    //account for periodic boundary in phi residual
    double phiResidual = p.getResiduals().segment(0, 5)(1);
    phiResidual = std::fmod(phiResidual + pi, twoPi);
    if (phiResidual < 0) {
      phiResidual += twoPi;
    }
    phiResidual -= pi;
    p.getResiduals().segment(0, 5)(1) = phiResidual;

    p.getV().triangularView<Eigen::Lower>() = covarianceMatrix.triangularView<Eigen::Lower>();
    p.getH().block<5, 3>(0, posIndexMother) = -1.0 * jacobian.block<5, 3>(0, 0);
    p.getH().block<5, 3>(0, momindex)       = -1.0 * jacobian.block<5, 3>(0, 3);
    return status;
  }

  bool InternalTrack::hasPosition() const
  {
    // does this particle have a position index (decayvertex)
    // true in any case
    return true;
  }

  bool InternalTrack::hasEnergy() const
  {
    return !m_massconstraint && !m_noEnergySum;
  }

  int InternalTrack::posIndex() const
  {
    return index();
  }

  int InternalTrack::momIndex() const
  {
    return index() + 3;
  }

  int InternalTrack::dim() const
  {
    return hasEnergy() ? 7 : 6;
  }

  int InternalTrack::tauIndex() const
  {
    return -1;
  }

  void InternalTrack::forceP4Sum(FitParams& fitparams) const
  {
    for (ParticleBase* daughter : m_daughters) {
      daughter->forceP4Sum(fitparams);
    }
    const int posindex = posIndex();
    Eigen::Matrix<double, 1, 6> positionAndMomentumOut = Eigen::Matrix<double, 1, 6>::Zero(1, 6);
    positionAndMomentumOut.segment(0, 3)              = fitparams.getStateVector().segment(posindex, 3);
    double Energy = 0.;
    for (ParticleBase* daughter : m_daughters) {
      const int momIndexDaughter = daughter->momIndex();
      positionAndMomentumOut.segment(3, 3) += fitparams.getStateVector().segment(momIndexDaughter, 3);
      Energy += fitparams.getStateVector()(momIndexDaughter + 3);
    }
    Belle2::Helix helixOut = Belle2::Helix(
                               TVector3(positionAndMomentumOut(0), positionAndMomentumOut(1), positionAndMomentumOut(2)),
                               TVector3(positionAndMomentumOut(3), positionAndMomentumOut(4), positionAndMomentumOut(5)),
                               charge(),
                               m_bfield
                             );
    const int posIndexMother = mother()->posIndex();
    double x                 = fitparams.getStateVector()(posIndexMother);
    double y                 = fitparams.getStateVector()(posIndexMother + 1);
    double arcLength2D       = helixOut.getArcLength2DAtXY(x, y);
    TVector3 pAtProduction   = helixOut.getMomentumAtArcLength2D(arcLength2D, m_bfield);

    const int momindex = momIndex();
    fitparams.getStateVector()(momindex) = pAtProduction.X();
    fitparams.getStateVector()(momindex + 1) = pAtProduction.Y();
    fitparams.getStateVector()(momindex + 2) = pAtProduction.Z();
    if (m_noEnergySum) {
      double newEnergy = sqrt(pdgMass() * pdgMass() + pAtProduction.Mag2());
      Energy = newEnergy;
    }
    fitparams.getStateVector()(momindex + 3) = Energy;
  }

};
