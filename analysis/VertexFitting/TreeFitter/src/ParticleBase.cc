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
#include <TMath.h>

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>
#include <analysis/VertexFitting/TreeFitter/RecoComposite.h>
#include <analysis/VertexFitting/TreeFitter/RecoResonance.h>
#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>

#include <analysis/VertexFitting/TreeFitter/RecoPhoton.h>
#include <analysis/VertexFitting/TreeFitter/Resonance.h>
#include <analysis/VertexFitting/TreeFitter/InteractionPoint.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>

#include <framework/gearbox/Const.h>

namespace TreeFitter {

  int vtxverbose = 0;
  std::vector<int> massConstraintList;//FT: this is not the best place to place this, but that's where the other extern goes.

  ParticleBase::ParticleBase(Belle2::Particle* particle, const ParticleBase* mother)
    : m_particle(particle), m_mother(mother), m_index(0), m_pdgMass(0), m_pdgWidth(0), m_pdgLifeTime(0), m_charge(0), m_name("Unknown")
  {
    if (particle) {
      const int pdgcode = particle->getPDGCode();
      if (pdgcode) { // PDG code != 0

        m_pdgMass      = particle->getPDGMass();//getMass() might also work
        m_pdgWidth     = TDatabasePDG::Instance()->GetParticle(pdgcode)->Width();
        //  m_pdgLifeTime = TDatabasePDG::Instance()->GetParticle(pdgcode)->Lifetime();
        m_pdgLifeTime = pdgLifeTime(particle);
        double fltcharge = particle->getCharge();//FT: could also come from PDG
        //FT: getCharge() divides by 3.0 and gives a float, make sure it doesn't create problems
        //  round to nearest integer
        m_charge = fltcharge < 0 ? int(fltcharge - 0.5) : int(fltcharge + 0.5);
        m_name = particle->getName();
      } else {// PDG code = 0
        m_charge = particle->getCharge() > 0 ? 1 : (particle->getCharge() < 0 ? -1 : 0);
      }
    }
  }


  ParticleBase::ParticleBase(const std::string& name)
    : m_particle(NULL), m_mother(NULL), m_index(0), m_pdgMass(0), m_pdgWidth(0), m_pdgLifeTime(0), m_charge(0), m_name(name) {}


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
    ParticleBase::iter it = std::find(m_daughters.begin(), m_daughters.end(), pb);
    if (it != m_daughters.end()) {
      delete *it;
      m_daughters.erase(it);
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
    int pdgcode = particle->getPDGCode();

    bool validfit  = false;

    if (Belle2::Const::ParticleType(pdgcode) == Belle2::Const::pi0 && validfit) {
      B2ERROR("ParticleBase::createParticle: found pi0 with valid fit. This is likely a configuration error.");
    }
    if (!mother) { // 'head of tree' particles
      if (!particle->getMdstArrayIndex()) { //0 means it's a composite
        rc = new InternalParticle(particle, 0, forceFitAll);
      } else {
        rc = new InternalParticle(particle, 0, forceFitAll);
      }
    } else if (particle->getMdstArrayIndex() || particle->getTrack() || particle->getECLCluster()) { // external particles
      if (particle->getTrack()) {
        rc = new RecoTrack(particle, mother); // reconstructed track
      } else if (particle->getECLCluster()) {
        rc = new RecoPhoton(particle, mother); // reconstructed photon
      } else if (isAResonance(particle)) {
        rc = new RecoResonance(particle, mother);
      }  else {
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


  double ParticleBase::pdgLifeTime(Belle2::Particle* particle) //FT: This is actually the decay length in cm (in the CMS)
  {
    int pdgcode = particle->getPDGCode();
    double lifetime = 0;
    double decaylen = 0;
    if (pdgcode) {
      lifetime = TDatabasePDG::Instance()->GetParticle(pdgcode)->Lifetime();
    }
    decaylen = Belle2::Const::speedOfLight * lifetime * Belle2::Unit::s;
    return decaylen ;
  }


  bool ParticleBase::isAResonance(Belle2::Particle* particle)
  {
    bool rc = false ;
    const int pdgcode = particle->getPDGCode();
    //    if( pdgcode && particle->isComposite() ) {
    if (pdgcode && !(particle->getMdstArrayIndex())) {  //FT: this should work for compositeness
      switch (pdgcode) { //FT: (to do) I'd like this not to be hardcoded
        //      case PDGCode::gamma:  // conversions are not treated as a resonance
        case 22: // conversions are not treated as a resonance
          rc = false;
          break ;
        //      case PDGCode::e_plus: // bremstrahlung is treated as a resonance
        case -11: // bremstrahlung is treated as a resonance
        //      case PDGCode::e_minus:
        case 11:
          rc = true ;
          break ;
        default: // this should take care of the pi0
          //  rc = particle->isAResonance() || (pdgcode && pdgLifeTime(particle)<1.e-8) ;
          double ctau = pdgLifeTime(particle) / Belle2::Unit::um; //ctau in [um]
          //    B2DEBUG(80, "Particle code is " << pdgcode << " with a lifetime of " << TDatabasePDG::Instance()->GetParticle(
          //    pdgcode)->Lifetime() << " seconds and a decay lenght of " << ctau << " um.");
          rc = (pdgcode && ctau < 1); //FT: this cut comes from the article
      }
    }
    return rc ;
  }

  //FT: this was moved from InternalParticle::addToDaughterList
  void ParticleBase::collectVertexDaughters(std::vector<ParticleBase*>& particles, int posindex)
  {
    // collect all particles emitted from vertex with position posindex
    if (mother() && mother()->posIndex() == posindex) {
      //FT: this recursively adds everything that has a vertex reconstructed to the intended vertex, very weird implementation
      particles.push_back(this);
    }
    for (auto daughter : m_daughters) {
      daughter->collectVertexDaughters(particles, posindex);
    }
  }

  ErrCode ParticleBase::initCovariance(FitParams* fitparams) const
  {
    ErrCode status;
    // position
    int posindex = posIndex();
    if (posindex >= 0) {
      //JFK: the fits diverge if we init the off elements dont ask me ... 2017-09-30
      for (int i = 0; i < 3; ++i) {
        fitparams->getCovariance()(posindex + i, posindex + i) = 400;
      }
    }

    // momentum
    int momindex = momIndex();
    if (momindex >= 0) {
      //JFK: init Value in  GeV squared 2017-09-25
      const double initVal = 1;
      int maxrow = hasEnergy() ? 4 : 3;
      //JFK: same here: fit diverges if this has off elments 2017-09-30
      for (int i = 0; i < maxrow; ++i) {
        fitparams->getCovariance()(momindex + i, momindex + i) = initVal;
      }
      //if (maxrow == 4) {
      //  fitparams->getCovariance().block<4, 4>(momindex, momindex).triangularView<Eigen::Lower>() =
      //    EigenTypes::MatrixXd::Constant(4, 4, initVal);
      //} else if (maxrow == 3) {
      //  fitparams->getCovariance().block<3, 3>(momindex, momindex).triangularView<Eigen::Lower>() =
      //    EigenTypes::MatrixXd::Constant(3, 3, initVal);
      //}
    }
    // JFK: tau is a length in our version of the code Thu 24 Aug 2017 04:30:17 AM CEST
    int tauindex = tauIndex();
    if (tauindex >= 0) {
      double tau = pdgTau();
      double sigtau = tau > 0 ? 20 * tau : 999;
      const double maxdecaylength = 20; // [cm] (okay for Ks->pi0pi0)
      double bcP = particle()->getP();
      if (bcP > 0.0) {
        sigtau = std::min(maxdecaylength / bcP, sigtau);
      }
      if (tau > 0) {
        sigtau = 20 * tau;
      } else {
        sigtau = 100;
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
    //    const ParticleBase* rc = 0;
    //    if( particle() && ( particle()==aparticle || particle()->isCloneOf(*aparticle,true) ) ) rc = this ;

    //FT: seems like isCloneOf searches down the decay tree to find the particle, which will then be cached in the ParticleMap
    //    we don't have anything like that, so we have to do it explicitly.

    const ParticleBase* rc = (m_particle == particle) ? this : 0; //if it corresponds to the stored pointer to the head, then it's easy
    if (!rc) { //otherwise sift through the daughters
      //FT:  LHCb iteration method, requires to define properly m_daughters in ParticleBase instead of InternalParticle, OR to do some hacks
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
    int posindexmother = mother()->posIndex();
    int posindex = posIndex();
    int tauindex = tauIndex();
    int momindex = momIndex();

    double tau =  fitparams.getStateVector()(tauindex);

    EigenTypes::ColVector momentumVec = fitparams.getStateVector().segment(momindex, 3);
    double p2 = momentumVec.transpose() * momentumVec;
    double mom = std::sqrt(p2);
    double posxmother = 0, posx = 0, momx = 0;

    // linear approximation is fine
    //JFK: TODO move to block operations 2017-09-28
    for (int row = 0; row < 3; ++row) {
      posxmother = fitparams.getStateVector()(posindexmother + row);
      posx       = fitparams.getStateVector()(posindex + row);
      momx       = fitparams.getStateVector()(momindex + row);
      p.getResiduals()(row) = posxmother - posx + tau * momx / mom;
      p.getH()(row, posindexmother + row) =    1;
      p.getH()(row, posindex + row) =        -1;
      p.getH()(row, momindex + row) = tau / mom; // JFK was just tau before
      p.getH()(row, tauindex)       =      momx;
    }
    return ErrCode::success;
  }

  ErrCode ParticleBase::projectMassConstraint(const FitParams& fitparams,
                                              Projection& p) const
  {
    double mass = pdgMass();
    double mass2 = mass * mass;
    int momindex = momIndex();
    // initialize the value
    double px = fitparams.getStateVector()(momindex);
    double py = fitparams.getStateVector()(momindex + 1);
    double pz = fitparams.getStateVector()(momindex + 2);
    double E  = fitparams.getStateVector()(momindex + 3);
    p.getResiduals()(0) =  E * E - px * px - py * py - pz * pz - mass2;
    // calculate the projection matrix
    p.getH()(0, momindex) = -2.0 * px;
    p.getH()(0, momindex + 1) = -2.0 * py;
    p.getH()(0, momindex + 2) = -2.0 * pz;
    p.getH()(0, momindex + 3) =  2.0 * E;
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
    int tauindex = tauIndex();
    if (tauindex >= 0 && hasPosition()) {
      const ParticleBase* amother = mother();
      int momposindex = amother ? amother->posIndex() : -1;
      int posindex = posIndex();
      int momindex = momIndex();
      assert(momposindex >= 0); // check code logic: no mother -> no tau
      //assert(fitparams->par(momposindex+1)!=0 ||fitparams->par(momposindex+2)!=0
      //       ||fitparams->par(momposindex+3)!=0) ; // mother must be initialized

      EigenTypes::ColVector dxVec = fitparams->getStateVector().segment(posindex, 3)
                                    - fitparams->getStateVector().segment(momposindex, 3);
      EigenTypes::ColVector pxVec = fitparams->getStateVector().segment(momindex, 3);
      double momdX = dxVec.transpose() * pxVec;
      double mom2 =  pxVec.transpose() * pxVec;


      // JFK: tau is a length now! Thu 24 Aug 2017 04:33:11 AM CEST
      // if tau should be a time devide by mom2 insertad of the sqrt
      double tau = std::abs(momdX) / std::sqrt(mom2); //scalar product

      // JFK: pdgTau returns nan because we init the mass with 0... Thu 24 Aug 2017 04:40:38 AM CEST
      //      5 cm as an init value is a stupid but okish guess
      if (tau == 0) {
        tau = 5 ;//pdgTau();
      }
      if (std::isnan(tau)) {tau = 999;}
      fitparams->getStateVector()(tauindex) = tau;
    }
    return ErrCode::success;
  }
} //end namespace TreeFitter

