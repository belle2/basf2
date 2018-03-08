/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <TMath.h>
#include <TVector.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>

#include <analysis/VertexFitting/TreeFitter/EigenStackConfig.h>

#include <analysis/VertexFitting/TreeFitter/FitManager.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/DecayChain.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>


namespace TreeFitter {

  extern std::vector<int> massConstraintList ;

  FitManager::FitManager(Belle2::Particle* particle,
                         double prec,
                         int ipDimension,
                         bool updateDaughters) :
    m_particle(particle),
    m_decaychain(0),
    m_fitparams(0),
    m_status(VertexStatus::UnFitted),
    m_chiSquare(-1),
    m_niter(-1),
    m_prec(prec),
    m_updateDaugthers(updateDaughters)
  {
    m_decaychain =  new DecayChain(particle, false, ipDimension);

    m_fitparams  = new FitParams(m_decaychain->dim());
  }

  FitManager::~FitManager()
  {
    delete m_decaychain;
    delete m_fitparams;
  }

  bool FitManager::fit()
  {

    const int nitermax = 10;
    const int maxndiverging = 3;
    const double dChisqConv = m_prec;
    m_chiSquare = -1;
    m_errCode.reset();

    if (m_status == VertexStatus::UnFitted) {
      m_errCode = m_decaychain->initialize(m_fitparams);
    }

    if (m_errCode.failure()) {
      // the input tracks are too far apart
      m_status = VertexStatus::BadInput;
    } else {
      m_status = VertexStatus::UnFitted;
      int ndiverging = 0;
      bool finished = false;
      double deltachisq = 1e10;
      for (m_niter = 0; m_niter < nitermax && !finished; ++m_niter) {
        //std::cout << m_niter << " ---------------------------------------------------------------------------------------------"  <<
        //          std::endl;

        Eigen::Matrix < double, -1, 1, 0, MAX_MATRIX_SIZE, 1 > prevpar = m_fitparams->getStateVector();

        bool firstpass = (m_niter == 0);
        m_errCode = m_decaychain->filter(*m_fitparams, firstpass);

        double chisq = m_fitparams->chiSquare();
        double dChisqQuit = std::max(double(2 * nDof()), 2 * m_chiSquare);

        deltachisq = chisq - m_chiSquare;

        if (m_errCode.failure()) {
          finished = true ;
          m_status = VertexStatus::Failed;
        } else {
          B2DEBUG(80, "FitManager: m_errCode.success()");

          if (m_niter > 0) {
            if (fabs(deltachisq) < dChisqConv) {
              m_chiSquare = chisq;
              m_status = VertexStatus::Success;
              finished = true ;
            } else if (m_niter > 1 && deltachisq > dChisqQuit) {
              m_fitparams->getStateVector() = prevpar;
              m_status  = VertexStatus::Failed;
              m_errCode = ErrCode::fastdivergingfit;
              finished = true;
            } else if (deltachisq > 0 && ++ndiverging >= maxndiverging) {
              m_fitparams->getStateVector() = prevpar;
              m_status = VertexStatus::NonConverged;
              m_errCode = ErrCode::slowdivergingfit;
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
        m_status = VertexStatus::NonConverged;
      }

      if (!(m_fitparams->testCovariance())) {
        m_status = VertexStatus::Failed;
      }
    }

    m_chi2sum = m_decaychain->getChi2Sum();

    updateTree(*m_particle);

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

  double FitManager::globalChiSquare() const
  {
    return m_decaychain->chiSquare(m_fitparams);
  }

  void FitManager::getCovFromPB(const ParticleBase* pb, TMatrixFSym& returncov) const
  {

    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> cov = m_fitparams->getCovariance().selfadjointView<Eigen::Lower>();
    B2DEBUG(80, "       FitManager::getCovFromPB");
    int posindex = pb->posIndex();
    // hack: for tracks and photons, use the production vertex
    if (posindex < 0 && pb->mother()) {
      posindex = pb->mother()->posIndex();
    }
    int momindex = pb->momIndex();
    if (pb->hasEnergy() || (pb->type() == ParticleBase::TFParticleType::kRecoPhoton)) {

      B2DEBUG(80, "       FitManager::getCovFromPB for a particle with energy");
      // if particle has energy, get full p4 from fitparams and put them directly in the return type
      // very important! Belle2 uses p,E,x! Change order here!
      for (int row = 0; row < 4; ++row) {
        for (int col = 0; col <= row; ++col) {
          returncov(row, col) = cov(momindex + row, momindex + col);
        }
      }

      for (int row = 0; row < 3; ++row) {
        for (int col = 0; col <= row; ++col) {
          returncov(row + 4, col + 4) = cov(posindex + row, posindex + col);
        }
      }

    } else {
      B2DEBUG(80, "       FitManager::getCovFromPB for a particle with energy");
      // if not, use the pdttable mass
      Eigen::Matrix<double, 6, 6> cov6 =
        Eigen::Matrix<double, 6, 6>::Zero(6, 6);

      for (int row = 0; row < 3; ++row) {
        for (int col = 0; col <= row; ++col) {
          cov6(row, col) = cov(posindex + row, posindex + col);
          cov6(row + 3, col + 3) = cov(momindex + row, momindex + col);
        }
      }

      // now fill the jacobian
      double mass = pb->pdgMass();
      Eigen::Matrix<double, 3, 1> momVec =
        m_fitparams->getStateVector().segment(momindex, 3);

      double energy2 = momVec.transpose() * momVec;
      energy2 += mass * mass;
      double energy = sqrt(energy2);

      Eigen::Matrix<double, 7, 6> jacobian =
        Eigen::Matrix<double, 7, 6>::Zero(7, 6);

      for (int col = 0; col < 3; ++col) {
        jacobian(col, col) = 1; // don't modify momentum
        jacobian(3, col) = m_fitparams->getStateVector()(momindex + col) / energy; //add energy row
        jacobian(col + 4, col + 3) = 1; // position indeces
      }

      Eigen::Matrix<double, 7, 7> cov7 =
        Eigen::Matrix<double, 7, 7>::Zero(7, 7);
      cov7 = jacobian * cov6.selfadjointView<Eigen::Lower>() * jacobian.transpose();

      for (int row = 0; row < 7; ++row) {
        for (int col = 0; col <= row; ++col) {
          returncov(row, col) = cov7(row, col);
        }
      }

    } // else

  }

  bool FitManager::updateCand(Belle2::Particle& cand) const
  {
    // assigns fitted parameters to a candidate
    const ParticleBase* pb = m_decaychain->locate(&cand);
    if (pb) {
      updateCand(*pb, cand);
    } else {
      B2ERROR("Can't find candidate " << cand.getName() << "in tree " << m_particle->getName());
    }
    return pb != 0;
  }

  void FitManager::updateCand(const ParticleBase& pb,
                              Belle2::Particle& cand) const
  {
    int posindex = pb.posIndex();
    bool hasPos = true;
    if (posindex < 0 && pb.mother()) {
      posindex = pb.mother()->posIndex();
      hasPos = false;
    }

    const int momindex = pb.momIndex();
    const TVector3 pos(m_fitparams->getStateVector()(posindex),
                       m_fitparams->getStateVector()(posindex + 1),
                       m_fitparams->getStateVector()(posindex + 2));
    TLorentzVector p;
    p.SetPx(m_fitparams->getStateVector()(momindex));
    p.SetPy(m_fitparams->getStateVector()(momindex + 1));
    p.SetPz(m_fitparams->getStateVector()(momindex + 2));

    if (pb.hasEnergy()) {
      p.SetE(m_fitparams->getStateVector()(momindex + 3));
      cand.set4Vector(p);
    } else {
      const double mass = cand.getPDGMass();
      p.SetE(std::sqrt(p.Vect()*p.Vect() + mass * mass));
      cand.set4Vector(p);
    }

    TMatrixFSym cov7b2(7);

    getCovFromPB(&pb, cov7b2);
    cand.setMomentumVertexErrorMatrix(cov7b2);
    std::tuple<double, double>tau  = getDecayLength(cand);

    std::tuple<double, double>life = getLifeTime(cand);
    if (cand.hasExtraInfo("decayLength")) {
      cand.setExtraInfo("decayLength", std::get<0>(tau));
    } else {
      cand.addExtraInfo("decayLength", std::get<0>(tau));
    }

    if (cand.hasExtraInfo("decayLengthErr")) {
      cand.setExtraInfo("decayLengthErr", std::get<1>(tau));
    } else {
      cand.addExtraInfo("decayLengthErr", std::get<1>(tau));
    }

    if (cand.hasExtraInfo("lifeTime")) {
      cand.setExtraInfo("lifeTime", std::get<0>(life));
    } else {
      cand.addExtraInfo("lifeTime", std::get<0>(life));
    }

    if (cand.hasExtraInfo("lifeTimeErr")) {
      cand.setExtraInfo("lifeTimeErr", std::get<1>(life));
    } else {
      cand.addExtraInfo("lifeTimeErr", std::get<1>(life));
    }

    //FIXME klong are final states and for debugging I care for the vertex
    if (pb.type() == ParticleBase::kRecoKlong) {
      cand.setVertex(pos);
    }

    if (hasPos) {
      cand.setVertex(pos);
      if (&pb == m_decaychain->cand()) { // if head
        const double NDFs = nDof();
        const double fitparchi2 = m_fitparams->chiSquare();
        cand.setPValue(TMath::Prob(fitparchi2, NDFs));   //FT: (to do) p-values of fit must be verified
      }
    }
  }

  void FitManager::updateTree(Belle2::Particle& cand) const
  {
    const bool updateableMother = updateCand(cand);

    if (updateableMother && m_updateDaugthers) {

      const int ndaughters = cand.getNDaughters();
      Belle2::Particle* daughter;

      for (int i = 0; i < ndaughters; i++) {
        daughter = const_cast<Belle2::Particle*>(cand.getDaughter(i));
        updateTree(*daughter);
      }
    }
  }

  std::tuple<double, double> FitManager::getLifeTime(Belle2::Particle& cand) const
  {
    // returns the lifetime in the rest frame of the candidate
    std::tuple<double, double> rc;
    const ParticleBase* pb = m_decaychain->locate(&cand);

    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      const int momindex = pb->momIndex();

      const Eigen::Matrix<double, 1, 3> mom_vec = m_fitparams->getStateVector().segment(momindex, 3);

      const Eigen::Matrix<double, 3, 3> mom_cov = m_fitparams->getCovariance().block<3, 3>(momindex, momindex);
      Eigen::Matrix<double, 4, 4> comb_cov =  Eigen::Matrix<double, 4, 4>::Zero(4, 4);

      std::tuple<double, double> lenTuple  = getDecayLength(cand);

      comb_cov(0, 0) = std::get<1>(lenTuple);
      comb_cov.block<3, 3>(1, 1) = mom_cov;

      const double mass = pb->pdgMass();
      const double mBYc = mass / Belle2::Const::speedOfLight;
      const double mom = mom_vec.norm();
      const double mom3 = mom * mom * mom;

      const double len = std::get<0>(lenTuple);
      const double t = len / mom * mBYc;

      Eigen::Matrix<double, 1, 4> jac = Eigen::Matrix<double, 1, 4>::Zero();
      jac(0) =  1. / mom * mBYc;
      jac(1) = -1. * mom_vec(0) / mom3 * mBYc;
      jac(2) = -1. * mom_vec(1) / mom3 * mBYc;
      jac(3) = -1. * mom_vec(2) / mom3 * mBYc;

      const double tErr = jac * comb_cov.selfadjointView<Eigen::Lower>() * jac.transpose();

      // time in nanosec
      return std::make_tuple(t, tErr);
    }


    return std::make_tuple(-999, -999);
  }

  std::tuple<double, double> FitManager::getDecayLength(const ParticleBase* pb) const
  {
    // returns the decaylength in the lab frame
    return getDecayLength(pb, m_fitparams);
  }

  std::tuple<double, double> FitManager::getDecayLength(const ParticleBase* pb, const FitParams* fitparams) const
  {
    if (pb->tauIndex() >= 0 && pb->mother()) {

      const int tauindex = pb->tauIndex();

      const double len = fitparams->getStateVector()(tauindex);

      const double lenErr = fitparams->getCovariance()(tauindex, tauindex);

      return std::make_tuple(len, lenErr);
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
