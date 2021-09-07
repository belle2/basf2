/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: Wouter Hulsbergen                                *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <TMath.h>
#include <Math/Vector4D.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <analysis/VertexFitting/TreeFitter/FitManager.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/DecayChain.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

namespace TreeFitter {

  FitManager::FitManager(Belle2::Particle* particle,
                         const ConstraintConfiguration& config,
                         double prec,
                         bool updateDaughters,
                         const bool useReferencing
                        ) :
    m_particle(particle),
    m_decaychain(nullptr),
    m_status(VertexStatus::UnFitted),
    m_chiSquare(-1),
    m_niter(-1),
    m_prec(prec),
    m_updateDaugthers(updateDaughters),
    m_ndf(0),
    m_fitparams(nullptr),
    m_useReferencing(useReferencing),
    m_config(config)
  {
    m_decaychain =  new DecayChain(particle,
                                   config,
                                   false
                                  );
    m_fitparams  = new FitParams(m_decaychain->dim());
  }

  FitManager::~FitManager()
  {
    delete m_decaychain;
    delete m_fitparams;
  }

  void FitManager::setExtraInfo(Belle2::Particle* part, const std::string& name, const double value) const
  {
    if (part) {
      if (part->hasExtraInfo(name)) {
        part->setExtraInfo(name, value);
      } else {
        part->addExtraInfo(name, value);
      }
    }
  }

  bool FitManager::fit()
  {
    const int nitermax = 100;
    const int maxndiverging = 3;
    double dChisqConv = m_prec;
    m_chiSquare = -1;
    m_errCode.reset();

    if (m_status == VertexStatus::UnFitted) {
      m_errCode = m_decaychain->initialize(*m_fitparams);
    }

    if (m_errCode.failure()) {
      m_status = VertexStatus::BadInput;
    } else {
      m_status = VertexStatus::UnFitted;
      int ndiverging = 0;
      bool finished = false;
      for (m_niter = 0; m_niter < nitermax && !finished; ++m_niter) {
        if (0 == m_niter) {
          m_errCode = m_decaychain->filter(*m_fitparams);
        } else if (m_niter > 0 && m_useReferencing) {
          auto* tempState = new FitParams(*m_fitparams);
          m_errCode = m_decaychain->filterWithReference(*m_fitparams, *tempState);
          delete tempState;
        }
        m_ndf = nDof();
        double chisq = m_fitparams->chiSquare();
        double deltachisq = chisq - m_chiSquare;
        if (m_errCode.failure()) {
          finished = true ;
          m_status = VertexStatus::Failed;
          setExtraInfo(m_particle, "failed", 1);
        } else {
          if (m_niter > 0) {
            if ((std::abs(deltachisq) / m_chiSquare < dChisqConv)) {
              m_chiSquare = chisq;
              m_status = VertexStatus::Success;
              finished = true ;
              setExtraInfo(m_particle, "failed", 0);
            } else if (deltachisq > 0 && ++ndiverging >= maxndiverging) {
              setExtraInfo(m_particle, "failed", 2);
              m_status = VertexStatus::NonConverged;
              m_errCode = ErrCode(ErrCode::Status::slowdivergingfit);
              finished = true ;
            } else if (deltachisq > 0) {
            }
          }
          if (deltachisq < 0) {
            ndiverging = 0;
          }
          m_chiSquare = chisq;
        }
      }
      if (m_niter == nitermax && m_status != VertexStatus::Success) {
        setExtraInfo(m_particle, "failed", 3);
        m_status = VertexStatus::NonConverged;
      }
      if (!(m_fitparams->testCovariance())) {
        setExtraInfo(m_particle, "failed", 4);
        m_status = VertexStatus::Failed;
      }
    }

    if (m_status == VertexStatus::Success) {
      // mass constraints comes after kine so we have to
      // update the mothers with the values set by the mass constraint
      if (m_config.m_massConstraintListPDG.size() != 0) {
        m_decaychain->locate(m_particle)->forceP4Sum(*m_fitparams);
      }
      updateTree(*m_particle, true);
    }

    return (m_status == VertexStatus::Success);
  }

  int FitManager::nDof() const
  {
    return m_fitparams->nDof();
  }

  int FitManager::posIndex(Belle2::Particle* particle) const
  {
    return m_decaychain->posIndex(particle);
  }

  int FitManager::momIndex(Belle2::Particle* particle) const
  {
    return m_decaychain->momIndex(particle);
  }

  int FitManager::tauIndex(Belle2::Particle* particle) const
  {
    return m_decaychain->tauIndex(particle);
  }

  // cppcheck-suppress constParameter ; returncov is clearly changed in the function
  void FitManager::getCovFromPB(const ParticleBase* pb, TMatrixFSym& returncov) const
  {

    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> cov = m_fitparams->getCovariance().selfadjointView<Eigen::Lower>();
    int posindex = pb->posIndex();
    // hack: for tracks and photons, use the production vertex
    if (posindex < 0 && pb->mother()) {
      posindex = pb->mother()->posIndex();
    }
    int momindex = pb->momIndex();
    if (pb->hasEnergy()) {
      // if particle has energy, get full p4 from fitparams and put them directly in the return type
      // very important! Belle2 uses p,E,x! Change order here!
      for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
          returncov(row, col) = cov(momindex + row, momindex + col);
        }
      }

      for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
          returncov(row + 4, col + 4) = cov(posindex + row, posindex + col);
        }
      }

    } else {
      Eigen::Matrix<double, 6, 6> cov6 =
        Eigen::Matrix<double, 6, 6>::Zero(6, 6);

      for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
          cov6(row, col) = cov(momindex + row, momindex + col);
          cov6(row + 3, col + 3) = cov(posindex + row, posindex + col);
        }
      }

      double mass = pb->pdgMass();
      Eigen::Matrix<double, 3, 1> momVec =
        m_fitparams->getStateVector().segment(momindex, 3);

      double energy2 = momVec.transpose() * momVec;
      energy2 += mass * mass;
      double energy = sqrt(energy2);

      Eigen::Matrix<double, 7, 6> jacobian =
        Eigen::Matrix<double, 7, 6>::Zero(7, 6);

      for (int col = 0; col < 3; ++col) {
        jacobian(col, col) = 1;
        jacobian(3, col) = m_fitparams->getStateVector()(momindex + col) / energy;
        jacobian(col + 4, col + 3) = 1;
      }

      Eigen::Matrix<double, 7, 7> cov7
        = jacobian * cov6.selfadjointView<Eigen::Lower>() * jacobian.transpose();

      for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 7; ++col) {
          returncov(row, col) = cov7(row, col);
        }
      }
    } // else
  }

  bool FitManager::updateCand(Belle2::Particle& cand, const bool isTreeHead) const
  {
    const ParticleBase* pb = m_decaychain->locate(&cand);
    if (pb) {
      updateCand(*pb, cand, isTreeHead);
    } else {
      B2ERROR("Can't find candidate " << cand.getName() << " in tree " << m_particle->getName());
    }
    return pb != nullptr;
  }

  void FitManager::updateCand(const ParticleBase& pb,
                              Belle2::Particle& cand, const bool isTreeHead) const
  {
    int posindex = pb.posIndex();
    if (posindex < 0 && pb.mother()) {
      posindex = pb.mother()->posIndex();
    }

    if (m_updateDaugthers || isTreeHead) {
      if (posindex >= 0) {
        const ROOT::Math::XYZVector pos(m_fitparams->getStateVector()(posindex),
                                        m_fitparams->getStateVector()(posindex + 1),
                                        m_fitparams->getStateVector()(posindex + 2));
        cand.setVertex(pos);
        if (&pb == m_decaychain->cand()) { // if head
          const double fitparchi2 = m_fitparams->chiSquare();
          cand.setPValue(TMath::Prob(fitparchi2, m_ndf));//if m_ndf<1, this is 0.
          setExtraInfo(&cand, "chiSquared", fitparchi2);
          setExtraInfo(&cand, "modifiedPValue", TMath::Prob(fitparchi2, 3));
          setExtraInfo(&cand, "ndf", m_ndf);
        }
        if (pb.mother()) {
          int motherPosIndex = pb.mother()->posIndex();
          if (motherPosIndex >= 0) {
            setExtraInfo(&cand, "prodVertexX", m_fitparams->getStateVector()(motherPosIndex));
            setExtraInfo(&cand, "prodVertexY", m_fitparams->getStateVector()(motherPosIndex + 1));
            if (pb.mother()->dim() > 2)
              setExtraInfo(&cand, "prodVertexZ", m_fitparams->getStateVector()(motherPosIndex + 2));
          }
        }
      }

      const int momindex = pb.momIndex();
      ROOT::Math::PxPyPzEVector p;
      p.SetPx(m_fitparams->getStateVector()(momindex));
      p.SetPy(m_fitparams->getStateVector()(momindex + 1));
      p.SetPz(m_fitparams->getStateVector()(momindex + 2));
      if (pb.hasEnergy()) {
        p.SetE(m_fitparams->getStateVector()(momindex + 3));
        cand.set4Vector(p);
      } else {
        const double mass = cand.getPDGMass();
        p.SetE(std::sqrt(p.P2() + mass * mass));
        cand.set4Vector(p);
      }
      TMatrixFSym cov7b2(7);
      getCovFromPB(&pb, cov7b2);
      cand.setMomentumVertexErrorMatrix(cov7b2);
    }

    if (pb.tauIndex() > 0) {
      std::tuple<double, double>tau  = getDecayLength(cand);
      std::tuple<double, double>life = getLifeTime(cand);
      setExtraInfo(&cand, std::string("decayLength"), std::get<0>(tau));
      setExtraInfo(&cand, std::string("decayLengthErr"), std::get<1>(tau));
      setExtraInfo(&cand, std::string("lifeTime"), std::get<0>(life));
      setExtraInfo(&cand, std::string("lifeTimeErr"), std::get<1>(life));
    }
  }

  void FitManager::updateTree(Belle2::Particle& cand, const bool isTreeHead) const
  {
    const bool updateableMother = updateCand(cand, isTreeHead);

    if (updateableMother and not cand.hasExtraInfo("bremsCorrected")) {
      const int ndaughters = cand.getNDaughters();
      for (int i = 0; i < ndaughters; i++) {
        auto* daughter = const_cast<Belle2::Particle*>(cand.getDaughter(i));
        updateTree(*daughter, false);
      }
    }
  }

  std::tuple<double, double> FitManager::getLifeTime(Belle2::Particle& cand) const
  {
    const ParticleBase* pb = m_decaychain->locate(&cand);

    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      const int momindex = pb->momIndex();
      const int tauIndex = pb->tauIndex();
      const Eigen::Matrix<double, 1, 3> mom_vec = m_fitparams->getStateVector().segment(momindex, 3);

      const Eigen::Matrix<double, 3, 3> mom_cov = m_fitparams->getCovariance().block<3, 3>(momindex, momindex);
      Eigen::Matrix<double, 4, 4> comb_cov =  Eigen::Matrix<double, 4, 4>::Zero(4, 4);

      std::tuple<double, double> lenTuple  = getDecayLength(cand);

      const double lenErr = std::get<1>(lenTuple);
      comb_cov(0, 0) = lenErr * lenErr;
      comb_cov(1, 0) = m_fitparams->getCovariance()(momindex, tauIndex);
      comb_cov(2, 0) = m_fitparams->getCovariance()(momindex + 1, tauIndex);
      comb_cov(3, 0) = m_fitparams->getCovariance()(momindex + 2, tauIndex);

      comb_cov.block<3, 3>(1, 1) = mom_cov;

      const double mass = pb->pdgMass();
      const double mBYc = mass / Belle2::Const::speedOfLight;
      const double mom = mom_vec.norm();
      const double mom3 = mom * mom * mom;

      const double len = std::get<0>(lenTuple);
      const double t = len / mom * mBYc;

      Eigen::Matrix<double, 1, 4> jac = Eigen::Matrix<double, 1, 4>::Zero();
      jac(0) =  1. / mom * mBYc;
      jac(1) = -1. * len * mom_vec(0) / mom3 * mBYc;
      jac(2) = -1. * len * mom_vec(1) / mom3 * mBYc;
      jac(3) = -1. * len * mom_vec(2) / mom3 * mBYc;

      const double tErr2 = jac * comb_cov.selfadjointView<Eigen::Lower>() * jac.transpose();
      // time in nanosec
      return std::make_tuple(t, std::sqrt(tErr2));
    }
    return std::make_tuple(-999, -999);
  }

  std::tuple<double, double> FitManager::getDecayLength(const ParticleBase* pb) const
  {
    // returns the decaylength in the lab frame
    return getDecayLength(pb, *m_fitparams);
  }

  std::tuple<double, double> FitManager::getDecayLength(const ParticleBase* pb, const FitParams& fitparams) const
  {
    if (pb->tauIndex() >= 0 && pb->mother()) {
      const int tauindex = pb->tauIndex();
      const double len = fitparams.getStateVector()(tauindex);
      const double lenErr2 = fitparams.getCovariance()(tauindex, tauindex);
      return std::make_tuple(len, std::sqrt(lenErr2));
    }
    return std::make_tuple(-999, -999);
  }

  std::tuple<double, double> FitManager::getDecayLength(Belle2::Particle& cand) const
  {
    std::tuple<double, double> rc = std::make_tuple(-999, -999);
    const ParticleBase* pb = m_decaychain->locate(&cand) ;
    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      rc = getDecayLength(pb);
    }
    return rc;
  }

}//end module namespace
