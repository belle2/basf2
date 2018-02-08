/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Main implementation of the fitter.

//#include <iomanip>
//#include <stdio.h>
//#include <sstream>

#include <TMath.h>
#include <TVector.h>

#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

#include <analysis/VertexFitting/TreeFitter/Fitter.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/DecayChain.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>

#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/LorentzVector.h>

using std::vector;

namespace TreeFitter {

  extern int vtxverbose ;
  extern std::vector<int> massConstraintList ;

  Fitter::Fitter(Belle2::Particle* particle, double prec, int ipDimension)
    : m_particle(particle), m_decaychain(0), m_fitparams(0), m_status(VertexStatus::UnFitted),
      m_chiSquare(-1), m_niter(-1), m_prec(prec)
  {
    // build the tree,
    m_decaychain = new DecayChain(particle, false, ipDimension);

    // allocate the fit parameters
    m_fitparams  = new FitParams(m_decaychain->dim());
  }

  Fitter::~Fitter()
  {
    delete m_decaychain;
    delete m_fitparams;
  }

  bool Fitter::fit()
  {
    const int nitermax = 10;
    const int maxndiverging = 3;
    const double dChisqConv = m_prec;
    // initialize
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
        //std::endl;
        EigenTypes::ColVector prevpar = m_fitparams->getStateVector();

        bool firstpass = (m_niter == 0);
        m_errCode = m_decaychain->filter(*m_fitparams, firstpass);

        double chisq = m_fitparams->chiSquare();//   m_fitparams->nDof();
        double dChisqQuit = std::max(double(2 * nDof()), 2 * m_chiSquare);

        deltachisq = chisq - m_chiSquare;

        if (m_errCode.failure()) {
          finished = true ;
          m_status = VertexStatus::Failed;
        } else {
          B2DEBUG(80, "Fitter: m_errCode.success()");

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
        B2WARNING("TreeFitter::Fitter: A covariance matrix diag element is < 0 (after performing the fit). Changing status to failed.");
        m_status = VertexStatus::Failed;
      }
    }

    m_chi2sum = m_decaychain->getChi2Sum();

    updateTree(*m_particle);

    return (m_status == VertexStatus::Success);
  }

  const EigenTypes::MatrixXd& Fitter::getCovariance() const
  {
    return m_fitparams->getCovariance();
  }

  const EigenTypes::ColVector& Fitter::getStateVector() const
  {
    return m_fitparams->getStateVector();
  }

  int Fitter::nDof() const
  {
    return m_fitparams->nDof();
  }

  int Fitter::posIndex(Belle2::Particle* particle) const
  {
    return m_decaychain->posIndex(particle);
  }

  int Fitter::momIndex(Belle2::Particle* particle) const
  {
    return m_decaychain->momIndex(particle);
  }

  int Fitter::tauIndex(Belle2::Particle* particle) const
  {
    return m_decaychain->tauIndex(particle);
  }

  double Fitter::globalChiSquare() const
  {
    return m_decaychain->chiSquare(m_fitparams);
  }

  void Fitter::getCovFromPB(const ParticleBase* pb, TMatrixFSym& returncov) const
  {

    EigenTypes::MatrixXd cov = m_fitparams->getCovariance().selfadjointView<Eigen::Lower>();
    B2DEBUG(80, "       Fitter::getCovFromPB");
    int posindex = pb->posIndex();
    // hack: for tracks and photons, use the production vertex
    if (posindex < 0 && pb->mother()) {
      posindex = pb->mother()->posIndex();
    }
    int momindex = pb->momIndex();
    if (pb->hasEnergy() || (pb->type() == ParticleBase::TFParticleType::kRecoPhoton)) {

      B2DEBUG(80, "       Fitter::getCovFromPB for a particle with energy");
      // if particle has energy, get full p4 from fitparams and put them directly in the return type
      // very important! Belle2 uses p,E,x! Change order here!
      //FIXME fixme I changed this
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
      B2DEBUG(80, "       Fitter::getCovFromPB for a particle with energy");
      // if not, use the pdttable mass
      EigenTypes::MatrixXd cov6 = EigenTypes::MatrixXd::Zero(6, 6);
      for (int row = 0; row < 3; ++row) {
        for (int col = 0; col <= row; ++col) {
          cov6(row, col) = cov(posindex + row, posindex + col);
          cov6(row + 3, col + 3) = cov(momindex + row, momindex + col);
        }
      }
      // now fill the jacobian
      double mass = pb->pdgMass();
      EigenTypes::ColVector momVec = m_fitparams->getStateVector().segment(momindex, 3);
      double energy2 = momVec.transpose() * momVec;
      energy2 += mass * mass;
      double energy = sqrt(energy2);
      EigenTypes::MatrixXd jacobian = EigenTypes::MatrixXd::Zero(7, 6);
      //JFK: there was an old comment on the part that set the diagonal se below. does this make sense? 2017-09-28
      // don't modify momentum

      for (int col = 0; col < 3; ++col) {
        jacobian(col, col) = 1; // don't modify momentum
        jacobian(3, col) = m_fitparams->getStateVector()(momindex + col) / energy; //add energy row
        jacobian(col + 4, col + 3) = 1; // position indeces
      }
      EigenTypes::MatrixXd cov7 = EigenTypes::MatrixXd::Zero(7, 7);
      cov7 = jacobian * cov6.selfadjointView<Eigen::Lower>() * jacobian.transpose();
      //JFK: now put everything in the return type 2017-09-28

      for (int row = 0; row < 7; ++row) {
        for (int col = 0; col <= row; ++col) {
          returncov(row, col) = cov7(row, col);
        }
      }
      //std::cout << "cov6 \n" << cov6  << std::endl;
      //std::cout << "jacobian \n" << jacobian  << std::endl;
      //std::cout << "return \n" << cov7  << std::endl;

    } // else

  }

  bool Fitter::updateCand(Belle2::Particle& cand) const
  {
    // assigns fitted parameters to a candidate
    const ParticleBase* pb = m_decaychain->locate(&cand);
    B2DEBUG(80, " Fitter::updateCand(" << cand.getName() << ") is " << pb);
    if (pb) {
      updateCand(*pb, cand);
    } else {
      B2ERROR("Can't find candidate " << cand.getName() << "in tree " << m_particle->getName());
    }
    return pb != 0;
  }

  void Fitter::updateCand(const ParticleBase& pb,
                          Belle2::Particle& cand) const
  {

    int posindex = pb.posIndex();
    bool hasPos = true;
    if (posindex < 0 && pb.mother()) {
      posindex = pb.mother()->posIndex();
      hasPos = false;
    }

    int momindex = pb.momIndex();
    TVector3 pos; //FT: this is not optimal
    pos[0] = m_fitparams->getStateVector()(posindex);
    pos[1] = m_fitparams->getStateVector()(posindex + 1);
    pos[2] = m_fitparams->getStateVector()(posindex + 2);

    TLorentzVector p;
    p.SetPx(m_fitparams->getStateVector()(momindex));
    p.SetPy(m_fitparams->getStateVector()(momindex + 1));
    p.SetPz(m_fitparams->getStateVector()(momindex + 2));

    if (pb.hasEnergy()) {
      p.SetE(m_fitparams->getStateVector()(momindex + 3));
      cand.set4Vector(p);
    } else {
      double mass = cand.getPDGMass();
      p.SetE(std::sqrt(p.Vect()*p.Vect() + mass * mass)); //I risk rounding errors for no benefit
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

    if (hasPos) {  //if not final state
      cand.setVertex(pos);
      if (&pb == m_decaychain->cand()) { // if head
        double NDFs = nDof();
        double fitparchi2 = m_fitparams->chiSquare();
        cand.setPValue(TMath::Prob(fitparchi2, NDFs));   //FT: (to do) p-values of fit must be verified
      }

    }
  }

  void Fitter::updateTree(Belle2::Particle& cand) const
  {
    if (updateCand(cand)) { // if the mother can be updated, update the daughters
      int ndaughters = cand.getNDaughters();
      Belle2::Particle* daughter;
      for (int i = 0; i < ndaughters; i++) {
        daughter = const_cast<Belle2::Particle*>(cand.getDaughter(i));
        updateTree(*daughter);
      }
    }
  }

  std::tuple<double, double> Fitter::getLifeTime(Belle2::Particle& cand) const
  {
    // returns the lifetime in the rest frame of the candidate
    std::tuple<double, double> rc;
    const ParticleBase* pb = m_decaychain->locate(&cand);
    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      int tauindex = pb->tauIndex();
      double tau    = m_fitparams->getStateVector()(tauindex);
      double taucov = m_fitparams->getCovariance()(tauindex, tauindex);
      double mass   = pb->pdgMass();
      double convfac = mass / Belle2::Const::speedOfLight;
      return std::make_tuple(convfac * tau, convfac * convfac * taucov);
    }
    return std::make_tuple(-999, -999);
  }

  std::tuple<double, double> Fitter::getDecayLength(const ParticleBase* pb) const
  {
    // returns the decaylength in the lab frame
    return getDecayLength(pb, m_fitparams);
  }

  std::tuple<double, double> Fitter::getDecayLength(const ParticleBase* pb, const FitParams* fitparams) const
  {
    if (pb->tauIndex() >= 0 && pb->mother()) {
      int tauindex = pb->tauIndex();
      double len   = fitparams->getStateVector()(tauindex);
      double lenErr = fitparams->getCovariance()(tauindex, tauindex);
      return std::make_tuple(len, lenErr);
    }
    return std::make_tuple(-999, -999);
  }

  std::tuple<double, double> Fitter::getDecayLength(Belle2::Particle& cand) const
  {
    std::tuple<double, double> rc = std::make_tuple(999, -999);
    const ParticleBase* pb = m_decaychain->locate(&cand) ;
    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      rc = getDecayLength(pb);
    }
    return rc;
  }


}//end module namespace

