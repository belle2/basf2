/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates a particle. Base class for all other particle classes.

#include <TDatabasePDG.h>

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>
#include <analysis/VertexFitting/TreeFitter/RecoComposite.h>
#include <analysis/VertexFitting/TreeFitter/RecoResonance.h>
#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>

#include <analysis/VertexFitting/TreeFitter/RecoPhoton.h>
#include <analysis/VertexFitting/TreeFitter/Resonance.h>
#include <analysis/VertexFitting/TreeFitter/InteractionPoint.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>


namespace TreeFitter {

  std::vector<int> massConstraintList;//FT: this is not the best place to place this, but that's where the other extern goes.

  ParticleBase::ParticleBase(Belle2::Particle* particle, const ParticleBase* mother) :
    m_particle(particle),
    m_mother(mother),
    m_index(0),
    m_pdgMass(particle->getPDGMass()),
    m_pdgWidth(0),
    m_pdgLifeTime(TDatabasePDG::Instance()->GetParticle(particle->getPDGCode())->Lifetime() * 1e9),
    m_charge(0),
    m_name("Unknown")
  {
    if (particle) {
      const int pdgcode = particle->getPDGCode();
      if (pdgcode) { // PDG code != 0

        double fltcharge = particle->getCharge();

        //  round to nearest integer
        m_charge = fltcharge < 0 ? int(fltcharge - 0.5) : int(fltcharge + 0.5);
        m_name = particle->getName();
      } else {// PDG code = 0
        m_charge = particle->getCharge() > 0 ? 1 : (particle->getCharge() < 0 ? -1 : 0);
      }
    }
  }


  ParticleBase::ParticleBase(const std::string& name) :
    m_particle(NULL),
    m_mother(NULL),
    m_index(0),
    m_pdgMass(0),
    m_pdgWidth(0),
    m_pdgLifeTime(0),
    m_charge(0),
    m_name(name)
  {}


  ParticleBase::~ParticleBase()
  {
    for (auto daughter : m_daughters) {
      delete daughter;
    }
    m_daughters.clear();
  }

  ParticleBase* ParticleBase::addDaughter(Belle2::Particle* cand, bool forceFitAll)
  {
    auto newDaughter = ParticleBase::createParticle(cand, this, forceFitAll);
    m_daughters.push_back(newDaughter);
    return m_daughters.back();
  }


  void ParticleBase::removeDaughter(const ParticleBase* pb)
  {
    std::vector<ParticleBase*>::iterator iter = std::find(m_daughters.begin(), m_daughters.end(), pb);
    if (iter != m_daughters.end()) {
      m_daughters.erase(iter);
      delete *iter;
    } else {
      B2ERROR("Cannot remove particle, because not found ...");
    }
  }

  void ParticleBase::updateIndex(int& offset)
  {
    for (auto daughter : m_daughters) {
      daughter->updateIndex(offset);
    }
    m_index = offset;
    offset += dim();
  }

  ParticleBase* ParticleBase::createInteractionPoint(Belle2::Particle* daughter, bool forceFitAll, int dimension)
  {
    return new InteractionPoint(daughter, forceFitAll, dimension);
  }


  ParticleBase* ParticleBase::createParticle(Belle2::Particle* particle, const ParticleBase* mother, bool forceFitAll)
  {
    ParticleBase* rc = 0;
    const int pdgcode = particle->getPDGCode();

    bool validfit  = false;

    if (Belle2::Const::ParticleType(pdgcode) == Belle2::Const::pi0 && validfit) {
      B2ERROR("ParticleBase::createParticle: found pi0 with valid fit. This is likely a configuration error.");
    }

    if (!mother) { // 'head of tree' particles
      if (!particle->getMdstArrayIndex()) { //0 means it's a composite
        rc = new InternalParticle(particle, 0, forceFitAll);
      } else {
        rc = new InternalParticle(particle, 0, forceFitAll); //FIXME ????????
      }

    } else if (particle->getMdstArrayIndex() || particle->getTrack() || particle->getECLCluster()) { // external particles
      if (particle->getTrack()) {
        rc = new RecoTrack(particle, mother); // reconstructed track

      } else if (particle->getECLCluster()) {
        rc = new RecoPhoton(particle, mother); // reconstructed photon

      } else if (isAResonance(particle)) {
        rc = new RecoResonance(particle, mother);

      }  else {// FIXME or make this Klong??

        rc = new RecoComposite(particle, mother);
      }

    } else { // 'internal' particles

      if (validfit) {   // fitted composites
        if (isAResonance(particle)) {

          rc = new RecoResonance(particle, mother);

        } else {
          rc = new RecoComposite(particle, mother);
        }

      } else {         // unfitted composites

        if (isAResonance(particle)) {
          rc = new Resonance(particle, mother, forceFitAll);

        } else {
          rc = new InternalParticle(particle, mother, forceFitAll);
        }
      }
    }
    return rc;
  }


  double ParticleBase::pdgLifeTime(Belle2::Particle* particle)
  {
    int pdgcode = particle->getPDGCode();
    double lifetime = 0;

    if (pdgcode) {
      lifetime = TDatabasePDG::Instance()->GetParticle(pdgcode)->Lifetime() * 1e9;
    }

    return lifetime ;
  }


  bool ParticleBase::isAResonance(Belle2::Particle* particle)
  {
    bool rc = false ;
    const int pdgcode = particle->getPDGCode();

    if (pdgcode && !(particle->getMdstArrayIndex())) {
      switch (pdgcode) {

        case 22: //TODO converted photons ???
          rc = false;
          break ;

        case -11: //this is meant for bremsstrahlung do we need this TODO?
        case 11:

          rc = true ;
          break ;

        default:

          //everything with boosted flight length less than 1 micrometer
          rc = (pdgcode && pdgLifeTime(particle) < 1e-5);
      }
    }
    return rc ;
  }

  void ParticleBase::collectVertexDaughters(std::vector<ParticleBase*>& particles, int posindex)
  {
    if (mother() && mother()->posIndex() == posindex) {
      particles.push_back(this);
    }

    for (auto daughter : m_daughters) {
      daughter->collectVertexDaughters(particles, posindex);
    }
  }

  ErrCode ParticleBase::initCovariance(FitParams* fitparams) const
  {
    ErrCode status;

    const int posindex = posIndex();
    if (posindex >= 0) {
      for (int i = 0; i < 3; ++i) {
        fitparams->getCovariance()(posindex + i, posindex + i) = 400;
      }
    }

    // momentum
    const int momindex = momIndex();
    if (momindex >= 0) {
      const double initVal = 1;
      const int maxrow = hasEnergy() ? 4 : 3;

      for (int i = 0; i < maxrow; ++i) {
        fitparams->getCovariance()(momindex + i, momindex + i) = initVal;
      }
    }

    const int tauindex = tauIndex();

    if (tauindex >= 0) {

      /** cant multiply by momentum here because unknown but average mom is 3 Gev */
      double tau = pdgTime() * Belle2::Const::speedOfLight / pdgMass() * 3;

      double sigtau = tau > 0 ? 20 * tau : 999;

      const double maxDecayLength = 2000;
      double mom = particle()->getP();

      if (mom > 0.0) {
        sigtau = std::min(maxDecayLength, sigtau);
      }

      if (tau > 0) {
        sigtau = 20 * tau;

      } else {
        sigtau = 1000;

      }
      fitparams->getCovariance()(tauindex, tauindex) = sigtau * sigtau;
    }

    return status;
  }

  std::string ParticleBase::parname(int thisindex) const
  {
    std::string rc = name();
    switch (thisindex) {
      case 0: rc += "_x  "; break;
      case 1: rc += "_y  "; break;
      case 2: rc += "_z  "; break;
      case 3: rc += "_tau"; break;
      case 4: rc += "_px "; break;
      case 5: rc += "_py "; break;
      case 6: rc += "_pz "; break;
      case 7: rc += "_E  "; break;
      default:;
    }
    return rc;
  }

  const ParticleBase* ParticleBase::locate(Belle2::Particle* particle) const
  {
    const ParticleBase* rc = (m_particle == particle) ? this : 0;
    if (!rc) {
      for (auto daughter : m_daughters) {
        rc = daughter->locate(particle);
        if (rc) {break;}
      }
    }
    return rc;
  }

  void ParticleBase::retrieveIndexMap(indexmap& indexmap) const
  {
    indexmap.push_back(std::pair<const ParticleBase*, int>(this, index()));
    for (auto daughter : m_daughters) {
      daughter->retrieveIndexMap(indexmap);
    }
  }

  double ParticleBase::chiSquare(const FitParams* fitparams) const
  {
    double rc = 0;
    for (auto daughter : m_daughters) {
      double  chi2 = daughter->chiSquare(fitparams);
      rc += chi2;
    }
    return rc;
  }

  int ParticleBase::nFinalChargedCandidates() const
  {
    int rc = 0;
    for (auto daughter : m_daughters) {
      rc += daughter->nFinalChargedCandidates();
    }
    return rc;
  }

  ErrCode ParticleBase::projectGeoConstraint(const FitParams& fitparams, Projection& p) const
  {
    const int posindexmother = mother()->posIndex();
    const int posindex = posIndex();
    const int tauindex = tauIndex();
    const int momindex = momIndex();

    const Eigen::Matrix<double, 1, 3> p_vec = fitparams.getStateVector().segment(momindex, 3);
    const double mom = p_vec.norm();
    const double mom3 = mom * mom * mom;
    const Eigen::Matrix<double, 1, 3> x_vec = fitparams.getStateVector().segment(posindex, 3);
    const Eigen::Matrix<double, 1, 3> x_m = fitparams.getStateVector().segment(posindexmother, 3);

    double tau = fitparams.getStateVector()(tauindex);

    double posxmother = 0, posx = 0, momx = 0;

    // linear approximation is fine
    for (int row = 0; row < 3; ++row) {
      posxmother = x_m(row);
      posx       = x_vec(row);
      momx       = p_vec(row);

      /** the direction of the momentum is very well known from the kinematic constraints
       *  that is why we do not extract the distance as a vector here
       * */
      p.getResiduals()(row) = posxmother + tau * momx / mom - posx ;

      p.getH()(row, posindexmother + row) = 1;
      p.getH()(row, posindex + row) = -1;
      p.getH()(row, tauindex) = momx / mom;
    }

    p.getH()(0, momindex)     = tau * (p_vec(1) * p_vec(1) + p_vec(2) * p_vec(2)) / mom3 ;
    p.getH()(1, momindex + 1) = tau * (p_vec(0) * p_vec(0) + p_vec(2) * p_vec(2)) / mom3 ;
    p.getH()(2, momindex + 2) = tau * (p_vec(1) * p_vec(1) + p_vec(0) * p_vec(0)) / mom3 ;


    p.getH()(0, momindex + 1) = - tau * p_vec(0) * p_vec(1) / mom3 ;
    p.getH()(0, momindex + 2) = - tau * p_vec(0) * p_vec(2) / mom3 ;

    p.getH()(1, momindex + 0) = - tau * p_vec(1) * p_vec(0) / mom3 ;
    p.getH()(1, momindex + 2) = - tau * p_vec(1) * p_vec(2) / mom3 ;

    p.getH()(2, momindex + 0) = - tau * p_vec(2) * p_vec(0) / mom3 ;
    p.getH()(2, momindex + 1) = - tau * p_vec(2) * p_vec(1) / mom3 ;

    return ErrCode::success;
  }

  ErrCode ParticleBase::projectMassConstraint(const FitParams& fitparams,
                                              Projection& p) const
  {
    const double mass = pdgMass();
    const double mass2 = mass * mass;
    const int momindex = momIndex();
    const double px = fitparams.getStateVector()(momindex);
    const double py = fitparams.getStateVector()(momindex + 1);
    const double pz = fitparams.getStateVector()(momindex + 2);
    const double E  = fitparams.getStateVector()(momindex + 3);

    /** be awawre that the signs here are important
     * E-|p|-m extracts a negative mass and messes with the momentum !
     * */
    p.getResiduals()(0) = mass2 -  E * E + px * px + py * py + pz * pz;

    p.getH()(0, momindex)     = 2.0 * px;
    p.getH()(0, momindex + 1) = 2.0 * py;
    p.getH()(0, momindex + 2) = 2.0 * pz;
    p.getH()(0, momindex + 3) = -2.0 * E;

    return ErrCode::success;
  }

  ErrCode ParticleBase::projectConstraint(Constraint::Type, const FitParams&, Projection&) const
  {
    B2ERROR("Trying to project constraint of ParticleBase type. This is undefined.");
    return ErrCode::badsetup;
  }

  double ParticleBase::bFieldOverC() //FT: (to do) BField is already called in RecoTrack, unify
  {
    // in BaBar, BField::cmTeslaToGeVc = Constants::c/1.0E3
    // but is badly named as units are mm, ns, MeV, T
    // this is all very confusing, but we try to get it right
    // Conversion from Tesla to Belle2 units is already done, so no need for Unit::T (unlike in RecoTrack)
    static const double Bz  = Belle2::BFieldManager::getField(TVector3(0, 0, 0)).Z() * Belle2::Const::speedOfLight;
    B2DEBUG(80, "ParticleBase::bFieldOverC = " << Bz);
    return Bz;
  }

  ErrCode ParticleBase::initTau(FitParams* fitparams) const
  {
    const int tauindex = tauIndex();
    if (tauindex >= 0 && hasPosition()) {

      assert(mother());

      /**
       * In principle we have to devide by a momentum here but since the initial momentum is unknown
       * and is maximally a factor of 5 we dont need to do that here
       * */
      const double value = pdgTime() * Belle2::Const::speedOfLight / pdgMass();

      fitparams->getStateVector()(tauindex) = value;
    }

    return ErrCode::success;
  }
} //end namespace TreeFitter

