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

  bool Fitter::fitUseEigen()
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
      // reset the status flag
      m_status = VertexStatus::UnFitted;
      int ndiverging = 0;
      bool finished = false;
      double deltachisq = 1e10;

      B2DEBUG(80, "############################");
      B2DEBUG(80, "# Fitter: Begin Iterations #");
      B2DEBUG(80, "############################");

      for (m_niter = 0; m_niter < nitermax && !finished; ++m_niter) {

        EigenTypes::ColVector prevpar = m_fitparams->getStateVector();

        bool firstpass = (m_niter == 0);
        m_errCode = m_decaychain->filterCopy(*m_fitparams, firstpass);

        double chisq = m_fitparams->chiSquare();//   m_fitparams->nDof();
        // JFK: this should be chi2 quit so that if chi2 > that the fit is aborted Tue 05 Sep 2017 08:58:51 AM CEST
        double dChisqQuit = std::max(double(2 * nDof()), 2 * m_chiSquare);

        deltachisq = chisq - m_chiSquare;
        // if chi2 increases by more than this --> fit failed
        //const double dChisqQuit = std::max(double(2 * nDof()), 2 * m_chiSquare);
        B2DEBUG(80, "----------------------------------------------------------------------");
        B2DEBUG(80, "Fitter: Iteration #" << m_niter << " with Chi2 = " << chisq << " and deltachisq = " << deltachisq);
        B2DEBUG(80, "----------------------------------------------------------------------");

        if (m_errCode.failure()) {
          B2DEBUG(80, "____________________________________________________________");
          B2DEBUG(80, "-----------> Fitter: VertexStatus::FAILED      <-----------");
          B2DEBUG(80, "____________________________________________________________");
          finished = true ;
          m_status = VertexStatus::Failed;
        } else {
          B2DEBUG(80, "Fitter: m_errCode.success()");

          if (m_niter > 0) {
            if (fabs(deltachisq) < dChisqConv) {
              m_chiSquare = chisq;
              m_status = VertexStatus::Success;
              finished = true ;
              B2DEBUG(10, "____________________________________________________________");
              B2DEBUG(10, "->->->->->-> Fitter: VertexStatus::SUCCESS      <-<-<-<-<-<-");
              B2DEBUG(10, "____________________________________________________________");
            } else if (m_niter > 1 && deltachisq > dChisqQuit) {
              m_fitparams->getStateVector() = prevpar;
              m_status  = VertexStatus::Failed;
              m_errCode = ErrCode::fastdivergingfit;
              B2DEBUG(10, "____________________________________________________________");
              B2DEBUG(10, "-----------> Fitter: ErrCode::fastdivergingfit  <-----------");
              B2DEBUG(10, "____________________________________________________________");
              finished = true;
            } else if (deltachisq > 0 && ++ndiverging >= maxndiverging) {
              m_fitparams->getStateVector() = prevpar;
              m_status = VertexStatus::NonConverged;
              m_errCode = ErrCode::slowdivergingfit;
              B2DEBUG(10, "____________________________________________________________");
              B2DEBUG(10, "-----------> Fitter: ErrCode::slowdivergingfit  <-----------");
              B2DEBUG(10, "____________________________________________________________");
              finished = true ;
            } else if (deltachisq > 0) {
              B2DEBUG(10, "____________________________________________________________");
              B2DEBUG(10, "-----------> Fitter: reduce stepsize            <-----------");
              B2DEBUG(10, "____________________________________________________________");
            }
          }

          if (deltachisq < 0) {
            ndiverging = 0;
          } // start over with counting
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
    //
    //FT adding this to save the results, implementation could use improvements
    //
    m_chi2sum = m_decaychain->getChi2Sum();
    updateTreeCopy(*m_particle);
    return (m_status == VertexStatus::Success); //FT make the function non-void
  }




  const CLHEP::HepSymMatrix& Fitter::cov() const
  {
    return m_fitparams->cov();
  }
  const EigenTypes::MatrixXd& Fitter::getCovariance() const
  {
    return m_fitparams->getCovariance();
  }



  const CLHEP::HepVector& Fitter::par() const
  {
    return m_fitparams->par();
  }
  const EigenTypes::ColVector& Fitter::getStateVector() const
  {
    return m_fitparams->getStateVector();
  }




  CLHEP::HepSymMatrix Fitter::cov(const vector<int>& indexVec) const
  {
    return m_fitparams->cov(indexVec);
  }

  CLHEP::HepVector Fitter::par(const vector<int>& indexVec) const
  {
    return m_fitparams->par(indexVec);
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

  //FIXME unused, delte? (the version with an argument is however not unused)
  void Fitter::updateIndex()
  {
    int offset = 0;
    m_decaychain->mother()->updateIndex(offset);
    m_fitparams->resize(offset);
  }

  double Fitter::globalChiSquare() const
  {
    return m_decaychain->chiSquare(m_fitparams);
  }

  //FT: this used to be btaFitParams, but since we don't use BtaFitParams and pass everything individually
  //it now only does only the one complicated part, i.e. building cov7

  CLHEP::HepSymMatrix Fitter::extractCov7(const ParticleBase* pb) const
  //FT: having a matrix as an output is incredibly slow for allocation reasons, this must change;
  //also this needs to output a TMatrixFSym because it's only used to feed back into the Belle2::Particle
  {
    int posindex = pb->posIndex();
    // hack: for tracks and photons, use the production vertex
    if (posindex < 0 && pb->mother()) {
      posindex = pb->mother()->posIndex();
    }
    int momindex = pb->momIndex();

    CLHEP::HepSymMatrix cov7(7, 0); //very important! Belle2 uses p,E,x! Change order here!
    if (pb->hasEnergy()) {
      // if particle has energy, get full p4 from fitparams
      int parmap[7] ;
      for (int i = 0; i < 4; ++i) {
        parmap[i] = momindex + i;
      }

      for (int i = 0; i < 3; ++i) {
        parmap[i + 4]   = posindex + i;
      }

      for (int row = 1; row <= 7; ++row) {
        for (int col = 1; col <= row; ++col) {
          cov7.fast(row, col) = m_fitparams->cov()(parmap[row - 1] + 1, parmap[col - 1] + 1);
        }
      }
    } else {
      // if not, use the pdttable mass

      CLHEP::HepSymMatrix cov6(6, 0);
      int parmap[6];
      for (int i = 0; i < 3; ++i) {
        parmap[i] = momindex + i; //energy should be after this
      }

      for (int i = 0; i < 3; ++i) {
        parmap[i + 3]   = posindex + i;
      }

      for (int row = 1; row <= 6; ++row) {
        for (int col = 1; col <= row; ++col) {
          cov6.fast(row, col) = m_fitparams->cov()(parmap[row - 1] + 1, parmap[col - 1] + 1); //used to use fast access in the original
        }
      }

      // now fill the jacobian
      double mass = pb->pdgMass();
      double energy2 = mass * mass;
      for (int row = 1; row <= 3; ++row) {
        double px = m_fitparams->par()(momindex + row);
        energy2 += px * px;
      }

      double energy = sqrt(energy2);

      CLHEP::HepMatrix jacobian(7, 6, 0);
      for (int col = 1; col <= 3; ++col) {
        jacobian(col, col) = 1; // don't modify momentum
        jacobian(4, col) = m_fitparams->par()(momindex + col) / energy; //add energy row
        jacobian(col + 4, col + 3) = 1; // slightly off diagonal position identity
      }

      cov7 = cov6.similarity(jacobian);
    }
    return cov7;
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
    if (pb->hasEnergy()) {
      B2DEBUG(80, "       Fitter::getCovFromPB for a particle with energy");
      // if particle has energy, get full p4 from fitparams and put them directly in the return type
      //very important! Belle2 uses p,E,x! Change order here!
      int parmap[7] ;
      for (int i = 0; i < 4; ++i) {
        parmap[i] = momindex + i;
      }
      for (int i = 0; i < 3; ++i) {
        parmap[i + 4]   = posindex + i;
      }
      for (int row = 0; row < 7; ++row) {
        for (int col = 0; col <= row; ++col) {
          returncov(row, col) = cov(parmap[row], parmap[col]);
        }
      }

    } else {


      B2DEBUG(80, "       Fitter::getCovFromPB for a particle with energy");
      // if not, use the pdttable mass
      EigenTypes::MatrixXd cov6 = EigenTypes::MatrixXd::Zero(6, 6);
      int parmap[6];
      for (int i = 0; i < 3; ++i) {
        parmap[i] = momindex + i; //energy should be after this
      }
      for (int i = 0; i < 3; ++i) {
        parmap[i + 3]   = posindex + i;
      }


      for (int row = 0; row < 6; ++row) {
        for (int col = 0; col <= row; ++col) {
          cov6(row, col) = cov(parmap[row], parmap[col]);
        }
      }
      // now fill the jacobian
      double mass = pb->pdgMass();
      EigenTypes::ColVector momVec = m_fitparams->getStateVector().segment(momindex, 3);
      double energy2 = momVec.transpose() * momVec;
      //double energy2 = momVec.transpos() * momVec.transpose();
      energy2 += mass * mass;
      double energy = sqrt(energy2);
      EigenTypes::MatrixXd jacobian = EigenTypes::MatrixXd::Zero(7, 6);
      //JFK: there was an old comment on the part that set the diagonal se below. does this make sense? 2017-09-28
      // don't modify momentum
      for (int col = 0; col < 3; ++col) {
        jacobian(col, col) = 1; // don't modify momentum
        jacobian(4, col) = m_fitparams->getStateVector()(momindex + col) / energy; //add energy row
        jacobian(col + 4, col + 3) = 1; // slightly off diagonal position identity
      }
      EigenTypes::MatrixXd cov7 = EigenTypes::MatrixXd::Zero(7, 7);

      cov7 = jacobian * cov6.selfadjointView<Eigen::Lower>() * jacobian.transpose();
      //JFK: now put everything in the return type 2017-09-28
      for (int row = 0; row < 7; ++row) {
        for (int col = 0; col <= row; ++col) {
          returncov(row, col) = cov7(row, col);
        }
      }
    }
  }




  //FIXME unused, delete?
  Belle2::Particle Fitter::getFitted()
  {
    Belle2::Particle thecand = *particle(); //fishy use of pointers...FIXME
    updateCand(thecand);
    B2DEBUG(80, "Fitter::getFitted()");
    return thecand;
  }

  //FIXME unused, delete?
  Belle2::Particle Fitter::getFitted(Belle2::Particle& cand)
  {
    Belle2::Particle thecand = cand;
    updateCand(thecand);
    return thecand;
  }

  //FT: Unused but kept for future use
  //JFK FIXME can we remove this??
  /*
  Particle* Fitter::fittedCand(Particle& cand, Particle* headOfTree)
  {
    B2WARNING("TreeFitter::Fitter::fittedCand: WARNING: Not implemented");
    return 0 ;
    // assigns fitted parameters to candidate in tree
    //    Particle* acand = const_cast<Particle*>(headOfTree->cloneInTree(cand)) ;
    //    updateCand(*acand) ;
    //    return acand ;
  }
  */

  Belle2::Particle Fitter::getFittedTree()
  {
    Belle2::Particle cand = *particle();
    updateTree(cand);
    return cand;
  }

  bool Fitter::updateCand(Belle2::Particle& cand) const
  {
    // assigns fitted parameters to a candidate
    const ParticleBase* pb = m_decaychain->locate(&cand);
    B2DEBUG(80, "Fitter::updateCand(" << cand.getName() << ") is " << pb);

    if (pb) {
      updateCand(*pb, cand);
    } else {
      // this error message does not make sense, because it is also FIXME
      // triggered for daughters that were simply not refitted. we
      // have to do something about that.
      //B2ERROR("Can't find candidate " << cand.getName() << "in tree " << m_particle->getName());
    }
    return pb != 0;
  }
  bool Fitter::updateCandCopy(Belle2::Particle& cand) const
  {
    // assigns fitted parameters to a candidate
    const ParticleBase* pb = m_decaychain->locate(&cand);
    B2DEBUG(80, " Fitter::updateCand(" << cand.getName() << ") is " << pb);

    if (pb) {
      updateCandCopy(*pb, cand);
    } else {
      // this error message does not make sense, because it is also FIXME
      // triggered for daughters that were simply not refitted. we
      // have to do something about that.
      //B2ERROR("Can't find candidate " << cand.getName() << "in tree " << m_particle->getName());
    }
    return pb != 0;
  }


  void Fitter::updateCand(const ParticleBase& pb,
                          Belle2::Particle& cand) const //FT: this is very delicate, come back here in case of errors
  {
    B2DEBUG(80, "Updating the candidate " << cand.getName());
    //B2DEBUG(19, "Updating the candidate " << cand.getName());
    //    assert( pb->particle()->getPDGCode() == cand.getPDGCode() ) ; //sanity check
    int posindex = pb.posIndex();
    bool hasPos = true;
    // hack: for tracks and photons, use the production vertex

    if (posindex < 0 && pb.mother()) {
      posindex = pb.mother()->posIndex();
      hasPos = false;
    }

    int momindex = pb.momIndex();

    TVector3 pos; //FT: this is not optimal
    pos[0] = m_fitparams->par()(posindex + 1);
    pos[1] = m_fitparams->par()(posindex + 2);
    pos[2] = m_fitparams->par()(posindex + 3);

    TLorentzVector p;
    p.SetPx(m_fitparams->par()(momindex + 1));
    p.SetPy(m_fitparams->par()(momindex + 2));
    p.SetPz(m_fitparams->par()(momindex + 3));

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

    CLHEP::HepSymMatrix cov7 = extractCov7(&pb);
    //    B2DEBUG(80,"Fitter::updateCand - extracted cov7 matrix is " << cov7.num_row() << "x" << cov7.num_col() << ".");
    TMatrixFSym cov7b2(7);

    //FT: this can be improved
    for (int row = 1; row <= 7; ++row) {
      for (int col = 1; col <= row; ++col) {
        cov7b2[row - 1][col - 1] = cov7.fast(row, col);
      }
    }
    //    B2DEBUG(80,"Fitter::updateCand - converted cov7b2 matrix is " << cov7b2.GetNrows() << "x" << cov7b2.GetNcols() << ".");
    //if has position, update position;
    //update p4
    //update cov7
    //if is the head, update chi square and dof

    cand.setMomentumVertexErrorMatrix(cov7b2);

    //FT: save lifetime and decay length as ExtraInfo. This is must be interfaced with the FlightInfo NtupleTool.
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

    if (hasPos) {  //if not final state
      cand.setVertex(pos);
      if (&pb == m_decaychain->cand()) { // if head

        // JFK: this is the confusing part. there are 3 chi2. The kalman iteration, the newton iteration
        // the chi2 of the fit should be the one summed down the deacay tree.  Wed 06 Sep 2017 09:04:53 AM CEST
        //double chi2byndf = chiSquare();
        double NDFs = nDof();
        //double decchi2 = m_decaychain->getChainsChi2(m_fitparams);
        double fitparchi2 = m_fitparams->chiSquare();

        // B2DEBUG(19, "____________________________________________ "  );
        // B2DEBUG(19, "  -- FINAL chi2 newton    :    " << chi2byndf << " with ndf = " << NDFs );
        // B2DEBUG(19, "  -- Chains chi2/ndf      :    " << decchi2 / NDFs   );
        // B2DEBUG(19, "  -- Fitpars chi2/ndf     :    " << fitparchi2 / NDFs   );
        // B2DEBUG(19, "  -- FINAL p VALUE decaychain: " << TMath::Prob(decchi2   , NDFs)  );
        // B2DEBUG(19, "  -- FINAL p VALUE fitpars   : " << TMath::Prob(fitparchi2   , NDFs)  );
        //cand.setPValue(TMath::Prob(chiSquare() , nDof()));   //FT: (to do) p-values of fit must be verified
        //B2DEBUG(19, "____________________________________________ "  );
        //cand.setPValue(fitparchi2);   //FT: (to do) p-values of fit must be
        B2DEBUG(80, "Fitter::setting PVal: " << TMath::Prob(fitparchi2, NDFs) << " Chi2: " << fitparchi2 << " NDF: " << NDFs);
        cand.setPValue(TMath::Prob(fitparchi2, NDFs));   //FT: (to do) p-values of fit must be verified

      }
    }
  }
  void Fitter::updateCandCopy(const ParticleBase& pb,
                              Belle2::Particle& cand) const //FT: this is very delicate, come back here in case of errors
  {
    B2DEBUG(80, "   Updating the candidate " << cand.getName() << " internal " << pb.name());
    //    assert( pb->particle()->getPDGCode() == cand.getPDGCode() ) ; //sanity check
    int posindex = pb.posIndex();
    bool hasPos = true;
    // hack: for tracks and photons, use the production vertex
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

    //FT: set energy not needed for FS since there's a mass hypotesis, but we need invariant mass for others
    if (pb.hasEnergy()) { //FT: should be the same as posindex <0 (double check?)
      // if particle has energy, full p4 from fitparams
      p.SetE(m_fitparams->getStateVector()(momindex + 3));
      B2DEBUG(80, "   Setting momentum of candidate to " << p.Px() << " " << p.Py() << " " << p.Pz() << " " << p.E());
      cand.set4Vector(p);
      //      TLorentzVector p_temp = cand.get4Vector();//FT: test printout
      //      B2DEBUG(80, "Momentum of candidate is " << p_temp.Px() << " " << p_temp.Py() << " " << p_temp.Pz() << " " << p_temp.E());
    } else {
      double mass = cand.getMass();           //since when I feed a p4 in Particle what gets stored is actually the mass,
      p.SetE(p.Vect()*p.Vect() + mass * mass); //I risk rounding errors for no benefit
      cand.set4Vector(p);
    }

    //JFK: if the particle does not have energy we fill with -999 2017-09-28
    //JFK: FIXME does this make sense? 2017-09-28
    TMatrixFSym cov7b2(7);

    getCovFromPB(&pb, cov7b2);
    B2DEBUG(80, "   Setting momentum error Matrix (first diag elms): " << cov7b2(0, 0) << " " << cov7b2(1, 1) << " " << cov7b2(2, 2));
    cand.setMomentumVertexErrorMatrix(cov7b2);
    //FT: save lifetime and decay length as ExtraInfo. This is must be interfaced with the FlightInfo NtupleTool.
    std::tuple<double, double>tau  = getDecayLength(cand);

    std::tuple<double, double>life = getLifeTime(cand);
    if (cand.hasExtraInfo("decayLength")) {
      B2DEBUG(80, "   Fitter::updateCand - existing previous instance of ExtraInfo(decayLength) = " << std::get<0>(tau));
      cand.setExtraInfo("decayLength", std::get<0>(tau));
    } else {
      B2DEBUG(80, "   Fitter::updateCand - saving ExtraInfo(decayLength) = " << std::get<0>(tau));
      cand.addExtraInfo("decayLength", std::get<0>(tau));
    }

    if (cand.hasExtraInfo("decayLengthErr")) {
      B2DEBUG(80, "   Fitter::updateCand - existing previous instance of ExtraInfo(decayLengthErr) = " << std::get<1>(tau));
      cand.setExtraInfo("decayLengthErr", std::get<1>(tau));
    } else {
      B2DEBUG(80, "   Fitter::updateCand - saving ExtraInfo(decayLengthErr) = " << std::get<1>(tau));
      cand.addExtraInfo("decayLengthErr", std::get<1>(tau));
    }

    if (cand.hasExtraInfo("lifeTime")) {
      B2DEBUG(80, "   Fitter::updateCand - existing previous instance of ExtraInfo(lifeTime) = " << std::get<0>(life));
      cand.setExtraInfo("lifeTime", std::get<0>(life));
    } else {
      B2DEBUG(80, "   Fitter::updateCand - saving ExtraInfo(lifeTime) = " << std::get<0>(life));
      cand.addExtraInfo("lifeTime", std::get<0>(life));
    }

    if (cand.hasExtraInfo("lifeTimeErr")) {
      B2DEBUG(80, "   Fitter::updateCand - existing previous instance of ExtraInfo(lifeTimeErr) = " << std::get<1>(life));
      cand.setExtraInfo("lifeTimeErr", std::get<1>(life));
    } else {
      B2DEBUG(80, "   Fitter::updateCand - saving ExtraInfo(lifeTimeErr) = " << std::get<1>(life));
      cand.addExtraInfo("lifeTimeErr", std::get<1>(life));
    }


    if (hasPos) {  //if not final state
      cand.setVertex(pos);
      if (&pb == m_decaychain->cand()) { // if head

        // JFK: this is the confusing part. there are 3 chi2. The kalman iteration, the newton iteration
        // the chi2 of the fit should be the one summed down the deacay tree.  Wed 06 Sep 2017 09:04:53 AM CEST
        //double chi2byndf = chiSquare();
        double NDFs = nDof();
        //double decchi2 = m_decaychain->getChainsChi2(m_fitparams);
        double fitparchi2 = m_fitparams->chiSquare();

        // B2DEBUG(19, "____________________________________________ "  );
        // B2DEBUG(19, "  -- FINAL chi2 newton    :    " << chi2byndf << " with ndf = " << NDFs );
        // B2DEBUG(19, "  -- Chains chi2/ndf      :    " << decchi2 / NDFs   );
        // B2DEBUG(19, "  -- FINAL p VALUE decaychain: " << TMath::Prob(decchi2   , NDFs)  );
        //cand.setPValue(TMath::Prob(chiSquare() , nDof()));   //FT: (to do) p-values of fit must be verified
        //B2DEBUG(19, "____________________________________________ "  );
        //cand.setPValue(fitparchi2);   //FT: (to do) p-values of fit must be verified
        //
        B2DEBUG(80, "   Fitter::setting PVal: " << TMath::Prob(fitparchi2,
                                                               NDFs) << " Chi2/NDF = " << fitparchi2 / NDFs   << " Chi2: " << fitparchi2 << " NDF: " << NDFs);
        cand.setPValue(TMath::Prob(fitparchi2, NDFs));   //FT: (to do) p-values of fit must be verified

      }

    }
  }




  void Fitter::updateTree(Belle2::Particle& cand) const
  {
    B2DEBUG(80, " Fitter::fit: Updating tree " << cand.getName());
    // assigns fitted parameters to all candidates in a decay tree

    //    B2DEBUG(80,"Fitter::updateTree:: Printing tree from" << cand.getName());

    if (updateCand(cand)) { // if the mother can be updated, update the daughters
      int ndaughters = cand.getNDaughters();
      Belle2::Particle* daughter;
      for (int i = 0; i < ndaughters; i++) {
        daughter = const_cast<Belle2::Particle*>(cand.getDaughter(i));
        updateTree(*daughter);
      }
    }
  }
  void Fitter::updateTreeCopy(Belle2::Particle& cand) const
  {

    B2DEBUG(80, " Fitter::fit: Updating tree " << cand.getName());
    if (updateCandCopy(cand)) { // if the mother can be updated, update the daughters
      int ndaughters = cand.getNDaughters();
      Belle2::Particle* daughter;
      for (int i = 0; i < ndaughters; i++) {
        daughter = const_cast<Belle2::Particle*>(cand.getDaughter(i));
        updateTreeCopy(*daughter);
      }
    }
  }




  TVector2 Fitter::lifeTime(Belle2::Particle& cand) const
  {
    // returns the lifetime in the rest frame of the candidate
    TVector2 rc;
    const ParticleBase* pb = m_decaychain->locate(&cand);

    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      //B2DEBUG(0, "Calculating life time for " << cand.getPDGCode() << " ");
      int tauindex = pb->tauIndex();
      double tau    = m_fitparams->par()(tauindex + 1);
      //B2DEBUG(0, "tau " << tau << " ");
      double taucov = m_fitparams->cov()(tauindex + 1, tauindex + 1);
      double mass   = pb->pdgMass();
      double convfac = mass / Belle2::Const::speedOfLight;
      rc = {convfac * tau, convfac* convfac * taucov};
    }
    return rc;
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




  TVector2 Fitter::decayLength(const ParticleBase* pb) const
  {
    // returns the decaylength in the lab frame
    return decayLength(pb, m_fitparams);
  }
  std::tuple<double, double> Fitter::getDecayLength(const ParticleBase* pb) const
  {
    // returns the decaylength in the lab frame
    return getDecayLength(pb, m_fitparams);
  }


  TVector2 Fitter::decayLength(const ParticleBase* pb, const FitParams* fitparams)
  {
    // returns the decaylength in the lab frame
    TVector2 rc;
//    B2DEBUG(19, pb->tauIndex() << " tau index " << pb->name() << " momindex " << pb->momIndex() << " posindex  " << pb->posIndex() );
    if (pb->tauIndex() >= 0 && pb->mother()) {
      // one can calculate the error in many ways. I managed to make
      // them all agree, with a few outliers. this one seems to be
      // most conservative/stable/simple one.
      // len = tau |mom|
      int tauindex = pb->tauIndex();
      int momindex = pb->momIndex();
      //int posindex = pb->posIndex();
      double tau   = fitparams->par()(tauindex + 1);
      double mom2(0);

      for (int row = 1; row <= 3; ++row) {
        double px = fitparams->par()(momindex + row - 1);
        mom2 += px * px;
      }

      //double mom = sqrt(mom2);
      //double len = mom * tau;
      double len = tau; //JFK:: tau = momDX/ p2 ->  tau = momdx / |p| (pb::initTau)
      // so that we dont have to multiply with p anymore
      // this should also fix the covariance error problem

      //vector<int> indexvec(1);
      vector<int> indexvec(1);
      indexvec[0] = tauindex + 1;
      // indexvec[1] = momindex;
      // indexvec[2] = momindex + 1;
      // indexvec[3] = momindex + 2;

      //indexvec[0] = posindex + 1;
      //indexvec[1] = posindex + 2;
      //indexvec[2] = posindex + 3;

      CLHEP::HepVector jacobian(1); //was 4
      // jacobian[0] = mom;              //JFK :: FIXME think about this (was =mom)
      // jacobian[1] = tau * fitparams->par()(momindex + 1) / mom;
      // jacobian[2] = tau * fitparams->par()(momindex + 2) / mom;
      // jacobian[3] = tau * fitparams->par()(momindex + 3) / mom;

      jacobian[0] = 1;              //JFK :: FIXME think about this (was =mom)
      //jacobian(1) = std::sqrt(std::abs(fitparams->par()(momindex + 1)) / mom);
      //jacobian(2) = std::sqrt(std::abs(fitparams->par()(momindex + 2)) / mom);
      //jacobian(3) = std::sqrt(std::abs(fitparams->par()(momindex + 3)) / mom);


      rc.SetX(len);
      //B2DEBUG(19, "calculated decay length of " << len << " +- " << std::sqrt(fitparams->cov(indexvec).similarity(
      //            jacobian)) );
      rc.SetY(fitparams->cov(indexvec).similarity(jacobian));
      //rc.SetY(fitparams->cov(indexvec));
    }
    return rc; //returns (0,0) if the particle has no decay length
  }

  std::tuple<double, double> Fitter::getDecayLength(const ParticleBase* pb, const FitParams* fitparams) const
  {
    if (pb->tauIndex() >= 0 && pb->mother()) {
      // one can calculate the error in many ways. I managed to make
      // them all agree, with a few outliers. this one seems to be
      // most conservative/stable/simple one.
      // len = tau |mom|
      //int tauindex = pb->tauIndex();
      //int momindex = pb->momIndex();
      //int posindex = pb->posIndex();
      //double tau   = fitparams->getStateVector()(tauindex);
      //EigenTypes::ColVector momentum = fitparams->getStateVector().segment(momindex, 3);
      //double mom2 = momentum * momentum.transpose();

      //double mom = sqrt(mom2);
      //double len = mom * tau;
      //double len = tau; //JFK:: tau = momDX/ p2 ->  tau = momdx / |p| (pb::initTau)

      //vector<int> indexvec(4);
      //indexvec[0] = tauindex;
      // indexvec[1] = momindex;
      // indexvec[2] = momindex + 1;
      // indexvec[3] = momindex + 2;

      //indexvec[0] = posindex;
      //indexvec[1] = posindex + 1;
      //indexvec[2] = posindex + 2;

      //EigenTypes::MatrixXd jacobian(4);
      //jacobian(0) = mom;              //JFK :: FIXME think about this
      //jacobian(1) = std::sqrt(std::abs(fitparams->getSateVector()(momindex    )) / mom);
      //jacobian(2) = std::sqrt(std::abs(fitparams->getSateVector()(momindex + 1)) / mom);
      //jacobian(3) = std::sqrt(std::abs(fitparams->getSateVector()(momindex + 2)) / mom);

      //JFK: If we decide to change the above calculate:
      //EigenTypes::MatrixXd m = fitparams->getCovariance(indexvec);
      //double lenErr = m.selfadjointView<Eigen::Lower> * jacobian * m.selfadjointView<Eigen::Lower>
      //since m is symmetric you dont need to transpose (which has 0 runtimecost anyway...)
      // 2017-09-28

      int tauindex = pb->tauIndex();
      double len   = fitparams->getStateVector()(tauindex);
      double lenErr = fitparams->getCovariance()(tauindex, tauindex);
      return std::make_tuple(len, lenErr);
    }
    return std::make_tuple(-999, -999);
  }

  TVector2 Fitter::decayLength(Belle2::Particle& cand) const //FT: are all these instances of decayLength required?
  {
    TVector2 rc;
    const ParticleBase* pb = m_decaychain->locate(&cand) ;

    if (pb && pb->tauIndex() >= 0 && pb->mother()) {
      rc = decayLength(pb);
    }

    return rc;
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




  //FIXME unused can we delete this?
  TVector2 Fitter::decayLengthSum(const ParticleBase* pbA, const ParticleBase* pbB) const
  {
    // returns the decaylengthsum in the lab frame
    TVector2 rc;

    if (pbA->tauIndex() >= 0 && pbA->mother() &&
        pbB->tauIndex() >= 0 && pbB->mother()) {

      // len = tau |mom|
      int tauindexA = pbA->tauIndex();
      int momindexA = pbA->momIndex();
      double tauA   = m_fitparams->par()(tauindexA + 1);
      double mom2A(0);

      for (int row = 1; row <= 3; ++row) {
        double px = m_fitparams->par()(momindexA + row);
        mom2A += px * px;
      }

      double momA = sqrt(mom2A);
      double lenA = momA * tauA;

      int tauindexB = pbB->tauIndex();
      int momindexB = pbB->momIndex();
      double tauB    = m_fitparams->par()[tauindexB];
      double mom2B(0);

      for (int row = 1; row <= 3; ++row) {
        double px = m_fitparams->par()[momindexB + row - 1];
        mom2B += px * px;
      }

      double momB = sqrt(mom2B);
      double lenB = momB * tauB;

      vector<int> indexvec;
      indexvec.push_back(tauindexA);

      for (int i = 0; i < 3; ++i) {
        indexvec.push_back(momindexA + i);
      }

      indexvec.push_back(tauindexB);
      for (int i = 0; i < 3; ++i) {
        indexvec.push_back(momindexB + i);
      }

      CLHEP::HepVector jacobian(8);
      jacobian(1) = momA;

      for (int irow = 1; irow <= 3; ++irow) {
        jacobian(irow + 1) = tauA * m_fitparams->par()(momindexA + irow) / momA;
      }

      jacobian(5) = momB;
      for (int irow = 1; irow <= 3; ++irow) {
        jacobian(irow + 5) = tauB * m_fitparams->par()(momindexB + irow) / momB;
      }

      rc.SetX(lenA + lenB);
      rc.SetY(cov(indexvec).similarity(jacobian));
    }
    return rc;
  }

  //FIXME unused can we delete it ?
  TVector2 Fitter::decayLengthSum(Belle2::Particle& candA, Belle2::Particle& candB) const
  {
    TVector2 rc;
    const ParticleBase* pbA = m_decaychain->locate(&candA);
    const ParticleBase* pbB = m_decaychain->locate(&candB);

    if (pbA && pbB)  {
      rc = decayLengthSum(pbA, pbB);
    }
    return rc;
  }
}

