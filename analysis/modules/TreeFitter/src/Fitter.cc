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

#include <analysis/modules/TreeFitter/Fitter.h>
#include <analysis/modules/TreeFitter/FitParams.h>
#include <analysis/modules/TreeFitter/DecayChain.h>
#include <analysis/modules/TreeFitter/ParticleBase.h>

#include <CLHEP/Matrix/Vector.h>
#include <CLHEP/Vector/LorentzVector.h>

using std::vector;

namespace TreeFitter {

  extern int vtxverbose ;

  bool Fitter::printDaughters(Particle* mother)
  {
    Particle* daughter;
    unsigned int i = 0;
    while (daughter = const_cast<Belle2::Particle*>(mother->getDaughter(i))) {
      std::cout << "The mother (" << mother->getName() << ") has a daughter, (" << daughter->getName() << ")." << std::endl;
      Fitter::printDaughters(daughter);
      i++;
    }
  }

  Fitter::Fitter(Particle* particle, double prec)
    : m_particle(particle), m_decaychain(0), m_fitparams(0), m_status(VertexStatus::UnFitted),
      m_chiSquare(-1), m_niter(-1), m_prec(prec)
  {
    // build the tree
    if (vtxverbose >= 2)
      std::cout << "TreeFitter::Fitter: building the tree" << std::endl ;
    m_decaychain = new DecayChain(particle, false) ;

    // allocate the fit parameters
    if (vtxverbose >= 2)
      std::cout << "allocating fit parameters" << std::endl ;
    m_fitparams  = new FitParams(m_decaychain->dim()) ;
  }

  Fitter::~Fitter()
  {
    //delete m_particle ; //do not delete, particle is owned by the framework
    delete m_decaychain ;
    delete m_fitparams ;
  }

  bool Fitter::fit()
  {
    const int nitermax = 10;
    const int maxndiverging = 3;
    const double dChisqConv = m_prec ;
    //const double dChisqQuit = nDof() ; // if chi2 increases by more than this --> fit failed

    // initialize
    m_chiSquare = -1 ;
    m_errCode.reset() ;
    if (m_status == VertexStatus::UnFitted)
      m_errCode = m_decaychain->init(m_fitparams) ;

    if (m_errCode.failure()) {
      // the input tracks are too far apart
      m_status = VertexStatus::BadInput ;

    } else {
      // reset the status flag
      m_status = VertexStatus::UnFitted ;

      int ndiverging = 0 ;
      bool finished = false ;
      B2DEBUG(80, "Fitter: Begin Iterations");
      for (m_niter = 0; m_niter < nitermax && !finished; ++m_niter) {
        HepVector prevpar = m_fitparams->par() ;
        bool firstpass = m_niter == 0 ;
        m_errCode = m_decaychain->filter(*m_fitparams, firstpass) ;
        double chisq = m_fitparams->chiSquare() ;
        double deltachisq = chisq - m_chiSquare ;
        // if chi2 increases by more than this --> fit failed
        const double dChisqQuit = std::max(double(2 * nDof()), 2 * m_chiSquare) ;
        B2DEBUG(80, "Fitter: Iteration #" << m_niter << " with Chi2=" << chisq << " and deltachisq=" << deltachisq);
        //
        if (m_errCode.failure()) {
          B2DEBUG(80, "Fitter: m_errCode.failure()");
          finished = true ;
          m_status = VertexStatus::Failed ;
        } else {
          B2DEBUG(80, "Fitter: m_errCode.success()");
          if (m_niter > 0) {
            if (fabs(deltachisq) < dChisqConv) {
              m_chiSquare = chisq ;
              m_status = VertexStatus::Success ;
              finished = true ;
              B2DEBUG(80, "Fitter: VertexStatus::Success");
            } else if (m_niter > 1 && deltachisq > dChisqQuit) {
              m_fitparams->par() = prevpar ;
              m_status  = VertexStatus::Failed ;
              m_errCode = ErrCode::fastdivergingfit ;
              B2DEBUG(80, "Fitter: ErrCode::fastdivergingfit");
              finished = true ;
            } else if (deltachisq > 0 && ++ndiverging >= maxndiverging) {
              m_fitparams->par() = prevpar ;
              m_status = VertexStatus::NonConverged ;
              m_errCode = ErrCode::slowdivergingfit ;
              B2DEBUG(80, "Fitter: ErrCode::slowdivergingfit");
              finished = true ;
            } else if (deltachisq > 0) {
              // make a half step and reduce stepsize
              m_fitparams->par()   += prevpar ;
              m_fitparams->par()   *= 0.5 ;
              B2DEBUG(80, "Fitter: reduce stepsize");
              //if( m_niter>10) m_fitparams->scale() *= 0.5 ;
            }
          }
          if (deltachisq < 0) ndiverging = 0 ; // start over with counting
          if (!finished) m_chiSquare = chisq ;
        }
        //
        if (vtxverbose >= 1) {
          std::cout << "step, chiSquare: "
                    << std::setw(3) << m_niter
                    << std::setw(3) << m_status
                    << std::setw(3) << nDof()
                    << std::setprecision(6)
                    << std::setw(12) << chisq
                    << std::setw(12) << deltachisq << std::endl ;
        }
        //
        if (vtxverbose >= 10) {
          print() ;
          std::cout << "press a key ...." << std::endl ;
          getchar() ;
        }
      }
      //
      if (m_niter == nitermax && m_status != VertexStatus::Success)
        m_status = VertexStatus::NonConverged ;

      if (!(m_fitparams->testCov())) {
        B2WARNING("TreeFitter::Fitter: Error matrix not positive definite. Changing status to failed.");
        m_status = VertexStatus::Failed ;
        //print() ;
      }
    }
    //
    //FT adding this to save the results, implementation could use improvements
    updateTree(*m_particle);
    //
    return (m_status == VertexStatus::Success); //FT make the function non-void
  }

  void Fitter::fitOneStep()
  {
    bool firstpass = m_status == VertexStatus::UnFitted ;
    if (firstpass) m_decaychain->init(m_fitparams) ;
    m_decaychain->filter(*m_fitparams, firstpass) ;
    m_chiSquare = m_fitparams->chiSquare() ;
    if (vtxverbose >= 1)
      std::cout << "In Fitter::fitOneStep(): " << m_status << " " << firstpass << " " << m_chiSquare << std::endl ;
    m_status = VertexStatus::Success ;
  }

  void Fitter::print() const
  {
    m_decaychain->mother()->print(m_fitparams) ;
    std::cout << "chisq,ndof,ncontr,niter,status: "
              << chiSquare() << " "
              << nDof() << " " << m_fitparams->nConstraints() << " "
              << nIter() << " " << status() << " " << m_errCode << std::endl ;
  }

  void Fitter::printConstraints(std::ostream& os) const
  {
    m_decaychain->printConstraints(os) ;
  }

  const HepSymMatrix& Fitter::cov() const
  {
    return m_fitparams->cov() ;
  }

  const HepVector& Fitter::par() const
  {
    return m_fitparams->par() ;
  }

  HepSymMatrix Fitter::cov(const vector<int>& indexVec) const
  {
    return m_fitparams->cov(indexVec) ;
  }

  HepVector Fitter::par(const vector<int>& indexVec) const
  {
    return m_fitparams->par(indexVec) ;
  }

  int Fitter::nDof() const
  {
    return m_fitparams->nDof() ;
  }

  int Fitter::posIndex(Particle* particle) const
  {
    return m_decaychain->posIndex(particle) ;
  }

  int Fitter::momIndex(Particle* particle) const
  {
    return m_decaychain->momIndex(particle) ;
  }

  int Fitter::tauIndex(Particle* particle) const
  {
    return m_decaychain->tauIndex(particle) ;
  }

  double Fitter::add(Particle& cand)
  {
    ParticleBase* bp = m_decaychain->mother()->addDaughter(&cand) ;
    int offset = m_fitparams->dim() ;
    bp->updateIndex(offset) ;
    double deltachisq(999) ;
    if (bp) {
      // resize the fitparams
      m_fitparams->resize(offset) ;
      // initialize the added track, filter and return the delta chisq
      bp->initPar1(m_fitparams) ;
      bp->initPar2(m_fitparams) ;
      bp->initCov(m_fitparams) ;

      ParticleBase::constraintlist constraints ;
      bp->addToConstraintList(constraints, 0) ;
      double chisq = m_fitparams->chiSquare() ;
      ErrCode status ;
      for (ParticleBase::constraintlist::const_iterator it = constraints.begin() ;
           it != constraints.end(); ++it)
        status |= it->filter(m_fitparams) ;

      deltachisq = m_fitparams->chiSquare() - chisq ;
      m_chiSquare = m_fitparams->chiSquare() ;

      // we want this somewhere else, but too much work now
      decaychain()->initConstraintList() ;

      //    print() ;
    } else {
      B2WARNING("Warning: cannot add track to this vertex ..." << m_decaychain->mother()->type());
    }
    return deltachisq ;
  }

  double Fitter::remove(Particle& cand)
  {
    ParticleBase* pb = const_cast<ParticleBase*>(m_decaychain->locate(&cand)) ;
    ErrCode status ;
    double dchisq(0) ;
    if (pb) {
      // filter with negative weight
      ParticleBase::constraintlist constraints ;
      pb->addToConstraintList(constraints, 0) ;
      double chisq = m_fitparams->chiSquare() ;
      for (ParticleBase::constraintlist::iterator it = constraints.begin() ;
           it != constraints.end(); ++it) {
        it->setWeight(-1) ;
        status |= it->filter(m_fitparams) ;
      }
      dchisq = chisq - m_fitparams->chiSquare() ;
      // remove
      ParticleBase* themother = const_cast<ParticleBase*>(pb->mother()) ;
      if (themother) themother->removeDaughter(pb);
    }
    return dchisq ;
  }

  void Fitter::updateIndex()
  {
    int offset = 0;
    m_decaychain->mother()->updateIndex(offset);
    m_fitparams->resize(offset);
  }

  double Fitter::globalChiSquare() const
  {
    return m_decaychain->chiSquare(m_fitparams) ;
  }

  //FT: this used to be btaFitParams, but since we don't use BtaFitParams and pass everything individually
  //it now only does only the one complicated part, i.e. building cov7

  HepSymMatrix Fitter::extractCov7(const ParticleBase* pb) const
  //FT: having a matrix as an output is incredibly slow for allocation reasons, this must change;
  //also this needs to output a TMatrixFSym because it's only used to feed back into the Belle2::Particle
  {
    int posindex = pb->posIndex() ;
    // hack: for tracks and photons, use the production vertex
    if (posindex < 0 && pb->mother()) posindex = pb->mother()->posIndex() ;
    int momindex = pb->momIndex() ;

    HepSymMatrix cov7(7, 0) ; //very important! Belle2 uses p,E,x! Change order here!
    if (pb->hasEnergy()) {
      // if particle has energy, get full p4 from fitparams
      int parmap[7] ;
      for (int i = 0; i < 4; ++i) parmap[i] = momindex + i ;
      for (int i = 0; i < 3; ++i) parmap[i + 4]   = posindex + i ;
      for (int row = 1; row <= 7; ++row)
        for (int col = 1; col <= row; ++col)
          cov7.fast(row, col) = m_fitparams->cov()(parmap[row - 1] + 1, parmap[col - 1] + 1) ;
    } else {
      // if not, use the pdttable mass

      HepSymMatrix cov6(6, 0) ;
      int parmap[6] ;
      for (int i = 0; i < 3; ++i) parmap[i] = momindex + i ; //energy should be after this
      for (int i = 0; i < 3; ++i) parmap[i + 3]   = posindex + i ;
      for (int row = 1; row <= 6; ++row)
        for (int col = 1; col <= row; ++col)
          cov6.fast(row, col) = m_fitparams->cov()(parmap[row - 1] + 1, parmap[col - 1] + 1) ; //used to use fast access in the original

      // now fill the jacobian
      double mass = pb->pdgMass() ;
      double energy2 = mass * mass ;
      for (int row = 1; row <= 3; ++row) {
        double px = m_fitparams->par()(momindex + row) ;
        energy2 += px * px ;
      }
      double energy = sqrt(energy2) ;
      //
      HepMatrix jacobian(7, 6, 0);
      for (int col = 1; col <= 3; ++col) {
        jacobian(col, col) = 1; // don't modify momentum
        jacobian(4, col) = m_fitparams->par()(momindex + col) / energy ; //add energy row
        jacobian(col + 4, col + 3) = 1; // slightly off diagonal position identity
      }
      //
      cov7 = cov6.similarity(jacobian) ;
    }
    return cov7;
  }

  /*
  //This is now done in the updater or not at all
  BtaFitParams Fitter::btaFitParams(const Particle& cand) const
  {
    const ParticleBase* pb = m_decaychain->locate(&cand) ;
    if(pb==0) {
      std::cout << "Error: " << "can't find candidate in tree: " << cand.getName()
        << " head of tree = " << m_particle.getName()
        << std::endl;
      return BtaFitParams() ;
    }
    return btaFitParams(pb) ;
  }
  */

  Particle Fitter::getFitted()
  {
    Particle thecand = *particle() ; //fishy use of pointers...
    updateCand(thecand) ;
    B2DEBUG(80, "Fitter::getFitted()");
    return thecand ;
  }

  Particle Fitter::getFitted(Particle& cand)
  {
    Particle thecand = cand ;
    updateCand(thecand) ;
    return thecand ;
  }

  Particle* Fitter::fittedCand(Particle& cand, Particle* headOfTree)
  {
    B2WARNING("TreeFitter::Fitter::fittedCand: WARNING: Not implemented");
    return 0 ;
    // assigns fitted parameters to candidate in tree
    //    Particle* acand = const_cast<Particle*>(headOfTree->cloneInTree(cand)) ;
    //    updateCand(*acand) ;
    //    return acand ;
  }

  Particle Fitter::getFittedTree()
  {
    Particle cand = *particle() ;
    updateTree(cand) ;
    return cand ;
  }

  bool Fitter::updateCand(Particle& cand) const
  {
    // assigns fitted parameters to a candidate
    const ParticleBase* pb = m_decaychain->locate(&cand) ;
    B2DEBUG(80, "Fitter::updateCand(" << cand.getName() << ") is " << pb);
    if (pb) updateCand(*pb, cand);
    else {
      // this error message does not make sense, because it is also
      // triggered for daughters that were simply not refitted. we
      // have to do something about that.
      std::cout << "Can't find candidate " << cand.getName() << std::endl
                << "in tree " << m_particle->getName() << std::endl;
      return pb != 0 ;
    }
  }


  void Fitter::updateCand(const ParticleBase& pb, Particle& cand) const //FT: this is very delicate, come back here in case of errors
  {
    B2DEBUG(80, "Updating the candidate " << cand.getName());
    //    assert( pb->particle()->getPDGCode() == cand.getPDGCode() ) ; //sanity check
    int posindex = pb.posIndex() ;
    bool hasPos = true;
    // hack: for tracks and photons, use the production vertex
    if (posindex < 0 && pb.mother()) {
      posindex = pb.mother()->posIndex() ;
      hasPos = false;
    }
    int momindex = pb.momIndex() ;

    TVector3 pos; //FT: this is not optimal
    pos[0] = m_fitparams->par()(posindex + 1);
    pos[1] = m_fitparams->par()(posindex + 2);
    pos[2] = m_fitparams->par()(posindex + 3);

    TLorentzVector p;
    p.SetPx(m_fitparams->par()(momindex + 1)) ;
    p.SetPy(m_fitparams->par()(momindex + 2)) ;
    p.SetPz(m_fitparams->par()(momindex + 3)) ;
    //FT: set energy not needed for FS since there's a mass hypotesis, but we need invariant mass for others
    if (pb.hasEnergy()) { //FT: should be the same as posindex <0 (double check?)
      // if particle has energy, full p4 from fitparams
      p.SetE(m_fitparams->par()(momindex + 4));
      B2DEBUG(80, "Setting momentum of candidate to " << p.Px() << " " << p.Py() << " " << p.Pz() << " " << p.E());
      cand.set4Vector(p);
      //      TLorentzVector p_temp = cand.get4Vector();//FT: test printout
      //      B2DEBUG(80, "Momentum of candidate is " << p_temp.Px() << " " << p_temp.Py() << " " << p_temp.Pz() << " " << p_temp.E());
    } else {
      double mass = cand.getMass();           //since when I feed a p4 in Particle what gets stored is actually the mass,
      p.SetE(p.Vect()*p.Vect() + mass * mass); //I risk rounding errors for no benefit
      cand.set4Vector(p);
    }
    HepSymMatrix cov7 = extractCov7(&pb);
    //    B2DEBUG(80,"Fitter::updateCand - extracted cov7 matrix is " << cov7.num_row() << "x" << cov7.num_col() << ".");
    TMatrixFSym cov7b2(7);
    //FT: this can be improved
    for (int row = 1; row <= 7; ++row)
      for (int col = 1; col <= row; ++col)
        cov7b2[row - 1][col - 1] = cov7.fast(row, col);
    //    B2DEBUG(80,"Fitter::updateCand - converted cov7b2 matrix is " << cov7b2.GetNrows() << "x" << cov7b2.GetNcols() << ".");
    //if has position, update position;
    //update p4
    //update cov7
    //if is the head, update chi square and dof

    cand.setMomentumVertexErrorMatrix(cov7b2);

    //FT: save lifetime and decay length as ExtraInfo
    TVector2 tau  = decayLength(cand);
    TVector2 life = lifeTime(cand);
    if (cand.hasExtraInfo("decayLength")) {
      B2DEBUG(80, "Fitter::updateCand - existing previous instance of ExtraInfo(decayLength)");
      cand.setExtraInfo("decayLength", tau.X());
    } else {
      B2DEBUG(80, "Fitter::updateCand - saving ExtraInfo(decayLength)");
      cand.addExtraInfo("decayLength", tau.X());
    }
    if (cand.hasExtraInfo("decayLengthErr")) {
      B2DEBUG(80, "Fitter::updateCand - existing previous instance of ExtraInfo(decayLengthErr)");
      cand.setExtraInfo("decayLengthErr", tau.Y());
    } else {
      B2DEBUG(80, "Fitter::updateCand - saving ExtraInfo(decayLengthErr)");
      cand.addExtraInfo("decayLengthErr", tau.Y());
    }
    if (cand.hasExtraInfo("lifeTime")) {
      B2DEBUG(80, "Fitter::updateCand - existing previous instance of ExtraInfo(lifeTime)");
      cand.setExtraInfo("lifeTime", life.X());
    } else {
      B2DEBUG(80, "Fitter::updateCand - saving ExtraInfo(lifeTime)");
      cand.addExtraInfo("lifeTime", life.X());
    }
    if (cand.hasExtraInfo("lifeTimeErr")) {
      B2DEBUG(80, "Fitter::updateCand - existing previous instance of ExtraInfo(lifeTimeErr)");
      cand.setExtraInfo("lifeTimeErr", life.Y());
    } else {
      B2DEBUG(80, "Fitter::updateCand - saving ExtraInfo(lifeTimeErr)");
      cand.addExtraInfo("lifeTimeErr", life.Y());
    }
    //
    if (hasPos) {  //if not final state
      cand.setVertex(pos);
      if (&pb == m_decaychain->cand()) { // if head
        cand.setPValue(TMath::Prob(chiSquare(), nDof())); //FT: (to do) p-values of fit must be verified
      }
    }
  }

  void Fitter::updateTree(Particle& cand) const
  {
    B2DEBUG(80, "Fitter::fit: Updating tree " << cand.getName());
    // assigns fitted parameters to all candidates in a decay tree

    //    B2DEBUG(80,"Fitter::updateTree:: Printing tree from" << cand.getName());
    //    printDaughters(const_cast<Belle2::Particle*>(&cand));
    //    B2DEBUG(80,"... and now let's update.");

    if (updateCand(cand)) { // if the mother can be updated, update the daughters

      int ndaughters = cand.getNDaughters();
      Particle* daughter;
      for (int i = 0; i < ndaughters; i++) {
        daughter = const_cast<Belle2::Particle*>(cand.getDaughter(i));
        updateTree(*daughter);
      }
    }
  }

  TVector2 Fitter::lifeTime(Particle& cand) const
  {
    // returns the lifetime in the rest frame of the candidate
    TVector2 rc;
    const ParticleBase* pb = m_decaychain->locate(&cand) ;
    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      int tauindex = pb->tauIndex() ;
      double tau    = m_fitparams->par()(tauindex + 1) ;
      double taucov = m_fitparams->cov()(tauindex + 1, tauindex + 1) ;
      double mass   = pb->pdgMass() ;
      double convfac = mass / Const::speedOfLight;
      rc = {convfac * tau, convfac* convfac * taucov} ;
    }
    return rc ;
  }

  TVector2 Fitter::decayLength(const ParticleBase* pb) const
  {
    // returns the decaylength in the lab frame
    return decayLength(pb, m_fitparams) ;
  }

  TVector2 Fitter::decayLength(const ParticleBase* pb, const FitParams* fitparams)
  {
    // returns the decaylength in the lab frame
    TVector2 rc;
    if (pb->tauIndex() >= 0 && pb->mother()) {
      // one can calculate the error in many ways. I managed to make
      // them all agree, with a few outliers. this one seems to be
      // most conservative/stable/simple one.

      // len = tau |mom|
      int tauindex = pb->tauIndex() ;
      int momindex = pb->momIndex() ;
      double tau   = fitparams->par()(tauindex + 1) ;
      double mom2(0) ;
      for (int row = 1; row <= 3; ++row) {
        double px = fitparams->par()(momindex + row - 1) ;
        mom2 += px * px ;
      }
      double mom = sqrt(mom2);
      double len = mom * tau;

      vector<int> indexvec ;
      indexvec.push_back(tauindex) ;
      indexvec.push_back(momindex + 0) ;
      indexvec.push_back(momindex + 1) ;
      indexvec.push_back(momindex + 2) ;

      HepVector jacobian(4) ;
      jacobian[0] = mom ;
      jacobian[1] = tau * fitparams->par()(momindex + 1) / mom ;
      jacobian[2] = tau * fitparams->par()(momindex + 2) / mom ;
      jacobian[3] = tau * fitparams->par()(momindex + 3) / mom ;

      rc.SetX(len);
      rc.SetY(fitparams->cov(indexvec).similarity(jacobian));
    }
    return rc ; //returns (0,0) if the particle has no decay length
  }

  TVector2 Fitter::decayLength(Particle& cand) const //FT: are all these instances of decayLength required?
  {
    TVector2 rc;
    const ParticleBase* pb = m_decaychain->locate(&cand) ;
    if (pb && pb->tauIndex() >= 0 && pb->mother()) rc = decayLength(pb) ;
    return rc ;
  }

  TVector2 Fitter::decayLengthSum(const ParticleBase* pbA, const ParticleBase* pbB) const
  {
    // returns the decaylengthsum in the lab frame
    TVector2 rc;
    if (pbA->tauIndex() >= 0 && pbA->mother() &&
        pbB->tauIndex() >= 0 && pbB->mother()) {

      // len = tau |mom|
      int tauindexA = pbA->tauIndex() ;
      int momindexA = pbA->momIndex() ;
      double tauA   = m_fitparams->par()(tauindexA + 1) ;
      double mom2A;
      for (int row = 1; row <= 3; ++row) {
        double px = m_fitparams->par()(momindexA + row) ;
        mom2A += px * px ;
      }
      double momA = sqrt(mom2A) ;
      double lenA = momA * tauA ;

      int tauindexB = pbB->tauIndex() ;
      int momindexB = pbB->momIndex() ;
      double tauB    = m_fitparams->par()[tauindexB] ;
      double mom2B;
      for (int row = 1; row <= 3; ++row) {
        double px = m_fitparams->par()[momindexB + row - 1] ;
        mom2B += px * px ;
      }
      double momB = sqrt(mom2B) ;
      double lenB = momB * tauB ;

      vector<int> indexvec ;
      indexvec.push_back(tauindexA);
      for (int i = 0; i < 3; ++i) indexvec.push_back(momindexA + i) ;
      indexvec.push_back(tauindexB);
      for (int i = 0; i < 3; ++i) indexvec.push_back(momindexB + i) ;

      HepVector jacobian(8) ;
      jacobian(1) = momA ;
      for (int irow = 1; irow <= 3; ++irow)
        jacobian(irow + 1) = tauA * m_fitparams->par()(momindexA + irow) / momA ;
      jacobian(5) = momB ;
      for (int irow = 1; irow <= 3; ++irow)
        jacobian(irow + 5) = tauB * m_fitparams->par()(momindexB + irow) / momB ;

      rc.SetX(lenA + lenB);
      rc.SetY(cov(indexvec).similarity(jacobian));
    }
    return rc ;
  }

  TVector2 Fitter::decayLengthSum(Particle& candA, Particle& candB) const
  {
    TVector2 rc;
    const ParticleBase* pbA = m_decaychain->locate(&candA) ;
    const ParticleBase* pbB = m_decaychain->locate(&candB) ;
    if (pbA && pbB)  rc = decayLengthSum(pbA, pbB) ;
    return rc ;
  }

}

