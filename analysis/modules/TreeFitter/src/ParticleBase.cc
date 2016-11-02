/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates a particle. Base class for all other particle classes.

//#include <iomanip>
//#include <analysis/dataobjects/Particle.h>

#include <TDatabasePDG.h>
#include <TMath.h>

#include <analysis/modules/TreeFitter/ParticleBase.h>
#include <analysis/modules/TreeFitter/InternalParticle.h>
#include <analysis/modules/TreeFitter/RecoComposite.h>
#include <analysis/modules/TreeFitter/RecoResonance.h>
#include <analysis/modules/TreeFitter/RecoTrack.h>
#include <analysis/modules/TreeFitter/RecoPhoton.h>
#include <analysis/modules/TreeFitter/Resonance.h>
#include <analysis/modules/TreeFitter/InteractionPoint.h>
#include <analysis/modules/TreeFitter/FitParams.h>

#include <framework/gearbox/Const.h>

namespace TreeFitter {

  int vtxverbose = 0;

  // Default constructor
  ParticleBase::ParticleBase(Particle* particle, const ParticleBase* mother)
    : m_particle(particle), m_mother(mother), m_index(0), m_pdgMass(0), m_pdgWidth(0), m_pdgLifeTime(0), m_charge(0), m_name("Unknown")
  {
    if (particle) {
      const int pdgcode = particle->getPDGCode();
      if (pdgcode) { // PDG code != 0
        m_pdgMass      = particle->getPDGMass();//getMass() might also work
        //      m_pdgMass      =TDatabasePDG::Instance()->GetParticle(pdgcode)->Mass();
        m_pdgWidth     = TDatabasePDG::Instance()->GetParticle(pdgcode)->Width();
        //  m_pdgLifeTime = TDatabasePDG::Instance()->GetParticle(pdgcode)->Lifetime();
        m_pdgLifeTime = pdgLifeTime(particle);
        double fltcharge = particle->getCharge();//FT: could also come from PDG
        //FT: getCharge() divides by 3.0 and gives a float, make sure it doesn't create problems
        //  round to nearest integer
        m_charge = fltcharge < 0 ? int(fltcharge - 0.5) : int(fltcharge + 0.5) ;
        m_name = particle->getName();
      } else {// PDG code = 0
        m_charge = particle->getCharge() > 0 ? 1 : (particle->getCharge() < 0 ? -1 : 0) ;
      }
    }
  }

  ParticleBase::~ParticleBase()
  {
    for (daucontainer::iterator it = m_daughters.begin() ;
         it != m_daughters.end() ; ++it)
      delete *it ;
    m_daughters.clear() ;
  } ;

  ParticleBase* ParticleBase::addDaughter(Particle* cand, bool forceFitAll)
  {
    m_daughters.push_back(ParticleBase::createParticle(cand, this, forceFitAll)) ;
    return m_daughters.back() ;
  }

  void ParticleBase::removeDaughter(const ParticleBase* pb)
  {
    daucontainer::iterator it = std::find(m_daughters.begin(), m_daughters.end(), pb) ;
    if (it != m_daughters.end()) {
      delete *it ;
      m_daughters.erase(it) ;
    } else {
      std::cout << "ERROR: cannot remove particle, because not found ..." << std::endl ;
    }
  }

  void ParticleBase::updateIndex(int& offset)
  {
    // first the daughters
    for (const_iterator it = begin();
         it != end(); ++it)(*it)->updateIndex(offset) ;
    // now the real work
    m_index = offset ;
    offset += dim() ;
  }

  ParticleBase* ParticleBase::createParticle(Particle* particle, const ParticleBase* mother, bool forceFitAll)
  {
    // This routine interpretes a Particle dataobject as a 'Particle' used by the fitter.
    if (vtxverbose >= 2)
      std::cout << "ParticleBase::createParticle: " << forceFitAll << std::endl ;
    ParticleBase* rc = 0 ;
    int pdgcode = particle->getPDGCode();
    //    bool isupsilon = pdgcode % 1000 == 553; //FT: Y(any) ends with 553. But we don't use this anyway.

    //FT: Beamspot constraint still needs some development
    /*
    bool bsconstraint = bc->constraint(BtaConstraint::Beam) !=0 ;
    bool beconstraint = bc->constraint(BtaConstraint::BeamEnergy) != 0;
    */

    // We refit invalid fits, kinematic fits and composites with beamspot
    // constraint if not at head of tree.
    TVector3 zero; //initializes to (0,0,0)
    bool validfit  = !forceFitAll && mother
                     && !(particle->getVertex() == zero); // && //FT: stopgap since we don't have decay vertex flagging
    //      (particle->decayVtx()->status() == BtaAbsVertex::Success ) &&
    //      (particle->decayVtx()->type() == BtaAbsVertex::Geometric ) &&
    //      !(bsconstraint||beconstraint) ;
    //
    //    if(validfit && (bc->fitParams().cov7()(7,1)==0 &&
    //        bc->fitParams().cov7()(6,1)!=0) ) {
    // this is an awfull hack to detect whether the covariance
    // matrix contains a x-E row. if not, then the matrix is
    // unusable. GeoKin does not fill the xE row.
    //
    //      static int printit=10 ;
    //      if(--printit>0)
    //  ErrMsg(warning) << "ParticleBase: this btacandidate (" << bc->pdtEntry()->name()
    //      << ") has a bad cov matrix. I'll refit it. " << endmsg ;
    //      validfit = false ;
    //    }

    // FT:leave this one for now
    if (Const::ParticleType(pdgcode) == Const::pi0 && validfit) {
      static int printit = 10 ;
      if (--printit >= 0)
        B2ERROR("ParticleBase::createParticle: found pi0 with valid fit. This is likely a configuration error.") ;
      validfit = false ;
    }

    if (!mother) { // 'head of tree' particles
      //      if( beconstraint )
      //  rc = isupsilon ? new Upsilon(bc,forceFitAll) : new Upsilon(bc,forceFitAll,true) ;
      //      else if ( bsconstraint )
      //  rc = isupsilon ? new InteractionPoint(bc,forceFitAll) : new InteractionPoint(bc,forceFitAll,true) ;
      //      else if( bc->isComposite() )

      //FT: we don't have a way to properly flag particles as composites, and we don't have a stored value for decay time. Some hacking is required.
      //if( particle->isComposite() )
      if (!particle->getMdstArrayIndex()) { //0 means it's a composite
        B2DEBUG(90, "We attempt to create the mother, named " << particle->getName());
        rc = new InternalParticle(particle, 0, forceFitAll) ; // still need proper head-of-tree class
        B2DEBUG(80, "We create the mother, named " << particle->getName());
      } else {
        B2ERROR("ParticleBase::createParticle: You are fitting a decay tree that exists of a single, non-composite particle and which does not have a beamconstraint.");
        B2ERROR("I do not understand what you want me to do with this.");
        rc = new InternalParticle(particle, 0, forceFitAll) ; // still need proper head-of-tree class
        B2DEBUG(80, "We create... something, named " << particle->getName());
      }
      //    } else if( !(particle->isComposite()) ) { // external particles
    } else if (particle->getMdstArrayIndex() || particle->getTrack() || particle->getECLCluster()) { // external particles
      //FT: added request for tracks/ECL, very inefficient way because I do it twice, double check if getMdstArrayIndex is doing what I want it to
      //I'll go back and optimize this later on
      if (particle->getTrack()) {
        B2DEBUG(90, "We attempt to create a track, named " << particle->getName());
        rc = new RecoTrack(particle, mother) ; // reconstructed track
        B2DEBUG(80, "We create a track, named " << particle->getName());
      } else if (particle->getECLCluster()) {
        B2DEBUG(90, "We attempt to create a photon, named " << particle->getName());
        rc = new RecoPhoton(particle, mother) ; // reconstructed photon
        B2DEBUG(80, "We create a photon, named " << particle->getName());
        //      else if(!validfit || particle->constraint(BtaConstraint::MissingMass) ) //FT disabled for now
        //  rc = new MissingParticle(particle,mother) ; // missing particle
      } else if (isAResonance(particle)) {
        B2DEBUG(90, "We attempt to create an external resonance, named " << particle->getName());
        rc = new RecoResonance(particle, mother) ;
        B2DEBUG(80, "We create an external resonance, named " << particle->getName());
      }  else {
        B2DEBUG(90, "We attempt to create an external composite, named " << particle->getName());
        rc = new RecoComposite(particle, mother) ;
        B2DEBUG(80, "We create an external composite, named " << particle->getName());
      }
    } else { // 'internal' particles
      if (validfit /*|| isconversion*/) {   // fitted composites
        if (isAResonance(particle)) {
          B2DEBUG(90, "We attempt to create an internal resonance, named " << particle->getName());
          rc = new RecoResonance(particle, mother) ;
          B2DEBUG(80, "We create an internal resonance, named " << particle->getName());
        } else {
          B2DEBUG(90, "We attempt to create an internal composite, named " << particle->getName());
          rc = new RecoComposite(particle, mother) ;
          B2DEBUG(80, "We create an internal composite, named " << particle->getName());
        }
      } else {         // unfitted composites
        if (isAResonance(particle)) {
          B2DEBUG(90, "We attempt to create an internal unfitted resonance, named " << particle->getName());
          rc = new Resonance(particle, mother, forceFitAll) ;
          B2DEBUG(80, "We create an internal unfitted resonance, named " << particle->getName());
        } else {
          B2DEBUG(90, "We attempt to create a nonresonant, unfitted internal particle, named " << particle->getName());
          rc = new InternalParticle(particle, mother, forceFitAll) ;
          B2DEBUG(80, "We create a nonresonant, unfitted internal particle, named " << particle->getName());
        }
      }
    }

    if (vtxverbose >= 2)
      std::cout << "ParticleBase::createParticle returns " << rc->type()
                << " " << rc->index() << std::endl ;
    return rc ;
  }

  //moved to constructor
  /*
  const std::string ParticleBase::name() const
  {
    return (particle() && particle()->getName()) ? particle()->getName() : "Unknown" ;
  }
  */

  double ParticleBase::pdgLifeTime(Particle* particle) //FT: This is actually the decay length in cm (in the CMS)
  {
    int pdgcode = particle->getPDGCode();
    double lifetime = 0;
    double decaylen = 0;
    if (pdgcode)
      lifetime = TDatabasePDG::Instance()->GetParticle(pdgcode)->Lifetime();
    decaylen = Const::speedOfLight * lifetime * Unit::s;
    return decaylen ;
  }


  bool ParticleBase::isAResonance(Particle* particle)
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
          double ctau = pdgLifeTime(particle) / Unit::um; //ctau in [um]
          //    B2DEBUG(80, "Particle code is " << pdgcode << " with a lifetime of " << TDatabasePDG::Instance()->GetParticle(
          //    pdgcode)->Lifetime() << " seconds and a decay lenght of " << ctau << " um.");
          rc = (pdgcode && ctau < 1); //FT: this cut comes from the article
      }
    }
    return rc ;
  }

  //FT: this was moved from InternalParticle::addToDaughterList
  void ParticleBase::collectVertexDaughters(daucontainer& particles, int posindex)
  {
    // collect all particles emitted from vertex with position posindex
    if (mother() && mother()->posIndex() == posindex)
      particles.push_back(
        this);  //FT: this recursively adds everything that has a vertex reconstructed to the intended vertex, very weird implementation
    for (daucontainer::const_iterator idau = daughters().begin() ;
         idau != daughters().end() ; ++idau)
      (*idau)->collectVertexDaughters(particles, posindex) ;
  }

  ErrCode ParticleBase::initCov(FitParams* fitparams) const
  {
    ErrCode status ;

    // position
    int posindex = posIndex() ;
    if (posindex >= 0) {
      //double decaylength = pdtLifeTime() ;
      const double sigpos = 20 ; // cm
      // that's how good the initalization should be
      for (int row = posindex + 1; row <= posindex + 3 ; ++row)
        fitparams->cov().fast(row, row) = sigpos * sigpos ;
    }

    // momentum
    int momindex = momIndex() ;
    if (momindex >= 0) {
      const double sigmom = 1 ; // GeV
      int maxrow = hasEnergy() ? 4 : 3 ;
      for (int row = momindex + 1; row <= momindex + maxrow; ++row)
        fitparams->cov().fast(row, row) = sigmom * sigmom ;
    }

    // lifetime
    int tauindex = tauIndex() ;
    if (tauindex >= 0) {
      double tau = pdgTau() ;
      double sigtau = tau > 0 ? 20 * tau : 999 ;
      const double maxdecaylength = 20; // [cm] (okay for Ks->pi0pi0)
      double bcP = particle()->getP();
      if (bcP > 0.0)
        sigtau = std::min(maxdecaylength / bcP, sigtau) ;
      fitparams->cov().fast(tauindex + 1, tauindex + 1) = sigtau * sigtau ;
    }

    return status ;
  }

  std::string ParticleBase::parname(int thisindex) const
  {
    std::string rc = name() ;
    switch (thisindex) {
      case 0: rc += "_x  " ; break ;
      case 1: rc += "_y  " ; break ;
      case 2: rc += "_z  " ; break ;
      case 3: rc += "_tau" ; break ;
      case 4: rc += "_px " ; break ;
      case 5: rc += "_py " ; break ;
      case 6: rc += "_pz " ; break ;
      case 7: rc += "_E  " ; break ;
      default: ;
    }
    return rc ;
  }

  void ParticleBase::print(const FitParams* fitpar) const
  {
    std::cout << std::setw(5) << "[" << type() << "]" << std::setw(15) << std::flush << name().c_str()
              << " val" << std::setw(15) << "err" << std::endl ;
    std::cout << std::setprecision(5) ;
    for (int i = 0; i < dim(); ++i) {
      int theindex = index() + i ;
      std::cout << std::setw(2) << theindex << " "
                << std::setw(20) << parname(i).c_str()
                << std::setw(15) << fitpar->par()(theindex + 1)
                << std::setw(15) << sqrt(fitpar->cov()(theindex + 1, theindex + 1))
                << std::setw(15) << fitpar->cov()(theindex + 1, theindex + 1) << std::endl ;
    }
    if (hasEnergy()) {
      int momindex = momIndex() ;
      double E  = fitpar->par()(momindex + 4) ;
      double px = fitpar->par()(momindex + 1) ;
      double py = fitpar->par()(momindex + 2) ;
      double pz = fitpar->par()(momindex + 3) ;
      double mass2 = E * E - px * px - py * py - pz * pz ;
      double mass = mass2 > 0 ? sqrt(mass2) : -sqrt(-mass2) ;

      HepSymMatrix cov = fitpar->cov().sub(momindex + 1, momindex + 4) ;
      HepVector G(4, 0) ;
      G(1) = -px / mass ;
      G(2) = -py / mass ;
      G(3) = -pz / mass ;
      G(4) =   E / mass ;
      double massvar = cov.similarity(G) ;
      std::cout << std::setw(2) << std::setw(20) << "mass: "
                << std::setw(15) << mass
                << std::setw(15) << sqrt(massvar) << std::endl ;
    }

    //FT: also print daughters
    for (daucontainer::const_iterator it = m_daughters.begin() ;
         it != m_daughters.end() ; ++it)
      (*it)->print(fitpar) ;
    //
  }

  const ParticleBase* ParticleBase::locate(Particle* aparticle) const
  {
    //    const ParticleBase* rc = 0;
    //    if( particle() && ( particle()==aparticle || particle()->isCloneOf(*aparticle,true) ) ) rc = this ;

    //FT: seems like isCloneOf searches down the decay tree to find the particle, which will then be cached in the ParticleMap
    //    we don't have anything like that, so we have to do it explicitly.

    //    if( particle() && particle()==aparticle) rc = this ; //old fix, insufficient
    const ParticleBase* rc = (m_particle == aparticle) ? this : 0
                             ; //if it corresponds to the stored pointer to the head, then it's easy
    if (!rc) { //otherwise sift through the daughters
      //FT:  LHCb iteration method, requires to define properly m_daughters in ParticleBase instead of InternalParticle, OR to do some hacks
      for (daucontainer::const_iterator it = m_daughters.begin() ; !rc && it != m_daughters.end(); ++it) {
        rc = (*it)->locate(aparticle) ;
      }
    }
    return rc ;
  }

  void ParticleBase::retrieveIndexMap(indexmap& anindexmap) const
  {
    anindexmap.push_back(std::pair<const ParticleBase*, int>(this, index())) ;
    for (daucontainer::const_iterator it = m_daughters.begin() ;
         it != m_daughters.end() ; ++it)
      (*it)->retrieveIndexMap(anindexmap) ;
  }

  double ParticleBase::chiSquare(const FitParams* fitparams) const
  {
    double rc = 0;
    for (daucontainer::const_iterator it = m_daughters.begin() ;
         it != m_daughters.end(); ++it)
      rc += (*it)->chiSquare(fitparams) ;
    return rc ;
  }

  int ParticleBase::nFinalChargedCandidates() const
  {
    int rc = 0;
    for (daucontainer::const_iterator it = m_daughters.begin() ;
         it != m_daughters.end() ; ++it)
      rc += (*it)->nFinalChargedCandidates() ;
    return rc ;
  }

  ErrCode ParticleBase::projectGeoConstraint(const FitParams& fitparams, Projection& p) const
  {
    int posindexmother = mother()->posIndex() ;
    int posindex = posIndex();
    int tauindex = tauIndex() ;
    int momindex = momIndex() ;

    double tau =  fitparams.par()(tauindex + 1) ;

    // linear approximation is fine
    for (int row = 1; row <= 3; ++row) {
      double posxmother = fitparams.par()(posindexmother + row) ;
      double posx       = fitparams.par()(posindex + row) ;
      double momx       = fitparams.par()(momindex + row) ;
      p.r(row) = posxmother - (posx - tau * momx) ;
      p.H(row, posindexmother + row) = 1 ;
      p.H(row, posindex + row)       = -1 ;
      p.H(row, momindex + row)       = tau ;
      p.H(row, tauindex + 1)         = momx ;
    }

    if (charge() != 0) {
      double lambda = bFieldOverC() * charge() ;
      double px0 = fitparams.par()(momindex + 1) ;
      double py0 = fitparams.par()(momindex + 2) ;
      double pt0 = sqrt(px0 * px0 + py0 * py0) ;
      const double posprecision = 1e-4 ; // 1mu
      if (fabs(pt0 * lambda * tau * tau) > posprecision) {
        // use the helix, but as if it were a 'correction'
        double sinlt = sin(lambda * tau) ;
        double coslt = cos(lambda * tau) ;
        double px = px0 * coslt - py0 * sinlt ;
        double py = py0 * coslt + px0 * sinlt ;

        p.r(1) += -tau * px0 + (py - py0) / lambda ;
        p.r(2) += -tau * py0 - (px - px0) / lambda ;

        p.H(1, tauindex + 1) += -px0 + px ;
        p.H(1, momindex + 1) += -tau + sinlt / lambda ;
        p.H(1, momindex + 2) += (coslt - 1) / lambda ;
        p.H(2, tauindex + 1) += -py0 + py ;
        p.H(2, momindex + 1) +=      - (coslt - 1) / lambda ;
        p.H(2, momindex + 2) += -tau + sinlt / lambda ;

        if (vtxverbose >= 2)
          std::cout << "Using helix for position of particle: " << name().c_str() << " "
                    << lambda << " " << lambda* tau
                    << "  delta-x,y: " << -tau* px0 + (py - py0) / lambda << "  "
                    << -tau* py0 - (px - px0) / lambda << std::endl ;
      }
    }
    return ErrCode::success ;
  }

  ErrCode ParticleBase::projectMassConstraint(const FitParams& fitparams,
                                              Projection& p) const
  {
    double mass = pdgMass() ;
    double mass2 = mass * mass ;
    int momindex = momIndex() ;

    // initialize the value
    double px = fitparams.par()(momindex + 1) ;
    double py = fitparams.par()(momindex + 2) ;
    double pz = fitparams.par()(momindex + 3) ;
    double E  = fitparams.par()(momindex + 4) ;
    p.r(1) = E * E - px * px - py * py - pz * pz - mass2 ;

    // calculate the projection matrix
    p.H(1, momindex + 1) = -2.0 * px ;
    p.H(1, momindex + 2) = -2.0 * py ;
    p.H(1, momindex + 3) = -2.0 * pz ;
    p.H(1, momindex + 4) =  2.0 * E ;

    return ErrCode::success ;
  }

  ErrCode ParticleBase::projectConstraint(Constraint::Type atype, const FitParams&, Projection&) const
  {
    std::cout << "no method to project this constaint: "
              << name().c_str() << " " << type() << " " << atype << std::endl;
    return ErrCode::badsetup ;
  }

  double ParticleBase::bFieldOverC() //FT: (to do) BField is already called in RecoTrack, unify
  {
    // in BaBar, BField::cmTeslaToGeVc = Constants::c/1.0E3
    // but is badly named as units are mm, ns, MeV, T
    // this is all very confusing, but we try to get it right
    // Conversion from Tesla to Belle2 units is already done, so no need for Unit::T (unlike in RecoTrack)
    static const double Bz  = BFieldManager::getField(TVector3(0, 0, 0)).Z() * Const::speedOfLight;
    B2DEBUG(80, "ParticleBase::bFieldOverC = " << Bz);
    return Bz;
  }

  ErrCode ParticleBase::initTau(FitParams* fitparams) const
  {
    int tauindex = tauIndex() ;
    if (tauindex >= 0 && hasPosition()) {
      const ParticleBase* amother = mother() ;
      int momposindex = amother ? amother->posIndex() : -1 ;
      int posindex = posIndex() ;
      int momindex = momIndex() ;
      assert(momposindex >= 0) ; // check code logic: no mother -> no tau
      //assert(fitparams->par(momposindex+1)!=0 ||fitparams->par(momposindex+2)!=0
      //       ||fitparams->par(momposindex+3)!=0) ; // mother must be initialized

      HepVector dX(3), mom(3);
      double mom2 = 0;
      for (int irow = 1; irow <= 3; ++irow) {
        dX(irow)  = fitparams->par(posindex + irow) - fitparams->par(momposindex + irow) ;
        double px = fitparams->par(momindex + irow) ;
        mom(irow) = px ;
        mom2 += px * px ;
      }
      double tau = dot(dX, mom) / mom2 ; //scalar product
      // we don't like 0 and we don't like very negative values
      if (tau == 0) tau = pdgTau() ;
      //tau = tau==0 ? pdtTau() : std::max(tau,-pdtTau()) ;
      fitparams->par(tauindex + 1) = tau;
    }
    return ErrCode::success ;
  }
} //end namespace TreeFitter

